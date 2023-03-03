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
#ifndef DRUNKFLY_PARSER_H
#define DRUNKFLY_PARSER_H

#include <stddef.h>
#include <stdint.h>

class Expr
{
public:
    typedef int Value;
    typedef unsigned int UnsignedValue;

    ///////////////////////////////////////////////////////

    class Error
    {
    public:
        Error(const char* message, ...);
        const char* message() const { return m_message; }

    private:
        char m_message[2048];
    };

    ///////////////////////////////////////////////////////

    struct ValuePtr
    {
        Value (*readValue)(void);
        const void* ptr;
        size_t sizeInBytes;
    };

    enum { MAX_FUNC_ARGS = 3 };
    typedef Value (*Callback0)(void);
    typedef Value (*Callback1)(Value v1);
    typedef Value (*Callback2)(Value v1, Value v2);
    typedef Value (*Callback3)(Value v1, Value v2, Value v3);

    class Resolver
    {
    public:
        virtual ~Resolver() {}
        virtual Callback0 resolveFunc0(const char* name) { (void)name; return NULL; }
        virtual Callback1 resolveFunc1(const char* name) { (void)name; return NULL; }
        virtual Callback2 resolveFunc2(const char* name) { (void)name; return NULL; }
        virtual Callback3 resolveFunc3(const char* name) { (void)name; return NULL; }
        virtual bool resolveVariable(const char* name, ValuePtr& result) { (void)name; (void)result; return false; }
    };

    ///////////////////////////////////////////////////////

    class Evaluator
    {
    public:
        Evaluator(Value pc) : m_pc(pc) {}
        virtual ~Evaluator() {}

        Value pc() const { return m_pc; }

        virtual uint8_t memByte(Value address) const { (void)address; return 0; }
        virtual uint16_t memWord(Value address) const { (void)address; return 0; }
        virtual uint32_t memDword(Value address) const { (void)address; return 0; }

    private:
        Value m_pc;
    };

    ///////////////////////////////////////////////////////

    virtual ~Expr() {}
    virtual Value evaluate(Evaluator& e) const = 0;

    static Expr* parse(const char* input, Resolver& resolver);
};

#endif
