#include <memory>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include <lexer.h>
#include <parser.h>

namespace {

Parser::ParseResult parse_file(const std::string& path) {
    Lexer lexer(path);
    if (lexer.has_errors()) {
        throw std::runtime_error("Lexer failed for parser test input");
    }

    Parser parser(lexer);
    return parser.release_parse_result();
}

const Node& child(const Node& node, std::size_t index) {
    return *node.children_nodes.at(index);
}

void expect_kind(const Node& node, NodeKind kind) {
    EXPECT_EQ(node.node_kind, kind);
}

} // namespace

TEST(ParserTest, ParsesBasicFunctionStructure) {
    auto parse_result =
        parse_file("../../unittests/parser/code_snippets/basic.paracl");

    ASSERT_NE(parse_result.program, nullptr);

    const TranslationUnitDecl& program = *parse_result.program;
    expect_kind(program, NodeKind::TranslationUnitDecl);
    ASSERT_EQ(program.children_nodes.size(), 1);

    const auto* function_def =
        static_cast<const FunctionDecl*>(program.children_nodes[0].get());

    EXPECT_EQ(function_def->get_function_id(), "function");
    ASSERT_EQ(function_def->children_nodes.size(), 2);
    expect_kind(child(*function_def, 0), NodeKind::ParameterList);
    expect_kind(child(*function_def, 1), NodeKind::CompoundStmt);

    const Node& parameter_list = child(*function_def, 0);
    ASSERT_EQ(parameter_list.children_nodes.size(), 1);
    expect_kind(child(parameter_list, 0), NodeKind::ParmVarDecl);

    const Node& compound_stmt = child(*function_def, 1);
    ASSERT_EQ(compound_stmt.children_nodes.size(), 6);
    expect_kind(child(compound_stmt, 0), NodeKind::ExprStmt);
    expect_kind(child(compound_stmt, 1), NodeKind::ExprStmt);
    expect_kind(child(compound_stmt, 2), NodeKind::ForStmt);
    expect_kind(child(compound_stmt, 3), NodeKind::IfStmt);
    expect_kind(child(compound_stmt, 4), NodeKind::ExprStmt);
    expect_kind(child(compound_stmt, 5), NodeKind::ReturnStmt);
}

TEST(ParserTest, ParsesForAndIfElseShape) {
    auto parse_result =
        parse_file("../../unittests/parser/code_snippets/basic.paracl");

    const TranslationUnitDecl& program = *parse_result.program;
    const Node& function_def = child(program, 0);
    const Node& compound_stmt = child(function_def, 1);

    const auto* for_stmt =
        static_cast<const ForStmt*>(compound_stmt.children_nodes[2].get());
    EXPECT_TRUE(for_stmt->has_init());
    EXPECT_TRUE(for_stmt->has_cond());
    EXPECT_TRUE(for_stmt->has_step());
    ASSERT_EQ(for_stmt->children_nodes.size(), 4);
    expect_kind(child(*for_stmt, 0), NodeKind::AssignmentExpr);
    expect_kind(child(*for_stmt, 1), NodeKind::BinaryOperator);
    expect_kind(child(*for_stmt, 2), NodeKind::PostfixExpr);
    expect_kind(child(*for_stmt, 3), NodeKind::CompoundStmt);

    const Node& if_stmt = child(compound_stmt, 3);
    ASSERT_EQ(if_stmt.children_nodes.size(), 3);
    expect_kind(child(if_stmt, 0), NodeKind::BinaryOperator);
    expect_kind(child(if_stmt, 1), NodeKind::CompoundStmt);
    expect_kind(child(if_stmt, 2), NodeKind::CompoundStmt);
}

TEST(ParserTest, ParsesBinaryPrecedenceCorrectly) {
    auto parse_result =
        parse_file("../../unittests/parser/code_snippets/precedence.paracl");

    const TranslationUnitDecl& program = *parse_result.program;
    const Node& function_def = child(program, 0);
    const Node& compound_stmt = child(function_def, 1);
    const Node& return_stmt = child(compound_stmt, 0);

    ASSERT_EQ(return_stmt.children_nodes.size(), 1);
    const Node& expr = child(return_stmt, 0);

    expect_kind(expr, NodeKind::BinaryOperator);
    EXPECT_EQ(expr.node_name, "AdditiveExpr");

    const Node& left = child(expr, 0);
    const Node& right = child(expr, 1);

    expect_kind(left, NodeKind::IntegerLiteral);
    expect_kind(right, NodeKind::BinaryOperator);
    EXPECT_EQ(right.node_name, "MultiplicativeExpr");
}

TEST(ParserTest, FunctionSpanCoversFullDefinition) {
    auto parse_result =
        parse_file("../../unittests/parser/code_snippets/basic.paracl");

    const TranslationUnitDecl& program = *parse_result.program;
    const Node& function_def = child(program, 0);

    ASSERT_LT(function_def.span_.begin, parse_result.token_list.size());
    ASSERT_LT(function_def.span_.end, parse_result.token_list.size());

    EXPECT_EQ(parse_result.token_list[function_def.span_.begin].lexeme, "int");
    EXPECT_EQ(parse_result.token_list[function_def.span_.end].lexeme, "}");
}

TEST(ParserTest, RejectsElseWithoutIf) {
    Lexer lexer("../../unittests/parser/code_snippets/invalid_else.paracl");
    ASSERT_FALSE(lexer.has_errors());

    EXPECT_THROW(
        {
            Parser parser(lexer);
        },
        std::runtime_error
    );
}

TEST(ParserTest, RejectsMissingSemicolon) {
    Lexer lexer("../../unittests/parser/code_snippets/missing_semicolon.paracl");
    ASSERT_FALSE(lexer.has_errors());

    EXPECT_THROW(
        {
            Parser parser(lexer);
        },
        std::runtime_error
    );
}
