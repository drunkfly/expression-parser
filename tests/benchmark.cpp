/*
Copyright (c) 2023 Drunk Fly

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "tests/common.h"
#include "parser/parser_oop.h"
#include "parser/parser_lessoop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#error "Sorry, benchmark only supports Win32!"
#endif

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

static LARGE_INTEGER freq;

static double getTime()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (double)((long double)counter.QuadPart / (long double)freq.QuadPart);
}

static ParserOop::Expr* oopCompile(const char* input)
{
    try {
        MyResolver r;
        return ParserOop::Expr::parse(input, r);
    } catch (const ExprError& e) {
        printf("[ FAIL ] \"%s\" unexpected error: %s\n", input, e.message());
        exit(1);
    }
}

static ParserLessOop::Expr* lessOopCompile(const char* input)
{
    try {
        MyResolver r;
        return ParserLessOop::exprParse(input, r);
    } catch (const ExprError& e) {
        printf("[ FAIL ] \"%s\" unexpected error: %s\n", input, e.message());
        exit(1);
    }
}

static void benchmark(const char* input)
{
    const size_t ITER_COUNT = 10000000;
    MyEvaluator e;

    // Compile

    ParserOop::Expr* oopExpr = oopCompile(input);
    ParserLessOop::Expr* lessOopExpr = lessOopCompile(input);

    // ParserOop

    // Heat up caches, etc.
    for (size_t i = 0; i < ITER_COUNT; i++)
        oopExpr->evaluate(e);

    // Measure
    double oopStart = getTime();
    for (size_t i = 0; i < ITER_COUNT; i++)
        oopExpr->evaluate(e);
    double oopEnd = getTime();

    // ParserLessOop

    // Heat up caches, etc.
    for (size_t i = 0; i < ITER_COUNT; i++)
        ParserLessOop::exprEvaluate(lessOopExpr, e);

    // Measure
    double lessOopStart = getTime();
    for (size_t i = 0; i < ITER_COUNT; i++)
        ParserLessOop::exprEvaluate(lessOopExpr, e);
    double lessOopEnd = getTime();

    // Print results and cleanup

    printf("\"%s\": oop %.3f seconds, lessoop: %.3f seconds.\n",
        input, oopEnd - oopStart, lessOopEnd - lessOopStart);

    delete oopExpr;
    ParserLessOop::exprFree(lessOopExpr);
}

int main()
{
    QueryPerformanceFrequency(&freq);
    benchmark("4");
    benchmark("4 + fn1(8) * 19 - var.32");
}
