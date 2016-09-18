//
//  CHNumber.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/9/16.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "CHNumber.hpp"
#include "types.h"

struct CHNumberPrivate
{
    volatile uint32_t valueType = 0;
    volatile union {
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

CHNumber::operator unsigned char() const
{
    if (is_tagged_pointer()) {
        return (unsigned char)(((uintptr_t)this ^ TAGGED_POINTER_FLAG) >> 1);
    }
    return d_d(this, charValue);
}

CHNumber::operator char() const
{
    return this->operator unsigned char();
}

CHNumber::operator unsigned short() const
{
    if (is_tagged_pointer()) {
        return (unsigned short)(((uintptr_t)this ^ TAGGED_POINTER_FLAG) >> 1);
    }
    return d_d(this, shortValue);
}

CHNumber::operator short() const
{
    return this->operator unsigned short();
}

CHNumber::operator unsigned int() const
{
    if (is_tagged_pointer()) {
        return (unsigned int)(((uintptr_t)this ^ TAGGED_POINTER_FLAG) >> 1);
    }
    return d_d(this, intValue);
}

CHNumber::operator int() const
{
    return this->operator unsigned int();
}

CHNumber::operator unsigned long() const
{
#ifdef __LP64__
    if (is_tagged_pointer()) {
        return this->operator unsigned long long();
    }
    return d_d(this, longValue);
#else
    if (is_tagged_pointer()) {
        return (unsigned long)(((uintptr_t)this ^ TAGGED_POINTER_FLAG) >> 1);
    }
    return d_d(this, longValue);
#endif
}

CHNumber::operator long() const
{
    return this->operator unsigned long();
}

CHNumber::operator unsigned long long() const
{
    if (is_tagged_pointer()) {
        return (unsigned long long)(((uintptr_t)this ^ TAGGED_POINTER_FLAG) >> 1);
    }
    return d_d(this, longLongValue);
}

CHNumber::operator long long() const
{
    return this->operator unsigned long long();
}

CHNumber::operator float() const
{
    if (is_tagged_pointer()) {
        uintptr_t ret = ((uintptr_t)this ^ TAGGED_POINTER_FLAG) >> 1;
        char *p = (char *)&ret;
        float v = 0;
        char *target = (char *)&v;
        target[0] = p[3];
        target[1] = p[2];
        target[2] = p[1];
        target[3] = p[0];
        return v;
    }
    return d_d(this, floatValue);
}

CHNumber::operator double() const
{
    return d_d(this, doubleValue);
}

CHNumber *objectWithValue(char v)
{
    CHNumber * o = reinterpret_cast<CHNumber *>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

CHNumber *objectWithValue(unsigned char v)
{
    CHNumber * o = reinterpret_cast<CHNumber *>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

CHNumber *objectWithValue(short v)
{
    CHNumber * o = reinterpret_cast<CHNumber *>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

CHNumber *objectWithValue(unsigned short v)
{
    CHNumber * o = reinterpret_cast<CHNumber *>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

CHNumber *objectWithValue(int v)
{
    CHNumber * o = reinterpret_cast<CHNumber *>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

CHNumber *objectWithValue(unsigned int v)
{
    CHNumber * o = reinterpret_cast<CHNumber *>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

CHNumber *objectWithValue(float v)
{
    char *p = (char *)(&v);
    uintptr_t ret = 0;
    char *target = (char *)&ret;
    target[0] = p[3];
    target[1] = p[2];
    target[2] = p[1];
    target[3] = p[0];
    CHNumber * o = reinterpret_cast<CHNumber *>((ret)<<1 | TAGGED_POINTER_FLAG);
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

CHNumber *objectWithValue(double v)
{
    CHNumber * o = reinterpret_cast<CHNumber *>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

CHNumber *objectWithValue(long v)
{
#ifdef __LP64__
    if (v & MAX_INDICATE_FLAG) {
        CHNumber *o = CHNumberHelper::standardNumber(v);
        return o;
    }
    CHNumber * o = reinterpret_cast<CHNumber *>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
#else // bug, same blow.
    CHNumber * o = reinterpret_cast<CHNumber *>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
#endif
}

CHNumber *objectWithValue(unsigned long v)
{
#ifdef __LP64__
    if (v & MAX_INDICATE_FLAG) {
        CHNumber *o = CHNumberHelper::standardNumber(v);
        return o;
    }
    CHNumber * o = reinterpret_cast<CHNumber *>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
#else
    CHNumber * o = reinterpret_cast<CHNumber *>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
#endif
}

CHNumber *objectWithValue(long long v)
{
    if (v & MAX_INDICATE_FLAG) {
        CHNumber *o = CHNumberHelper::standardNumber(v);
        return o;
    }
    CHNumber * o = reinterpret_cast<CHNumber *>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

CHNumber *objectWithValue(unsigned long long v)
{
    if (v & MAX_INDICATE_FLAG) {
        CHNumber *o = CHNumberHelper::standardNumber(v);
        return o;
    }
    CHNumber * o = reinterpret_cast<CHNumber *>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}
