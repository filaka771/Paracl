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
class Node {
public:
    using TokenList = std::vector<Lexer::Token>;
    using TokenIter = TokenList::iterator;

    Node* parent_node = nullptr;
    std::vector<std::unique_ptr<Node>> children_nodes;

    std::string node_name;

    Node(
        std::string node_name,
        Node* parent,
        std::vector<std::unique_ptr<Node>> children
    )
    : parent_node(parent),
      children_nodes(std::move(children)),
      node_name(std::move(node_name))
    {}

    virtual ~Node() = default;

    virtual void print_node() = 0;

    void add_child(std::unique_ptr<Node> child) {
        child->parent_node = this;
        children_nodes.push_back(std::move(child));
    }
};

//----------------Types----------------


//----------------Program----------------
class Program : public Node {
public:
    Program(std::string node_name)
    : Node(std::move(node_name), nullptr, {})
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
        std::string function_id,
        std::tuple<TokenIter, TokenIter> parameter_list,
        std::tuple<TokenIter, TokenIter> compound_stmt
    )
    : Node(std::move(node_name), parent, {}),
      function_id_(std::move(function_id)),
      parameter_list_(parameter_list),
      compound_stmt_(compound_stmt)
    {}

    void print_node() override {
        std::cout << node_name << ": " << function_id_ << '\n';
    }

private:
    std::string function_id_;
    std::tuple<TokenIter, TokenIter> parameter_list_;
    std::tuple<TokenIter, TokenIter> compound_stmt_;
};

class ParameterList : public Node {
public:
    ParameterList(
        Node* parent_ptr,
        std::tuple<TokenIter, TokenIter> parameter_list
    )
    : Node("ParameterList", parent_ptr, {}),
      parameter_list_(parameter_list)
    {}

    void print_node() override {
        std::cout << node_name << '\n';
    }

private:
    std::tuple<TokenIter, TokenIter> parameter_list_;
};

class CompoundStmt : public Node {
public:
    CompoundStmt(
        Node* parent_ptr,
        std::tuple<TokenIter, TokenIter> compound_stmt
    )
    : Node("CompoundStmt", parent_ptr, {}),
      compound_stmt_(compound_stmt)
    {}

    void print_node() override {
        std::cout << node_name << '\n';
    }

private:
    std::tuple<TokenIter, TokenIter> compound_stmt_;
};

//----------------Statements----------------
class Statement : public Node {
public:
    Statement(std::string node_name, Node* parent = nullptr)
    : Node(std::move(node_name), parent, {})
    {}

};

//----------------Expressions----------------
class Expression : public Node {
public:
    Expression(std::string node_name, Node* parent = nullptr)
    : Node(std::move(node_name), parent, {})
    {}

};

class IdentifierExpr : public Node {
public:
    IdentifierExpr(TokenIter ident_iter)
    : Node("IdentifierExpr", nullptr, {}), ident_iter_(ident_iter)
    {}

    void print_node() override {std::cout << ident_iter_->lexeme;}
    void add_shild() {

    }

private:
    TokenIter ident_iter_;

};

class IntegerExpr : public Node {
public:
    IntegerExpr(TokenIter token_iter)
    : Node("IntegerExpr", nullptr, {}),
      token_iter_(token_iter)
    {}

    void print_node() override {
        std::cout << token_iter_->lexeme << "\n";
    }

private:
    TokenIter token_iter_;
};

class PostfixExpr : public Node {
public:
    PostfixExpr(std::tuple<TokenIter, TokenIter> postfix_expr_iter)
    : Node("PostfixExpr", nullptr, {}),
      postfix_iter_iter_(postfix_expr_iter)
    {}

    void print_node() override {
        std::cout << "PostfixOp" << std::get<1>(postfix_iter_iter_)->lexeme << "\n";
    }

private:
    std::tuple<TokenIter, TokenIter> postfix_iter_iter_;
};


class FuncCallExpr : public Node {
public:
    FuncCallExpr(
        TokenIter func_ident_iter,
        std::tuple<TokenIter, TokenIter> argument_list_iter
    ) : Node("FunctionCall", nullptr, {}),
        func_ident_iter_(func_ident_iter),
        argument_list_iter_(argument_list_iter)
    {}

    void print_node() override {
        std::cout << "FunctionCall " << func_ident_iter_->lexeme << "\n";
    }

private:
    TokenIter func_ident_iter_;
    std::tuple<TokenIter, TokenIter> argument_list_iter_;
};

class ArgumentListExpr : public Node {
public:
    ArgumentListExpr()
    : Node("ArgumentList", nullptr, {})
    {}

    void print_node() override {
        std::cout << node_name << "\n";
    }
};

class BinaryOperExpr : public Node {
public:
    BinaryOperExpr(
        std::string node_name,
        TokenIter mul_op_iter,
        std::tuple<TokenIter, TokenIter> left_expr,
        std::tuple<TokenIter, TokenIter> right_expr
    ) : Node(node_name, nullptr, {}),
        mul_op_iter_(mul_op_iter),
        left_expr_(left_expr),
        right_expr_(right_expr)
    {}

    void print_node() override {
        std::cout << node_name << " " << mul_op_iter_->lexeme << "\n";
    }

private:
    TokenIter mul_op_iter_;
    std::tuple<TokenIter, TokenIter> left_expr_;
    std::tuple<TokenIter, TokenIter> right_expr_;
};

class AssignmentExpr : public Node {
public:
    AssignmentExpr(TokenIter assign_iter)
    : Node("AssignmentExpr", nullptr, {}),
      assign_iter_(assign_iter)
    {}

    void print_node() override {
        std::cout << "AssignmentExpr " << assign_iter_->lexeme << "\n";
    }

private:
    TokenIter assign_iter_;
};
//----------------Parser----------------
class Parser {
public:
    using TokenList = std::vector<Lexer::Token>;
    using TokenIter = TokenList::iterator;
    using ChildNode = std::vector<std::unique_ptr<Node>>;

    Parser(Lexer lexer)
    : token_list_(lexer.get_token_list())
    {
        program_ = parse_program();
    }

    const TokenList& get_token_list() const {
        return token_list_;
    }

private:
    TokenList token_list_;
    std::unique_ptr<Program> program_;

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
                  << " "
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
        auto program = std::make_unique<Program>("Program");

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
            func_name_iter->lexeme,
            param_list_iter,
            compound_stmt_iter
        );

        function_node->add_child(
        parse_parameters_list(param_list_iter)
        );

        function_node->add_child(
            std::make_unique<CompoundStmt>(nullptr, compound_stmt_iter)
        );

        return function_node;
    }

    std::unique_ptr<ParameterList> parse_parameters_list(
        std::tuple<TokenIter, TokenIter> param_list
    ) {
        auto parameter_list = std::make_unique<ParameterList>(nullptr, param_list);

        auto token_iter = std::get<0>(param_list) + 1;
        auto end_iter = std::get<1>(param_list);

        while (token_iter < end_iter) {
            if ((token_iter + 1) < end_iter && token_iter->type == Lexer::TokenType::TOKEN_INT && (token_iter + 1)->type == Lexer::TokenType::TOKEN_IDENT) {
                parameter_list->add_child(
                    std::make_unique<IntegerExpr>(token_iter + 1)
                );

                token_iter += 2;
            }
            else if (token_iter->type == Lexer::TokenType::TOKEN_DECIMAL_INT) {
                parameter_list->add_child(
                    std::make_unique<IntegerExpr>(token_iter)
                );

                ++token_iter;
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

    // EXPRASSIONS
    using ExprRange = std::tuple<TokenIter, TokenIter>;

    bool is_postfix_operator(TokenIter iter) {
        return iter->type == Lexer::TokenType::TOKEN_INC ||
            iter->type == Lexer::TokenType::TOKEN_DEC;
    }
    
    std::unique_ptr<Node> parse_primary_expr(TokenIter primary_iter) {
        if (primary_iter->type == Lexer::TokenType::TOKEN_IDENT) {
            return std::make_unique<IdentifierExpr>(primary_iter);
        }

        if (primary_iter->type == Lexer::TokenType::TOKEN_DECIMAL_INT) {
            return std::make_unique<IntegerExpr>(primary_iter);
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

    std::unique_ptr<ArgumentListExpr> parse_argument_list(ExprRange args) {
        auto arg_list = std::make_unique<ArgumentListExpr>();

        auto begin = std::get<0>(args);
        auto end   = std::get<1>(args);

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
                std::make_tuple(begin, end)
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
                begin,
                std::make_tuple(begin + 1, end)
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
            op_iter,
            left_expr,
            right_expr
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
            op_iter,
            left_expr,
            right_expr
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
            op_iter,
            left_expr,
            right_expr
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
            op_iter,
            left_expr,
            right_expr
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
            op_iter,
            left_expr,
            right_expr
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
            op_iter,
            left_expr,
            right_expr
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

                auto node = std::make_unique<AssignmentExpr>(iter);

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
