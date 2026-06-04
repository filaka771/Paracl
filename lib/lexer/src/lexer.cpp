#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <cctype>
#include <algorithm>
#include "lexer.h"
#include "cli-tabel.h"

const std::unordered_map<std::string_view, Lexer::TokenType> Lexer::keywords = {
    {"int",    TokenType::TOKEN_INT},
    {"for",    TokenType::TOKEN_FOR},
    {"if",     TokenType::TOKEN_IF},
    {"else",   TokenType::TOKEN_ELSE},
    {"continue", TokenType::TOKEN_CONTINUE},
    {"break", TokenType::TOKEN_BREAK},
    {"return", TokenType::TOKEN_RETURN}
};

const std::unordered_map<Lexer::TokenType, std::string> Lexer::token_names = {
    {TokenType::TOKEN_INT,           "INT"},
    {TokenType::TOKEN_FOR,           "FOR"},
    {TokenType::TOKEN_IF,            "IF"},
    {TokenType::TOKEN_ELSE,          "ELSE"},
    {TokenType::TOKEN_CONTINUE,      "CONTINUE"},
    {TokenType::TOKEN_BREAK,         "BREAK"},
    {TokenType::TOKEN_RETURN,        "RETURN"},
    {TokenType::TOKEN_IDENT,         "ID"},
    {TokenType::TOKEN_ADD,           "PLUS"},
    {TokenType::TOKEN_SUB,           "MINUS"},
    {TokenType::TOKEN_MUL,           "ASTERISC"},
    {TokenType::TOKEN_DIV,           "DIVISION"},
    {TokenType::TOKEN_PERCENT,       "PERCENT"},
    {TokenType::TOKEN_EQUAL,         "EQUAL"},
    {TokenType::TOKEN_NEQUAL,        "NOT_EQUAL"},
    {TokenType::TOKEN_ASSIGN,        "ASSIGN"},
    {TokenType::TOKEN_GREATER_OR_EQ, "GREATER_OR_EQ"},
    {TokenType::TOKEN_GREATER,       "GREATER"},
    {TokenType::TOKEN_LESS_OR_EQ,    "LESS_OR_EQ"},
    {TokenType::TOKEN_LESS,          "LESS"},
    {TokenType::TOKEN_INC,           "INC"},
    {TokenType::TOKEN_DEC,           "DEC"},
    {TokenType::TOKEN_LOGICAL_AND,   "LOG_AND"},
    {TokenType::TOKEN_LOGICAL_OR,    "LOG_OR"},
    {TokenType::TOKEN_NOT,           "NOT"},
    {TokenType::TOKEN_L_PARENTH,     "L_PARENTH"},
    {TokenType::TOKEN_R_PARENTH,     "R_PARENTH"},
    {TokenType::TOKEN_L_BRACE,       "L_BRACE"},
    {TokenType::TOKEN_R_BRACE,       "R_BRACE"},
    {TokenType::TOKEN_L_BRACKET,     "L_BRACKET"},
    {TokenType::TOKEN_R_BRACKET,     "R_BRACKET"},
    {TokenType::TOKEN_SEMICOLON,     "SEMICOLON"},
    {TokenType::TOKEN_COMMA,         "COMMA"},
    {TokenType::TOKEN_STRING,        "STR_LITERAL"},
    {TokenType::TOKEN_DECIMAL_INT,   "DECIMAL_INT"}
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
// ---------- Getters ---------------
const std::vector<Lexer::Token>& Lexer::get_token_list() const {
    return static_cast<const std::vector<Lexer::Token>&>(token_list_);
};
// ---------- Diagnostic ----------
const std::vector<Lexer::Diagnostic>& Lexer::get_diagnostics() const {
    return diagnostics_;
}

bool Lexer::has_errors() const {
    return !diagnostics_.empty();
}

void Lexer::report_error(int line, int column,
                         std::string message,
                         std::string lexeme) {
    diagnostics_.push_back({
        line,
        column,
        std::move(message),
        lexeme
    });

}
// ---------- Print_tokens ----------
void Lexer::print_tokens(const int initial_pad) const {
    std::vector<TokenPrintRow> rows;

    rows.push_back({
        "TOKEN NAME",
        "POSITION",
        "TOKEN CODE",
        "LEXEME"
    });

    for (const auto& tok : token_list_) {
        rows.push_back({
            Lexer::token_names.at(tok.type),
            std::to_string(tok.line) + ":" + std::to_string(tok.column),
            std::to_string(static_cast<int>(tok.type)),
            tok.lexeme
        });
    }

    if (initial_pad > 0) {
        std::cout << std::string(initial_pad, ' ');
    }

    CliTable<TokenPrintRow> table;
    table.print(rows, " | ", true);
}

// ---------- Parse_tokens ----------
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
        else if (std::isdigit(static_cast<unsigned char>(code_text_[char_pos_]))) {
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
        case '*': type = TokenType::TOKEN_MUL;     lexeme = "*"; char_pos_++; break;
        case '/': type = TokenType::TOKEN_DIV;     lexeme = "/"; char_pos_++; break;
        case ',': type = TokenType::TOKEN_COMMA;   lexeme = ","; char_pos_++; break;
        case '%': type = TokenType::TOKEN_PERCENT; lexeme = "%"; char_pos_++; break;
        case '-':
            if(code_text_[char_pos_ + 1] == '-') {
                type = TokenType::TOKEN_DEC;
                lexeme = "--";
                char_pos_ += 2;
            }
            else {
                type = TokenType::TOKEN_SUB;
                lexeme = "-";
                char_pos_++;
            }
            break;
        case '+':
            if(code_text_[char_pos_ + 1] == '+') {
                type = TokenType::TOKEN_INC;
                lexeme = "++";
                char_pos_ += 2;
            }
            else {
                type = TokenType::TOKEN_ADD;
                lexeme = "+";
                char_pos_++;
            }
            break;
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
                type = TokenType::TOKEN_NOT;
                lexeme = "!";
                char_pos_++;
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
        case '|':
            if (code_text_[char_pos_ + 1] == '|') {
                type = TokenType::TOKEN_LOGICAL_OR;
                lexeme = "||";
                char_pos_ += 2;
            } else {
                report_error(
                    line_,
                    column_,
                    "Unexpected character. Did you mean '||'?",
                    std::string(1, code_text_[char_pos_])
                );
                char_pos_++;
                column_++;
                return;
            }
            break;
        case '&':
            if (code_text_[char_pos_ + 1] == '&') {
                type = TokenType::TOKEN_LOGICAL_AND;
                lexeme = "&&";
                char_pos_ += 2;
            } else {
                report_error(
                    line_,
                    column_,
                    "Unexpected character. Did you mean '&&'?",
                    std::string(1, code_text_[char_pos_])
                );
                char_pos_++;
                column_++;
                return;
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
            report_error(
                line_,
                column_,
                "Unexpected character!",
                std::string(1, code_text_[char_pos_])
            );
            char_pos_++;
            column_++;
            return;
    }

    token_list_.push_back({type, line_, column_, lexeme});
    column_ += static_cast<int>(lexeme.size());
}

void Lexer::parse_number() {
    std::size_t start = char_pos_;
    // Decimal integers
    while (std::isdigit(static_cast<unsigned char>(code_text_[char_pos_]))) {
        char_pos_++;
    }
    std::string lexeme = code_text_.substr(start, char_pos_ - start);
    token_list_.push_back({TokenType::TOKEN_DECIMAL_INT, line_, column_, lexeme});
    column_ += static_cast<int>(lexeme.size());
}

void Lexer::parse_string() {
    int start_line = line_;
    int start_column = column_;

    char_pos_++; // skip opening "
    column_++;

    std::size_t start = char_pos_;

    while (code_text_[char_pos_] != '"' &&
           code_text_[char_pos_] != '\0' &&
           code_text_[char_pos_] != '\n') {
        char_pos_++;
        column_++;
    }

    std::string lexeme = code_text_.substr(start, char_pos_ - start);

    if (code_text_[char_pos_] == '"') {
        char_pos_++;
        column_++;

        token_list_.push_back({
            TokenType::TOKEN_STRING,
            start_line,
            start_column,
            lexeme
        });
    } else {
        report_error(
            start_line,
            start_column,
            "Unterminated string literal",
            lexeme
        );
    }
}

void Lexer::parse_comment() {
    int start_line = line_;
    int start_column = column_;

    // Could be singleline or multiline comment
    if (code_text_[char_pos_ + 1] == '/') {
        char_pos_ += 2;
        column_ += 2;

        while (code_text_[char_pos_] != '\n' &&
               code_text_[char_pos_] != '\0') {
            char_pos_++;
            column_++;
        }
    }
    else if (code_text_[char_pos_ + 1] == '*') {
        char_pos_ += 2;
        column_ += 2;

        while (!(code_text_[char_pos_] == '*' &&
                 code_text_[char_pos_ + 1] == '/') &&
               code_text_[char_pos_] != '\0') {

            if (code_text_[char_pos_] == '\n') {
                line_++;
                column_ = 1;
            } else {
                column_++;
            }

            char_pos_++;
        }

        if (code_text_[char_pos_] == '\0') {
            report_error(
                start_line,
                start_column,
                "Unterminated multiline comment!",
                "/*"
            );
            return;
        }

        // skip closing */
        char_pos_ += 2;
        column_ += 2;
    }
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
