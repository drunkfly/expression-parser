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
#include <string.h>

static bool printPassed = true;
static int total;
static int passed;
static int failed;

static void check(const char* input, ExprValue expected)
{
    int result;
    bool success;

    // ParserOop

    ++total;

    try {
        MyResolver r;
        ParserOop::Expr* expr = ParserOop::Expr::parse(input, r);
        MyEvaluator e;
        result = expr->evaluate(e);
        success = true;
    } catch (const ExprError& e) {
        printf("[ FAIL ] ParserOop: \"%s\" unexpected error: %s\n", input, e.message());
        ++failed;
        success = false;
    }

    if (success) {
        if (result != expected) {
            printf("[ FAIL ] ParserOop: \"%s\" => result %ld != expected %ld\n", input, (long)result, (long)expected);
            ++failed;
        } else {
            if (printPassed)
                printf("[PASSED] ParserOop: \"%s\" => %ld\n", input, (long)result);
            ++passed;
        }
    }

    // ParserLessOop

    ++total;

    try {
        MyResolver r;
        ParserLessOop::Expr* expr = ParserLessOop::exprParse(input, r);
        MyEvaluator e;
        result = ParserLessOop::exprEvaluate(expr, e);
        success = true;
    } catch (const ExprError& e) {
        printf("[ FAIL ] ParserLessOop: \"%s\" unexpected error: %s\n", input, e.message());
        ++failed;
        success = false;
    }

    if (success) {
        if (result != expected) {
            printf("[ FAIL ] ParserLessOop: \"%s\" => result %ld != expected %ld\n", input, (long)result, (long)expected);
            ++failed;
        } else {
            if (printPassed)
                printf("[PASSED] ParserLessOop: \"%s\" => %ld\n", input, (long)result);
            ++passed;
        }
    }
}

static void checkError(const char* input, const char* message)
{
    int result;
    bool success;

    // ParserOop

    ++total;

    try {
        MyResolver r;
        ParserOop::Expr* expr = ParserOop::Expr::parse(input, r);
        MyEvaluator e;
        result = expr->evaluate(e);
        success = true;
    } catch (const ExprError& e) {
        if (!strcmp(e.message(), message)) {
            if (printPassed)
                printf("[PASSED] [ParserOop] \"%s\" => error %s\n", input, e.message());
            ++passed;
        } else {
            printf("[ FAIL ] [ParserOop] \"%s\" unexpected error: %s (was expecting: %s)\n", input, e.message(), message);
            ++failed;
        }
        success = false;
    }

    if (success) {
        printf("[ FAIL ] [ParserOop] \"%s\" => unexpected success (was expecting: %s)\n", input, message);
        ++failed;
    }

    // ParserLessOop

    ++total;

    try {
        MyResolver r;
        ParserLessOop::Expr* expr = ParserLessOop::exprParse(input, r);
        MyEvaluator e;
        result = ParserLessOop::exprEvaluate(expr, e);
        success = true;
    } catch (const ExprError& e) {
        if (!strcmp(e.message(), message)) {
            if (printPassed)
                printf("[PASSED] [ParserLessOop] \"%s\" => error %s\n", input, e.message());
            ++passed;
        } else {
            printf("[ FAIL ] [ParserLessOop] \"%s\" unexpected error: %s (was expecting: %s)\n", input, e.message(), message);
            ++failed;
        }
        success = false;
    }

    if (success) {
        printf("[ FAIL ] [ParserLessOop] \"%s\" => unexpected success (was expecting: %s)\n", input, message);
        ++failed;
    }
}

int main()
{
    check("0", 0);
    check(" 0", 0);
    check("32768", 32768);
    check("1 ", 1);
    check("0xf", 15);
    check("0x1234", 0x1234);
    check("0b0001010", 10);
    check("0b0", 0);
    check("0b1", 1);
    check("0b11111111111111111111111111111111", 0xffffffff);
    check("0o777", 0777);
    check("0o345", 0345);
    check("0o0", 0);
    check("$1234", 0x1234);
    check("#1234", 0x1234);
    check("$", PC_VALUE);
    check("\t4+   48", 4+48);
    check("8 + 16-3", 8+16-3);
    check("9*4", 9*4);
    check("9*4+3", 9*4+3);
    check("9+4 * 3 ", 9+4*3);
    check(" 2 + 2 * 2 ", 2+2*2);
    check("(  2 + 2) * 2 ", (2+2)*2);
    check("1234/9", 1234/9);
    check("1234 % 9", 1234%9);
    check("3 - (8 * (14 - 9) + 12 + 4) / 7 - 9", 3 - (8 * (14 - 9) + 12 + 4) / 7 - 9);
    check("1?2:3", 2);
    check("1?2+4:3+8", 2+4);
    check("0?2+4:3+8", 3+8);
    check("0?1?4:8:3", 3);
    check("1?1?4:8:3", 4);
    check("1?0?4:8:3", 8);
    check("0x10^0x01", 0x11);
    check("1 ^ 0x01", 0);
    check("3&&4", 1);
    check("0&&4", 0);
    check("1&&1||1", 1&&1||1);
    check("0&&1||1", 0&&1||1);
    check("1&&0||1", 1&&0||1);
    check("1&&1||0", 1&&1||0);
    check("0&&0||1", 0&&0||1);
    check("1&&0||0", 1&&0||0);
    check("0&&1||0", 0&&1||0);
    check("0&&0||0", 0&&0||0);
    check("1&1|1", 1&1|1);
    check("0&1|1", 0&1|1);
    check("1&0|1", 1&0|1);
    check("1&1|0", 1&1|0);
    check("0&0|1", 0&0|1);
    check("1&0|0", 1&0|0);
    check("0&1|0", 0&1|0);
    check("0&0|0", 0&0|0);
    check("0xffff & 0xaaaa", 0xaaaa);
    check("0x5555 | 0xaaaa", 0xffff);
    check("4<5=5<4", 4<5==5<4);
    check("4<5==5<4", 4<5==5<4);
    check("4<5=5>4", 4<5==5>4);
    check("4<5==5>4", 4<5==5>4);
    check("4<5=4>5", 4<5==4>5);
    check("4<5==4>5", 4<5==4>5);
    check("4<5=5>4", 4<5==5>4);
    check("4<5==5>4", 4<5==5>4);
    check("4<=5", 4<=5);
    check("4<=4", 4<=4);
    check("4<=3", 4<=3);
    check("4>=5", 4>=5);
    check("4>=4", 4>=4);
    check("4>=3", 4>=3);
    check("4==3", 4==3);
    check("4==4", 4==4);
    check("1<<3", 1<<3);
    check("0x8000>>5", 0x8000>>5);
    check("- 5", -5);
    check("-12", -12);
    check(" --3", 3);
    check(" ~0x0f", ~0x0f);
    check(" !0x0f", 0);
    check(" !0", 1);
    check(" !!0", 0);
    check(" !(!0)", 0);
    check(" -(3)", -3);
    check("-( - 3 )", 3);
    check("var.8", 0xda);
    check("var.8.1", 0xca);
    check("var.16", 0xcada);
    check("var.16.1", 0x0aba);
    check("var.24", 0xbacada);
    check("var.32", 0x0abacada);
    check("varFn", 0xb0b0);
    check("varFn+var.32", 0xb0b0+0x0abacada);
    check("fn0()", 0x7777);
    check("fn1(0x1111)", 0x8888 + 0x1111);
    check("fn2(0x1111, 0x2222)", 0x9999 + (0x1111 * 0x2222));
    check("fn3(0x1111,0x2222,0x3333)", 0xaaaa + (0x1111 * 0x2222 - 0x3333));
    check("varFn+var.32==0xb0b0+0x0abacada", 1);
    check("varFn + var.32 != 0xb0b0 + 0x0abacada", 0);
    check("af'", 0xbccb);
    check("(af')", 0xbccb);
    check("[0xaaaa]", 0xaa+0x10);
    check("b@[0xaaba]", 0xba+0x10);
    check("w@ [0xbcbb]", 0xbcbb-0xb0);
    check("d@ [0x1515]", 0x1515*4);
    check("d @[4+fn3(0x1111,0x2222,0x3333)]", (4+(0xaaaa + (0x1111 * 0x2222 - 0x3333)))*4);

    checkError("", "syntax error in expression.");
    checkError("8 9", "syntax error in expression.");
    checkError("9c", "syntax error in number.");
    checkError("9.", "syntax error in number.");
    checkError("9.0", "syntax error in number.");
    checkError("9_", "syntax error in number.");
    checkError("0x", "syntax error in hexadecimal number.");
    checkError("0xgaf", "syntax error in hexadecimal number.");
    checkError("0xafg", "syntax error in hexadecimal number.");
    checkError("0xaf_", "syntax error in hexadecimal number.");
    checkError("0xaf.", "syntax error in hexadecimal number.");
    checkError("0b", "syntax error in binary number.");
    checkError("0b013", "syntax error in binary number.");
    checkError("0b0.", "syntax error in binary number.");
    checkError("0b0_", "syntax error in binary number.");
    checkError("0b3", "syntax error in binary number.");
    checkError("0bc", "syntax error in binary number.");
    checkError("0o8", "syntax error in octal number.");
    checkError("0o358", "syntax error in octal number.");
    checkError("0o3.", "syntax error in octal number.");
    checkError("0o3_", "syntax error in octal number.");
    checkError("07", "numbers starting with '0' are not supported, use '0o' prefix for octal numbers.");
    checkError("8+", "syntax error in expression.");
    checkError("-", "syntax error in expression.");
    checkError("~", "syntax error in expression.");
    checkError("!", "syntax error in expression.");
    checkError("3-", "syntax error in expression.");
    checkError("(4", "missing ')'.");
    checkError("(", "syntax error in expression.");
    checkError("()", "syntax error in expression.");
    checkError(")", "syntax error in expression.");
    checkError("12+4)", "syntax error in expression.");
    checkError("1?", "syntax error in expression.");
    checkError("1?2", "missing ':'.");
    checkError("1?2:", "syntax error in expression.");
    checkError("?2:3", "syntax error in expression.");
    checkError("$$", "syntax error in expression.");
    checkError("(~)8", "syntax error in expression.");
    checkError("var. 32", "unknown identifier 'var.'.");
    checkError("var .32", "unknown identifier 'var'.");
    checkError("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwx", "unknown identifier 'abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwx'.");
    checkError("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxy", "identifier too long.");
    checkError("fn0", "unknown identifier 'fn0'.");
    checkError("fn0(1)", "invalid number of arguments for function 'fn0' (expected 0, got 1).");
    checkError("fn1(1,2)", "invalid number of arguments for function 'fn1' (expected 1, got 2).");
    checkError("fn2(1)", "invalid number of arguments for function 'fn2' (expected 2, got 1).");
    checkError("fn3(1,5)", "invalid number of arguments for function 'fn3' (expected 3, got 2).");
    checkError("fn3(1,5,7,8)", "too many arguments for function 'fn3' (expected 3).");
    checkError("fn1(1", "missing ','.");
    checkError("fn2(1 1)", "missing ','.");
    checkError("varFn()", "unknown function 'varFn'.");
    checkError("bc'", "unknown identifier 'bc''.");
    checkError("b@[", "syntax error in expression.");
    checkError("b@[0x1515", "missing ']'.");
    checkError("b@", "missing '[' after '@'.");
    checkError("q@[0x1515]", "unknown data type 'q'.");
    checkError("fn0@[0x1515]", "unknown data type 'fn0'.");
    checkError("var.8@[0x1515]", "unknown data type 'var.8'.");
    checkError("#", "syntax error in expression.");
    checkError("#q", "syntax error in expression.");
    checkError("$q", "syntax error in expression.");
    checkError("2/0", "division by zero.");
    checkError("3 % (1 - 1)", "division by zero.");

    printf("----------\n");
    if (failed)
        printf("ERROR! %d total, %d passed, %d failed\n", total, passed, failed);
    else
        printf("SUCCESS! %d total\n", total);

    return 0;
}
