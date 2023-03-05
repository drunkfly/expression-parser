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
#ifndef PARSER_PARSER_RESOLVE_OOP_H
#define PARSER_PARSER_RESOLVE_OOP_H

#include "parser/common.h"

class ExprResolver
{
public:
    virtual ~ExprResolver() {}
    virtual ExprCallback0 resolveFunc0(const char* name) { (void)name; return NULL; }
    virtual ExprCallback1 resolveFunc1(const char* name) { (void)name; return NULL; }
    virtual ExprCallback2 resolveFunc2(const char* name) { (void)name; return NULL; }
    virtual ExprCallback3 resolveFunc3(const char* name) { (void)name; return NULL; }
    virtual bool resolveVariable(const char* name, ExprValuePtr& result) { (void)name; (void)result; return false; }
};

class ExprEvaluator
{
public:
    ExprEvaluator(ExprValue pc) : m_pc(pc) {}
    virtual ~ExprEvaluator() {}

    ExprValue pc() const { return m_pc; }

    virtual uint8_t memByte(ExprValue address) const { (void)address; return 0; }
    virtual uint16_t memWord(ExprValue address) const { (void)address; return 0; }
    virtual uint32_t memDword(ExprValue address) const { (void)address; return 0; }

private:
    ExprValue m_pc;
};

#endif
