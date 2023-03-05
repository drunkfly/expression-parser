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
#include "parser/parser_lessoop.h"
#include "parser/lexer.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

namespace ParserLessOop
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define EVAL(expr) (exprEvaluate(expr, eval))

ExprValue exprEvaluate(const Expr* expr, ExprEvaluator& eval)
{
    switch (expr->op) {
        case OP_NUMBER: return expr->number;
        case OP_CALLBACKVALUE: return expr->valuePtr.readValue();
        case OP_BYTEVALUE: return *(uint8_t*)expr->valuePtr.ptr;
        case OP_WORDVALUE: return *(uint16_t*)expr->valuePtr.ptr;
        case OP_U24VALUE: return *(uint16_t*)expr->valuePtr.ptr | (*((uint8_t*)expr->valuePtr.ptr + 2) << 16);
        case OP_DWORDVALUE: return *(uint32_t*)expr->valuePtr.ptr;
        case OP_FUNC0: return expr->cb0();
        case OP_FUNC1: return expr->cb1(EVAL(expr->op1));
        case OP_FUNC2: return expr->cb2(EVAL(expr->op1), EVAL(expr->op2));
        case OP_FUNC3: return expr->cb3(EVAL(expr->op1), EVAL(expr->op2), EVAL(expr->op3));
        case OP_MEMBYTE: return eval.memByte(EVAL(expr->op1));
        case OP_MEMWORD: return eval.memWord(EVAL(expr->op1));
        case OP_MEMDWORD: return eval.memDword(EVAL(expr->op1));
        case OP_DOLLAR: return eval.pc();
        case OP_COND: return (EVAL(expr->op1) ? EVAL(expr->op2) : EVAL(expr->op3));
        case OP_LOGICOR: return EVAL(expr->op1) || EVAL(expr->op2);
        case OP_LOGICAND: return EVAL(expr->op1) && EVAL(expr->op2);
        case OP_LOGICNOT: return !EVAL(expr->op1);
        case OP_BITOR: return EVAL(expr->op1) | EVAL(expr->op2);
        case OP_BITAND: return EVAL(expr->op1) & EVAL(expr->op2);
        case OP_BITXOR: return EVAL(expr->op1) ^ EVAL(expr->op2);
        case OP_BITNOT: return ~EVAL(expr->op1);
        case OP_EQUAL: return EVAL(expr->op1) == EVAL(expr->op2);
        case OP_NOTEQUAL: return EVAL(expr->op1) != EVAL(expr->op2);
        case OP_LESS: return EVAL(expr->op1) < EVAL(expr->op2);
        case OP_LESSEQUAL: return EVAL(expr->op1) <= EVAL(expr->op2);
        case OP_GREATER: return EVAL(expr->op1) > EVAL(expr->op2);
        case OP_GREATEREQUAL: return EVAL(expr->op1) >= EVAL(expr->op2);
        case OP_SHL: return EVAL(expr->op1) << EVAL(expr->op2);
        case OP_SHR: return EVAL(expr->op1) >> EVAL(expr->op2);
        case OP_PLUS: return EVAL(expr->op1) + EVAL(expr->op2);
        case OP_MINUS: return EVAL(expr->op1) - EVAL(expr->op2);
        case OP_NEGATE: return -EVAL(expr->op1);
        case OP_MULTIPLY: return EVAL(expr->op1) * EVAL(expr->op2);
        case OP_DIVIDE: { int d = EVAL(expr->op2); if (d == 0) throw ExprError("division by zero."); return EVAL(expr->op1) / d; }
        case OP_REMAINDER: { int d = EVAL(expr->op2); if (d == 0) throw ExprError("division by zero."); return EVAL(expr->op1) % d; }
        default: throw ExprError("internal error.");
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Parser

struct Context
{
    ExprToken* curToken;
    ExprResolver* resolver;
};

static Expr* expression(Context* c);

static Expr* primaryExpression(Context* c)
{
    const char* type;
    Expr* result;

    switch (c->curToken->id) {
        case TOK_LPAREN:
            c->curToken = c->curToken->next;
            result = expression(c);
            if (c->curToken->id != TOK_RPAREN)
                throw ExprError("missing ')'.");
            c->curToken = c->curToken->next;
            return result;

        case TOK_DOLLAR:
            result = new Expr;
            result->op = OP_DOLLAR;
            c->curToken = c->curToken->next;
            return result;

        case TOK_NUMBER:
            result = new Expr;
            result->op = OP_NUMBER;
            result->number = c->curToken->number;
            c->curToken = c->curToken->next;
            return result;

        case TOK_LBRACKET:
            type = "b";
          mem:
            c->curToken = c->curToken->next;
            /*
            if (c->curToken->id == TOK_IDENT) {
                type = c->curToken->text;
                c->curToken = c->curToken->next;
                if (c->curToken->id != TOK_COLON)
                    throw ExprError("missing ':'.");
                c->curToken = c->curToken->next;
            }
            */
            result = expression(c);
            if (c->curToken->id != TOK_RBRACKET)
                throw ExprError("missing ']'.");
            c->curToken = c->curToken->next;
            if (!strcmp(type, "b")) {
                Expr* expr = new Expr;
                expr->op = OP_MEMBYTE;
                expr->op1 = result;
                return expr;
            } else if (!strcmp(type, "w")) {
                Expr* expr = new Expr;
                expr->op = OP_MEMWORD;
                expr->op1 = result;
                return expr;
            } else if (!strcmp(type, "d")) {
                Expr* expr = new Expr;
                expr->op = OP_MEMDWORD;
                expr->op1 = result;
                return expr;
            } else
                throw ExprError("unknown data type '%s'.", type);

        case TOK_IDENT: {
            if (c->curToken->next->id == TOK_AT) {
                type = c->curToken->text;
                c->curToken = c->curToken->next->next;
                if (c->curToken->id != TOK_LBRACKET)
                    throw ExprError("missing '[' after '@'.");
                goto mem;
            } else if (c->curToken->next->id == TOK_LPAREN) {
                // Function
                const char* name = c->curToken->text;
                c->curToken = c->curToken->next->next;
                ExprCallback0 cb0 = c->resolver->resolveFunc0(name); 
                ExprCallback1 cb1 = c->resolver->resolveFunc1(name);
                ExprCallback2 cb2 = c->resolver->resolveFunc2(name);
                ExprCallback3 cb3 = c->resolver->resolveFunc3(name);
                if (!cb0 && !cb1 && !cb2 && !cb3)
                    throw ExprError("unknown function '%s'.", name);
                int expectedArgs;
                if (cb0)
                    expectedArgs = 0;
                else if (cb1)
                    expectedArgs = 1;
                else if (cb2)
                    expectedArgs = 2;
                else if (cb3)
                    expectedArgs = 3;
                Expr* args[EXPR_MAX_FUNC_ARGS] = {NULL};
                int numArgs = 0;
                if (c->curToken->id != TOK_RPAREN) {
                    for (;;) {
                        if (numArgs >= EXPR_MAX_FUNC_ARGS)
                            throw ExprError("too many arguments for function '%s' (expected %d).", name, expectedArgs);
                        args[numArgs++] = expression(c);
                        if (c->curToken->id == TOK_RPAREN)
                            break;
                        if (c->curToken->id != TOK_COMMA)
                            throw ExprError("missing ','.");
                        c->curToken = c->curToken->next;
                    }
                }
                c->curToken = c->curToken->next;
                switch (numArgs) {
                    case 0:
                        if (!cb0)
                            throw ExprError("invalid number of arguments for function '%s' (expected %d, got %d).", name, expectedArgs, numArgs);
                        result = new Expr;
                        result->op = OP_FUNC0;
                        result->cb0 = cb0;
                        return result;
                    case 1:
                        if (!cb1)
                            throw ExprError("invalid number of arguments for function '%s' (expected %d, got %d).", name, expectedArgs, numArgs);
                        result = new Expr;
                        result->op = OP_FUNC1;
                        result->cb1 = cb1;
                        result->op1 = args[0];
                        return result;
                    case 2:
                        if (!cb2)
                            throw ExprError("invalid number of arguments for function '%s' (expected %d, got %d).", name, expectedArgs, numArgs);
                        result = new Expr;
                        result->op = OP_FUNC2;
                        result->cb2 = cb2;
                        result->op1 = args[0];
                        result->op2 = args[1];
                        return result;
                    case 3:
                        if (!cb3)
                            throw ExprError("invalid number of arguments for function '%s' (expected %d, got %d).", name, expectedArgs, numArgs);
                        result = new Expr;
                        result->op = OP_FUNC3;
                        result->cb3 = cb3;
                        result->op1 = args[0];
                        result->op2 = args[1];
                        result->op3 = args[2];
                        return result;
                    default:
                        throw ExprError("internal error.");
                }
            } else {
                // Label, register, etc.
                ExprValuePtr ptr;
                ptr.readValue = NULL;
                ptr.ptr = NULL;
                ptr.sizeInBytes = 0;
                if (!c->resolver->resolveVariable(c->curToken->text, ptr))
                    throw ExprError("unknown identifier '%s'.", c->curToken->text);

                c->curToken = c->curToken->next;
                if (ptr.readValue) {
                    if (ptr.ptr != NULL)
                        throw ExprError("internal error.");
                    result = new Expr;
                    result->op = OP_CALLBACKVALUE;
                } else {
                    if (ptr.ptr == NULL)
                        throw ExprError("internal error.");
                    result = new Expr;
                    switch (ptr.sizeInBytes) {
                        case 1: result->op = OP_BYTEVALUE; break;
                        case 2: result->op = OP_WORDVALUE; break;
                        case 3: result->op = OP_U24VALUE; break;
                        case 4: result->op = OP_DWORDVALUE; break;
                        default: throw ExprError("internal error.");
                    }
                }
                result->valuePtr = ptr;
                return result;
            }
        }

        default:
            throw ExprError("syntax error in expression.");
    }
}


static Expr* unaryExpression(Context* c)
{
    #define NEXT primaryExpression
    Expr* op;

    switch (c->curToken->id) {
        case TOK_MINUS:
            c->curToken = c->curToken->next;
            op = new Expr;
            op->op = OP_NEGATE;
            op->op1 = unaryExpression(c);
            return op;

        case TOK_EXCLAMATION:
            c->curToken = c->curToken->next;
            op = new Expr;
            op->op = OP_LOGICNOT;
            op->op1 = unaryExpression(c);
            return op;

        case TOK_TILDE:
            c->curToken = c->curToken->next;
            op = new Expr;
            op->op = OP_BITNOT;
            op->op1 = unaryExpression(c);
            return op;
    }

    return NEXT(c);
    #undef NEXT
}

static Expr* multiplicativeExpression(Context* c)
{
    #define NEXT unaryExpression
    Expr* left = NEXT(c);

    while (c->curToken->id == TOK_ASTERISK || c->curToken->id == TOK_SLASH || c->curToken->id == TOK_PERCENT) {
        int op = c->curToken->id;
        c->curToken = c->curToken->next;
        Expr* right = NEXT(c);

        Expr* result = new Expr;
        switch (op) {
            case TOK_ASTERISK: result->op = OP_MULTIPLY; break;
            case TOK_SLASH: result->op = OP_DIVIDE; break;
            case TOK_PERCENT: result->op = OP_REMAINDER; break;
        }
        result->op1 = left;
        result->op2 = right;
        left = result;
    }

    #undef NEXT
    return left;
}

static Expr* additiveExpression(Context* c)
{
    #define NEXT multiplicativeExpression
    Expr* left = NEXT(c);

    while (c->curToken->id == TOK_PLUS || c->curToken->id == TOK_MINUS) {
        int op = c->curToken->id;
        c->curToken = c->curToken->next;
        Expr* right = NEXT(c);

        Expr* result = new Expr;
        switch (op) {
            case TOK_PLUS: result->op = OP_PLUS; break;
            case TOK_MINUS: result->op = OP_MINUS; break;
        }
        result->op1 = left;
        result->op2 = right;
        left = result;
    }

    #undef NEXT
    return left;
}

static Expr* shiftExpression(Context* c)
{
    #define NEXT additiveExpression
    Expr* left = NEXT(c);

    while (c->curToken->id == TOK_SHL || c->curToken->id == TOK_SHR) {
        int op = c->curToken->id;
        c->curToken = c->curToken->next;
        Expr* right = NEXT(c);

        Expr* result = new Expr;
        switch (op) {
            case TOK_SHL: result->op = OP_SHL; break;
            case TOK_SHR: result->op = OP_SHR; break;
        }
        result->op1 = left;
        result->op2 = right;
        left = result;
    }

    #undef NEXT
    return left;
}

static Expr* relationalExpression(Context* c)
{
    #define NEXT shiftExpression
    Expr* left = NEXT(c);

    while (c->curToken->id == TOK_LESS || c->curToken->id == TOK_LESS_EQUAL
            || c->curToken->id == TOK_GREATER || c->curToken->id == TOK_GREATER_EQUAL) {
        int op = c->curToken->id;
        c->curToken = c->curToken->next;
        Expr* right = NEXT(c);

        Expr* result = new Expr;
        switch (op) {
            case TOK_LESS: result->op = OP_LESS; break;
            case TOK_LESS_EQUAL: result->op = OP_LESSEQUAL; break;
            case TOK_GREATER: result->op = OP_GREATER; break;
            case TOK_GREATER_EQUAL: result->op = OP_GREATEREQUAL; break;
        }
        result->op1 = left;
        result->op2 = right;
        left = result;
    }

    #undef NEXT
    return left;
}

static Expr* equalityExpression(Context* c)
{
    #define NEXT relationalExpression
    Expr* left = NEXT(c);

    while (c->curToken->id == TOK_EQUAL || c->curToken->id == TOK_DOUBLE_EQUAL || c->curToken->id == TOK_NOT_EQUAL) {
        int op = c->curToken->id;
        c->curToken = c->curToken->next;
        Expr* right = NEXT(c);

        Expr* result = new Expr;
        switch (op) {
            case TOK_DOUBLE_EQUAL:
            case TOK_EQUAL: result->op = OP_EQUAL; break;
            case TOK_NOT_EQUAL: result->op = OP_NOTEQUAL; break;
        }
        result->op1 = left;
        result->op2 = right;
        left = result;
    }

    #undef NEXT
    return left;
}

static Expr* bitwiseAndExpression(Context* c)
{
    #define NEXT equalityExpression
    Expr* left = NEXT(c);

    while (c->curToken->id == TOK_AMPERSAND) {
        c->curToken = c->curToken->next;
        Expr* right = NEXT(c);

        Expr* op = new Expr;
        op->op = OP_BITAND;
        op->op1 = left;
        op->op2 = right;
        left = op;
    }

    #undef NEXT
    return left;
}

static Expr* bitwiseXorExpression(Context* c)
{
    #define NEXT bitwiseAndExpression
    Expr* left = NEXT(c);

    while (c->curToken->id == TOK_CARET) {
        c->curToken = c->curToken->next;
        Expr* right = NEXT(c);

        Expr* op = new Expr;
        op->op = OP_BITXOR;
        op->op1 = left;
        op->op2 = right;
        left = op;
    }

    #undef NEXT
    return left;
}

static Expr* bitwiseOrExpression(Context* c)
{
    #define NEXT bitwiseXorExpression
    Expr* left = NEXT(c);

    while (c->curToken->id == TOK_VBAR) {
        c->curToken = c->curToken->next;
        Expr* right = NEXT(c);

        Expr* op = new Expr;
        op->op = OP_BITOR;
        op->op1 = left;
        op->op2 = right;
        left = op;
    }

    #undef NEXT
    return left;
}

static Expr* logicalAndExpression(Context* c)
{
    #define NEXT bitwiseOrExpression
    Expr* left = NEXT(c);

    while (c->curToken->id == TOK_DOUBLE_AMPERSAND) {
        c->curToken = c->curToken->next;
        Expr* right = NEXT(c);

        Expr* op = new Expr;
        op->op = OP_LOGICAND;
        op->op1 = left;
        op->op2 = right;
        left = op;
    }

    #undef NEXT
    return left;
}

static Expr* logicalOrExpression(Context* c)
{
    #define NEXT logicalAndExpression
    Expr* left = NEXT(c);

    while (c->curToken->id == TOK_DOUBLE_VBAR) {
        c->curToken = c->curToken->next;
        Expr* right = NEXT(c);

        Expr* op = new Expr;
        op->op = OP_LOGICOR;
        op->op1 = left;
        op->op2 = right;
        left = op;
    }

    #undef NEXT
    return left;
}

static Expr* conditionalExpression(Context* c)
{
    #define NEXT logicalOrExpression
    Expr* expr = NEXT(c);

    if (c->curToken->id == TOK_QUESTION) {
        c->curToken = c->curToken->next;
        Expr* trueCase = expression(c);
        if (c->curToken->id != TOK_COLON)
            throw ExprError("missing ':'.");
        c->curToken = c->curToken->next;
        Expr* falseCase = expression(c);

        Expr* cond = new Expr;
        cond->op = OP_COND;
        cond->op1 = expr;
        cond->op2 = trueCase;
        cond->op3 = falseCase;
        expr = cond;
    }

    #undef NEXT
    return expr;
}

static Expr* expression(Context* c)
{
    #define NEXT conditionalExpression
    return NEXT(c);
    #undef NEXT
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Expr* exprParse(const char* input, ExprResolver& resolver)
{
    ExprTokenList list = exprLexer(input);

    Context c;
    c.curToken = list.first;
    c.resolver = &resolver;
    Expr* result = expression(&c);

    if (c.curToken->id != TOK_END)
        throw ExprError("syntax error in expression.");

    exprFreeTokens(&list);
    return result;
}

void exprFree(Expr* expr)
{
    if (!expr)
        return;

    switch (expr->op) {
        case OP_NUMBER:
        case OP_CALLBACKVALUE:
        case OP_BYTEVALUE:
        case OP_WORDVALUE:
        case OP_U24VALUE:
        case OP_DWORDVALUE:
        case OP_FUNC0:
        case OP_DOLLAR:
            return;

        case OP_FUNC1:
        case OP_MEMBYTE:
        case OP_MEMWORD:
        case OP_MEMDWORD:
        case OP_LOGICNOT:
        case OP_BITNOT:
        case OP_NEGATE:
            exprFree(expr->op1);
            return;

        case OP_FUNC2:
        case OP_LOGICOR:
        case OP_LOGICAND:
        case OP_BITOR:
        case OP_BITAND:
        case OP_BITXOR:
        case OP_EQUAL:
        case OP_NOTEQUAL:
        case OP_LESS:
        case OP_LESSEQUAL:
        case OP_GREATER:
        case OP_GREATEREQUAL:
        case OP_SHL:
        case OP_SHR:
        case OP_PLUS:
        case OP_MINUS:
        case OP_MULTIPLY:
        case OP_DIVIDE:
        case OP_REMAINDER:
            exprFree(expr->op1);
            exprFree(expr->op2);
            return;

        case OP_FUNC3:
        case OP_COND:
            exprFree(expr->op1);
            exprFree(expr->op2);
            exprFree(expr->op3);
            return;

        default:
            throw ExprError("internal error.");
    }
}

} // namespace
