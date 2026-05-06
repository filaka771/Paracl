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
class IntIdent : public Node {
public:
    IntIdent(TokenIter ident_iter)
    : Node("IntIdent", nullptr, {}),
      ident_iter_(ident_iter)
    {}

    void print_node() override {
        std::cout << ident_iter_->lexeme << "\n";
    }

private:
    TokenIter ident_iter_;
};

class DecimalInt : public Node {
public:
    DecimalInt(TokenIter ident_iter)
    : Node("DecimalInt", nullptr, {}),
      ident_iter_(ident_iter)
    {}

    void print_node() override {
        std::cout << ident_iter_->lexeme << "\n";
    }

private:
    TokenIter ident_iter_;
};

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
// Left unimplemented intentionally.
class Statement : public Node {
public:
    Statement(std::string node_name, Node* parent = nullptr)
    : Node(std::move(node_name), parent, {})
    {}

    // Still abstract because print_node() is not implemented here.
};

//----------------Expressions----------------
// Left unimplemented intentionally.
class Expression : public Node {
public:
    Expression(std::string node_name, Node* parent = nullptr)
    : Node(std::move(node_name), parent, {})
    {}

    // Still abstract because print_node() is not implemented here.
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
        {
            --line_start_iter;
        }

        while (std::next(line_end_iter) != token_list_.end() &&
               std::next(line_end_iter)->line == token_iter->line)
        {
            ++line_end_iter;
        }

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
            std::make_unique<ParameterList>(nullptr, param_list_iter)
        );

        function_node->add_child(
            std::make_unique<CompoundStmt>(nullptr, compound_stmt_iter)
        );

        return function_node;
    }
};
