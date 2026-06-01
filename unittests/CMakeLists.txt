add_executable(lexer_test lexer/src/lexer_test.cpp)
target_link_libraries(lexer_test PRIVATE lexer)

add_executable(parser_test parser/src/parser_test.cpp)
target_link_libraries(parser_test PRIVATE parser)

add_executable(codegen_test codegen/src/codegen_test.cpp)
target_link_libraries(codegen_test PRIVATE codegen)
