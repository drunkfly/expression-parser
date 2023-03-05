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
#include "tests/common.h"
#include <string.h>

static const uint32_t value32 = VALUE_32;

static ExprValue fn0()
{
    return 0x7777;
}

static ExprValue fn1(ExprValue v1)
{
    return 0x8888 + v1;
}

static ExprValue fn2(ExprValue v1, ExprValue v2)
{
    return 0x9999 + v1 * v2;
}

static ExprValue fn3(ExprValue v1, ExprValue v2, ExprValue v3)
{
    return 0xaaaa + (v1 * v2 - v3);
}

static ExprValue readVar()
{
    return 0xb0b0;
}

static ExprValue readAF()
{
    return 0xbccb;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Resolver

ExprCallback0 MyResolver::resolveFunc0(const char* name)
{
    if (!strcmp(name, "fn0"))
        return fn0;
    return NULL;
}

ExprCallback1 MyResolver::resolveFunc1(const char* name)
{
    if (!strcmp(name, "fn1"))
        return fn1;
    return NULL;
}

ExprCallback2 MyResolver::resolveFunc2(const char* name)
{
    if (!strcmp(name, "fn2"))
        return fn2;
    return NULL;
}

ExprCallback3 MyResolver::resolveFunc3(const char* name)
{
    if (!strcmp(name, "fn3"))
        return fn3;
    return NULL;
}

bool MyResolver::resolveVariable(const char* name, ExprValuePtr& result)
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
    if (!strcmp(name, "var.32") || !strcmp(name, "var_32")) {
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
