#include <fstream>
#include <string>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <stdexcept>
#include <vector>
#include <filesystem>

#include "../../parser/include/parser.h"

class CodeGen {
public: 
    CodeGen(const std::string& file_name, Parser::ParseResult parse_result)
    : asm_file_(file_name), parse_result_(std::move(parse_result)) {
        generate();
    }


private:
    struct FunctionInfo {
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
    std::unordered_map<std::string, FunctionInfo> function_table_;

    enum class Reg {
        RAX,
        RBX,
    };

    void generate() {
        register_stdlib_functions();
        collect_program(*parse_result_.program);
        emit_program(*parse_result_.program);
    }

    //-----------------Helpers-----------------
    void report_error(const Node& node, std::string error_msg) const {
        std::cout << error_msg;
        const auto line = parse_result_.token_list[node.span_.begin].line;
        const auto column = parse_result_.token_list[node.span_.begin].column; 
        std::cout << " " << line << ":" << column << "\n";
    }

    const std::string& get_identifier_name(const Node& node) const {
        switch (node.node_kind) {
            case NodeKind::DeclRefExpr:
                return parse_result_.token_list[node.span_.begin].lexeme;

            default:
                report_error(node, "Node does not contain identifier.");
                throw std::runtime_error("Node does not contain identifier");
        }
    }

    const char* reg_name(Reg reg) const {
        switch (reg) {
            case Reg::RAX:
                return "rax";
            case Reg::RBX:
                return "rbx";
        }

        throw std::runtime_error("Unexpected register");
    }

    const char* reg_byte_name(Reg reg) const {
        switch (reg) {
            case Reg::RAX:
                return "al";
            case Reg::RBX:
                return "bl";
        }

        throw std::runtime_error("Unexpected register");
    }

    Reg other_reg(Reg reg) const {
        return reg == Reg::RAX ? Reg::RBX : Reg::RAX;
    }

    void spill_reg(Reg reg) {
        asm_file_ << "push " << reg_name(reg) << "\n";
    }

    void restore_reg(Reg reg) {
        asm_file_ << "pop " << reg_name(reg) << "\n";
    }

    bool is_reserved_stdlib_name(const std::string& function_id) const {
        return function_id == "print" || function_id == "scan";
    }

    void register_stdlib_functions() {
        function_table_["print"] = FunctionInfo{1, "_std_print"};
        function_table_["scan"] = FunctionInfo{0, "_std_scan"};
    }

    std::filesystem::path stdlib_runtime_path() const {
        return std::filesystem::path(__FILE__)
            .parent_path()
            .parent_path()
            .parent_path()
            .parent_path()
            / "stdlib"
            / "runtime.asm";
    }

    void emit_stdlib_runtime() {
        std::ifstream runtime_file(stdlib_runtime_path());
        if (!runtime_file.is_open()) {
            throw std::runtime_error("Failed to open stdlib runtime.asm");
        }

        asm_file_ << runtime_file.rdbuf();
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
    //-----------------Ershov_number-----------------
    bool has_side_effects(const Node& expr) const {
        switch (expr.node_kind) {
            case NodeKind::AssignmentExpr:
            case NodeKind::PostfixExpr:
            case NodeKind::CallExpr:
                return true;

            case NodeKind::BinaryOperator:
                return has_side_effects(*expr.children_nodes[0]) ||
                        has_side_effects(*expr.children_nodes[1]);

            default:
                return false;
        }
    }

    std::size_t compute_ershov(const Node& expr) const {
        switch (expr.node_kind) {
            case NodeKind::DeclRefExpr:
            case NodeKind::IntegerLiteral:
            case NodeKind::PostfixExpr:
            case NodeKind::CallExpr:
                return 1;

            case NodeKind::AssignmentExpr:
                return compute_ershov(*expr.children_nodes[1]);

            case NodeKind::BinaryOperator: {
                const std::size_t left =
                    compute_ershov(*expr.children_nodes[0]);
                const std::size_t right =
                    compute_ershov(*expr.children_nodes[1]);

                if (left == right) {
                    return left + 1;
                }

                return std::max(left, right);
            }

            default:
                throw std::runtime_error("Unexpected expression node");
        }
    }

    //-----------------Functions-----------------
    void collect_program(const TranslationUnitDecl& program) {
        for (const auto& child : program.children_nodes) {
            const auto* function_def = static_cast<FunctionDecl*>(child.get());
            collect_function(*function_def);

        }
    }

    void collect_function(const FunctionDecl& function_def) {
        const std::string& function_id = function_def.get_function_id();
        // Add new function declaration in the function table
        if(function_table_.find(function_id) == function_table_.end()) {
            function_table_[function_id].function_label = "_int_" + function_id;
            // Calculate number of arguments
            const auto* param_list =
                static_cast<ParameterList*>(function_def.children_nodes[0].get());
            collect_parameter_list(*param_list, function_id);
        }
        else {
            const std::string err_msg = is_reserved_stdlib_name(function_id)
                ? "Reserved standard library function name: " + function_id
                : "Multiple definitions of function " + function_id;
            report_error(function_def, err_msg);
            throw std::runtime_error(err_msg);
        }
    }

    void collect_parameter_list(const ParameterList& parameter_list,
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
        FunctionContext& context,
        bool create_scope = true
    ) {
        if (create_scope) {
            push_scope(context);
        }

        for (const auto& node : compound_stmt.children_nodes) {
            collect_statement_locals(*node, context);
        }

        if (create_scope) {
            pop_scope(context);
        }
    }

    void collect_statement_locals(const Node& node, FunctionContext& context) {
        switch (node.node_kind) {
            case NodeKind::CompoundStmt:
                collect_compound_locals(
                    static_cast<const CompoundStmt&>(node),
                    context
                );
                break;

            case NodeKind::ExprStmt:
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
                report_error(node, "Unexpected statement node.");
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

            case NodeKind::BinaryOperator:
                collect_expr_locals(*node.children_nodes[0], context);
                collect_expr_locals(*node.children_nodes[1], context);
                break;

            case NodeKind::CallExpr:
                if (!node.children_nodes.empty()) {
                    for (const auto& child : node.children_nodes[0]->children_nodes) {
                        collect_expr_locals(*child, context);
                    }
                }
                break;

            case NodeKind::DeclRefExpr:
            case NodeKind::IntegerLiteral:
                break;

            default:
                report_error(node, "Unexpected expression node.");
                throw std::runtime_error("Unexpected expression node");
        }
    }

    void emit_program(const TranslationUnitDecl& program) {
        asm_file_ << "format ELF64 executable 3\n";
        asm_file_ << "entry _start\n";
        asm_file_ << "\n";
        asm_file_ << "segment readable executable\n";
        asm_file_ << "_start:\n";
        asm_file_ << "call _int_main\n";
        asm_file_ << "mov rdi, rax\n";
        asm_file_ << "mov rax, 60\n";
        asm_file_ << "syscall\n";
        asm_file_ << "\n";

        for (const auto& child : program.children_nodes) {
            const auto* function_def = static_cast<FunctionDecl*>(child.get());
            emit_function(*function_def);
        }

        emit_stdlib_runtime();
    }

    void emit_function(const FunctionDecl& function_def) {
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
        collect_compound_locals(*compound_stmt, context, false);

        asm_file_ << function_label << ":\n";
        asm_file_ << "push rbp\n";
        asm_file_ << "mov rbp, rsp\n";

        if (context.next_offset != 0) {
            asm_file_ << "sub rsp, " << context.next_offset << "\n";
        }

        emit_compound_stmt(*compound_stmt, context, false);
        asm_file_ << context.function_fallthrough_label << ":\n";
        asm_file_ << "ud2\n";
        asm_file_ << context.function_end_label << ":\n";
        asm_file_ << "mov rsp, rbp\n";
        asm_file_ << "pop rbp\n";
        asm_file_ << "ret\n";
    }


    void emit_compound_stmt(
        const CompoundStmt& compound_stmt,
        FunctionContext& context,
        bool create_scope = true
    ) {
        if (create_scope) {
            push_scope(context);
        }

        for (const auto& node : compound_stmt.children_nodes) {
            emit_statement(*node, context);
        }

        if (create_scope) {
            pop_scope(context);
        }
    }

    void emit_statement(const Node& node, FunctionContext& context) {
        switch (node.node_kind) {
            case NodeKind::CompoundStmt:
                emit_compound_stmt(
                    static_cast<const CompoundStmt&>(node),
                    context
                );
                break;

            case NodeKind::ExprStmt:
                emit_expression_stmt(
                    static_cast<const ExprStmt&>(node),
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
                report_error(node, "Unexpected statement node.");
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

        emit_expr_to_reg(condition, Reg::RAX, context);
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
            emit_expr_to_reg(*init_expr, Reg::RAX, context);
        }

        asm_file_ << loop_begin << ":\n";

        if (cond_expr != nullptr) {
            emit_expr_to_reg(*cond_expr, Reg::RAX, context);
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
            emit_expr_to_reg(*step_expr, Reg::RAX, context);
        }

        asm_file_ << "jmp " << loop_begin << "\n";
        asm_file_ << loop_end << ":\n";
    }

    void emit_return_stmt(const ReturnStmt& return_stmt, FunctionContext& context) {
        if (!return_stmt.children_nodes.empty()) {
            const auto* expr = return_stmt.children_nodes[0].get();
            emit_expr_to_reg(*expr, Reg::RAX, context);
        }

        asm_file_ << "jmp " << context.function_end_label << "\n";
    }

    void emit_break_stmt(const BreakStmt& break_stmt, FunctionContext& context) {
        if (context.break_labels.empty()) {
            report_error(break_stmt, "Unexpected 'break' outside loop.");
            throw std::runtime_error("Unexpected break outside loop");
        }

        asm_file_ << "jmp " << context.break_labels.back() << "\n";
    }

    void emit_continue_stmt(const ContinueStmt& continue_stmt, FunctionContext&
    context) {
        if (context.continue_labels.empty()) {
            report_error(continue_stmt, "Unexpected 'continue' outside loop.");
            throw std::runtime_error("Unexpected continue outside loop");
        }

        asm_file_ << "jmp " << context.continue_labels.back() << "\n";
    }

    void emit_expression_stmt(
        const ExprStmt& expression_stmt,
        FunctionContext& context
    ) {
        if (expression_stmt.children_nodes.empty()) {
            return;
        }

        const auto* expression = expression_stmt.children_nodes[0].get();
        emit_expr_to_reg(*expression, Reg::RAX, context);
    }

    void emit_expr_to_reg(const Node& expression, Reg target, FunctionContext& context) {
        switch (expression.node_kind) {
            case NodeKind::AssignmentExpr:
                emit_assignment_expr_to_reg(
                    static_cast<const AssignmentExpr&>(expression),
                    target,
                    context
                );
                break;

            case NodeKind::PostfixExpr:
                emit_postfix_expr_to_reg(
                    static_cast<const PostfixExpr&>(expression),
                    target,
                    context
                );
                break;

            case NodeKind::CallExpr:
                emit_func_call_expr_to_reg(
                    static_cast<const CallExpr&>(expression),
                    target,
                    context
                );
                break;

            case NodeKind::BinaryOperator:
                emit_binary_expr_to_reg(
                    static_cast<const BinaryOperator&>(expression),
                    target,
                    context
                );
                break;

            case NodeKind::DeclRefExpr:
                emit_identifier_expr_to_reg(
                    static_cast<const DeclRefExpr&>(expression),
                    target,
                    context
                );
                break;

            case NodeKind::IntegerLiteral:
                emit_integer_expr_to_reg(
                    static_cast<const IntegerLiteral&>(expression),
                    target,
                    context
                );
                break;

            default:
                report_error(expression, "Unexpected expression node.");
                throw std::runtime_error("Unexpected expression node");
        }
    }

    void emit_assignment_expr_to_reg(
        const AssignmentExpr& assignment_expr,
        Reg target,
        FunctionContext& context
    ) {
        const auto* ident =
            static_cast<const DeclRefExpr*>(assignment_expr.children_nodes[0].get());

        const std::string& identifier = get_identifier_name(*ident);
        const auto* symbol_info = find_symbol(context, identifier);

        if (symbol_info == nullptr) {
            report_error(assignment_expr, "Undefined identifier.");
            throw std::runtime_error("Undefined identifier.");
        }

        emit_expr_to_reg(*assignment_expr.children_nodes[1], target, context);

        if (symbol_info->is_param) {
            asm_file_ << "mov qword [rbp + " << symbol_info->offset
                      << "], " << reg_name(target) << "\n";
        }
        else {
            asm_file_ << "mov qword [rbp - " << symbol_info->offset
                      << "], " << reg_name(target) << "\n";
        }
    }

    void emit_postfix_expr_to_reg(
        const PostfixExpr& postfix_expr,
        Reg target,
        FunctionContext& context
    ) {
        const std::string& identifier =
            get_identifier_name(*postfix_expr.children_nodes[0].get());

        const auto* symbol_info = find_symbol(context, identifier);

        if (symbol_info == nullptr) {
            report_error(postfix_expr, "Undefined identifier.");
            throw std::runtime_error("Undefined identifier.");
        }

        const std::string& op_lexeme =
            parse_result_.token_list[postfix_expr.span_.end].lexeme;

        if (symbol_info->is_param) {
            asm_file_ << "mov " << reg_name(target)
                      << ", qword [rbp + " << symbol_info->offset << "]\n";
        }
        else {
            asm_file_ << "mov " << reg_name(target)
                      << ", qword [rbp - " << symbol_info->offset << "]\n";
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
            report_error(postfix_expr, "Unsupported postfix operator.");
            throw std::runtime_error("Unsupported postfix operator.");
        }
    }

    void emit_func_call_expr_to_reg(
        const CallExpr& func_call_expr,
        Reg target,
        FunctionContext& context
    ) {
        const std::string& function_id = func_call_expr.get_func_name();
        auto iter = function_table_.find(function_id);

        if (iter == function_table_.end()) {
            report_error(func_call_expr, "Undefined function.");
            throw std::runtime_error("Undefined function");
        }

        const auto* arg_list =
            static_cast<const ArgumentListExpr*>(func_call_expr.children_nodes[0].get());

        const std::size_t arg_count = arg_list->children_nodes.size();

        if (arg_count != iter->second.param_count) {
            report_error(func_call_expr, "Invalid count of arguments.");
            throw std::runtime_error("Invalid count of arguments");
        }

        for (auto arg_iter = arg_list->children_nodes.rbegin();
             arg_iter != arg_list->children_nodes.rend();
             ++arg_iter)
        {
            emit_expr_to_reg(**arg_iter, Reg::RAX, context);
            spill_reg(Reg::RAX);
        }

        asm_file_ << "call " << iter->second.function_label << "\n";

        if (arg_count > 0) {
            asm_file_ << "add rsp, " << arg_count * 8 << "\n";
        }

        if (target != Reg::RAX) {
            asm_file_ << "mov " << reg_name(target) << ", rax\n";
        }
    }

    void emit_binary_expr_to_reg(const BinaryOperator& binary_expr,
                            Reg target,
                            FunctionContext& context
    ) {
        const auto* left_expr = binary_expr.children_nodes[0].get();
        const auto* right_expr = binary_expr.children_nodes[1].get();
        const Reg spare = other_reg(target);

        const bool reorder_allowed =
            !has_side_effects(*left_expr) &&
            !has_side_effects(*right_expr);

        const std::size_t left_ershov = compute_ershov(*left_expr);
        const std::size_t right_ershov = compute_ershov(*right_expr);

        bool emit_left_first = true;

        if (reorder_allowed) {
            emit_left_first = left_ershov >= right_ershov;
        }

        if (emit_left_first) {
            emit_expr_to_reg(*left_expr, target, context);
            spill_reg(target);
            emit_expr_to_reg(*right_expr, target, context);
            asm_file_ << "mov " << reg_name(spare) << ", " << reg_name(target) << "\n";
            restore_reg(target);
        }
        else {
            emit_expr_to_reg(*right_expr, target, context);
            spill_reg(target);
            emit_expr_to_reg(*left_expr, target, context);
            restore_reg(spare);
        }

        const std::string& op = binary_expr.get_op_lexeme();
        const char* target_reg = reg_name(target);
        const char* spare_reg = reg_name(spare);
        const char* target_byte = reg_byte_name(target);
        const char* spare_byte = reg_byte_name(spare);

        if (binary_expr.node_name == "AdditiveExpr") {
            if (op == "+") {
                asm_file_ << "add " << target_reg << ", " << spare_reg << "\n";
            }
            else if (op == "-") {
                asm_file_ << "sub " << target_reg << ", " << spare_reg << "\n";
            }
        }
        else if (binary_expr.node_name == "MultiplicativeExpr") {
            if (op == "*") {
                asm_file_ << "imul " << target_reg << ", " << spare_reg << "\n";
            }
            else if (op == "/") {
                if (target != Reg::RAX) {
                    throw std::runtime_error("Division requires RAX target");
                }
                asm_file_ << "cqo\n";
                asm_file_ << "idiv " << spare_reg << "\n";
            }
            else if (op == "%") {
                if (target != Reg::RAX) {
                    throw std::runtime_error("Modulo requires RAX target");
                }
                asm_file_ << "cqo\n";
                asm_file_ << "idiv " << spare_reg << "\n";
                asm_file_ << "mov rax, rdx\n";
            }
        }
        else if (binary_expr.node_name == "RelationalExpr") {
            asm_file_ << "cmp " << target_reg << ", " << spare_reg << "\n";

            if (op == ">") {
                asm_file_ << "setg " << target_byte << "\n";
            }
            else if (op == ">=") {
                asm_file_ << "setge " << target_byte << "\n";
            }
            else if (op == "<") {
                asm_file_ << "setl " << target_byte << "\n";
            }
            else if (op == "<=") {
                asm_file_ << "setle " << target_byte << "\n";
            }

            asm_file_ << "movzx " << target_reg << ", " << target_byte << "\n";
        }
        else if (binary_expr.node_name == "EqualityExpr") {
            asm_file_ << "cmp " << target_reg << ", " << spare_reg << "\n";

            if (op == "==") {
                asm_file_ << "sete " << target_byte << "\n";
            }
            else if (op == "!=") {
                asm_file_ << "setne " << target_byte << "\n";
            }

            asm_file_ << "movzx " << target_reg << ", " << target_byte << "\n";
        }
        else if (binary_expr.node_name == "LogicalAndExpr") {
            asm_file_ << "cmp " << target_reg << ", 0\n";
            asm_file_ << "setne " << target_byte << "\n";
            asm_file_ << "movzx " << target_reg << ", " << target_byte << "\n";
            asm_file_ << "cmp " << spare_reg << ", 0\n";
            asm_file_ << "setne " << spare_byte << "\n";
            asm_file_ << "movzx " << spare_reg << ", " << spare_byte << "\n";
            asm_file_ << "and " << target_reg << ", " << spare_reg << "\n";
        }
        else if (binary_expr.node_name == "LogicalOrExpr") {
            asm_file_ << "cmp " << target_reg << ", 0\n";
            asm_file_ << "setne " << target_byte << "\n";
            asm_file_ << "movzx " << target_reg << ", " << target_byte << "\n";
            asm_file_ << "cmp " << spare_reg << ", 0\n";
            asm_file_ << "setne " << spare_byte << "\n";
            asm_file_ << "movzx " << spare_reg << ", " << spare_byte << "\n";
            asm_file_ << "or " << target_reg << ", " << spare_reg << "\n";
        }
        else {
            report_error(binary_expr, "Unsupported binary expression.");
            throw std::runtime_error("Unsupported binary expression");
        }
    }

    void emit_identifier_expr_to_reg(const DeclRefExpr& identifier_expr,
                                Reg target,
                                FunctionContext& context) {
        const std::string& identifier = get_identifier_name(identifier_expr);

        const auto* symbol_info = find_symbol(context, identifier);

        if (symbol_info != nullptr) {
            if (symbol_info->is_param) {
                asm_file_ << "mov " << reg_name(target)
                          << ", qword [rbp + " << symbol_info->offset << "]\n";
            }
            else {
                asm_file_ << "mov " << reg_name(target)
                          << ", qword [rbp - " << symbol_info->offset << "]\n";
            }
        }
        else {
            report_error(identifier_expr, "Undefined identifier.");
            throw std::runtime_error("Undefined identifier.");
        }
    }

  void emit_integer_expr_to_reg(const IntegerLiteral& integer_expr,
                         Reg target,
                         FunctionContext&) {
      const std::string& lexeme =
          parse_result_.token_list[integer_expr.span_.begin].lexeme;

      const int int_value = std::stoi(lexeme);

      asm_file_ << "mov " << reg_name(target) << ", " << int_value << "\n";
  }
};
