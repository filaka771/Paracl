#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cstdlib>
#include <tuple>
#include <stdexcept>
#include <iterator>

#include "lexer.h"

//----------------Base Node----------------
struct SourceSpan {
    std::size_t begin;
    std::size_t end;
};

enum class NodeKind {
    Program,
    FunctionDef,
    ParameterList,
    Parameter,
    CompoundStmt,
    ExpressionStmt,
    NullStmt,
    IfStmt,
    ForStmt,
    ReturnStmt,
    BreakStmt,
    ContinueStmt,
    IdentifierExpr,
    IntegerExpr,
    PostfixExpr,
    FuncCallExpr,
    ArgumentListExpr,
    BinaryOperExpr,
    AssignmentExpr
};

class Node {
public:
    using TokenList = std::vector<Lexer::Token>;
    using TokenIter = TokenList::iterator;

    Node* parent_node = nullptr;
    std::vector<std::unique_ptr<Node>> children_nodes;

    std::string node_name;
    NodeKind node_kind;
    SourceSpan span_;

    Node(
        std::string node_name,
        NodeKind node_kind,
        Node* parent,
        std::vector<std::unique_ptr<Node>> children,
        SourceSpan span
    )
    : parent_node(parent),
      children_nodes(std::move(children)),
      node_name(std::move(node_name)),
      node_kind(node_kind),
      span_(span)
    {}

    virtual ~Node() = default;

    virtual void print_node() = 0;

    void add_child(std::unique_ptr<Node> child) {
        child->parent_node = this;
        children_nodes.push_back(std::move(child));
    }
};

//----------------Program----------------
class Program : public Node {
public:
    Program(std::string node_name, SourceSpan span)
    : Node(std::move(node_name), NodeKind::Program, nullptr, {}, span)
    {}

    void print_node() override {
        std::cout << node_name << '\n';
    }
};

//----------------Functions----------------
class FunctionDef : public Node {
public:
    FunctionDef(
        std::string node_name,
        Node* parent,
        SourceSpan span,
        std::string function_id
    )
    : Node(std::move(node_name), NodeKind::FunctionDef, parent, {}, span),
      function_id_(std::move(function_id))
    {}

    void print_node() override {
        std::cout << node_name << ": " << function_id_ << '\n';
    }

    const std::string& get_function_id() const {return function_id_;}

private:
    std::string function_id_;
};

class ParameterList : public Node {
public:
    ParameterList(
        Node* parent_ptr,
        SourceSpan span
    )
    : Node("ParameterList", NodeKind::ParameterList, parent_ptr, {}, span)
    {}

    void print_node() override {
        std::cout << node_name << '\n';
    }

};

class Parameter : public Node {
public:
    Parameter(
        SourceSpan span,
        std::string type_name,
        std::string ident_name
    )
    : Node("Parameter", NodeKind::Parameter, nullptr, {}, span),
      type_name_(std::move(type_name)),
      ident_name_(std::move(ident_name))
    {}

    void print_node() override {
        std::cout << "Parameter "
                  << type_name_
                  << " "
                  << ident_name_
                  << "\n";
    }

private:
    std::string type_name_;
    std::string ident_name_;
};


//----------------Statements----------------
class CompoundStmt : public Node {
public:
    CompoundStmt(
        Node* parent_ptr,
        SourceSpan span
    )
    : Node("CompoundStmt", NodeKind::CompoundStmt, parent_ptr, {}, span)
    {}

    void print_node() override {
        std::cout << node_name << '\n';
    }

};

class ExpressionStmt : public Node {
public:
    ExpressionStmt(SourceSpan span)
    : Node("ExpressionStmt", NodeKind::ExpressionStmt, nullptr, {}, span)
    {}

    void print_node() override {
        std::cout << node_name << "\n";
    }

};

class NullStmt : public Node {
public:
    NullStmt(SourceSpan span)
    : Node("NullStmt", NodeKind::NullStmt, nullptr, {}, span)
    {}

    void print_node() override {
        std::cout << node_name << "\n";
    }

};

class IfStmt : public Node {
public:
    IfStmt(
        SourceSpan span,
        bool has_else
    )
    : Node("IfStmt", NodeKind::IfStmt, nullptr, {}, span),
      has_else_(has_else)
    {}

    void print_node() override {
        std::cout << node_name << "\n";
    }

private:
    bool has_else_;
};

class ForStmt : public Node {
public:
    ForStmt(
        SourceSpan span,
        bool has_init,
        bool has_cond,
        bool has_step
    )
    : Node("ForStmt", NodeKind::ForStmt, nullptr, {}, span),
      has_init_(has_init),
      has_cond_(has_cond),
      has_step_(has_step)
    {}

    void print_node() override {
        std::cout << node_name << "\n";
    }

    bool has_init() const {return has_init_;}
    bool has_cond() const {return has_cond_;}
    bool has_step() const {return has_step_;}

private:
    bool has_init_;
    bool has_cond_;
    bool has_step_;
};

class ReturnStmt : public Node {
public:
    ReturnStmt(
        SourceSpan span,
        bool has_expr
    )
    : Node("ReturnStmt", NodeKind::ReturnStmt, nullptr, {}, span),
      has_expr_(has_expr)
    {}

    void print_node() override {
        std::cout << node_name << "\n";
    }

private:
    bool has_expr_;
};

class BreakStmt : public Node {
public:
    BreakStmt(SourceSpan span)
    : Node("BreakStmt", NodeKind::BreakStmt, nullptr, {}, span)
    {}

    void print_node() override {
        std::cout << node_name << "\n";
    }

};

class ContinueStmt : public Node {
public:
    ContinueStmt(SourceSpan span)
    : Node("ContinueStmt", NodeKind::ContinueStmt, nullptr, {}, span)
    {}

    void print_node() override {
        std::cout << node_name << "\n";
    }

};

//----------------Expressions----------------
class IdentifierExpr : public Node {
public:
    IdentifierExpr(SourceSpan span, std::string ident_name)
    : Node("IdentifierExpr", NodeKind::IdentifierExpr, nullptr, {}, span),
      ident_name_(std::move(ident_name))
    {}

    void print_node() override {
        std::cout << node_name<< " \"" << ident_name_ << "\"\n";
    }

private:
    std::string ident_name_;
};

class IntegerExpr : public Node {
public:
    IntegerExpr(SourceSpan span, std::string token_value)
    : Node("IntegerExpr", NodeKind::IntegerExpr, nullptr, {}, span),
      token_value_(std::move(token_value))
    {}

    void print_node() override {
        std::cout << node_name << " \"" << token_value_ << "\"\n";
    }

private:
    std::string token_value_;
};

class PostfixExpr : public Node {
public:
    PostfixExpr(SourceSpan span, std::string op_lexeme)
    : Node("PostfixExpr", NodeKind::PostfixExpr, nullptr, {}, span),
      op_lexeme_(std::move(op_lexeme))
    {}

    void print_node() override {
        std::cout << "PostfixOp \"" << op_lexeme_ << "\"\n";
    }

private:
    std::string op_lexeme_;
};


class FuncCallExpr : public Node {
public:
    FuncCallExpr(
        SourceSpan span,
        std::string func_name
    ) : Node("FunctionCall", NodeKind::FuncCallExpr, nullptr, {}, span),
        func_name_(std::move(func_name))
    {}

    void print_node() override {
        std::cout << "FunctionCall " << func_name_ << "\n";
    }

    const std::string& get_func_name() const {return func_name_;}

private:
    std::string func_name_;
};

class ArgumentListExpr : public Node {
public:
    ArgumentListExpr(SourceSpan span)
    : Node("ArgumentList", NodeKind::ArgumentListExpr, nullptr, {}, span)
    {}

    void print_node() override {
        std::cout << node_name << "\n";
    }
};

class BinaryOperExpr : public Node {
public:
    BinaryOperExpr(
        std::string node_name,
        SourceSpan span,
        std::string op_lexeme
    ) : Node(node_name, NodeKind::BinaryOperExpr, nullptr, {}, span),
        op_lexeme_(std::move(op_lexeme))
    {}

    void print_node() override {
        std::cout << node_name << " \"" << op_lexeme_ << "\"\n";
    }

    const std::string& get_op_lexeme() const {return op_lexeme_;}

private:
    std::string op_lexeme_;
};

class AssignmentExpr : public Node {
public:
    AssignmentExpr(SourceSpan span, std::string assign_lexeme)
    : Node("AssignmentExpr", NodeKind::AssignmentExpr, nullptr, {}, span),
      assign_lexeme_(std::move(assign_lexeme))
    {}

    void print_node() override {
        std::cout << "AssignmentExpr \"" << assign_lexeme_ << "\"\n";
    }

private:
    std::string assign_lexeme_;
};
//----------------Parser----------------
class Parser {
public:
    using TokenList = std::vector<Lexer::Token>;
    using TokenIter = TokenList::iterator;
    using ChildNode = std::vector<std::unique_ptr<Node>>;
    using ExprRange = std::tuple<TokenIter, TokenIter>;
    struct ParseResult {
        TokenList token_list;
        std::unique_ptr<Program> program;
    };

    Parser(const Lexer& lexer)
    : token_list_(lexer.get_token_list())
    {
        program_ = parse_program();
    }

    const TokenList& get_token_list() const {
        return token_list_;
    }

    ParseResult release_parse_result() {
        return ParseResult{
            std::move(token_list_),
            std::move(program_)
        };
    }

    void print_ast() {
        print_ast(*program_, "", true);
    }


private:
    TokenList token_list_;
    std::unique_ptr<Program> program_;

    std::size_t to_index(TokenIter token_iter) {
        return static_cast<std::size_t>(
            std::distance(token_list_.begin(), token_iter)
        );
    }

    SourceSpan make_span(TokenIter token_iter) {
        const auto index = to_index(token_iter);
        return SourceSpan{index, index};
    }

    SourceSpan make_span(
        const std::tuple<TokenIter, TokenIter>& token_range
    ) {
        return SourceSpan{
            to_index(std::get<0>(token_range)),
            to_index(std::get<1>(token_range))
        };
    }

    void print_ast(
        Node& node,
        const std::string& prefix,
        bool is_last_child
    ) {
        if (&node != program_.get()) {
            std::cout << prefix
                      << (is_last_child ? "\u2514" : "\u251c");
        }

        node.print_node();

        const std::string child_prefix =
            &node == program_.get()
                ? " "
                : prefix + (is_last_child ? "   " : "\u2502  ");

        for (size_t i = 0; i < node.children_nodes.size(); ++i) {
            print_ast(
                *node.children_nodes[i],
                child_prefix,
                i + 1 == node.children_nodes.size()
            );
        }
    }

    void print_error(TokenIter token_iter, const std::string& error_msg) {
        if (token_list_.empty()) {
            std::cout << "Error: " << error_msg << '\n';
            return;
        }

        if (token_iter == token_list_.end()) {
            token_iter = std::prev(token_list_.end());
        }

        auto line_start_iter = token_iter;
        auto line_end_iter = token_iter;

        while (line_start_iter != token_list_.begin() &&
               std::prev(line_start_iter)->line == token_iter->line) 

            --line_start_iter;

        while (std::next(line_end_iter) != token_list_.end() &&
               std::next(line_end_iter)->line == token_iter->line)

            ++line_end_iter;

        std::cout << "Error "
                  << token_iter->line
                  << ":"
                  << token_iter->column
                  << ""
                  << error_msg
                  << std::endl;

        for (auto iter = line_start_iter; iter <= line_end_iter; ++iter) {
            if (iter != token_iter) {
                std::cout << iter->lexeme;
            } else {
                std::cout << "\033[4;31m"
                          << token_iter->lexeme
                          << "\033[0m";
            }
        }

        std::cout << '\n';
    }

    //---------------------------------Helpers------------------------------
    std::tuple<TokenIter, TokenIter> find_brackets_pair(
        TokenIter token_iter,
        Lexer::TokenType open_type,
        Lexer::TokenType close_type,
        const std::string& err_msg_no_open_bracket,
        const std::string& err_msg_no_close_bracket
    ) {
        if (token_iter == token_list_.end()) {
            print_error(token_iter, err_msg_no_open_bracket);
            throw std::runtime_error("No open bracket");
        }

        std::tuple<TokenIter, TokenIter> brackets_pair;

        if (token_iter->type != open_type) {
            print_error(token_iter, err_msg_no_open_bracket);
            throw std::runtime_error("No open bracket");
        }

        int open_count = 1;
        int close_count = 0;

        std::get<0>(brackets_pair) = token_iter;

        while (open_count != close_count) {
            ++token_iter;

            if (token_iter == token_list_.end()) {
                print_error(std::prev(token_iter), err_msg_no_close_bracket);
                throw std::runtime_error("No close bracket");
            }

            if (token_iter->type == open_type) {
                ++open_count;
            }

            if (token_iter->type == close_type) {
                ++close_count;
            }
        }

        std::get<1>(brackets_pair) = token_iter;
        return brackets_pair;
    }

    //---------------------------------Parsers------------------------------
    std::unique_ptr<Program> parse_program() {
        auto program_span = token_list_.empty()
            ? SourceSpan{0, 0}
            : SourceSpan{0, token_list_.size() - 1};

        auto program = std::make_unique<Program>("Program", program_span);

        auto token_iter = token_list_.begin();

        while (token_iter != token_list_.end()) {
            program->add_child(parse_function(token_iter));
        }

        return program;
    }

    std::unique_ptr<FunctionDef> parse_function(TokenIter& token_iter) {
        TokenIter func_name_iter;

        std::tuple<TokenIter, TokenIter> param_list_iter;
        std::tuple<TokenIter, TokenIter> compound_stmt_iter;

        // Function type and name parsing.
        // Currently only int functions are supported.
        if ((token_iter + 1) < token_list_.end() &&
            token_iter->type == Lexer::TokenType::TOKEN_INT &&
            (token_iter + 1)->type == Lexer::TokenType::TOKEN_IDENT)
        {
            func_name_iter = token_iter + 1;
            token_iter += 2;
        }
        else {
            print_error(
                token_iter,
                "On the top level program may contain only function definitions."
            );

            throw std::runtime_error("Invalid function declaration");
        }

        // Parameter list parsing.
        param_list_iter = find_brackets_pair(
            token_iter,
            Lexer::TokenType::TOKEN_L_PARENTH,
            Lexer::TokenType::TOKEN_R_PARENTH,
            "Invalid function definition. Function definition must contain list of parameters!",
            "No matching right parenthesis in function definition!"
        );

        token_iter = std::get<1>(param_list_iter);
        ++token_iter;

        if (token_iter == token_list_.end()) {
            print_error(
                func_name_iter,
                "Invalid function declaration! Function declaration lacks a compound statement!"
            );

            throw std::runtime_error("Invalid function declaration");
        }

        compound_stmt_iter = find_brackets_pair(
            token_iter,
            Lexer::TokenType::TOKEN_L_BRACE,
            Lexer::TokenType::TOKEN_R_BRACE,
            "Function definition must contain compound statement after list of parameters!",
            "No matching right brace in function definition!"
        );

        token_iter = std::get<1>(compound_stmt_iter);
        ++token_iter;

        auto function_node = std::make_unique<FunctionDef>(
            "FunctionDef",
            nullptr,
            SourceSpan{to_index(func_name_iter - 1), to_index(std::get<1>(compound_stmt_iter))},
            func_name_iter->lexeme
        );

        function_node->add_child(
            parse_parameters_list(param_list_iter)
        );

        auto compound_begin = std::get<0>(compound_stmt_iter);

        function_node->add_child(
            parse_compound_statement(compound_begin)
        );

        return function_node;
    }

    std::unique_ptr<ParameterList> parse_parameters_list(
        std::tuple<TokenIter, TokenIter> param_list
    ) {
        auto parameter_list = std::make_unique<ParameterList>(
            nullptr,
            make_span(param_list)
        );

        auto token_iter = std::get<0>(param_list) + 1;
        auto end_iter = std::get<1>(param_list);

        while (token_iter < end_iter) {
            if ((token_iter + 1) < end_iter &&
                token_iter->type == Lexer::TokenType::TOKEN_INT &&
                (token_iter + 1)->type == Lexer::TokenType::TOKEN_IDENT)
            {
                parameter_list->add_child(
                    std::make_unique<Parameter>(
                        SourceSpan{to_index(token_iter), to_index(token_iter + 1)},
                        token_iter->lexeme,
                        (token_iter + 1)->lexeme
                    )
                );

                token_iter += 2;
            }
            else {
                print_error(token_iter, "Invalid parameter.");
                throw std::runtime_error("Invalid parameter");
            }

            if (token_iter == end_iter) {
                break;
            }

            if (token_iter->type != Lexer::TokenType::TOKEN_COMMA) {
                print_error(token_iter, "Expected ',' between parameters.");
                throw std::runtime_error("Expected comma between parameters");
            }

            ++token_iter;

            if (token_iter == end_iter) {
                print_error(std::prev(token_iter), "Expected parameter after ','.");
                throw std::runtime_error("Expected parameter after comma");
            }
        }

        return parameter_list;
    }
    // STATEMENTS
    void expect_token(
        TokenIter token_iter,
        Lexer::TokenType expected_type,
        const std::string& error_msg
    ) {
        if (token_iter == token_list_.end() || token_iter->type != expected_type) {
            print_error(token_iter, error_msg);
            throw std::runtime_error(error_msg);
        }
    }

    TokenIter find_top_level_semicolon(TokenIter begin, TokenIter end_iter) {
        int paren_depth = 0;
        int bracket_depth = 0;

        for (auto iter = begin; iter != end_iter; ++iter) {
            if (iter->type == Lexer::TokenType::TOKEN_L_PARENTH) {
                ++paren_depth;
            }
            else if (iter->type == Lexer::TokenType::TOKEN_R_PARENTH) {
                --paren_depth;
            }
            else if (iter->type == Lexer::TokenType::TOKEN_L_BRACKET) {
                ++bracket_depth;
            }
            else if (iter->type == Lexer::TokenType::TOKEN_R_BRACKET) {
                --bracket_depth;
            }

            if (paren_depth == 0 &&
                bracket_depth == 0 &&
                iter->type == Lexer::TokenType::TOKEN_SEMICOLON)
            {
                return iter;
            }
        }

        return end_iter;
    }

    std::unique_ptr<Node> parse_statement(TokenIter& token_iter, TokenIter end_iter) {
        if (token_iter == end_iter) {
            print_error(token_iter, "Expected statement.");
            throw std::runtime_error("Expected statement");
        }

        switch (token_iter->type) {
            case Lexer::TokenType::TOKEN_L_BRACE:
                return parse_compound_statement(token_iter);

            case Lexer::TokenType::TOKEN_SEMICOLON:
                return parse_null_statement(token_iter);

            case Lexer::TokenType::TOKEN_IF:
                return parse_if_statement(token_iter, end_iter);

            case Lexer::TokenType::TOKEN_FOR:
                return parse_for_statement(token_iter, end_iter);

            case Lexer::TokenType::TOKEN_RETURN:
                return parse_return_statement(token_iter, end_iter);

            case Lexer::TokenType::TOKEN_BREAK:
                return parse_break_statement(token_iter, end_iter);

            case Lexer::TokenType::TOKEN_CONTINUE:
                return parse_continue_statement(token_iter, end_iter);

            case Lexer::TokenType::TOKEN_ELSE:
                print_error(token_iter, "Unexpected 'else' without matching 'if'.");
                throw std::runtime_error("Unexpected else");

            default:
                return parse_expression_statement(token_iter, end_iter);
        }
    }

    std::unique_ptr<CompoundStmt> parse_compound_statement(TokenIter& token_iter) {
        auto compound_range = find_brackets_pair(
            token_iter,
            Lexer::TokenType::TOKEN_L_BRACE,
            Lexer::TokenType::TOKEN_R_BRACE,
            "Expected '{' to start compound statement.",
            "Expected matching '}' for compound statement."
        );

        auto compound_node = std::make_unique<CompoundStmt>(
            nullptr,
            make_span(compound_range)
        );

        auto stmt_iter = std::get<0>(compound_range) + 1;
        auto compound_end = std::get<1>(compound_range);

        while (stmt_iter < compound_end) {
            compound_node->add_child(
                parse_statement(stmt_iter, compound_end)
            );
        }

        token_iter = compound_end + 1;

        return compound_node;
    }

    std::unique_ptr<Node> parse_expression_statement(
        TokenIter& token_iter,
        TokenIter end_iter
    ) {
        auto stmt_begin = token_iter;

        auto semicolon_iter = find_top_level_semicolon(token_iter, end_iter);

        if (semicolon_iter == end_iter) {
            print_error(token_iter, "Expected ';' after expression.");
            throw std::runtime_error("Expected semicolon after expression");
        }

        auto stmt_range = std::make_tuple(stmt_begin, semicolon_iter);
        auto expr_range = std::make_tuple(stmt_begin, semicolon_iter - 1);

        auto expr_stmt = std::make_unique<ExpressionStmt>(make_span(stmt_range));

        expr_stmt->add_child(
            parse_expr(expr_range)
        );

        token_iter = semicolon_iter + 1;

        return expr_stmt;
    }

    std::unique_ptr<Node> parse_null_statement(TokenIter& token_iter) {
        auto stmt_begin = token_iter;

        expect_token(
            token_iter,
            Lexer::TokenType::TOKEN_SEMICOLON,
            "Expected ';'."
        );

        auto stmt_range = std::make_tuple(stmt_begin, token_iter);

        ++token_iter;

        return std::make_unique<NullStmt>(make_span(stmt_range));
    }

    std::unique_ptr<Node> parse_if_statement(
        TokenIter& token_iter,
        TokenIter end_iter
    ) {
        auto stmt_begin = token_iter;

        expect_token(
            token_iter,
            Lexer::TokenType::TOKEN_IF,
            "Expected 'if'."
        );

        ++token_iter;

        auto condition_full_range = find_brackets_pair(
            token_iter,
            Lexer::TokenType::TOKEN_L_PARENTH,
            Lexer::TokenType::TOKEN_R_PARENTH,
            "Expected '(' after 'if'.",
            "Expected ')' after if condition."
        );

        auto cond_begin = std::get<0>(condition_full_range) + 1;
        auto cond_end = std::get<1>(condition_full_range) - 1;

        if (cond_begin > cond_end) {
            print_error(std::get<0>(condition_full_range), "If condition cannot be empty.");
            throw std::runtime_error("Empty if condition");
        }

        auto condition_range = std::make_tuple(cond_begin, cond_end);
        auto condition_node = parse_expr(condition_range);

        token_iter = std::get<1>(condition_full_range) + 1;

        if (token_iter == end_iter) {
            print_error(token_iter, "Expected statement after if condition.");
            throw std::runtime_error("Expected if body");
        }

        auto then_stmt = parse_statement(token_iter, end_iter);

        bool has_else = false;
        std::unique_ptr<Node> else_stmt = nullptr;

        if (token_iter != end_iter &&
            token_iter->type == Lexer::TokenType::TOKEN_ELSE)
        {
            has_else = true;
            ++token_iter;

            if (token_iter == end_iter) {
                print_error(token_iter, "Expected statement after 'else'.");
                throw std::runtime_error("Expected else body");
            }

            else_stmt = parse_statement(token_iter, end_iter);
        }

        auto stmt_end = token_iter - 1;

        auto if_node = std::make_unique<IfStmt>(
            SourceSpan{to_index(stmt_begin), to_index(stmt_end)},
            has_else
        );

        if_node->add_child(std::move(condition_node));
        if_node->add_child(std::move(then_stmt));

        if (has_else) {
            if_node->add_child(std::move(else_stmt));
        }

        return if_node;
    }

    std::unique_ptr<Node> parse_for_statement(
        TokenIter& token_iter,
        TokenIter end_iter
    ) {
        auto stmt_begin = token_iter;

        expect_token(
            token_iter,
            Lexer::TokenType::TOKEN_FOR,
            "Expected 'for'."
        );

        ++token_iter;

        auto header_full_range = find_brackets_pair(
            token_iter,
            Lexer::TokenType::TOKEN_L_PARENTH,
            Lexer::TokenType::TOKEN_R_PARENTH,
            "Expected '(' after 'for'.",
            "Expected ')' after for header."
        );

        auto header_begin = std::get<0>(header_full_range) + 1;
        auto header_end = std::get<1>(header_full_range) - 1;

        TokenIter first_semicolon = token_list_.end();
        TokenIter second_semicolon = token_list_.end();

        int paren_depth = 0;
        int bracket_depth = 0;

        for (auto iter = header_begin; iter <= header_end; ++iter) {
            if (iter->type == Lexer::TokenType::TOKEN_L_PARENTH) {
                ++paren_depth;
            }
            else if (iter->type == Lexer::TokenType::TOKEN_R_PARENTH) {
                --paren_depth;
            }
            else if (iter->type == Lexer::TokenType::TOKEN_L_BRACKET) {
                ++bracket_depth;
            }
            else if (iter->type == Lexer::TokenType::TOKEN_R_BRACKET) {
                --bracket_depth;
            }

            if (paren_depth == 0 &&
                bracket_depth == 0 &&
                iter->type == Lexer::TokenType::TOKEN_SEMICOLON)
            {
                if (first_semicolon == token_list_.end()) {
                    first_semicolon = iter;
                }
                else {
                    second_semicolon = iter;
                    break;
                }
            }
        }

        if (first_semicolon == token_list_.end() ||
            second_semicolon == token_list_.end())
        {
            print_error(std::get<0>(header_full_range), "For header must contain two semicolons.");
            throw std::runtime_error("Invalid for header");
        }

        bool has_init = header_begin < first_semicolon;
        bool has_cond = first_semicolon + 1 < second_semicolon;
        bool has_step = second_semicolon + 1 <= header_end;

        ExprRange init_range = has_init
            ? std::make_tuple(header_begin, first_semicolon - 1)
            : std::make_tuple(first_semicolon, first_semicolon);

        ExprRange cond_range = has_cond
            ? std::make_tuple(first_semicolon + 1, second_semicolon - 1)
            : std::make_tuple(second_semicolon, second_semicolon);

        ExprRange step_range = has_step
            ? std::make_tuple(second_semicolon + 1, header_end)
            : std::make_tuple(std::get<1>(header_full_range), std::get<1>(header_full_range));

        std::unique_ptr<Node> init_node = nullptr;
        std::unique_ptr<Node> cond_node = nullptr;
        std::unique_ptr<Node> step_node = nullptr;

        if (has_init) {
            init_node = parse_expr(init_range);
        }

        if (has_cond) {
            cond_node = parse_expr(cond_range);
        }

        if (has_step) {
            step_node = parse_expr(step_range);
        }

        token_iter = std::get<1>(header_full_range) + 1;

        if (token_iter == end_iter) {
            print_error(token_iter, "Expected statement after for header.");
            throw std::runtime_error("Expected for body");
        }

        auto body_stmt = parse_statement(token_iter, end_iter);

        auto stmt_end = token_iter - 1;

        auto for_node = std::make_unique<ForStmt>(
            SourceSpan{to_index(stmt_begin), to_index(stmt_end)},
            has_init,
            has_cond,
            has_step
        );

        if (has_init) {
            for_node->add_child(std::move(init_node));
        }

        if (has_cond) {
            for_node->add_child(std::move(cond_node));
        }

        if (has_step) {
            for_node->add_child(std::move(step_node));
        }

        for_node->add_child(std::move(body_stmt));

        return for_node;
    }

    std::unique_ptr<Node> parse_return_statement(
        TokenIter& token_iter,
        TokenIter end_iter
    ) {
        auto stmt_begin = token_iter;

        expect_token(
            token_iter,
            Lexer::TokenType::TOKEN_RETURN,
            "Expected 'return'."
        );

        ++token_iter;

        auto semicolon_iter = find_top_level_semicolon(token_iter, end_iter);

        if (semicolon_iter == end_iter) {
            print_error(stmt_begin, "Expected ';' after return statement.");
            throw std::runtime_error("Expected semicolon after return");
        }

        bool has_expr = token_iter < semicolon_iter;

        ExprRange expr_range = has_expr
            ? std::make_tuple(token_iter, semicolon_iter - 1)
            : std::make_tuple(semicolon_iter, semicolon_iter);

        auto return_node = std::make_unique<ReturnStmt>(
            SourceSpan{to_index(stmt_begin), to_index(semicolon_iter)},
            has_expr
        );

        if (has_expr) {
            return_node->add_child(
                parse_expr(expr_range)
            );
        }

        token_iter = semicolon_iter + 1;

        return return_node;
    }

    std::unique_ptr<Node> parse_break_statement(
        TokenIter& token_iter,
        TokenIter end_iter
    ) {
        auto stmt_begin = token_iter;

        expect_token(
            token_iter,
            Lexer::TokenType::TOKEN_BREAK,
            "Expected 'break'."
        );

        ++token_iter;

        expect_token(
            token_iter,
            Lexer::TokenType::TOKEN_SEMICOLON,
            "Expected ';' after break."
        );

        auto stmt_end = token_iter;
        auto stmt_range = std::make_tuple(stmt_begin, stmt_end);

        ++token_iter;

        return std::make_unique<BreakStmt>(make_span(stmt_range));
    }

    std::unique_ptr<Node> parse_continue_statement(
        TokenIter& token_iter,
        TokenIter end_iter
    ) {
        auto stmt_begin = token_iter;

        expect_token(
            token_iter,
            Lexer::TokenType::TOKEN_CONTINUE,
            "Expected 'continue'."
        );

        ++token_iter;

        expect_token(
            token_iter,
            Lexer::TokenType::TOKEN_SEMICOLON,
            "Expected ';' after continue."
        );

        auto stmt_end = token_iter;
        auto stmt_range = std::make_tuple(stmt_begin, stmt_end);

        ++token_iter;

        return std::make_unique<ContinueStmt>(make_span(stmt_range));
    }

    // EXPRASSIONS

    bool is_postfix_operator(TokenIter iter) {
        return iter->type == Lexer::TokenType::TOKEN_INC ||
            iter->type == Lexer::TokenType::TOKEN_DEC;
    }
    
    std::unique_ptr<Node> parse_primary_expr(TokenIter primary_iter) {
        if (primary_iter->type == Lexer::TokenType::TOKEN_IDENT) {
            return std::make_unique<IdentifierExpr>(
                make_span(primary_iter),
                primary_iter->lexeme
            );
        }

        if (primary_iter->type == Lexer::TokenType::TOKEN_DECIMAL_INT) {
            return std::make_unique<IntegerExpr>(
                make_span(primary_iter),
                primary_iter->lexeme
            );
        }

        print_error(primary_iter, "Expected primary expression.");
        throw std::runtime_error("Expected primary expression");
    }

    std::unique_ptr<Node> parse_primary_expr(ExprRange expr) {
        auto begin = std::get<0>(expr);
        auto end   = std::get<1>(expr);

        if (begin > end) {
            print_error(begin, "Expected primary expression.");
            throw std::runtime_error("Expected primary expression");
        }

        if (begin == end) {
            return parse_primary_expr(begin);
        }

        if (begin->type == Lexer::TokenType::TOKEN_L_PARENTH &&
            end->type == Lexer::TokenType::TOKEN_R_PARENTH)
        {
            return parse_expr(std::make_tuple(begin + 1, end - 1));
        }

        print_error(begin, "Invalid primary expression.");
        throw std::runtime_error("Invalid primary expression");
    }

    std::unique_ptr<Node> parse_argument_list(ExprRange args) {
        auto begin = std::get<0>(args);
        auto end   = std::get<1>(args);

        auto arg_list = begin > end
            ? std::make_unique<ArgumentListExpr>(SourceSpan{0, 0})
            : std::make_unique<ArgumentListExpr>(make_span(args));

        if (begin > end) {
            return arg_list;
        }

        int paren_depth = 0;
        auto arg_begin = begin;

        for (auto iter = begin; iter <= end; ++iter) {
            if (iter->type == Lexer::TokenType::TOKEN_L_PARENTH) {
                ++paren_depth;
            }
            else if (iter->type == Lexer::TokenType::TOKEN_R_PARENTH) {
                --paren_depth;
            }

            if (paren_depth == 0 &&
                iter->type == Lexer::TokenType::TOKEN_COMMA)
            {
                if (arg_begin > iter - 1) {
                    print_error(iter, "Expected argument before comma.");
                    throw std::runtime_error("Expected argument before comma");
                }

                arg_list->add_child(
                    parse_expr(std::make_tuple(arg_begin, iter - 1))
                );

                arg_begin = iter + 1;
            }
        }

        if (arg_begin <= end) {
            arg_list->add_child(
                parse_expr(std::make_tuple(arg_begin, end))
            );
        }

        return arg_list;
    }

    std::unique_ptr<Node> parse_postfix_expr(ExprRange expr) {
        auto begin = std::get<0>(expr);
        auto end   = std::get<1>(expr);

        if (begin > end) {
            print_error(begin, "Expected postfix expression.");
            throw std::runtime_error("Expected postfix expression");
        }

        // x++ / x--
        if (begin < end && is_postfix_operator(end)) {
            auto postfix_node = std::make_unique<PostfixExpr>(
                make_span(expr),
                end->lexeme
            );

            postfix_node->add_child(
                parse_postfix_expr(std::make_tuple(begin, end - 1))
            );

            return postfix_node;
        }

        // foo() / foo(a, b)
        if (begin->type == Lexer::TokenType::TOKEN_IDENT &&
            begin + 1 <= end &&
            (begin + 1)->type == Lexer::TokenType::TOKEN_L_PARENTH &&
            end->type == Lexer::TokenType::TOKEN_R_PARENTH)
        {
            auto call_node = std::make_unique<FuncCallExpr>(
                make_span(expr),
                begin->lexeme
            );

            call_node->add_child(
                parse_argument_list(std::make_tuple(begin + 2, end - 1))
            );

            return call_node;
        }

        return parse_primary_expr(expr);
    }

    TokenIter find_binary_operator(
        ExprRange expr,
        const std::vector<Lexer::TokenType>& operators
    ) {
        auto begin = std::get<0>(expr);
        auto end   = std::get<1>(expr);

        int paren_depth = 0;

        for (auto iter = end;; --iter) {
            if (iter->type == Lexer::TokenType::TOKEN_R_PARENTH) {
                ++paren_depth;
            }
            else if (iter->type == Lexer::TokenType::TOKEN_L_PARENTH) {
                --paren_depth;
            }

            if (paren_depth == 0) {
                for (auto op : operators) {
                    if (iter->type == op) {
                        return iter;
                    }
                }
            }

            if (iter == begin) {
                break;
            }
        }

        return token_list_.end();
    }

    std::unique_ptr<Node> parse_mul_expr(ExprRange expr) {
        auto op_iter = find_binary_operator(
            expr,
            {
                Lexer::TokenType::TOKEN_MUL,
                Lexer::TokenType::TOKEN_DIV,
                Lexer::TokenType::TOKEN_PERCENT
            }
        );

        if (op_iter == token_list_.end()) {
            return parse_postfix_expr(expr);
        }

        auto left_expr  = std::make_tuple(std::get<0>(expr), op_iter - 1);
        auto right_expr = std::make_tuple(op_iter + 1, std::get<1>(expr));

        auto node = std::make_unique<BinaryOperExpr>(
            "MultiplicativeExpr",
            make_span(expr),
            op_iter->lexeme
        );

        node->add_child(parse_mul_expr(left_expr));
        node->add_child(parse_postfix_expr(right_expr));

        return node;
    }

    std::unique_ptr<Node> parse_add_expr(ExprRange expr) {
        auto op_iter = find_binary_operator(
            expr,
            {
                Lexer::TokenType::TOKEN_ADD,
                Lexer::TokenType::TOKEN_SUB
            }
        );

        if (op_iter == token_list_.end()) {
            return parse_mul_expr(expr);
        }

        auto left_expr  = std::make_tuple(std::get<0>(expr), op_iter - 1);
        auto right_expr = std::make_tuple(op_iter + 1, std::get<1>(expr));

        auto node = std::make_unique<BinaryOperExpr>(
            "AdditiveExpr",
            make_span(expr),
            op_iter->lexeme
        );

        node->add_child(parse_add_expr(left_expr));
        node->add_child(parse_mul_expr(right_expr));

        return node;
    }

    std::unique_ptr<Node> parse_rel_expr(ExprRange expr) {
        auto op_iter = find_binary_operator(
            expr,
            {
                Lexer::TokenType::TOKEN_GREATER,
                Lexer::TokenType::TOKEN_GREATER_OR_EQ,
                Lexer::TokenType::TOKEN_LESS,
                Lexer::TokenType::TOKEN_LESS_OR_EQ
            }
        );

        if (op_iter == token_list_.end()) {
            return parse_add_expr(expr);
        }

        auto left_expr  = std::make_tuple(std::get<0>(expr), op_iter - 1);
        auto right_expr = std::make_tuple(op_iter + 1, std::get<1>(expr));

        auto node = std::make_unique<BinaryOperExpr>(
            "RelationalExpr",
            make_span(expr),
            op_iter->lexeme
        );

        node->add_child(parse_rel_expr(left_expr));
        node->add_child(parse_add_expr(right_expr));

        return node;
    }

    std::unique_ptr<Node> parse_equality_expr(ExprRange expr) {
        auto op_iter = find_binary_operator(
            expr,
            {
                Lexer::TokenType::TOKEN_EQUAL,
                Lexer::TokenType::TOKEN_NEQUAL
            }
        );

        if (op_iter == token_list_.end()) {
            return parse_rel_expr(expr);
        }

        auto left_expr  = std::make_tuple(std::get<0>(expr), op_iter - 1);
        auto right_expr = std::make_tuple(op_iter + 1, std::get<1>(expr));

        auto node = std::make_unique<BinaryOperExpr>(
            "EqualityExpr",
            make_span(expr),
            op_iter->lexeme
        );

        node->add_child(parse_equality_expr(left_expr));
        node->add_child(parse_rel_expr(right_expr));

        return node;
    }

    std::unique_ptr<Node> parse_logical_and_expr(ExprRange expr) {
        auto op_iter = find_binary_operator(
            expr,
            {
                Lexer::TokenType::TOKEN_LOGICAL_AND
            }
        );

        if (op_iter == token_list_.end()) {
            return parse_equality_expr(expr);
        }

        auto left_expr  = std::make_tuple(std::get<0>(expr), op_iter - 1);
        auto right_expr = std::make_tuple(op_iter + 1, std::get<1>(expr));

        auto node = std::make_unique<BinaryOperExpr>(
            "LogicalAndExpr",
            make_span(expr),
            op_iter->lexeme
        );

        node->add_child(parse_logical_and_expr(left_expr));
        node->add_child(parse_equality_expr(right_expr));

        return node;
    }

    std::unique_ptr<Node> parse_logical_or_expr(ExprRange expr) {
        auto op_iter = find_binary_operator(
            expr,
            {
                Lexer::TokenType::TOKEN_LOGICAL_OR
            }
        );

        if (op_iter == token_list_.end()) {
            return parse_logical_and_expr(expr);
        }

        auto left_expr  = std::make_tuple(std::get<0>(expr), op_iter - 1);
        auto right_expr = std::make_tuple(op_iter + 1, std::get<1>(expr));

        auto node = std::make_unique<BinaryOperExpr>(
            "LogicalOrExpr",
            make_span(expr),
            op_iter->lexeme
        );

        node->add_child(parse_logical_or_expr(left_expr));
        node->add_child(parse_logical_and_expr(right_expr));

        return node;
    }

    std::unique_ptr<Node> parse_assignment_expr(ExprRange expr) {
        auto begin = std::get<0>(expr);
        auto end   = std::get<1>(expr);

        int paren_depth = 0;

        for (auto iter = begin; iter <= end; ++iter) {
            if (iter->type == Lexer::TokenType::TOKEN_L_PARENTH) {
                ++paren_depth;
            }
            else if (iter->type == Lexer::TokenType::TOKEN_R_PARENTH) {
                --paren_depth;
            }

            if (paren_depth == 0 &&
                iter->type == Lexer::TokenType::TOKEN_ASSIGN)
            {
                auto left_expr  = std::make_tuple(begin, iter - 1);
                auto right_expr = std::make_tuple(iter + 1, end);

                auto node = std::make_unique<AssignmentExpr>(
                    make_span(expr),
                    iter->lexeme
                );

                node->add_child(parse_postfix_expr(left_expr));
                node->add_child(parse_assignment_expr(right_expr));

                return node;
            }
        }

        return parse_logical_or_expr(expr);
    }

    std::unique_ptr<Node> parse_expr(ExprRange expr) {
        return parse_assignment_expr(expr);
    }
};
