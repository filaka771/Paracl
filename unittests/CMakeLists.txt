add_executable(lexer_test lexer/src/lexer_test.cpp)
target_link_libraries(lexer_test PRIVATE lexer)

add_executable(parser_test parser/src/parser_test.cpp)
target_link_libraries(parser_test PRIVATE parser)
