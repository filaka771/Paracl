#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cstdlib>
#include <tuple>
#include "lexer.h"

class Node {
public:
    using TokenList = std::vector<Lexer::Token>;
    using TokenIter = std::vector<Lexer::Token>::iterator;


    Node* parent_node;
    std::vector<std::unique_ptr<Node>> children_nodes;

    std::string node_name;


    Node(std::string node_name, Node* parent, std::vector<std::unique_ptr<Node>> children)
    : parent_node(parent),
      children_nodes(std::move(children)),
      node_name(std::move(node_name)) {}

    virtual ~Node() = default;
    virtual void print_node() = 0;

    void add_child(std::unique_ptr<Node> child) {
        child->parent_node = this;
        children_nodes.push_back(std::move(child));
    }

};

//----------------Types----------------
class IntIdent: public Node {
public:
    IntIdent()
    : Node(std::move("IntIdent"), nullptr, {}) {}

    void print_node() override {std::cout << ident_iter_->lexeme << "\n";}

private:
    TokenIter ident_iter_;
};

class DecimalInt: public Node {
public:
    DecimalInt()
    : Node(std::move("DecimalInt"), nullptr, {}) {}

    void print_node() override {std::cout << ident_iter_->lexeme << "\n";}

private:
    TokenIter ident_iter_;
};

//----------------Program----------------
class Program: public Node {
public:
    Program(std::string node_name)
    :Node(std::move(node_name), nullptr, {}) {}

    void print_node() override {
        std::cout << node_name << '\n';
    }
};

//----------------Functions----------------
class FunctionDef: public Node {
public:
    FunctionDef(std::string node_name, Node* parent,
                std::string function_id,
                std::tuple<TokenIter, TokenIter> parameter_list,
                std::tuple<TokenIter, TokenIter> compound_stmt)
    : Node(std::move(node_name), parent, {}),
      function_id_(function_id),
      parameter_list_(parameter_list),
      compound_stmt_(compound_stmt)
    {};

    void print_node() override {
        std::cout << node_name << ": " << function_id_ << '\n';
    }

private:
    std::string function_id_;
    std::tuple<TokenIter, TokenIter> parameter_list_;
    std::tuple<TokenIter, TokenIter> compound_stmt_;

};

class ParameterList: public Node {
public:
    ParameterList(
                  Node* parent_ptr,
                  std::vector<std::unique_ptr<Node>> parameter,
                  std::tuple<TokenIter, TokenIter> parameter_list
                  )
    : Node(std::move("ParameterList"), nullptr, {}),
      parameter_list_(parameter_list)
    {}

    void print_node() override {std::cout << "\n";}
private:
    std::tuple<TokenIter, TokenIter> parameter_list_;
};

//----------------Statememnts----------------
class Statement: public Node {
public:

private:

};

class CompoundStmt: public Node {
public:
    CompoundStmt(Node* parent_ptr)
    : Node(std::move("FuncDefBody"), parent_ptr, {}) {}

    void print_node() override {std::cout << "\n";}
private:

};

//----------------Expressions----------------
class Expression: public Node {
public:

private:

};



class Parser {
public:
    using TokenList = std::vector<Lexer::Token>;
    using TokenIter = std::vector<Lexer::Token>::iterator;
    using ChildNode = std::vector<std::unique_ptr<Node>>;

    Parser(Lexer lexer) 
    : token_list_(lexer.get_token_list()) {
        program_ = parse_program();
    }

    const std::vector<Lexer::Token>& get_token_list () const {
        return static_cast<const TokenList&> (token_list_);
    }

private:
    std::vector<Lexer::Token> token_list_;
    std::unique_ptr<Program> program_;

    void print_error(TokenIter token_iter, std::string error_msg) {
        // Search for the beginning and end of the line
        auto line_start_iter = token_iter;
        auto line_end_iter = token_iter;

        while(line_start_iter->line == token_iter->line) {
            if(line_start_iter == token_list_.begin())
                break;

            line_start_iter --;
        }

        while(line_end_iter->line == token_iter->line) {
            if (line_end_iter == (token_list_.end() - 1))
                break;

            line_end_iter ++;
        }

        // Print error
        std::cout << "Error " << token_iter->line << ":" << token_iter->column << std::endl;

        // TODO: fix spaces between lexemes
        for(auto iter = line_start_iter; iter <=line_end_iter; iter ++) {
            if(iter !=token_iter) 
                std::cout << iter->lexeme;
            else 
                std::cout << "\033[4;31m" << token_iter->lexeme << "\033[0m";
        }
    }
    //---------------------------------Helpers------------------------------
    std::tuple<TokenIter, TokenIter> find_brackets_pair(
                                                        TokenIter token_iter,
                                                        Lexer::TokenType open_type,
                                                        Lexer::TokenType close_type,
                                                        std::string err_msg_no_open_bracket,
                                                        std::string err_msg_no_close_bracket
                                                        ) {
        std::tuple<TokenIter, TokenIter> brackets_pair;
        if(token_iter->type == open_type) {
            int open_count = 1;
            int close_count = 0;
            std::get<0>(brackets_pair) = token_iter;

            // Searching for the close bracket
            while (open_count != close_count) {
                ++token_iter;

                if (token_iter == token_list_.end()) {
                    print_error(std::prev(token_iter), err_msg_no_close_bracket);
                    throw std::runtime_error("No close bracket");
                }

                if (token_iter->type == open_type)
                    ++open_count;

                if (token_iter->type == close_type)
                    ++close_count;
            }

            if(open_count == close_count) 
                std::get<1>(brackets_pair) = token_iter;

            else {
                print_error(token_iter, err_msg_no_close_bracket);
                throw std::runtime_error("No close bracket");
            }
        }

        else {
            print_error(token_iter, err_msg_no_open_bracket);
            throw std::runtime_error("No open bracket");
        }

        return brackets_pair;
    }


    //---------------------------------Parsers------------------------------
    std::unique_ptr<Program> parse_program() {
        // Now program may contain only function definitions
        // TODO: Move magic 10 into Parser class field
        uint func_list_capacity = 10;
        ChildNode functions_list;
        functions_list.reserve(func_list_capacity);

        auto token_iter = token_list_.begin();
        while(token_iter != token_list_.end()) {
            TokenIter func_name_iter;
            std::tuple<TokenIter, TokenIter> param_list_iter;
            std::tuple<TokenIter, TokenIter> compound_stmt_iter;

            // Function type and name parsing
            // Now the only one type is supported
            if((token_iter + 1) < token_list_.end() &&
                token_iter->type == Lexer::TokenType::TOKEN_INT &&
                (token_iter + 1)->type == Lexer::TokenType::TOKEN_IDENT)
            {
                func_name_iter = (token_iter + 1);
                token_iter +=2;
            }
            else {
                print_error(token_iter,
                            "On the top level program may contain only functions definitions.");
                throw std::runtime_error("Invalid function declaration");
            }

            // Parameter list parsing
            param_list_iter = find_brackets_pair(
                                                token_iter,
                                                Lexer::TokenType::TOKEN_L_PARENTH,
                                                Lexer::TokenType::TOKEN_R_PARENTH,
                                                "Invalid function definition. Function "
                                                "definition must contain list of parameters!",
                                                "No matching right parenthese "
                                                "in function definition!"
                                                );
            if(token_iter != token_list_.end())
                token_iter ++;
            // Corner case: if file ends with function parameter list
            else {
                print_error(func_name_iter, "Invalid function declaration! "
                            "Function declaration luck of a compound statement!");
                throw std::runtime_error("Invalid function declaration");
            }

            // Compound statement parsing
            compound_stmt_iter = find_brackets_pair(
                                                    token_iter,
                                                    Lexer::TokenType::TOKEN_L_PARENTH,
                                                    Lexer::TokenType::TOKEN_R_PARENTH,
                                                    "Function definition must contain "
                                                    "compound statement after list of parameter!",
                                                    "No matching right brace "
                                                    "in function definition!"
                                                    );
            // One more function successfully parsed

            functions_list.push_back(
                std::make_unique<FunctionDef>(
                    "FunctionDef",
                    nullptr,
                    func_name_iter->lexeme,
                    param_list_iter,
                    compound_stmt_iter
                )
            );

            if(token_iter != token_list_.end())
                token_iter ++;

        }

        return functions_list;
    }

    void parse_funcion(Node* parent_ptr,
                       std::tuple<TokenIter, TokenIter> param_list,
                       std::tuple<TokenIter, TokenIter> compound_stmt
                      )
    {
        ParameterList(parent_ptr, {}, param_list);
        CompoundStmt();

    }
};
