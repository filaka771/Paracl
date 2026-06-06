#include <fstream>
#include <lexer.h>

int main() {
    std::ofstream ref_file("ref_token_lit.txt");
    Lexer lexer("../../unittests/lexer/code_snippets/all_tokens.paracl");
    const auto& token_list = lexer.get_token_list();
    ref_file << "{";
    for(auto iter = token_list.begin(); iter < token_list.end(); iter ++) {
        auto token = *iter;
        ref_file << "{";
        switch(token.type) {
            case Lexer::TokenType::TOKEN_INT:
                ref_file << "Lexer::TokenType::TOKEN_INT";
                break;

            case Lexer::TokenType::TOKEN_FOR:
                ref_file << "Lexer::TokenType::TOKEN_FOR";
                break;

            case Lexer::TokenType::TOKEN_IF:
                ref_file << "Lexer::TokenType::TOKEN_IF";
                break;

            case Lexer::TokenType::TOKEN_ELSE:
                ref_file << "Lexer::TokenType::TOKEN_ELSE";
                break;

            case Lexer::TokenType::TOKEN_CONTINUE:
                ref_file << "Lexer::TokenType::TOKEN_CONTINUE";
                break;

            case Lexer::TokenType::TOKEN_BREAK:
                ref_file << "Lexer::TokenType::TOKEN_BREAK";
                break;

            case Lexer::TokenType::TOKEN_RETURN:
                ref_file << "Lexer::TokenType::TOKEN_RETURN";
                break;

            case Lexer::TokenType::TOKEN_IDENT:
                ref_file << "Lexer::TokenType::TOKEN_IDENT";
                break;

            case Lexer::TokenType::TOKEN_ADD:
                ref_file << "Lexer::TokenType::TOKEN_ADD";
                break;

            case Lexer::TokenType::TOKEN_SUB:
                ref_file << "Lexer::TokenType::TOKEN_SUB";
                break;

            case Lexer::TokenType::TOKEN_MUL:
                ref_file << "Lexer::TokenType::TOKEN_MUL";
                break;

            case Lexer::TokenType::TOKEN_DIV:
                ref_file << "Lexer::TokenType::TOKEN_DIV";
                break;

            case Lexer::TokenType::TOKEN_PERCENT:
                ref_file << "Lexer::TokenType::TOKEN_PERCENT";
                break;

            case Lexer::TokenType::TOKEN_EQUAL:
                ref_file << "Lexer::TokenType::TOKEN_EQUAL";
                break;

            case Lexer::TokenType::TOKEN_NEQUAL:
                ref_file << "Lexer::TokenType::TOKEN_NEQUAL";
                break;

            case Lexer::TokenType::TOKEN_ASSIGN:
                ref_file << "Lexer::TokenType::TOKEN_ASSIGN";
                break;

            case Lexer::TokenType::TOKEN_GREATER_OR_EQ:
                ref_file << "Lexer::TokenType::TOKEN_GREATER_OR_EQ";
                break;

            case Lexer::TokenType::TOKEN_GREATER:
                ref_file << "Lexer::TokenType::TOKEN_GREATER";
                break;

            case Lexer::TokenType::TOKEN_LESS_OR_EQ:
                ref_file << "Lexer::TokenType::TOKEN_LESS_OR_EQ";
                break;

            case Lexer::TokenType::TOKEN_LESS:
                ref_file << "Lexer::TokenType::TOKEN_LESS";
                break;

            case Lexer::TokenType::TOKEN_INC:
                ref_file << "Lexer::TokenType::TOKEN_INC";
                break;

            case Lexer::TokenType::TOKEN_DEC:
                ref_file << "Lexer::TokenType::TOKEN_DEC";
                break;

            case Lexer::TokenType::TOKEN_LOGICAL_AND:
                ref_file << "Lexer::TokenType::TOKEN_LOGICAL_AND";
                break;

            case Lexer::TokenType::TOKEN_LOGICAL_OR:
                ref_file << "Lexer::TokenType::TOKEN_LOGICAL_OR";
                break;

            case Lexer::TokenType::TOKEN_NOT:
                ref_file << "Lexer::TokenType::TOKEN_NOT";
                break;

            case Lexer::TokenType::TOKEN_L_PAREN:
                ref_file << "Lexer::TokenType::TOKEN_L_PAREN";
                break;

            case Lexer::TokenType::TOKEN_R_PAREN:
                ref_file << "Lexer::TokenType::TOKEN_R_PAREN";
                break;

            case Lexer::TokenType::TOKEN_L_BRACE:
                ref_file << "Lexer::TokenType::TOKEN_L_BRACE";
                break;

            case Lexer::TokenType::TOKEN_R_BRACE:
                ref_file << "Lexer::TokenType::TOKEN_R_BRACE";
                break;

            case Lexer::TokenType::TOKEN_L_BRACKET:
                ref_file << "Lexer::TokenType::TOKEN_L_BRACKET";
                break;

            case Lexer::TokenType::TOKEN_R_BRACKET:
                ref_file << "Lexer::TokenType::TOKEN_R_BRACKET";
                break;

            case Lexer::TokenType::TOKEN_SEMICOLON:
                ref_file << "Lexer::TokenType::TOKEN_SEMICOLON";
                break;

            case Lexer::TokenType::TOKEN_COMMA:
                ref_file << "Lexer::TokenType::TOKEN_COMMA";
                break;

            case Lexer::TokenType::TOKEN_STRING:
                ref_file << "Lexer::TokenType::TOKEN_STRING";
                break;

            case Lexer::TokenType::TOKEN_DECIMAL_INT:
                ref_file << "Lexer::TokenType::TOKEN_DECIMAL_INT";
                break;

            case Lexer::TokenType::TOKEN_SPACE:
                ref_file << "Lexer::TokenType::TOKEN_SPACE";
                break;

            case Lexer::TokenType::TOKEN_NEWLINE:
                ref_file << "Lexer::TokenType::TOKEN_NEWLINE";
                break;

            case Lexer::TokenType::TOKEN_TAB:
                ref_file << "Lexer::TokenType::TOKEN_TAB";
                break;

            case Lexer::TokenType::TOKEN_FILE_END:
                ref_file << "Lexer::TokenType::TOKEN_FILE_END";
                break;

            default:
                break;
        }

        ref_file << ", " << token.line << ", " << token.column << ", \"" << token.lexeme << "\"}";
        if(iter == std::prev(token_list.end()))
            ref_file << "}";
        else
            ref_file << ", ";
    }
    return 0;
}
