#include <lexer.h>

int main() {
    Lexer lexer("../../unittests/lexer/code_snippets/all_tokens.paracl");
    lexer.print_tokens();
    return 0;
}
