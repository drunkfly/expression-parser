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
#include "parser/lexer.h"
#include <string.h>

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
    throw ExprError("internal error.");
}

static void emitToken(ExprTokenList* list, ExprTokenID id, ExprValue number = 0, const char* text = NULL)
{
    ExprToken* token = new ExprToken;
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

ExprTokenList exprLexer(const char* input)
{
    ExprTokenList list;
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
                        throw ExprError("syntax error in hexadecimal number.");
                  parseHex:
                    ExprValue value = 0;
                    do {
                        value = value << 4;
                        value += hexValue(*input++);
                    } while (isHexDigit(*input));
                    if (isDigit(*input) || isLetter(*input) || *input == '_' || *input == '.')
                        throw ExprError("syntax error in hexadecimal number.");
                    emitToken(&list, TOK_NUMBER, value);
                    continue;
                }
                if (input[1] == 'b' || input[1] == 'B') {
                    input += 2;
                    if (!isBinDigit(*input))
                        throw ExprError("syntax error in binary number.");
                    ExprValue value = 0;
                    do {
                        value = value << 1;
                        value += hexValue(*input++);
                    } while (isBinDigit(*input));
                    if (isDigit(*input) || isLetter(*input) || *input == '_' || *input == '.')
                        throw ExprError("syntax error in binary number.");
                    emitToken(&list, TOK_NUMBER, value);
                    continue;
                }
                if (input[1] == 'o' || input[1] == 'O') {
                    input += 2;
                    if (!isOctDigit(*input))
                        throw ExprError("syntax error in octal number.");
                    ExprValue value = 0;
                    do {
                        value = value << 3;
                        value += hexValue(*input++);
                    } while (isOctDigit(*input));
                    if (isDigit(*input) || isLetter(*input) || *input == '_' || *input == '.')
                        throw ExprError("syntax error in octal number.");
                    emitToken(&list, TOK_NUMBER, value);
                    continue;
                }
                if (isDigit(input[1]))
                    throw ExprError("numbers starting with '0' are not supported, use '0o' prefix for octal numbers.");
                // pass-through
            case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9': {
                ExprValue value = 0;
                do {
                    value = value * 10;
                    value += *input++ - '0';
                } while (isDigit(*input));
                if (isLetter(*input) || *input == '_' || *input == '.')
                    throw ExprError("syntax error in number.");
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
                        throw ExprError("identifier too long.");
                    buf[i++] = *input++;
                }
                if (*input == '\'') {
                    if (i >= sizeof(buf))
                        throw ExprError("identifier too long.");
                    buf[i++] = *input++;
                }
                char* ident = new char[i+1];
                memcpy(ident, buf, i);
                ident[i] = 0;
                emitToken(&list, TOK_IDENT, 0, ident);
                continue;
            }

            default:
                throw ExprError("unexpected character '%c'.", *input);
        }
    }
}

void exprFreeTokens(ExprTokenList* list)
{
    ExprToken* p = list->first;
    while (p) {
        ExprToken* t = p;
        p = p->next;
        delete[] t->text;
        delete t;
    }
}
