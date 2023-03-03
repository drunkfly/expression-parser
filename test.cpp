#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static bool printPassed = true;
static int total;
static int passed;
static int failed;

#define PC_VALUE 0xcafebabe

static Expr::Value fn0()
{
    return 0x7777;
}

static Expr::Value fn1(Expr::Value v1)
{
    return 0x8888 + v1;
}

static Expr::Value fn2(Expr::Value v1, Expr::Value v2)
{
    return 0x9999 + v1 * v2;
}

static Expr::Value fn3(Expr::Value v1, Expr::Value v2, Expr::Value v3)
{
    return 0xaaaa + (v1 * v2 - v3);
}

static Expr::Value readVar()
{
    return 0xb0b0;
}

static Expr::Value readAF()
{
    return 0xbccb;
}

static const uint32_t value32 = 0x0abacada;

class MyResolver : public Expr::Resolver
{
public:
    Expr::Callback0 resolveFunc0(const char* name)
    {
        if (!strcmp(name, "fn0"))
            return fn0;
        return NULL;
    }

    Expr::Callback1 resolveFunc1(const char* name)
    {
        if (!strcmp(name, "fn1"))
            return fn1;
        return NULL;
    }

    Expr::Callback2 resolveFunc2(const char* name)
    {
        if (!strcmp(name, "fn2"))
            return fn2;
        return NULL;
    }

    Expr::Callback3 resolveFunc3(const char* name)
    {
        if (!strcmp(name, "fn3"))
            return fn3;
        return NULL;
    }

    bool resolveVariable(const char* name, Expr::ValuePtr& result)
    {
        if (!strcmp(name, "var.8")) {
            result.ptr = &value32;
            result.sizeInBytes = 1;
            return true;
        }
        if (!strcmp(name, "var.8.1")) {
            result.ptr = (uint8_t*)&value32 + 1;
            result.sizeInBytes = 1;
            return true;
        }
        if (!strcmp(name, "var.16")) {
            result.ptr = &value32;
            result.sizeInBytes = 2;
            return true;
        }
        if (!strcmp(name, "var.16.1")) {
            result.ptr = (uint8_t*)&value32 + 2;
            result.sizeInBytes = 2;
            return true;
        }
        if (!strcmp(name, "var.24")) {
            result.ptr = &value32;
            result.sizeInBytes = 3;
            return true;
        }
        if (!strcmp(name, "var.32")) {
            result.ptr = &value32;
            result.sizeInBytes = 4;
            return true;
        }
        if (!strcmp(name, "varFn")) {
            result.readValue = readVar;
            return true;
        }
        if (!strcmp(name, "af'")) {
            result.readValue = readAF;
            return true;
        }
        return false;
    }
};

class MyEvaluator : public Expr::Evaluator
{
public:
    MyEvaluator() : Evaluator(PC_VALUE) {}

    uint8_t memByte(Expr::Value address) const { return address + 0x10; }
    uint16_t memWord(Expr::Value address) const { return address - 0xb0; }
    uint32_t memDword(Expr::Value address) const { return address * 4; }
};

static void check(const char* input, Expr::Value expected)
{
    int result;
    ++total;

    try {
        MyResolver r;
        Expr* expr = Expr::parse(input, r);
        MyEvaluator e;
        result = expr->evaluate(e);
    } catch (const Expr::Error& e) {
        printf("[ FAIL ] \"%s\" unexpected error: %s\n", input, e.message());
        ++failed;
        return;
    }

    if (result != expected) {
        printf("[ FAIL ] \"%s\" => result %ld != expected %ld\n", input, (long)result, (long)expected);
        ++failed;
    } else {
        if (printPassed)
            printf("[PASSED] \"%s\" => %ld\n", input, (long)result);
        ++passed;
    }
}

static void checkError(const char* input, const char* message)
{
    int result;
    ++total;

    try {
        MyResolver r;
        Expr* expr = Expr::parse(input, r);
        MyEvaluator e;
        result = expr->evaluate(e);
    } catch (const Expr::Error& e) {
        if (!strcmp(e.message(), message)) {
            if (printPassed)
                printf("[PASSED] \"%s\" => error %s\n", input, e.message());
            ++passed;
        } else {
            printf("[ FAIL ] \"%s\" unexpected error: %s (was expecting: %s)\n", input, e.message(), message);
            ++failed;
        }
        return;
    }

    printf("[ FAIL ] \"%s\" => unexpected success (was expecting: %s)\n", input, message);
    ++failed;
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
    checkError("fn0(1)", "invalid number of arguments for function 'fn0'.");
    checkError("fn1(1,2)", "invalid number of arguments for function 'fn1'.");
    checkError("fn2(1)", "invalid number of arguments for function 'fn2'.");
    checkError("fn3(1,5)", "invalid number of arguments for function 'fn3'.");
    checkError("fn3(1,5,7,8)", "too many function arguments.");
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

    printf("----------\n");
    if (failed)
        printf("ERROR! %d total, %d passed, %d failed\n", total, passed, failed);
    else
        printf("SUCCESS! %d total\n", total);

    return 0;
}
