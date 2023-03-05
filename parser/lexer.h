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
#ifndef DRUNKFLY_PARSER_LEXER_H
#define DRUNKFLY_PARSER_LEXER_H

#include "parser/common.h"

enum ExprTokenID
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

struct ExprToken
{
    ExprToken* next;
    int id;
    ExprValue number;
    const char* text;
};

struct ExprTokenList
{
    ExprToken* first;
    ExprToken* last;
};

ExprTokenList exprLexer(const char* input);
void exprFreeTokens(ExprTokenList* list);

#endif
