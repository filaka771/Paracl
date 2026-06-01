#include <fstream>
#include <string>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <stdexcept>
#include <vector>

#include "../../parser/include/parser.h"

class CodeGen {
public: 
    CodeGen(std::string &file_name, Parser::ParseResult parse_result)
    : asm_file_(file_name), parse_result_(std::move(parse_result)) {}

    void generate() {
        collect_program(*parse_result_.program);
        emit_program(*parse_result_.program);
    }

private:
    struct FunctionSign {
        std::size_t param_count;
        std::string function_label;
    };

    struct SymbolInfo {
        std::size_t offset;
        bool is_param;
    };

    struct ScopeContext {
        std::unordered_map<std::string, SymbolInfo> symbol_table;
    };

    struct FunctionContext {
        std::vector<ScopeContext> scopes;
        std::size_t next_offset = 0;
        std::size_t if_count = 0;
        std::size_t for_count = 0;
        std::string function_end_label;
        std::string function_fallthrough_label;
        std::vector<std::string> break_labels;
        std::vector<std::string> continue_labels;
    };

    std::ofstream asm_file_;
    Parser::ParseResult parse_result_;
    std::unordered_map<std::string, FunctionSign> function_table_;

    //-----------------Helpers-----------------
    void print_error(const Node& node, std::string error_msg) const {
        std::cout << error_msg;
        const auto line = parse_result_.token_list[node.span_.begin].line;
        const auto column = parse_result_.token_list[node.span_.begin].column; 
        std::cout << " " << line << ":" << column << "\n";
    }

    const std::string& get_identifier_name(const Node& node) const {
        switch (node.node_kind) {
            case NodeKind::IdentifierExpr:
                return parse_result_.token_list[node.span_.begin].lexeme;

            default:
                print_error(node, "Node does not contain identifier.");
                throw std::runtime_error("Node does not contain identifier");
        }
    }

    void discard_expr_result() {
        asm_file_ << "add rsp, 8\n";
    }

    void push_scope(FunctionContext& context) {
        context.scopes.emplace_back();
    }

    void pop_scope(FunctionContext& context) {
        if (context.scopes.empty()) {
            throw std::runtime_error("No scope to pop");
        }

        context.scopes.pop_back();
    }

    SymbolInfo* find_symbol(FunctionContext& context, const std::string& identifier) {
        for (auto iter = context.scopes.rbegin();
             iter != context.scopes.rend();
             ++iter)
        {
            auto symbol_iter = iter->symbol_table.find(identifier);
            if (symbol_iter != iter->symbol_table.end()) {
                return &symbol_iter->second;
            }
        }

        return nullptr;
    }

    const SymbolInfo* find_symbol(
        const FunctionContext& context,
        const std::string& identifier
    ) const {
        for (auto iter = context.scopes.rbegin();
             iter != context.scopes.rend();
             ++iter)
        {
            auto symbol_iter = iter->symbol_table.find(identifier);
            if (symbol_iter != iter->symbol_table.end()) {
                return &symbol_iter->second;
            }
        }

        return nullptr;
    }

    void add_symbol_to_current_scope(
        FunctionContext& context,
        const std::string& identifier,
        SymbolInfo symbol_info
    ) {
        if (context.scopes.empty()) {
            throw std::runtime_error("No active scope");
        }

        context.scopes.back().symbol_table[identifier] = symbol_info;
    }

    //-----------------Functions-----------------
    void collect_program(const Program& program) {
        for (const auto& child : program.children_nodes) {
            const auto* function_def = static_cast<FunctionDef*>(child.get());
            collect_function(*function_def);

        }
    }

    void collect_function(const FunctionDef& function_def) {
        const std::string& function_id = function_def.get_function_id();
        // Add new function declaration in the function table
        if(function_table_.find(function_id) == function_table_.end()) {
            function_table_[function_id].function_label = "_int_" + function_id;
            // Calculate number of arguments
            const auto* param_list =
                static_cast<ParameterList*>(function_def.children_nodes[0].get());
            collect_arg_list(*param_list, function_id);
        }
        else {
            const std::string err_msg = "Multiple definitions of function "
                                        + function_def.get_function_id();
            print_error(function_def, err_msg);
            throw std::runtime_error("Multiple definition!");
        }
    }

    void collect_arg_list(const ParameterList& parameter_list,
                          const std::string& function_id) {
        const std::size_t param_count = parameter_list.children_nodes.size();
        function_table_[function_id].param_count = param_count;
    }

    void collect_parameters(
        const ParameterList& parameter_list,
        FunctionContext& context
    ) {
        std::size_t param_offset = 16;

        for (const auto& child : parameter_list.children_nodes) {
            const std::string& identifier =
                parse_result_.token_list[child->span_.end].lexeme;

            add_symbol_to_current_scope(
                context,
                identifier,
                SymbolInfo{
                    param_offset,
                    true
                }
            );

            param_offset += 8;
        }
    }

    void collect_compound_locals(
        const CompoundStmt& compound_stmt,
        FunctionContext& context
    ) {
        push_scope(context);

        for (const auto& node : compound_stmt.children_nodes) {
            collect_statement_locals(*node, context);
        }

        pop_scope(context);
    }

    void collect_statement_locals(const Node& node, FunctionContext& context) {
        switch (node.node_kind) {
            case NodeKind::CompoundStmt:
                collect_compound_locals(
                    static_cast<const CompoundStmt&>(node),
                    context
                );
                break;

            case NodeKind::ExpressionStmt:
                if (!node.children_nodes.empty()) {
                    collect_expr_locals(*node.children_nodes[0], context);
                }
                break;

            case NodeKind::IfStmt:
                collect_expr_locals(*node.children_nodes[0], context);
                collect_statement_locals(*node.children_nodes[1], context);

                if (node.children_nodes.size() == 3) {
                    collect_statement_locals(*node.children_nodes[2], context);
                }
                break;

            case NodeKind::ForStmt: {
                const auto& for_stmt = static_cast<const ForStmt&>(node);
                std::size_t child_index = 0;

                if (for_stmt.has_init()) {
                    collect_expr_locals(*node.children_nodes[child_index++], context);
                }

                if (for_stmt.has_cond()) {
                    collect_expr_locals(*node.children_nodes[child_index++], context);
                }

                if (for_stmt.has_step()) {
                    collect_expr_locals(*node.children_nodes[child_index++], context);
                }

                collect_statement_locals(*node.children_nodes[child_index], context);
                break;
            }

            case NodeKind::ReturnStmt:
                if (!node.children_nodes.empty()) {
                    collect_expr_locals(*node.children_nodes[0], context);
                }
                break;

            case NodeKind::NullStmt:
            case NodeKind::BreakStmt:
            case NodeKind::ContinueStmt:
                break;

            default:
                print_error(node, "Unexpected statement node.");
                throw std::runtime_error("Unexpected statement node");
        }
    }

    void collect_expr_locals(const Node& node, FunctionContext& context) {
        switch (node.node_kind) {
            case NodeKind::AssignmentExpr: {
                const std::string& identifier =
                    get_identifier_name(*node.children_nodes[0]);

                if (find_symbol(context, identifier) == nullptr) {
                    context.next_offset += 8;
                    add_symbol_to_current_scope(
                        context,
                        identifier,
                        SymbolInfo{
                            context.next_offset,
                            false
                        }
                    );
                }

                collect_expr_locals(*node.children_nodes[1], context);
                break;
            }

            case NodeKind::PostfixExpr: {
                const std::string& identifier =
                    get_identifier_name(*node.children_nodes[0]);

                if (find_symbol(context, identifier) == nullptr) {
                    context.next_offset += 8;
                    add_symbol_to_current_scope(
                        context,
                        identifier,
                        SymbolInfo{
                            context.next_offset,
                            false
                        }
                    );
                }
                break;
            }

            case NodeKind::BinaryOperExpr:
                collect_expr_locals(*node.children_nodes[0], context);
                collect_expr_locals(*node.children_nodes[1], context);
                break;

            case NodeKind::FuncCallExpr:
                if (!node.children_nodes.empty()) {
                    for (const auto& child : node.children_nodes[0]->children_nodes) {
                        collect_expr_locals(*child, context);
                    }
                }
                break;

            case NodeKind::IdentifierExpr:
            case NodeKind::IntegerExpr:
                break;

            default:
                print_error(node, "Unexpected expression node.");
                throw std::runtime_error("Unexpected expression node");
        }
    }

    void emit_program(const Program& program) {
        asm_file_ << "format ELF64 executable 3\n";
        asm_file_ << "entry _start\n" << "\n";
        asm_file_ << "_start:\n";
        asm_file_ << "call _int_main\n";
        asm_file_ << "mov rdi, rax\n";
        asm_file_ << "mov rax, 60\n";
        asm_file_ << "syscall\n";
        asm_file_ << "\n";

        for (const auto& child : program.children_nodes) {
            const auto* function_def = static_cast<FunctionDef*>(child.get());
            emit_function(*function_def);
        }
    }

    void emit_function(const FunctionDef& function_def) {
        FunctionContext context;
        const std::string& function_label =
            function_table_.at(function_def.get_function_id()).function_label;
        context.function_end_label = function_label + "_end";
        context.function_fallthrough_label = function_label + "_fallthrough";

        const auto* param_list =
            static_cast<ParameterList*>(function_def.children_nodes[0].get());

        const auto* compound_stmt =
            static_cast<CompoundStmt*>(function_def.children_nodes[1].get());

        push_scope(context);
        collect_parameters(*param_list, context);
        collect_compound_locals(*compound_stmt, context);

        asm_file_ << function_label << ":\n";
        asm_file_ << "push rbp\n";
        asm_file_ << "mov rbp, rsp\n";

        if (context.next_offset != 0) {
            asm_file_ << "sub rsp, " << context.next_offset << "\n";
        }

        emit_compound_stmt(*compound_stmt, context);
        asm_file_ << context.function_fallthrough_label << ":\n";
        asm_file_ << "ud2\n";
        asm_file_ << context.function_end_label << ":\n";
        asm_file_ << "mov rsp, rbp\n";
        asm_file_ << "pop rbp\n";
        asm_file_ << "ret\n";
    }


    void emit_compound_stmt(
        const CompoundStmt& compound_stmt,
        FunctionContext& context
    ) {
        push_scope(context);

        for (const auto& node : compound_stmt.children_nodes) {
            emit_statement(*node, context);
        }

        pop_scope(context);
    }

    void emit_statement(const Node& node, FunctionContext& context) {
        switch (node.node_kind) {
            case NodeKind::CompoundStmt:
                emit_compound_stmt(
                    static_cast<const CompoundStmt&>(node),
                    context
                );
                break;

            case NodeKind::ExpressionStmt:
                emit_expression_stmt(
                    static_cast<const ExpressionStmt&>(node),
                    context
                );
                break;

            case NodeKind::NullStmt:
                emit_null_stmt(static_cast<const NullStmt&>(node), context);
                break;

            case NodeKind::IfStmt:
                emit_if_stmt(static_cast<const IfStmt&>(node), context);
                break;

            case NodeKind::ForStmt:
                emit_for_stmt(static_cast<const ForStmt&>(node), context);
                break;

            case NodeKind::ReturnStmt:
                emit_return_stmt(static_cast<const ReturnStmt&>(node), context);
                break;

            case NodeKind::BreakStmt:
                emit_break_stmt(static_cast<const BreakStmt&>(node), context);
                break;

            case NodeKind::ContinueStmt:
                emit_continue_stmt(
                    static_cast<const ContinueStmt&>(node),
                    context
                );
                break;

            default:
                print_error(node, "Unexpected statement node.");
                throw std::runtime_error("Unexpected statement node");
        }
    }

    void emit_null_stmt(const NullStmt&, FunctionContext&) {}

    void emit_if_stmt(const IfStmt& if_stmt, FunctionContext& context) {
        const std::size_t if_id = context.if_count;
        context.if_count++;

        const Node& condition = *if_stmt.children_nodes[0];
        const Node& then_stmt = *if_stmt.children_nodes[1];

        const bool has_else = if_stmt.children_nodes.size() == 3;

        emit_expr(condition, context);
        asm_file_ << "pop rax\n";
        asm_file_ << "cmp rax, 0\n";

        if (has_else) {
            asm_file_ << "je _else_" << if_id << "\n";
            emit_statement(then_stmt, context);
            asm_file_ << "jmp _ifend_" << if_id << "\n";
            asm_file_ << "_else_" << if_id << ":\n";

            const Node& else_stmt = *if_stmt.children_nodes[2];
            emit_statement(else_stmt, context);

            asm_file_ << "_ifend_" << if_id << ":\n";
        }
        else {
            asm_file_ << "je _ifend_" << if_id << "\n";
            emit_statement(then_stmt, context);
            asm_file_ << "_ifend_" << if_id << ":\n";
        }
    }

    void emit_for_stmt(const ForStmt& for_stmt, FunctionContext& context) {
        const std::size_t for_id = context.for_count;
        context.for_count++;

        const std::string loop_begin = "_for_begin_" + std::to_string(for_id);
        const std::string loop_step = "_for_step_" + std::to_string(for_id);
        const std::string loop_end = "_for_end_" + std::to_string(for_id);

        std::size_t child_index = 0;

        const Node* init_expr = nullptr;
        const Node* cond_expr = nullptr;
        const Node* step_expr = nullptr;

        if (for_stmt.has_init()) {
            init_expr = for_stmt.children_nodes[child_index++].get();
        }

        if (for_stmt.has_cond()) {
            cond_expr = for_stmt.children_nodes[child_index++].get();
        }

        if (for_stmt.has_step()) {
            step_expr = for_stmt.children_nodes[child_index++].get();
        }

        const Node* body_stmt = for_stmt.children_nodes[child_index].get();

        if (init_expr != nullptr) {
            emit_expr(*init_expr, context);
            discard_expr_result();
        }

        asm_file_ << loop_begin << ":\n";

        if (cond_expr != nullptr) {
            emit_expr(*cond_expr, context);
            asm_file_ << "pop rax\n";
            asm_file_ << "cmp rax, 0\n";
            asm_file_ << "je " << loop_end << "\n";
        }

        context.break_labels.push_back(loop_end);
        context.continue_labels.push_back(loop_step);

        emit_statement(*body_stmt, context);

        context.continue_labels.pop_back();
        context.break_labels.pop_back();

        asm_file_ << loop_step << ":\n";

        if (step_expr != nullptr) {
            emit_expr(*step_expr, context);
            discard_expr_result();
        }

        asm_file_ << "jmp " << loop_begin << "\n";
        asm_file_ << loop_end << ":\n";
    }

    void emit_return_stmt(const ReturnStmt& return_stmt, FunctionContext& context) {
        if (!return_stmt.children_nodes.empty()) {
            const auto* expr = return_stmt.children_nodes[0].get();
            emit_expr(*expr, context);
            asm_file_ << "pop rax\n";
        }

        asm_file_ << "jmp " << context.function_end_label << "\n";
    }

    void emit_break_stmt(const BreakStmt& break_stmt, FunctionContext& context) {
        if (context.break_labels.empty()) {
            print_error(break_stmt, "Unexpected 'break' outside loop.");
            throw std::runtime_error("Unexpected break outside loop");
        }

        asm_file_ << "jmp " << context.break_labels.back() << "\n";
    }

    void emit_continue_stmt(const ContinueStmt& continue_stmt, FunctionContext&
    context) {
        if (context.continue_labels.empty()) {
            print_error(continue_stmt, "Unexpected 'continue' outside loop.");
            throw std::runtime_error("Unexpected continue outside loop");
        }

        asm_file_ << "jmp " << context.continue_labels.back() << "\n";
    }

    void emit_expression_stmt(
        const ExpressionStmt& expression_stmt,
        FunctionContext& context
    ) {
        if (expression_stmt.children_nodes.empty()) {
            return;
        }

        const auto* expression = expression_stmt.children_nodes[0].get();
        emit_expr(*expression, context);
        discard_expr_result();
    }

    // Expression convention:
    // every emit_*_expr method must leave exactly one result value on stack.
    void emit_expr(const Node& expression, FunctionContext& context) {
        switch (expression.node_kind) {
            case NodeKind::AssignmentExpr:
                emit_assignment_expr(
                    static_cast<const AssignmentExpr&>(expression),
                    context
                );
                break;

            case NodeKind::PostfixExpr:
                emit_postfix_expr(
                    static_cast<const PostfixExpr&>(expression),
                    context
                );
                break;

            case NodeKind::FuncCallExpr:
                emit_func_call_expr(
                    static_cast<const FuncCallExpr&>(expression),
                    context
                );
                break;

            case NodeKind::BinaryOperExpr:
                emit_binary_expr(
                    static_cast<const BinaryOperExpr&>(expression),
                    context
                );
                break;

            case NodeKind::IdentifierExpr:
                emit_identifier_expr(
                    static_cast<const IdentifierExpr&>(expression),
                    context
                );
                break;

            case NodeKind::IntegerExpr:
                emit_integer_expr(
                    static_cast<const IntegerExpr&>(expression),
                    context
                );
                break;

            default:
                print_error(expression, "Unexpected expression node.");
                throw std::runtime_error("Unexpected expression node");
        }
    }

    void emit_assignment_expr(const AssignmentExpr& assignment_expr, FunctionContext& context) {
        const auto* ident =
            static_cast<const IdentifierExpr*>(assignment_expr.children_nodes[0].get());

        const std::string& identifier = get_identifier_name(*ident);
        const auto* symbol_info = find_symbol(context, identifier);

        if (symbol_info == nullptr) {
            print_error(assignment_expr, "Undefined identifier.");
            throw std::runtime_error("Undefined identifier.");
        }

        emit_expr(*assignment_expr.children_nodes[1], context);
        asm_file_ << "pop rax\n";

        if (symbol_info->is_param) {
            asm_file_ << "mov qword [rbp + " << symbol_info->offset << "], rax\n";
        }
        else {
            asm_file_ << "mov qword [rbp - " << symbol_info->offset << "], rax\n";
        }

        asm_file_ << "push rax\n";
    }

    void emit_postfix_expr(const PostfixExpr& postfix_expr, FunctionContext&
    context) {
        const std::string& identifier =
            get_identifier_name(*postfix_expr.children_nodes[0].get());

        const auto* symbol_info = find_symbol(context, identifier);

        if (symbol_info == nullptr) {
            print_error(postfix_expr, "Undefined identifier.");
            throw std::runtime_error("Undefined identifier.");
        }

        const std::string& op_lexeme =
            parse_result_.token_list[postfix_expr.span_.end].lexeme;

        if (symbol_info->is_param) {
            asm_file_ << "push qword [rbp + " << symbol_info->offset << "]\n";
        }
        else {
            asm_file_ << "push qword [rbp - " << symbol_info->offset << "]\n";
        }

        if (op_lexeme == "++") {
            if (symbol_info->is_param) {
                asm_file_ << "inc qword [rbp + " << symbol_info->offset << "]\n";
            }
            else {
                asm_file_ << "inc qword [rbp - " << symbol_info->offset << "]\n";
            }
        }
        else if (op_lexeme == "--") {
            if (symbol_info->is_param) {
                asm_file_ << "dec qword [rbp + " << symbol_info->offset << "]\n";
            }
            else {
                asm_file_ << "dec qword [rbp - " << symbol_info->offset << "]\n";
            }
        }
        else {
            print_error(postfix_expr, "Unsupported postfix operator.");
            throw std::runtime_error("Unsupported postfix operator.");
        }
    }

    void emit_func_call_expr(
        const FuncCallExpr& func_call_expr,
        FunctionContext& context
    ) {
        const std::string& function_id = func_call_expr.get_func_name();
        auto iter = function_table_.find(function_id);

        if (iter == function_table_.end()) {
            print_error(func_call_expr, "Undefined function.");
            throw std::runtime_error("Undefined function");
        }

        const auto* arg_list =
            static_cast<const ArgumentListExpr*>(func_call_expr.children_nodes[0].get());

        const std::size_t arg_count = arg_list->children_nodes.size();

        if (arg_count != iter->second.param_count) {
            print_error(func_call_expr, "Invalid count of arguments.");
            throw std::runtime_error("Invalid count of arguments");
        }

        for (auto arg_iter = arg_list->children_nodes.rbegin();
             arg_iter != arg_list->children_nodes.rend();
             ++arg_iter)
        {
            emit_expr(**arg_iter, context);
        }

        asm_file_ << "call " << iter->second.function_label << "\n";

        if (arg_count > 0) {
            asm_file_ << "add rsp, " << arg_count * 8 << "\n";
        }

        asm_file_ << "push rax\n";
    }

    void emit_binary_expr(const BinaryOperExpr& binary_expr,
                          FunctionContext&context
    ) {
        const auto* left_expr = binary_expr.children_nodes[0].get();
        const auto* right_expr = binary_expr.children_nodes[1].get();

        emit_expr(*left_expr, context);
        emit_expr(*right_expr, context);

        asm_file_ << "pop rbx\n";
        asm_file_ << "pop rax\n";

        const std::string& op = binary_expr.get_op_lexeme();

        if (binary_expr.node_name == "AdditiveExpr") {
            if (op == "+") {
                asm_file_ << "add rax, rbx\n";
            }
            else if (op == "-") {
                asm_file_ << "sub rax, rbx\n";
            }
        }
        else if (binary_expr.node_name == "MultiplicativeExpr") {
            if (op == "*") {
                asm_file_ << "imul rax, rbx\n";
            }
            else if (op == "/") {
                asm_file_ << "cqo\n";
                asm_file_ << "idiv rbx\n";
            }
            else if (op == "%") {
                asm_file_ << "cqo\n";
                asm_file_ << "idiv rbx\n";
                asm_file_ << "mov rax, rdx\n";
            }
        }
        else if (binary_expr.node_name == "RelationalExpr") {
            asm_file_ << "cmp rax, rbx\n";

            if (op == ">") {
                asm_file_ << "setg al\n";
            }
            else if (op == ">=") {
                asm_file_ << "setge al\n";
            }
            else if (op == "<") {
                asm_file_ << "setl al\n";
            }
            else if (op == "<=") {
                asm_file_ << "setle al\n";
            }

            asm_file_ << "movzx rax, al\n";
        }
        else if (binary_expr.node_name == "EqualityExpr") {
            asm_file_ << "cmp rax, rbx\n";

            if (op == "==") {
                asm_file_ << "sete al\n";
            }
            else if (op == "!=") {
                asm_file_ << "setne al\n";
            }

            asm_file_ << "movzx rax, al\n";
        }
        else if (binary_expr.node_name == "LogicalAndExpr") {
            asm_file_ << "cmp rax, 0\n";
            asm_file_ << "setne al\n";
            asm_file_ << "movzx rax, al\n";
            asm_file_ << "cmp rbx, 0\n";
            asm_file_ << "setne bl\n";
            asm_file_ << "movzx rbx, bl\n";
            asm_file_ << "and rax, rbx\n";
        }
        else if (binary_expr.node_name == "LogicalOrExpr") {
            asm_file_ << "cmp rax, 0\n";
            asm_file_ << "setne al\n";
            asm_file_ << "movzx rax, al\n";
            asm_file_ << "cmp rbx, 0\n";
            asm_file_ << "setne bl\n";
            asm_file_ << "movzx rbx, bl\n";
            asm_file_ << "or rax, rbx\n";
        }
        else {
            print_error(binary_expr, "Unsupported binary expression.");
            throw std::runtime_error("Unsupported binary expression");
        }

        asm_file_ << "push rax\n";
    }

    void emit_identifier_expr(const IdentifierExpr& identifier_expr,
                                FunctionContext& context) {
        const std::string& identifier = get_identifier_name(identifier_expr);

        const auto* symbol_info = find_symbol(context, identifier);

        if (symbol_info != nullptr) {
            if (symbol_info->is_param) {
                asm_file_ << "push qword [rbp + " << symbol_info->offset << "]\n";
            }
            else {
                asm_file_ << "push qword [rbp - " << symbol_info->offset << "]\n";
            }
        }
        else {
            print_error(identifier_expr, "Undefined identifier.");
            throw std::runtime_error("Undefined identifier.");
        }
    }

  void emit_integer_expr(const IntegerExpr& integer_expr,
                         FunctionContext& context) {
      const std::string& lexeme =
          parse_result_.token_list[integer_expr.span_.begin].lexeme;

      const int int_value = std::stoi(lexeme);

      asm_file_ << "push " << int_value << "\n";
  }
};
