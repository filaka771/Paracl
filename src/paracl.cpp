#include <string>
#include <iostream>
#include <stdexcept>

#include "lexer.h"
#include "parser.h"
#include "codegen.h"

void print_usage(const char* prog_name) {
    std::cerr << "Usage:\n"
              << "  " << prog_name << " -t <source.paracl>\n"
              << "  " << prog_name << " -a <source.paracl>\n"
              << "  " << prog_name << " -c <source.paracl> <out.asm>\n";
}

void print_usage_error(const char* prog_name, const std::string& message) {
    std::cerr << "Error: " << message << "\n\n";
    print_usage(prog_name);
}

bool verify_file_extension(
    const std::string& path,
    const std::string& expected_extension
) {
    const std::size_t extension_pos = path.rfind('.');
    if (extension_pos == std::string::npos) {
        return false;
    }

    return path.substr(extension_pos) == expected_extension;
}

char parse_mode_flag(const char* arg) {
    const std::string flag(arg);
    if (flag.size() != 2 || flag[0] != '-') {
        return '\0';
    }

    return flag[1];
}

int main(int argc, char* argv[]) {
    if (argc != 3 && argc != 4) {
        print_usage_error(argv[0], "invalid arguments");
        return 1;
    }

    const char flag = parse_mode_flag(argv[1]);
    if (flag != 't' && flag != 'a' && flag != 'c') {
        print_usage_error(argv[0], "unknown option");
        return 1;
    }

    const std::string source_file = argv[2];
    if (!verify_file_extension(source_file, ".paracl")) {
        print_usage_error(argv[0], "source file must have .paracl extension");
        return 1;
    }

    if (flag == 'c') {
        if (argc != 4) {
            print_usage_error(argv[0], "-c expects source file and output file");
            return 1;
        }

        if (!verify_file_extension(argv[3], ".asm")) {
            print_usage_error(argv[0], "output file must have .asm extension");
            return 1;
        }
    }
    else if (argc != 3) {
        print_usage_error(argv[0], "invalid arguments");
        return 1;
    }

    try {
        Lexer lexer(source_file);

        if (lexer.has_errors()) {
            for (const auto& diagnostic : lexer.get_diagnostics()) {
                std::cerr << diagnostic.line
                          << ":"
                          << diagnostic.column
                          << " "
                          << diagnostic.message
                          << "\n";
            }

            return 1;
        }

        if (flag == 't') {
            lexer.print_tokens();
            return 0;
        }

        Parser parser(lexer);

        if (flag == 'a') {
            parser.print_ast();
            return 0;
        }

        Parser::ParseResult ast = parser.release_parse_result();
        const std::string out_file = argv[3];
        CodeGen codegen(out_file, std::move(ast));
        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
        return 1;
    }
}
