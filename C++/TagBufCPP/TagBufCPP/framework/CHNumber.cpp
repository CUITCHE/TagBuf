//
//  CHNumber.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/9/16.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "CHNumber.hpp"
#include "TaggedPointer.h"
#include "TagBufDefines.h"
#include "tagBuf.hpp"
#include "runtime.hpp"

struct runtimeclass(CHNumber)
{
    static struct method_list_t *methods()
    {
        static method_list_t method[] = {
            {.method = {0, overloadFunc(Class(*)(std::nullptr_t),CHNumber::getClass), selector(getClass), __Static} },
            {.method = {0, overloadFunc(Class(CHNumber::*)()const, &CHNumber::getClass), selector(getClass), __Member|__Overload} },
            {.method = {0, funcAddr(&CHNumber::allocateInstance), selector(allocateInstance), __Static} },
        };
        return method;
    }
};

static class_t ClassNamed(CHNumber) = {
    CHObject::getClass(nullptr),
    selector(CHNumber),
    runtimeclass(CHNumber)::methods(),
    nullptr,
    allocateCache(),
    selector(^#CHNumber),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHNumber)), sizeof(CHNumber))),
    0,
    3
};

Implement(CHNumber);

struct CHNumberPrivate
{
    union {
        bool boolValue;
        char charValue;
        short shortValue;
        int intValue;
        float floatValue;
        long longValue;
        double doubleValue;
        long long longLongValue;
    } internal = {0};
};

CHNumber::CHNumber()
:CHObject()
{
}

CHNumber::~CHNumber()
{
    CHNumberPrivate *d = (CHNumberPrivate *)reserved();
    if (d) {
        delete d;
    }
}

#define d_d(obj, field) ((CHNumberPrivate *)obj->reserved())->internal.field

CHNumber::operator bool() const
{
    if (isTaggedPointer()) {
        return (bool)(((uintptr_t)this ^ TAGGED_POINTER_NUMBER_FLAG) >> 1);
    }
    return d_d(this, boolValue);
}

CHNumber::operator unsigned char() const
{
    if (isTaggedPointer()) {
        return (unsigned char)(((uintptr_t)this ^ TAGGED_POINTER_NUMBER_FLAG) >> 1);
    }
    return d_d(this, charValue);
}

CHNumber::operator char() const
{
    return this->operator unsigned char();
}

CHNumber::operator unsigned short() const
{
    if (isTaggedPointer()) {
        return (unsigned short)(((uintptr_t)this ^ TAGGED_POINTER_NUMBER_FLAG) >> 1);
    }
    return d_d(this, shortValue);
}

CHNumber::operator short() const
{
    return this->operator unsigned short();
}

CHNumber::operator unsigned int() const
{
    if (isTaggedPointer()) {
        return (unsigned int)(((uintptr_t)this ^ TAGGED_POINTER_NUMBER_FLAG) >> 1);
    }
    return d_d(this, intValue);
}

CHNumber::operator int() const
{
    return this->operator unsigned int();
}

CHNumber::operator unsigned long() const
{
    if (isTaggedPointer()) {
        return this->operator unsigned long long();
    }
    return d_d(this, longValue);
}

CHNumber::operator long() const
{
    return this->operator unsigned long();
}

CHNumber::operator unsigned long long() const
{
    if (isTaggedPointer()) {
        return (unsigned long long)(((uintptr_t)this ^ TAGGED_POINTER_NUMBER_FLAG) >> 1);
    }
    return d_d(this, longLongValue);
}

CHNumber::operator long long() const
{
    return this->operator unsigned long long();
}

CHNumber::operator float() const
{
    if (isTaggedPointer()) {
        uintptr_t ret = (uintptr_t)this;
        if (ret & 0xFFFFFFFF00000000ULL) { // may be a double value
            _double d{.dd = static_cast<uint64_t>(ret ^ TAGGED_POINTER_NUMBER_FLAG)};
            return (float)d.d;
        }
        ret = ((uintptr_t)this ^ TAGGED_POINTER_NUMBER_FLAG) >> 1;
        _float f{.ff = static_cast<uint32_t>(ret)};
        return f.f;
    }
    return d_d(this, floatValue);
}

CHNumber::operator double() const
{
    if (isTaggedPointer()) {
        uintptr_t ret = ((uintptr_t)this ^ TAGGED_POINTER_NUMBER_FLAG);
        _double d{.dd = static_cast<uint64_t>(ret)};
        return d.d;
    }
    return d_d(this, doubleValue);
}
CHNumber *numberWithValue(bool v)
{
    return numberWithValue((unsigned int)v);
}

CHNumber *numberWithValue(char v)
{
    return numberWithValue((unsigned int)v);
}

CHNumber *numberWithValue(unsigned char v)
{
    return numberWithValue((unsigned int)v);
}

CHNumber *numberWithValue(short v)
{
    return numberWithValue((unsigned int)v);
}

CHNumber *numberWithValue(unsigned short v)
{
    return numberWithValue((unsigned int)v);
}

CHNumber *numberWithValue(int v)
{
    return numberWithValue((unsigned int)v);
}

CHNumber *numberWithValue(unsigned int v)
{
    CHNumber * o = reinterpret_cast<CHNumber *>(((uintptr_t)v) << 1 | TAGGED_POINTER_NUMBER_FLAG);
    return o;
}

CHNumber *numberWithValue(float v)
{
    _float f{v};
    uintptr_t ret = f.ff;
    CHNumber * o = reinterpret_cast<CHNumber *>((ret) << 1 | TAGGED_POINTER_NUMBER_FLAG);
    return o;
}

struct CHNumberHelper
{
    inline static CHNumber *standardNumber(unsigned long long v)
    {
        CHNumber *o = new CHNumber;
        o->setReserved(new CHNumberPrivate);
        d_d(o, longLongValue) = v;
        return o;
    }
};

CHNumber *numberWithValue(double v)
{
    CHNumber *o = nullptr;
    _double d{v};
    if (v < 0) {
        o = CHNumberHelper::standardNumber(d.dd);
    } else {
        o = reinterpret_cast<CHNumber *>(d.dd | TAGGED_POINTER_NUMBER_FLAG);
    }
    return o;
}

CHNumber *numberWithValue(long v)
{
    return numberWithValue((unsigned long)v);
}

CHNumber *numberWithValue(unsigned long v)
{
    if (v & MAX_INDICATE_NUMBER) {
        CHNumber *o = CHNumberHelper::standardNumber(v);
        return o;
    }
    CHNumber * o = reinterpret_cast<CHNumber *>(((uintptr_t)v) << 1 | TAGGED_POINTER_NUMBER_FLAG);
    return o;
}

CHNumber *numberWithValue(long long v)
{
    return numberWithValue((unsigned long long)v);
}

CHNumber *numberWithValue(unsigned long long v)
{
    if (v & MAX_INDICATE_NUMBER) {
        CHNumber *o = CHNumberHelper::standardNumber(v);
        return o;
    }
    CHNumber * o = reinterpret_cast<CHNumber *>(((uintptr_t)v) << 1 | TAGGED_POINTER_NUMBER_FLAG);
    return o;
}
