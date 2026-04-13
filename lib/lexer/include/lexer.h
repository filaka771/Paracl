#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <cctype>

class Lexer {
public:
    Lexer(const std::string& code_file_name);
    void print_tokens() const;

private:
    // Token types
    enum class TokenType {
        // Keywords
        TOKEN_INT,
        TOKEN_FOR,
        TOKEN_IF,
        TOKEN_ELSE,
        TOKEN_RETURN,
        // Identifier
        TOKEN_IDENT,
        // Operators
        TOKEN_ADD,
        TOKEN_SUB,
        TOKEN_MUL,
        TOKEN_DIV,
        TOKEN_EQUAL,
        TOKEN_NEQUAL,
        TOKEN_ASSIGN,
        TOKEN_GREATER_OR_EQ,
        TOKEN_GREATER,
        TOKEN_LESS_OR_EQ,
        TOKEN_LESS,
        // Punctuators
        TOKEN_L_PARENTH,
        TOKEN_R_PARENTH,
        TOKEN_L_BRACE,
        TOKEN_R_BRACE,
        TOKEN_L_BRACKET,
        TOKEN_R_BRACKET,
        TOKEN_SEMICOLON,
        // String & numbers
        TOKEN_STRING,
        TOKEN_DECIMAL_INT,
        // Special tokens
        TOKEN_SPACE,
        TOKEN_NEWLINE,
        TOKEN_TAB,
        TOKEN_FILE_END
    };

    // Token structure
    struct Token {
        TokenType type;
        int line;
        int column;
        std::string lexeme;
    };

    // Member variables
    std::string code_text_;
    std::vector<Token> token_list_;
    std::size_t char_pos_;
    int line_;
    int column_;

    // Static lookup tables (declaration only)
    static const std::unordered_map<std::string_view, TokenType> keywords;
    static const std::unordered_map<TokenType, std::string> token_names;

    // Private methods
    void parse_tokens();
    bool parse_whitespace_and_newline();
    void parse_identifier_or_keyword();
    void parse_operator_or_punctuator();
    void parse_number();
    void parse_string();
    void parse_comment();

    // Helpers
    TokenType lookup_keyword(std::string_view word) const;
    bool is_alpha_or_underscore(char ch) const;
    bool is_keyword_or_id_char(char ch) const;
    bool is_decimal_integer_start() const;
};
