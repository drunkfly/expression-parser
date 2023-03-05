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
#ifndef DRUNKFLY_PARSER_COMMON_H
#define DRUNKFLY_PARSER_COMMON_H

#include <stddef.h>
#include <stdint.h>

typedef int ExprValue;
typedef unsigned int ExprUValue;

class ExprError
{
public:
    ExprError(const char* message, ...);
    const char* message() const { return m_message; }

private:
    char m_message[2048];
};

struct ExprValuePtr
{
    ExprValue (*readValue)(void);
    const void* ptr;
    size_t sizeInBytes;
};

enum { EXPR_MAX_FUNC_ARGS = 3 };
typedef ExprValue (*ExprCallback0)(void);
typedef ExprValue (*ExprCallback1)(ExprValue v1);
typedef ExprValue (*ExprCallback2)(ExprValue v1, ExprValue v2);
typedef ExprValue (*ExprCallback3)(ExprValue v1, ExprValue v2, ExprValue v3);

#endif
