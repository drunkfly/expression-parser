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
#ifndef DRUNKFLY_TESTS_COMMON_H
#define DRUNKFLY_TESTS_COMMON_H

#include "parser/resolve_oop.h"

#define PC_VALUE 0xcafebabe
#define VALUE_32 0x0abacada

class MyResolver : public ExprResolver
{
public:
    ExprCallback0 resolveFunc0(const char* name);
    ExprCallback1 resolveFunc1(const char* name);
    ExprCallback2 resolveFunc2(const char* name);
    ExprCallback3 resolveFunc3(const char* name);
    bool resolveVariable(const char* name, ExprValuePtr& result);
};

class MyEvaluator : public ExprEvaluator
{
public:
    MyEvaluator() : ExprEvaluator(PC_VALUE) {}

    uint8_t memByte(ExprValue address) const { return address + 0x10; }
    uint16_t memWord(ExprValue address) const { return address - 0xb0; }
    uint32_t memDword(ExprValue address) const { return address * 4; }
};

#endif
