add_executable(paracl ./paracl.cpp)
target_link_libraries(paracl PRIVATE lexer parser paracl_codegen)
