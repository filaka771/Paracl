#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/wait.h>

#include <gtest/gtest.h>

#include <lexer.h>
#include <parser.h>
#include <codegen.h>

namespace {

Parser::ParseResult parse_file(const std::string& path) {
    Lexer lexer(path);
    if (lexer.has_errors()) {
        throw std::runtime_error("Lexer failed for codegen test input");
    }

    Parser parser(lexer);
    return parser.release_parse_result();
}

std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string generate_asm(
    const std::string& snippet_path,
    const std::string& asm_path
) {
    Parser::ParseResult parse_result = parse_file(snippet_path);

    {
        CodeGen codegen(asm_path, std::move(parse_result));
    }

    return read_file(asm_path);
}

int assemble_and_run(
    const std::string& asm_path,
    const std::string& exe_path
) {
    const std::string build_cmd =
        "fasm " + asm_path + " " + exe_path + " >/tmp/paracl_codegen_test.log 2>&1";

    const int build_code = std::system(build_cmd.c_str());
    EXPECT_EQ(build_code, 0) << read_file("/tmp/paracl_codegen_test.log");

    if (build_code != 0) {
        return -1;
    }

    const int run_code = std::system(exe_path.c_str());

    if (WIFEXITED(run_code)) {
        return WEXITSTATUS(run_code);
    }

    return -1;
}

} // namespace

TEST(CodeGenTest, GeneratesExpectedAsmForFunctionCallSnippet) {
    const std::string asm_text = generate_asm(
        "../../unittests/codegen/code_snippets/basic.paracl",
        "/tmp/paracl_codegen_basic.asm"
    );

    const std::string ref_asm =
        "format ELF64 executable 3\n"
        "entry _start\n"
        "\n"
        "_start:\n"
        "call _int_main\n"
        "mov rdi, rax\n"
        "mov rax, 60\n"
        "syscall\n"
        "\n"
        "_int_foo:\n"
        "push rbp\n"
        "mov rbp, rsp\n"
        "mov rax, qword [rbp + 16]\n"
        "jmp _int_foo_end\n"
        "_int_foo_fallthrough:\n"
        "ud2\n"
        "_int_foo_end:\n"
        "mov rsp, rbp\n"
        "pop rbp\n"
        "ret\n"
        "_int_main:\n"
        "push rbp\n"
        "mov rbp, rsp\n"
        "mov rax, 1\n"
        "push rax\n"
        "call _int_foo\n"
        "add rsp, 8\n"
        "push rax\n"
        "mov rax, 2\n"
        "mov rbx, rax\n"
        "pop rax\n"
        "add rax, rbx\n"
        "jmp _int_main_end\n"
        "_int_main_fallthrough:\n"
        "ud2\n"
        "_int_main_end:\n"
        "mov rsp, rbp\n"
        "pop rbp\n"
        "ret\n";

    EXPECT_EQ(asm_text, ref_asm);
}

TEST(CodeGenTest, GeneratedProgramReturnsExpectedExitCode) {
    generate_asm(
        "../../unittests/codegen/code_snippets/basic.paracl",
        "/tmp/paracl_codegen_basic.asm"
    );

    const int exit_code = assemble_and_run(
        "/tmp/paracl_codegen_basic.asm",
        "/tmp/paracl_codegen_basic.out"
    );

    EXPECT_EQ(exit_code, 3);
}

TEST(CodeGenTest, IfElseReturnsExpectedBranch) {
    generate_asm(
        "../../unittests/codegen/code_snippets/if_else.paracl",
        "/tmp/paracl_codegen_if_else.asm"
    );

    const int exit_code = assemble_and_run(
        "/tmp/paracl_codegen_if_else.asm",
        "/tmp/paracl_codegen_if_else.out"
    );

    EXPECT_EQ(exit_code, 2);
}

TEST(CodeGenTest, ForLoopReturnsExpectedValue) {
    generate_asm(
        "../../unittests/codegen/code_snippets/for_loop.paracl",
        "/tmp/paracl_codegen_for_loop.asm"
    );

    const int exit_code = assemble_and_run(
        "/tmp/paracl_codegen_for_loop.asm",
        "/tmp/paracl_codegen_for_loop.out"
    );

    EXPECT_EQ(exit_code, 3);
}

TEST(CodeGenTest, PostfixExpressionReturnsOldValue) {
    generate_asm(
        "../../unittests/codegen/code_snippets/postfix_local.paracl",
        "/tmp/paracl_codegen_postfix.asm"
    );

    const int exit_code = assemble_and_run(
        "/tmp/paracl_codegen_postfix.asm",
        "/tmp/paracl_codegen_postfix.out"
    );

    EXPECT_EQ(exit_code, 1);
}

TEST(CodeGenTest, ThrowsOnUndefinedFunction) {
    EXPECT_THROW(
        {
            generate_asm(
                "../../unittests/codegen/code_snippets/undefined_function.paracl",
                "/tmp/paracl_codegen_invalid.asm"
            );
        },
        std::runtime_error
    );
}

TEST(CodeGenTest, ThrowsOnUndefinedIdentifier) {
    EXPECT_THROW(
        {
            generate_asm(
                "../../unittests/codegen/code_snippets/undefined_identifier.paracl",
                "/tmp/paracl_codegen_undefined_identifier.asm"
            );
        },
        std::runtime_error
    );
}

TEST(CodeGenTest, ThrowsOnWrongFunctionArgumentCount) {
    EXPECT_THROW(
        {
            generate_asm(
                "../../unittests/codegen/code_snippets/wrong_arg_count.paracl",
                "/tmp/paracl_codegen_wrong_arg_count.asm"
            );
        },
        std::runtime_error
    );
}
