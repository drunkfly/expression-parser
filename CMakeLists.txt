cmake_minimum_required(VERSION 3.8)
project(Parser)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_library(Parser STATIC
    parser/common.cpp
    parser/common.h
    parser/lexer.cpp
    parser/lexer.h
    parser/parser_oop.cpp
    parser/parser_oop.h
    )

add_executable(ParserTest
    tests/common.cpp
    tests/common.h
    tests/test.cpp
    )

target_link_libraries(ParserTest Parser)