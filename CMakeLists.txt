cmake_minimum_required(VERSION 3.8)
project(Parser)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_library(Parser STATIC
    parser/common.cpp
    parser/common.h
    parser/lexer.cpp
    parser/lexer.h
    parser/parser_lessoop.cpp
    parser/parser_lessoop.h
    parser/parser_oop.cpp
    parser/parser_oop.h
    parser/resolve_oop.h
    )

add_executable(ParserTest
    tests/common.cpp
    tests/common.h
    tests/test.cpp
    )

target_link_libraries(ParserTest Parser)

add_executable(ParserBenchmark
    tests/tinyexpr/tinyexpr.c
    tests/tinyexpr/tinyexpr.h
    tests/common.cpp
    tests/common.h
    tests/benchmark.cpp
    )

target_link_libraries(ParserBenchmark Parser)
