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
#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Expr::Error::Error(const char* message, ...)
{
    va_list args;

    va_start(args, message);
    vsnprintf(m_message, sizeof(m_message), message, args);
    va_end(args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Lexer

enum TokenID
{
    TOK_END,
    TOK_NUMBER,
    TOK_IDENT,
    TOK_AT,
    TOK_COMMA,
    TOK_PLUS,
    TOK_MINUS,
    TOK_ASTERISK,
    TOK_SLASH,
    TOK_PERCENT,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_QUESTION,
    TOK_COLON,
    TOK_AMPERSAND,
    TOK_DOUBLE_AMPERSAND,
    TOK_VBAR,
    TOK_DOUBLE_VBAR,
    TOK_CARET,
    TOK_TILDE,
    TOK_EXCLAMATION,
    TOK_HASH,
    TOK_DOLLAR,
    TOK_EQUAL,
    TOK_DOUBLE_EQUAL,
    TOK_NOT_EQUAL,
    TOK_LESS,
    TOK_LESS_EQUAL,
    TOK_GREATER,
    TOK_GREATER_EQUAL,
    TOK_SHR,
    TOK_SHL,
    TOK_LBRACKET,
    TOK_RBRACKET,
};

struct Token
{
    Token* next;
    int id;
    Expr::Value number;
    const char* text;
};

struct TokenList
{
    Token* first;
    Token* last;
};

static bool isDigit(char ch)
{
    switch (ch) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return true;
    }
    return false;
}

static bool isBinDigit(char ch)
{
    switch (ch) {
        case '0': case '1':
            return true;
    }
    return false;
}

static bool isOctDigit(char ch)
{
    switch (ch) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7':
            return true;
    }
    return false;
}

static bool isHexDigit(char ch)
{
    switch (ch) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            return true;
    }
    return false;
}

static bool isLetter(char ch)
{
    switch (ch) {
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
            return true;
    }
    return false;
}

static bool isIdent(char ch)
{
    return isLetter(ch) || isDigit(ch) || ch == '_' || ch == '.';
}

static int hexValue(char ch)
{
    switch (ch) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'a': case 'A': return 10;
        case 'b': case 'B': return 11;
        case 'c': case 'C': return 12;
        case 'd': case 'D': return 13;
        case 'e': case 'E': return 14;
        case 'f': case 'F': return 15;
    }
    throw Expr::Error("internal error.");
}

static void freeTokens(TokenList* list)
{
    Token* p = list->first;
    while (p) {
        Token* t = p;
        p = p->next;
        delete[] t->text;
        delete t;
    }
}

static void emitToken(TokenList* list, TokenID id, Expr::Value number = 0, const char* text = NULL)
{
    Token* token = new Token;
    token->next = NULL;
    token->id = id;
    token->number = number;
    token->text = text;

    if (!list->first)
        list->first = token;
    else
        list->last->next = token;
    list->last = token;
}

static TokenList scan(const char* input)
{
    TokenList list;
    list.first = NULL;
    list.last = NULL;

    for (;;) {
        switch (*input) {
            case 0:
                emitToken(&list, TOK_END);
                return list;

            case ' ':
            case '\t':
            case '\r':
            case '\n':
                ++input;
                continue;

            case ',':
                ++input;
                emitToken(&list, TOK_COMMA);
                continue;

            case '@':
                ++input;
                emitToken(&list, TOK_AT);
                continue;

            case '(':
                ++input;
                emitToken(&list, TOK_LPAREN);
                continue;

            case ')':
                ++input;
                emitToken(&list, TOK_RPAREN);
                continue;

            case '[':
                ++input;
                emitToken(&list, TOK_LBRACKET);
                continue;

            case ']':
                ++input;
                emitToken(&list, TOK_RBRACKET);
                continue;

            case '?':
                ++input;
                emitToken(&list, TOK_QUESTION);
                continue;

            case ':':
                ++input;
                emitToken(&list, TOK_COLON);
                continue;

            case '+':
                ++input;
                emitToken(&list, TOK_PLUS);
                continue;

            case '-':
                ++input;
                emitToken(&list, TOK_MINUS);
                continue;

            case '*':
                ++input;
                emitToken(&list, TOK_ASTERISK);
                continue;

            case '/':
                ++input;
                emitToken(&list, TOK_SLASH);
                continue;

            case '%':
                ++input;
                emitToken(&list, TOK_PERCENT);
                continue;

            case '^':
                ++input;
                emitToken(&list, TOK_CARET);
                continue;

            case '~':
                ++input;
                emitToken(&list, TOK_TILDE);
                continue;

            case '&':
                ++input;
                if (*input == '&') {
                    ++input;
                    emitToken(&list, TOK_DOUBLE_AMPERSAND);
                } else
                    emitToken(&list, TOK_AMPERSAND);
                continue;

            case '|':
                ++input;
                if (*input == '|') {
                    ++input;
                    emitToken(&list, TOK_DOUBLE_VBAR);
                } else
                    emitToken(&list, TOK_VBAR);
                continue;

            case '=':
                ++input;
                if (*input == '=') {
                    ++input;
                    emitToken(&list, TOK_DOUBLE_EQUAL);
                } else
                    emitToken(&list, TOK_EQUAL);
                continue;

            case '!':
                ++input;
                if (*input == '=') {
                    ++input;
                    emitToken(&list, TOK_NOT_EQUAL);
                } else
                    emitToken(&list, TOK_EXCLAMATION);
                continue;

            case '<':
                ++input;
                if (*input == '=') {
                    ++input;
                    emitToken(&list, TOK_LESS_EQUAL);
                } else if (*input == '<') {
                    ++input;
                    emitToken(&list, TOK_SHL);
                } else
                    emitToken(&list, TOK_LESS);
                continue;

            case '>':
                ++input;
                if (*input == '=') {
                    ++input;
                    emitToken(&list, TOK_GREATER_EQUAL);
                } else if (*input == '>') {
                    ++input;
                    emitToken(&list, TOK_SHR);
                } else
                    emitToken(&list, TOK_GREATER);
                continue;

            case '$':
                ++input;
                if (isHexDigit(*input))
                    goto parseHex;
                emitToken(&list, TOK_DOLLAR);
                continue;

            case '#':
                ++input;
                if (isHexDigit(*input))
                    goto parseHex;
                emitToken(&list, TOK_HASH);
                continue;

            case '0':
                if (input[1] == 'x' || input[1] == 'X') {
                    input += 2;
                    if (!isHexDigit(*input))
                        throw Expr::Error("syntax error in hexadecimal number.");
                  parseHex:
                    Expr::Value value = 0;
                    do {
                        value = value << 4;
                        value += hexValue(*input++);
                    } while (isHexDigit(*input));
                    if (isDigit(*input) || isLetter(*input) || *input == '_' || *input == '.')
                        throw Expr::Error("syntax error in hexadecimal number.");
                    emitToken(&list, TOK_NUMBER, value);
                    continue;
                }
                if (input[1] == 'b' || input[1] == 'B') {
                    input += 2;
                    if (!isBinDigit(*input))
                        throw Expr::Error("syntax error in binary number.");
                    Expr::Value value = 0;
                    do {
                        value = value << 1;
                        value += hexValue(*input++);
                    } while (isBinDigit(*input));
                    if (isDigit(*input) || isLetter(*input) || *input == '_' || *input == '.')
                        throw Expr::Error("syntax error in binary number.");
                    emitToken(&list, TOK_NUMBER, value);
                    continue;
                }
                if (input[1] == 'o' || input[1] == 'O') {
                    input += 2;
                    if (!isOctDigit(*input))
                        throw Expr::Error("syntax error in octal number.");
                    Expr::Value value = 0;
                    do {
                        value = value << 3;
                        value += hexValue(*input++);
                    } while (isOctDigit(*input));
                    if (isDigit(*input) || isLetter(*input) || *input == '_' || *input == '.')
                        throw Expr::Error("syntax error in octal number.");
                    emitToken(&list, TOK_NUMBER, value);
                    continue;
                }
                if (isDigit(input[1]))
                    throw Expr::Error("numbers starting with '0' are not supported, use '0o' prefix for octal numbers.");
                // pass-through
            case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9': {
                Expr::Value value = 0;
                do {
                    value = value * 10;
                    value += *input++ - '0';
                } while (isDigit(*input));
                if (isLetter(*input) || *input == '_' || *input == '.')
                    throw Expr::Error("syntax error in number.");
                emitToken(&list, TOK_NUMBER, value);
                continue;
            }

            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j':
            case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't':
            case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
            case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T':
            case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
            case '_': case '.': {
                char buf[128];
                size_t i = 0;
                buf[i++] = *input++;
                while (isIdent(*input)) {
                    if (i >= sizeof(buf))
                        throw Expr::Error("identifier too long.");
                    buf[i++] = *input++;
                }
                if (*input == '\'') {
                    if (i >= sizeof(buf))
                        throw Expr::Error("identifier too long.");
                    buf[i++] = *input++;
                }
                char* ident = new char[i+1];
                memcpy(ident, buf, i);
                ident[i] = 0;
                emitToken(&list, TOK_IDENT, 0, ident);
                continue;
            }

            default:
                throw Expr::Error("unexpected character '%c'.", *input);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Syntax tree

class NumberExpr : public Expr
{
public:
    explicit NumberExpr(Expr::Value number) : m_number(number) {}

    Value evaluate(Evaluator& e) const
    {
        return m_number;
    }

private:
    Value m_number;
};

class CallbackValueExpr : public Expr
{
public:
    explicit CallbackValueExpr(ValuePtr ptr) : m_ptr(ptr) {}

    Value evaluate(Evaluator& e) const
    {
        return m_ptr.readValue();
    }

private:
    ValuePtr m_ptr;
};

class ByteValueExpr : public Expr
{
public:
    explicit ByteValueExpr(ValuePtr ptr) : m_ptr(ptr) {}

    Value evaluate(Evaluator& e) const
    {
        return *(uint8_t*)m_ptr.ptr;
    }

private:
    ValuePtr m_ptr;
};

class WordValueExpr : public Expr
{
public:
    explicit WordValueExpr(ValuePtr ptr) : m_ptr(ptr) {}

    Value evaluate(Evaluator& e) const
    {
        return *(uint16_t*)m_ptr.ptr;
    }

private:
    ValuePtr m_ptr;
};

class U24ValueExpr : public Expr
{
public:
    explicit U24ValueExpr(ValuePtr ptr) : m_ptr(ptr) {}

    Value evaluate(Evaluator& e) const
    {
        uint16_t w = *(uint16_t*)m_ptr.ptr;
        uint8_t b = *((uint8_t*)m_ptr.ptr + 2);
        return w | (b << 16);
    }

private:
    ValuePtr m_ptr;
};

class DwordValueExpr : public Expr
{
public:
    explicit DwordValueExpr(ValuePtr ptr) : m_ptr(ptr) {}

    Value evaluate(Evaluator& e) const
    {
        return *(uint32_t*)m_ptr.ptr;
    }

private:
    ValuePtr m_ptr;
};

class Func0Expr : public Expr
{
public:
    explicit Func0Expr(Callback0 cb) : m_callback(cb) {}

    Value evaluate(Evaluator& e) const
    {
        return m_callback();
    }

private:
    Callback0 m_callback;
};

class Func1Expr : public Expr
{
public:
    Func1Expr(Callback1 cb, Expr* arg1) : m_callback(cb), m_arg1(arg1) {}
    ~Func1Expr() { delete m_arg1; }

    Value evaluate(Evaluator& e) const
    {
        return m_callback(m_arg1->evaluate(e));
    }

private:
    Callback1 m_callback;
    Expr* m_arg1;
};

class Func2Expr : public Expr
{
public:
    Func2Expr(Callback2 cb, Expr* arg1, Expr* arg2) : m_callback(cb), m_arg1(arg1), m_arg2(arg2) {}
    ~Func2Expr() { delete m_arg1; delete m_arg2; }

    Value evaluate(Evaluator& e) const
    {
        return m_callback(m_arg1->evaluate(e), m_arg2->evaluate(e));
    }

private:
    Callback2 m_callback;
    Expr* m_arg1;
    Expr* m_arg2;
};

class Func3Expr : public Expr
{
public:
    Func3Expr(Callback3 cb, Expr* arg1, Expr* arg2, Expr* arg3) : m_callback(cb), m_arg1(arg1), m_arg2(arg2), m_arg3(arg3) {}
    ~Func3Expr() { delete m_arg1; delete m_arg2; delete m_arg3; }

    Value evaluate(Evaluator& e) const
    {
        return m_callback(m_arg1->evaluate(e), m_arg2->evaluate(e), m_arg3->evaluate(e));
    }

private:
    Callback3 m_callback;
    Expr* m_arg1;
    Expr* m_arg2;
    Expr* m_arg3;
};

class MemByteExpr : public Expr
{
public:
    MemByteExpr(Expr* op) : m_op(op) {}
    ~MemByteExpr() { delete m_op; }

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
    {
        return e.memDword(m_op->evaluate(e));
    }

private:
    Expr* m_op;
};

class DollarExpr : public Expr
{
public:
    explicit DollarExpr() {}

    Value evaluate(Evaluator& e) const
    {
        return e.pc();
    }

private:
    Value m_number;
};

class ConditionalExpr : public Expr
{
public:
    ConditionalExpr(Expr* cond, Expr* t, Expr* f) : m_cond(cond), m_falseCase(f), m_trueCase(t) {}
    ~ConditionalExpr() { delete m_cond; delete m_trueCase; delete m_falseCase; }

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
    {
        return (Value)((UnsignedValue)m_left->evaluate(e) >> (UnsignedValue)m_right->evaluate(e));
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
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

    Value evaluate(Evaluator& e) const
    {
        int r = m_right->evaluate(e);
        if (r == 0)
            throw Error("division by zero.");
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

    Value evaluate(Evaluator& e) const
    {
        int r = m_right->evaluate(e);
        if (r == 0)
            throw Error("division by zero.");
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
    Token* curToken;
    Expr::Resolver* resolver;
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
                throw Expr::Error("missing ')'.");
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
                    throw Expr::Error("missing ':'.");
                c->curToken = c->curToken->next;
            }
            */
            result = expression(c);
            if (c->curToken->id != TOK_RBRACKET)
                throw Expr::Error("missing ']'.");
            c->curToken = c->curToken->next;
            if (!strcmp(type, "b"))
                return new MemByteExpr(result);
            else if (!strcmp(type, "w"))
                return new MemWordExpr(result);
            else if (!strcmp(type, "d"))
                return new MemDwordExpr(result);
            else
                throw Expr::Error("unknown data type '%s'.", type);

        case TOK_IDENT: {
            if (c->curToken->next->id == TOK_AT) {
                type = c->curToken->text;
                c->curToken = c->curToken->next->next;
                if (c->curToken->id != TOK_LBRACKET)
                    throw Expr::Error("missing '[' after '@'.");
                goto mem;
            } else if (c->curToken->next->id == TOK_LPAREN) {
                // Function
                const char* name = c->curToken->text;
                c->curToken = c->curToken->next->next;
                Expr* args[Expr::MAX_FUNC_ARGS] = {NULL};
                int numArgs = 0;
                if (c->curToken->id != TOK_RPAREN) {
                    for (;;) {
                        if (numArgs >= Expr::MAX_FUNC_ARGS)
                            throw Expr::Error("too many function arguments.");
                        args[numArgs++] = expression(c);
                        if (c->curToken->id == TOK_RPAREN)
                            break;
                        if (c->curToken->id != TOK_COMMA)
                            throw Expr::Error("missing ','.");
                        c->curToken = c->curToken->next;
                    }
                }
                c->curToken = c->curToken->next;
                Expr::Callback0 cb0 = c->resolver->resolveFunc0(name); 
                Expr::Callback1 cb1 = c->resolver->resolveFunc1(name);
                Expr::Callback2 cb2 = c->resolver->resolveFunc2(name);
                Expr::Callback3 cb3 = c->resolver->resolveFunc3(name);
                if (!cb0 && !cb1 && !cb2 && !cb3)
                    throw Expr::Error("unknown function '%s'.", name);
                switch (numArgs) {
                    case 0:
                        if (!cb0)
                            throw Expr::Error("invalid number of arguments for function '%s'.", name);
                        return new Func0Expr(cb0);
                    case 1:
                        if (!cb1)
                            throw Expr::Error("invalid number of arguments for function '%s'.", name);
                        return new Func1Expr(cb1, args[0]);
                    case 2:
                        if (!cb2)
                            throw Expr::Error("invalid number of arguments for function '%s'.", name);
                        return new Func2Expr(cb2, args[0], args[1]);
                    case 3:
                        if (!cb3)
                            throw Expr::Error("invalid number of arguments for function '%s'.", name);
                        return new Func3Expr(cb3, args[0], args[1], args[2]);
                    default:
                        throw Expr::Error("internal error.");
                }
            } else {
                // Label, register, etc.
                Expr::ValuePtr ptr;
                ptr.readValue = NULL;
                ptr.ptr = NULL;
                ptr.sizeInBytes = 0;
                if (!c->resolver->resolveVariable(c->curToken->text, ptr))
                    throw Expr::Error("unknown identifier '%s'.", c->curToken->text);

                c->curToken = c->curToken->next;
                if (ptr.readValue) {
                    if (ptr.ptr != NULL)
                        throw Expr::Error("internal error.");
                    return new CallbackValueExpr(ptr);
                } else {
                    if (ptr.ptr == NULL)
                        throw Expr::Error("internal error.");
                    switch (ptr.sizeInBytes) {
                        case 1: return new ByteValueExpr(ptr);
                        case 2: return new WordValueExpr(ptr);
                        case 3: return new U24ValueExpr(ptr);
                        case 4: return new DwordValueExpr(ptr);
                        default: throw Expr::Error("internal error.");
                    }
                }
                return result;
            }
        }

        default:
            throw Expr::Error("syntax error in expression.");
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
            throw Expr::Error("missing ':'.");
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

Expr* Expr::parse(const char* input, Resolver& resolver)
{
    TokenList list = scan(input);

    Context c;
    c.curToken = list.first;
    c.resolver = &resolver;
    Expr* result = expression(&c);

    if (c.curToken->id != TOK_END)
        throw Error("syntax error in expression.");

    freeTokens(&list);
    return result;
}
