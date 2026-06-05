# Libraries
add_library(lexer STATIC ${CMAKE_CURRENT_SOURCE_DIR}/lexer/src/lexer.cpp)
target_include_directories(lexer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lexer/include)
target_link_libraries(lexer PUBLIC cli-table)

add_library(parser STATIC ${CMAKE_CURRENT_SOURCE_DIR}/parser/src/parser.cpp)
target_include_directories(parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/parser/include)
target_link_libraries(parser PUBLIC lexer)

add_library(paracl_codegen STATIC ${CMAKE_CURRENT_SOURCE_DIR}/codegen/src/codegen.cpp)
target_include_directories(paracl_codegen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/codegen/include)
target_link_libraries(paracl_codegen PUBLIC parser lexer)
