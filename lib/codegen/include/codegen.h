#include <fstream>
#include <string>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <stdexcept>

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

    struct FunctionContext {
        std::unordered_map<std::string, std::size_t> symbol_table;
        std::size_t next_offset = 0;
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
            if(function_id == "main")
                function_table_[function_id].function_label = "start_";
            else 
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

    void emit_program(const Program& program) {
        asm_file_ << "format ELF64 executable 3\n";
        asm_file_ << "entry _start\n" << "\n";

        for (const auto& child : program.children_nodes) {
            const auto* function_def = static_cast<FunctionDef*>(child.get());
            emit_function(*function_def);
        }
    }

    void emit_function(const FunctionDef& function_def) {
        FunctionContext context;
        const std::string& function_label =
            function_table_.at(function_def.get_function_id()).function_label;

        const auto* compound_stmt =
            static_cast<CompoundStmt*>(function_def.children_nodes[1].get());

        asm_file_ << function_label << ":\n";
        asm_file_ << "push rbp\n";
        asm_file_ << "mov rbp, rsp\n";

        emit_compound_stmt(*compound_stmt, context);
    }

    void emit_compound_stmt(
        const CompoundStmt& compound_stmt,
        FunctionContext& context
    ) {
        for (const auto& node : compound_stmt.children_nodes) {
            emit_statement(*node, context);
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

    void emit_if_stmt(const IfStmt&, FunctionContext&) {}

    void emit_for_stmt(const ForStmt&, FunctionContext&) {}

    void emit_return_stmt(const ReturnStmt&, FunctionContext&) {}

    void emit_break_stmt(const BreakStmt&, FunctionContext&) {}

    void emit_continue_stmt(const ContinueStmt&, FunctionContext&) {}

    void emit_expression_stmt(
        const ExpressionStmt& expression_stmt,
        FunctionContext& context
    ) {
        if (expression_stmt.children_nodes.empty()) {
            return;
        }

        const auto* expression = expression_stmt.children_nodes[0].get();
        emit_expr(*expression, context);
    }

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

    void emit_assignment_expr(
        const AssignmentExpr& assignment_expr,
        FunctionContext& context
    ) {
        const auto* ident =
            static_cast<const IdentifierExpr*>(assignment_expr.children_nodes[0].get());

        const std::string& identifier = get_identifier_name(*ident);

        if (context.symbol_table.find(identifier) == context.symbol_table.end()) {
            context.next_offset += 8;
            context.symbol_table[identifier] = context.next_offset;
        }
    }

    void emit_postfix_expr(const PostfixExpr&, FunctionContext&) {}

    void emit_func_call_expr(const FuncCallExpr&, FunctionContext&) {}

    void emit_binary_expr(const BinaryOperExpr&, FunctionContext&) {}

    void emit_identifier_expr(const IdentifierExpr&, FunctionContext&) {}

    void emit_integer_expr(const IntegerExpr&, FunctionContext&) {}

};
