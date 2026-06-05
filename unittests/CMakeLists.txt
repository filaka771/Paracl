find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(lexer_test lexer/src/lexer_test.cpp)
target_link_libraries(lexer_test PRIVATE lexer GTest::gtest_main)
gtest_discover_tests(lexer_test)

add_executable(parser_test parser/src/parser_test.cpp)
target_link_libraries(parser_test PRIVATE parser GTest::gtest_main)
gtest_discover_tests(parser_test)

add_executable(codegen_test codegen/src/codegen_test.cpp)
target_link_libraries(codegen_test PRIVATE paracl_codegen)
