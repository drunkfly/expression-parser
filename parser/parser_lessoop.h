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
#ifndef DRUNKFLY_PARSER_LESSOOP_H
#define DRUNKFLY_PARSER_LESSOOP_H

#include "parser/common.h"
#include "parser/resolve_oop.h"

namespace ParserLessOop
{

enum ExprOp
{
    OP_NUMBER,
    OP_CALLBACKVALUE,
    OP_BYTEVALUE,
    OP_WORDVALUE,
    OP_U24VALUE,
    OP_DWORDVALUE,
    OP_FUNC0,
    OP_FUNC1,
    OP_FUNC2,
    OP_FUNC3,
    OP_MEMBYTE,
    OP_MEMWORD,
    OP_MEMDWORD,
    OP_DOLLAR,
    OP_COND,
    OP_LOGICOR,
    OP_LOGICAND,
    OP_LOGICNOT,
    OP_BITOR,
    OP_BITAND,
    OP_BITXOR,
    OP_BITNOT,
    OP_EQUAL,
    OP_NOTEQUAL,
    OP_LESS,
    OP_LESSEQUAL,
    OP_GREATER,
    OP_GREATEREQUAL,
    OP_SHL,
    OP_SHR,
    OP_PLUS,
    OP_MINUS,
    OP_NEGATE,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_REMAINDER,
};

struct Expr
{
    ExprOp op;
    ExprValue number;
    ExprValuePtr valuePtr;
    ExprCallback0 cb0;
    ExprCallback1 cb1;
    ExprCallback2 cb2;
    ExprCallback3 cb3;
    Expr* op1;
    Expr* op2;
    Expr* op3;
};

Expr* exprParse(const char* input, ExprResolver& resolver);
ExprValue exprEvaluate(const Expr* expr, ExprEvaluator& eval);
void exprFree(Expr* expr);

} // namespace

#endif
