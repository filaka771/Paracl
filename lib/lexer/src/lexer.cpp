#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <cctype>
#include "lexer.h"

const std::unordered_map<std::string_view, Lexer::TokenType> Lexer::keywords = {
    {"int",    TokenType::TOKEN_INT},
    {"for",    TokenType::TOKEN_FOR},
    {"if",     TokenType::TOKEN_IF},
    {"else",   TokenType::TOKEN_ELSE},
    {"return", TokenType::TOKEN_RETURN}
};

const std::unordered_map<Lexer::TokenType, std::string> Lexer::token_names = {
    {TokenType::TOKEN_INT,           "int"},
    {TokenType::TOKEN_FOR,           "for"},
    {TokenType::TOKEN_IF,            "if"},
    {TokenType::TOKEN_ELSE,          "else"},
    {TokenType::TOKEN_RETURN,        "return"},
    {TokenType::TOKEN_IDENT,         "identifier"},
    {TokenType::TOKEN_ADD,           "+"},
    {TokenType::TOKEN_SUB,           "-"},
    {TokenType::TOKEN_MUL,           "*"},
    {TokenType::TOKEN_DIV,           "/"},
    {TokenType::TOKEN_EQUAL,         "=="},
    {TokenType::TOKEN_NEQUAL,        "!="},
    {TokenType::TOKEN_ASSIGN,        "="},
    {TokenType::TOKEN_GREATER_OR_EQ, ">="},
    {TokenType::TOKEN_GREATER,       ">"},
    {TokenType::TOKEN_LESS_OR_EQ,    "<="},
    {TokenType::TOKEN_LESS,          "<"},
    {TokenType::TOKEN_L_PARENTH,     "("},
    {TokenType::TOKEN_R_PARENTH,     ")"},
    {TokenType::TOKEN_L_BRACE,       "{"},
    {TokenType::TOKEN_R_BRACE,       "}"},
    {TokenType::TOKEN_L_BRACKET,     "["},
    {TokenType::TOKEN_R_BRACKET,     "]"},
    {TokenType::TOKEN_SEMICOLON,     ";"},
    {TokenType::TOKEN_STRING,        "string literal"},
    {TokenType::TOKEN_DECIMAL_INT,   "integer constant"}
};
// ---------- Constructor ----------
Lexer::Lexer(const std::string& code_file_name)
    : char_pos_(0), line_(1), column_(1)
{
    std::ifstream file(code_file_name);
    if (!file.is_open())
        throw std::runtime_error("File open failed: " + code_file_name);
    code_text_ = std::string(std::istreambuf_iterator<char>(file),
                             std::istreambuf_iterator<char>());

    code_text_.push_back('\0');
    parse_tokens();
}

void Lexer::print_tokens() const {
    for (const auto& tok : token_list_) {
        std::cout << static_cast<int>(tok.type) << "  "
                  << tok.line << ":" << tok.column << "  "
                  << tok.lexeme << "\n";
    }
}

void Lexer::parse_tokens() {
    bool eof = false;
    while (!eof) {
        // Skip whitespace and newlines
        eof = parse_whitespace_and_newline();
        if (eof) break;

        // Try each token type in a sensible order
        if (is_alpha_or_underscore(code_text_[char_pos_])) {
            parse_identifier_or_keyword();
        }
        else if (std::isdigit(code_text_[char_pos_])) {
            parse_number();
        }
        else if (code_text_[char_pos_] == '"') {
            parse_string();
        }
        else if (code_text_[char_pos_] == '/') {
            // Could be a comment or division operator
            if (code_text_[char_pos_ + 1] == '/' || code_text_[char_pos_ + 1] == '*')
                parse_comment();
            else
                parse_operator_or_punctuator();
        }
        else {
            parse_operator_or_punctuator();
        }
    }
}

// ---------- Parser implementations ----------
bool Lexer::parse_whitespace_and_newline() {
    while (true) {
        char ch = code_text_[char_pos_];
        if (ch == ' ' || ch == '\t') {
            // Heandle tab as 4 spaces
            if (ch == '\t') column_ += 4;
            else column_++;
            char_pos_++;
        }
        else if (ch == '\n') {
            line_++;
            column_ = 1;
            char_pos_++;
        }
        else if (ch == '\r') {
            char_pos_++;
        }
        else {
            break;
        }
    }
    // Return true if end of file reached
    return code_text_[char_pos_] == '\0';
}

void Lexer::parse_identifier_or_keyword() {
    std::size_t start = char_pos_;
    while (is_keyword_or_id_char(code_text_[char_pos_])) {
        char_pos_++;
    }
    std::string lexeme = code_text_.substr(start, char_pos_ - start);

    TokenType type = lookup_keyword(lexeme);
    token_list_.push_back({type, line_, column_, lexeme});

    column_ += static_cast<int>(lexeme.size());
}

void Lexer::parse_operator_or_punctuator() {
    char ch = code_text_[char_pos_];
    std::string lexeme;
    TokenType type = TokenType::TOKEN_FILE_END; // dummy

    switch (ch) {
        case '+': type = TokenType::TOKEN_ADD; lexeme = "+"; char_pos_++; break;
        case '-': type = TokenType::TOKEN_SUB; lexeme = "-"; char_pos_++; break;
        case '*': type = TokenType::TOKEN_MUL; lexeme = "*"; char_pos_++; break;
        case '/': type = TokenType::TOKEN_DIV; lexeme = "/"; char_pos_++; break;
        case '=':
            if (code_text_[char_pos_ + 1] == '=') {
                type = TokenType::TOKEN_EQUAL;
                lexeme = "==";
                char_pos_ += 2;
            } else {
                type = TokenType::TOKEN_ASSIGN;
                lexeme = "=";
                char_pos_++;
            }
            break;
        case '!':
            if (code_text_[char_pos_ + 1] == '=') {
                type = TokenType::TOKEN_NEQUAL;
                lexeme = "!=";
                char_pos_ += 2;
            } else {
                // error: unexpected '!'
                char_pos_++;
                return;
            }
            break;
        case '>':
            if (code_text_[char_pos_ + 1] == '=') {
                type = TokenType::TOKEN_GREATER_OR_EQ;
                lexeme = ">=";
                char_pos_ += 2;
            } else {
                type = TokenType::TOKEN_GREATER;
                lexeme = ">";
                char_pos_++;
            }
            break;
        case '<':
            if (code_text_[char_pos_ + 1] == '=') {
                type = TokenType::TOKEN_LESS_OR_EQ;
                lexeme = "<=";
                char_pos_ += 2;
            } else {
                type = TokenType::TOKEN_LESS;
                lexeme = "<";
                char_pos_++;
            }
            break;
        case '(': type = TokenType::TOKEN_L_PARENTH; lexeme = "("; char_pos_++; break;
        case ')': type = TokenType::TOKEN_R_PARENTH; lexeme = ")"; char_pos_++; break;
        case '{': type = TokenType::TOKEN_L_BRACE;   lexeme = "{"; char_pos_++; break;
        case '}': type = TokenType::TOKEN_R_BRACE;   lexeme = "}"; char_pos_++; break;
        case '[': type = TokenType::TOKEN_L_BRACKET; lexeme = "["; char_pos_++; break;
        case ']': type = TokenType::TOKEN_R_BRACKET; lexeme = "]"; char_pos_++; break;
        case ';': type = TokenType::TOKEN_SEMICOLON; lexeme = ";"; char_pos_++; break;
        default:
            // Unknown character!
            char_pos_++;
            return;
    }

    token_list_.push_back({type, line_, column_, lexeme});
    column_ += static_cast<int>(lexeme.size());
}

void Lexer::parse_number() {
    std::size_t start = char_pos_;
    // Decimal integers
    while (std::isdigit(code_text_[char_pos_])) {
        char_pos_++;
    }
    std::string lexeme = code_text_.substr(start, char_pos_ - start);
    token_list_.push_back({TokenType::TOKEN_DECIMAL_INT, line_, column_, lexeme});
    column_ += static_cast<int>(lexeme.size());
}

void Lexer::parse_string() {
    char_pos_++; // skip opening "
    std::size_t start = char_pos_;
    while (code_text_[char_pos_] != '"' && code_text_[char_pos_] != '\0') {
        char_pos_++;
    }
    std::string lexeme = code_text_.substr(start, char_pos_ - start);
    if (code_text_[char_pos_] == '"') {
        char_pos_++; // skip closing "
    }
    token_list_.push_back({TokenType::TOKEN_STRING, line_, column_, lexeme});
    // column update: +2 for the quotes, + length of content
    column_ += static_cast<int>(lexeme.size()) + 2;
}

void Lexer::parse_comment() {
    // We already know current char is '/' and next is '/' or '*'
    if (code_text_[char_pos_ + 1] == '/') {
        // single‑line comment: consume until newline or EOF
        char_pos_ += 2;
        while (code_text_[char_pos_] != '\n' && code_text_[char_pos_] != '\0') {
            char_pos_++;
        }
        // newline will be handled by the whitespace parser on the next iteration
    }
    else if (code_text_[char_pos_ + 1] == '*') {
        // multi‑line comment
        char_pos_ += 2;
        while (!(code_text_[char_pos_] == '*' && code_text_[char_pos_ + 1] == '/') &&
               code_text_[char_pos_] != '\0') {
            if (code_text_[char_pos_] == '\n') {
                line_++;
                column_ = 1;
            } else {
                column_++;
            }
            char_pos_++;
        }
        if (code_text_[char_pos_] == '*') {
            char_pos_ += 2; // skip "*/"
        }
    }
    // No token is added for comments
}

// ---------- Helper functions ----------
Lexer::TokenType Lexer::lookup_keyword(std::string_view word) const {
    auto it = keywords.find(word);
    return (it != keywords.end()) ? it->second : TokenType::TOKEN_IDENT;
}

bool Lexer::is_alpha_or_underscore(char ch) const {
    return std::isalpha(static_cast<unsigned char>(ch)) || ch == '_';
}

bool Lexer::is_keyword_or_id_char(char ch) const {
    return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
}

bool Lexer::is_decimal_integer_start() const {
    char ch = code_text_[char_pos_];
    char next = code_text_[char_pos_ + 1];
    return (ch == '0' && !std::isdigit(static_cast<unsigned char>(next))) ||
           (ch != '0');
}
