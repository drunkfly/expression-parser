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
#include "parser/parser_oop.h"
#include "parser/lexer.h"
#include <string.h>

namespace ParserOop
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Syntax tree

class NumberExpr : public Expr
{
public:
    explicit NumberExpr(ExprValue number) : m_number(number) {}

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_number;
    }

private:
    ExprValue m_number;
};

class CallbackValueExpr : public Expr
{
public:
    explicit CallbackValueExpr(ExprValuePtr ptr) : m_ptr(ptr) {}

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_ptr.readValue();
    }

private:
    ExprValuePtr m_ptr;
};

class ByteValueExpr : public Expr
{
public:
    explicit ByteValueExpr(ExprValuePtr ptr) : m_ptr(ptr) {}

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return *(uint8_t*)m_ptr.ptr;
    }

private:
    ExprValuePtr m_ptr;
};

class WordValueExpr : public Expr
{
public:
    explicit WordValueExpr(ExprValuePtr ptr) : m_ptr(ptr) {}

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return *(uint16_t*)m_ptr.ptr;
    }

private:
    ExprValuePtr m_ptr;
};

class U24ValueExpr : public Expr
{
public:
    explicit U24ValueExpr(ExprValuePtr ptr) : m_ptr(ptr) {}

    ExprValue evaluate(ExprEvaluator& e) const
    {
        uint16_t w = *(uint16_t*)m_ptr.ptr;
        uint8_t b = *((uint8_t*)m_ptr.ptr + 2);
        return w | (b << 16);
    }

private:
    ExprValuePtr m_ptr;
};

class DwordValueExpr : public Expr
{
public:
    explicit DwordValueExpr(ExprValuePtr ptr) : m_ptr(ptr) {}

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return *(uint32_t*)m_ptr.ptr;
    }

private:
    ExprValuePtr m_ptr;
};

class Func0Expr : public Expr
{
public:
    explicit Func0Expr(ExprCallback0 cb) : m_callback(cb) {}

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_callback();
    }

private:
    ExprCallback0 m_callback;
};

class Func1Expr : public Expr
{
public:
    Func1Expr(ExprCallback1 cb, Expr* arg1) : m_callback(cb), m_arg1(arg1) {}
    ~Func1Expr() { delete m_arg1; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_callback(m_arg1->evaluate(e));
    }

private:
    ExprCallback1 m_callback;
    Expr* m_arg1;
};

class Func2Expr : public Expr
{
public:
    Func2Expr(ExprCallback2 cb, Expr* arg1, Expr* arg2) : m_callback(cb), m_arg1(arg1), m_arg2(arg2) {}
    ~Func2Expr() { delete m_arg1; delete m_arg2; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_callback(m_arg1->evaluate(e), m_arg2->evaluate(e));
    }

private:
    ExprCallback2 m_callback;
    Expr* m_arg1;
    Expr* m_arg2;
};

class Func3Expr : public Expr
{
public:
    Func3Expr(ExprCallback3 cb, Expr* arg1, Expr* arg2, Expr* arg3) : m_callback(cb), m_arg1(arg1), m_arg2(arg2), m_arg3(arg3) {}
    ~Func3Expr() { delete m_arg1; delete m_arg2; delete m_arg3; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_callback(m_arg1->evaluate(e), m_arg2->evaluate(e), m_arg3->evaluate(e));
    }

private:
    ExprCallback3 m_callback;
    Expr* m_arg1;
    Expr* m_arg2;
    Expr* m_arg3;
};

class MemByteExpr : public Expr
{
public:
    MemByteExpr(Expr* op) : m_op(op) {}
    ~MemByteExpr() { delete m_op; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return e.memByte(m_op->evaluate(e));
    }

private:
    Expr* m_op;
};

class MemWordExpr : public Expr
{
public:
    MemWordExpr(Expr* op) : m_op(op) {}
    ~MemWordExpr() { delete m_op; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return e.memWord(m_op->evaluate(e));
    }

private:
    Expr* m_op;
};

class MemDwordExpr : public Expr
{
public:
    MemDwordExpr(Expr* op) : m_op(op) {}
    ~MemDwordExpr() { delete m_op; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return e.memDword(m_op->evaluate(e));
    }

private:
    Expr* m_op;
};

class DollarExpr : public Expr
{
public:
    DollarExpr() {}

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return e.pc();
    }
};

class ConditionalExpr : public Expr
{
public:
    ConditionalExpr(Expr* cond, Expr* t, Expr* f) : m_cond(cond), m_falseCase(f), m_trueCase(t) {}
    ~ConditionalExpr() { delete m_cond; delete m_trueCase; delete m_falseCase; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        if (m_cond->evaluate(e))
            return m_trueCase->evaluate(e);
        else
            return m_falseCase->evaluate(e);
    }

private:
    Expr* m_cond;
    Expr* m_falseCase;
    Expr* m_trueCase;
};

class LogicOrExpr : public Expr
{
public:
    LogicOrExpr(Expr* left, Expr* right) : m_left(left), m_right(right) {}
    ~LogicOrExpr() { delete m_left; delete m_right; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_left->evaluate(e) || m_right->evaluate(e);
    }

private:
    Expr* m_left;
    Expr* m_right;
};

class LogicAndExpr : public Expr
{
public:
    LogicAndExpr(Expr* left, Expr* right) : m_left(left), m_right(right) {}
    ~LogicAndExpr() { delete m_left; delete m_right; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_left->evaluate(e) && m_right->evaluate(e);
    }

private:
    Expr* m_left;
    Expr* m_right;
};

class LogicNotExpr : public Expr
{
public:
    LogicNotExpr(Expr* op) : m_op(op) {}
    ~LogicNotExpr() { delete m_op; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return !m_op->evaluate(e);
    }

private:
    Expr* m_op;
};

class OrExpr : public Expr
{
public:
    OrExpr(Expr* left, Expr* right) : m_left(left), m_right(right) {}
    ~OrExpr() { delete m_left; delete m_right; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_left->evaluate(e) | m_right->evaluate(e);
    }

private:
    Expr* m_left;
    Expr* m_right;
};

class AndExpr : public Expr
{
public:
    AndExpr(Expr* left, Expr* right) : m_left(left), m_right(right) {}
    ~AndExpr() { delete m_left; delete m_right; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_left->evaluate(e) & m_right->evaluate(e);
    }

private:
    Expr* m_left;
    Expr* m_right;
};

class XorExpr : public Expr
{
public:
    XorExpr(Expr* left, Expr* right) : m_left(left), m_right(right) {}
    ~XorExpr() { delete m_left; delete m_right; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_left->evaluate(e) ^ m_right->evaluate(e);
    }

private:
    Expr* m_left;
    Expr* m_right;
};

class NotExpr : public Expr
{
public:
    NotExpr(Expr* op) : m_op(op) {}
    ~NotExpr() { delete m_op; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return ~m_op->evaluate(e);
    }

private:
    Expr* m_op;
};

class EqualityExpr : public Expr
{
public:
    EqualityExpr(Expr* left, Expr* right) : m_left(left), m_right(right) {}
    ~EqualityExpr() { delete m_left; delete m_right; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_left->evaluate(e) == m_right->evaluate(e);
    }

private:
    Expr* m_left;
    Expr* m_right;
};

class InequalityExpr : public Expr
{
public:
    InequalityExpr(Expr* left, Expr* right) : m_left(left), m_right(right) {}
    ~InequalityExpr() { delete m_left; delete m_right; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_left->evaluate(e) != m_right->evaluate(e);
    }

private:
    Expr* m_left;
    Expr* m_right;
};

class LessExpr : public Expr
{
public:
    LessExpr(Expr* left, Expr* right) : m_left(left), m_right(right) {}
    ~LessExpr() { delete m_left; delete m_right; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_left->evaluate(e) < m_right->evaluate(e);
    }

private:
    Expr* m_left;
    Expr* m_right;
};

class LessEqualExpr : public Expr
{
public:
    LessEqualExpr(Expr* left, Expr* right) : m_left(left), m_right(right) {}
    ~LessEqualExpr() { delete m_left; delete m_right; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_left->evaluate(e) <= m_right->evaluate(e);
    }

private:
    Expr* m_left;
    Expr* m_right;
};

class GreaterExpr : public Expr
{
public:
    GreaterExpr(Expr* left, Expr* right) : m_left(left), m_right(right) {}
    ~GreaterExpr() { delete m_left; delete m_right; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_left->evaluate(e) > m_right->evaluate(e);
    }

private:
    Expr* m_left;
    Expr* m_right;
};

class GreaterEqualExpr : public Expr
{
public:
    GreaterEqualExpr(Expr* left, Expr* right) : m_left(left), m_right(right) {}
    ~GreaterEqualExpr() { delete m_left; delete m_right; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_left->evaluate(e) >= m_right->evaluate(e);
    }

private:
    Expr* m_left;
    Expr* m_right;
};

class ShlExpr : public Expr
{
public:
    ShlExpr(Expr* left, Expr* right) : m_left(left), m_right(right) {}
    ~ShlExpr() { delete m_left; delete m_right; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_left->evaluate(e) << m_right->evaluate(e);
    }

private:
    Expr* m_left;
    Expr* m_right;
};

class ShrExpr : public Expr
{
public:
    ShrExpr(Expr* left, Expr* right) : m_left(left), m_right(right) {}
    ~ShrExpr() { delete m_left; delete m_right; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return (ExprValue)((ExprUValue)m_left->evaluate(e) >> (ExprUValue)m_right->evaluate(e));
    }

private:
    Expr* m_left;
    Expr* m_right;
};

class PlusExpr : public Expr
{
public:
    PlusExpr(Expr* left, Expr* right) : m_left(left), m_right(right) {}
    ~PlusExpr() { delete m_left; delete m_right; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_left->evaluate(e) + m_right->evaluate(e);
    }

private:
    Expr* m_left;
    Expr* m_right;
};

class MinusExpr : public Expr
{
public:
    MinusExpr(Expr* left, Expr* right) : m_left(left), m_right(right) {}
    ~MinusExpr() { delete m_left; delete m_right; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_left->evaluate(e) - m_right->evaluate(e);
    }

private:
    Expr* m_left;
    Expr* m_right;
};

class NegateExpr : public Expr
{
public:
    NegateExpr(Expr* op) : m_op(op) {}
    ~NegateExpr() { delete m_op; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return -m_op->evaluate(e);
    }

private:
    Expr* m_op;
};

class MultiplyExpr : public Expr
{
public:
    MultiplyExpr(Expr* left, Expr* right) : m_left(left), m_right(right) {}
    ~MultiplyExpr() { delete m_left; delete m_right; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        return m_left->evaluate(e) * m_right->evaluate(e);
    }

private:
    Expr* m_left;
    Expr* m_right;
};

class DivideExpr : public Expr
{
public:
    DivideExpr(Expr* left, Expr* right) : m_left(left), m_right(right) {}
    ~DivideExpr() { delete m_left; delete m_right; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        int r = m_right->evaluate(e);
        if (r == 0)
            throw ExprError("division by zero.");
        return m_left->evaluate(e) / r;
    }

private:
    Expr* m_left;
    Expr* m_right;
};

class RemainderExpr : public Expr
{
public:
    RemainderExpr(Expr* left, Expr* right) : m_left(left), m_right(right) {}
    ~RemainderExpr() { delete m_left; delete m_right; }

    ExprValue evaluate(ExprEvaluator& e) const
    {
        int r = m_right->evaluate(e);
        if (r == 0)
            throw ExprError("division by zero.");
        return m_left->evaluate(e) % r;
    }

private:
    Expr* m_left;
    Expr* m_right;
};

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
            result = new DollarExpr();
            c->curToken = c->curToken->next;
            return result;

        case TOK_NUMBER:
            result = new NumberExpr(c->curToken->number);
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
            if (!strcmp(type, "b"))
                return new MemByteExpr(result);
            else if (!strcmp(type, "w"))
                return new MemWordExpr(result);
            else if (!strcmp(type, "d"))
                return new MemDwordExpr(result);
            else
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
                Expr* args[EXPR_MAX_FUNC_ARGS] = {NULL};
                int numArgs = 0;
                if (c->curToken->id != TOK_RPAREN) {
                    for (;;) {
                        if (numArgs >= EXPR_MAX_FUNC_ARGS)
                            throw ExprError("too many arguments for function '%s'.", name);
                        args[numArgs++] = expression(c);
                        if (c->curToken->id == TOK_RPAREN)
                            break;
                        if (c->curToken->id != TOK_COMMA)
                            throw ExprError("missing ','.");
                        c->curToken = c->curToken->next;
                    }
                }
                c->curToken = c->curToken->next;
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
                switch (numArgs) {
                    case 0:
                        if (!cb0)
                            throw ExprError("invalid number of arguments for function '%s' (expected %d, got %d).", name, expectedArgs, numArgs);
                        return new Func0Expr(cb0);
                    case 1:
                        if (!cb1)
                            throw ExprError("invalid number of arguments for function '%s' (expected %d, got %d).", name, expectedArgs, numArgs);
                        return new Func1Expr(cb1, args[0]);
                    case 2:
                        if (!cb2)
                            throw ExprError("invalid number of arguments for function '%s' (expected %d, got %d).", name, expectedArgs, numArgs);
                        return new Func2Expr(cb2, args[0], args[1]);
                    case 3:
                        if (!cb3)
                            throw ExprError("invalid number of arguments for function '%s' (expected %d, got %d).", name, expectedArgs, numArgs);
                        return new Func3Expr(cb3, args[0], args[1], args[2]);
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
                    return new CallbackValueExpr(ptr);
                } else {
                    if (ptr.ptr == NULL)
                        throw ExprError("internal error.");
                    switch (ptr.sizeInBytes) {
                        case 1: return new ByteValueExpr(ptr);
                        case 2: return new WordValueExpr(ptr);
                        case 3: return new U24ValueExpr(ptr);
                        case 4: return new DwordValueExpr(ptr);
                        default: throw ExprError("internal error.");
                    }
                }
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

    switch (c->curToken->id) {
        case TOK_MINUS:
            c->curToken = c->curToken->next;
            return new NegateExpr(unaryExpression(c));

        case TOK_EXCLAMATION:
            c->curToken = c->curToken->next;
            return new LogicNotExpr(unaryExpression(c));

        case TOK_TILDE:
            c->curToken = c->curToken->next;
            return new NotExpr(unaryExpression(c));
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
        switch (op) {
            case TOK_ASTERISK: left = new MultiplyExpr(left, right); break;
            case TOK_SLASH: left = new DivideExpr(left, right); break;
            case TOK_PERCENT: left = new RemainderExpr(left, right); break;
        }
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
        switch (op) {
            case TOK_PLUS: left = new PlusExpr(left, right); break;
            case TOK_MINUS: left = new MinusExpr(left, right); break;
        }
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
        switch (op) {
            case TOK_SHL: left = new ShlExpr(left, right); break;
            case TOK_SHR: left = new ShrExpr(left, right); break;
        }
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
        switch (op) {
            case TOK_LESS: left = new LessExpr(left, right); break;
            case TOK_LESS_EQUAL: left = new LessEqualExpr(left, right); break;
            case TOK_GREATER: left = new GreaterExpr(left, right); break;
            case TOK_GREATER_EQUAL: left = new GreaterEqualExpr(left, right); break;
        }
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
        switch (op) {
            case TOK_DOUBLE_EQUAL:
            case TOK_EQUAL: left = new EqualityExpr(left, right); break;
            case TOK_NOT_EQUAL: left = new InequalityExpr(left, right); break;
        }
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
        left = new AndExpr(left, right);
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
        left = new XorExpr(left, right);
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
        left = new OrExpr(left, right);
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
        left = new LogicAndExpr(left, right);
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
        left = new LogicOrExpr(left, right);
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
        expr = new ConditionalExpr(expr, trueCase, falseCase);
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

Expr* Expr::parse(const char* input, ExprResolver& resolver)
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

} // namespace
