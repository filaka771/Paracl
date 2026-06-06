#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "cli-table.h"

class Lexer {
public:
    // Token types
    enum class TokenType {
        // Keywords
        TOKEN_INT,
        TOKEN_FOR,
        TOKEN_IF,
        TOKEN_ELSE,
        TOKEN_CONTINUE,
        TOKEN_BREAK,
        TOKEN_RETURN, 
        // Identifier
        TOKEN_IDENT,
        // Operators
        TOKEN_ADD,
        TOKEN_SUB,
        TOKEN_MUL,
        TOKEN_DIV,
        TOKEN_PERCENT,
        TOKEN_EQUAL,
        TOKEN_NEQUAL,
        TOKEN_ASSIGN,
        TOKEN_GREATER_OR_EQ,
        TOKEN_GREATER,
        TOKEN_LESS_OR_EQ,
        TOKEN_LESS,
        TOKEN_INC,
        TOKEN_DEC,
        TOKEN_LOGICAL_AND,
        TOKEN_LOGICAL_OR,
        TOKEN_NOT,
        // Punctuators
        TOKEN_L_PAREN,
        TOKEN_R_PAREN,
        TOKEN_L_BRACE,
        TOKEN_R_BRACE,
        TOKEN_L_BRACKET,
        TOKEN_R_BRACKET,
        TOKEN_SEMICOLON,
        TOKEN_COMMA,
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
    Lexer(const std::string& code_file_name);
    void print_tokens(const int initial_pad = 0) const;
    const std::vector<Token>& get_token_list() const;

    // Static lookup tables
    static const std::unordered_map<std::string_view, TokenType> keywords;
    static const std::unordered_map<TokenType, std::string> token_names;

    // Diagnostic
    struct Diagnostic {
        int line;
        int column;
        std::string message;
        std::string lexeme;
    };

    const std::vector<Diagnostic>& get_diagnostics() const;
    bool has_errors() const;

    // Print
    struct TokenPrintRow {
        std::string token_name;
        std::string position;
        std::string token_code;
        std::string lexeme;
    };

private:
    // Member variables
    std::vector<Token> token_list_;
    std::string code_text_;
    std::size_t char_pos_;
    int line_;
    int column_;

    // Diagnostic
    std::vector<Diagnostic> diagnostics_;

    void report_error(int line, int column,
                      std::string message,
                      std::string lexeme);

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

template<>
struct field_tuple<Lexer::TokenPrintRow> {
    static constexpr auto value = std::make_tuple(
        &Lexer::TokenPrintRow::token_name,
        &Lexer::TokenPrintRow::position,
        &Lexer::TokenPrintRow::token_code,
        &Lexer::TokenPrintRow::lexeme
    );
};
