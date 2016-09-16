//
//  id.cpp
//  TagBuf
//
//  Created by hejunqiu on 16/9/16.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "id.hpp"
#include "types.h"
#include "CHTagBuf.hpp"

struct idPrivate
{
    union {
        char charValue;
        short shortValue;
        int intValue;
        float floatValue;
        long longValue;
        long long longLongValue;
        double doubleValue;
        void *object;
    }internal;
    uint32_t valueType;
};

id_t::id_t()
:d(new idPrivate)
{
    ;
}

#define TAGGED_POINTER_FLAG 0xB000000000000001
#define is_tagged_pointer() ((uintptr_t)this & TAGGED_POINTER_FLAG)

id_t::~id_t()
{
    if (!((uintptr_t)this & TAGGED_POINTER_FLAG)) {
        delete d;
    }
}

id_t::operator unsigned char() const
{
    if (is_tagged_pointer()) {
        return (unsigned char)(((uintptr_t)this ^ TAGGED_POINTER_FLAG) >> 1);
    }
    return d->internal.charValue;
}

id_t::operator char() const
{
    return this->operator unsigned char();
}

id_t::operator unsigned short() const
{
    if (is_tagged_pointer()) {
        return (unsigned short)(((uintptr_t)this ^ TAGGED_POINTER_FLAG) >> 1);
    }
    return d->internal.shortValue;
}

id_t::operator short() const
{
    return this->operator unsigned short();
}

id_t::operator unsigned int() const
{
    if (is_tagged_pointer()) {
        return (unsigned int)(((uintptr_t)this ^ TAGGED_POINTER_FLAG) >> 1);
    }
    return d->internal.intValue;
}

id_t::operator int() const
{
    return this->operator unsigned int();
}

id_t::operator unsigned long() const
{
    if (is_tagged_pointer()) {
        return (unsigned long)(((uintptr_t)this ^ TAGGED_POINTER_FLAG) >> 1);
    }
    return d->internal.longValue;
}

id_t::operator long() const
{
    return this->operator unsigned long();
}

id_t::operator unsigned long long() const
{
    if (is_tagged_pointer()) {
        return (unsigned long long)(((uintptr_t)this ^ TAGGED_POINTER_FLAG) >> 1);
    }
    return d->internal.longValue;
}

id_t::operator long long() const
{
    return this->operator unsigned long long();
}

id_t::operator float() const
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
    return d->internal.floatValue;
}

id_t::operator double() const
{
    return d->internal.doubleValue;
}

id_t::operator void *() const
{
    return d->internal.object;
}

id_t::operator CHTagBuf *() const
{
    return (CHTagBuf *)d->internal.object;
}

// constructor
id objectWithValue(char v)
{
    id o = reinterpret_cast<id>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

id objectWithValue(unsigned char v)
{
    id o = reinterpret_cast<id>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

id objectWithValue(short v)
{
    id o = reinterpret_cast<id>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

id objectWithValue(unsigned short v)
{
    id o = reinterpret_cast<id>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

id objectWithValue(int v)
{
    id o = reinterpret_cast<id>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

id objectWithValue(unsigned int v)
{
    id o = reinterpret_cast<id>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

id objectWithValue(float v)
{
    char *p = (char *)(&v);
    uintptr_t ret = 0;
    char *target = (char *)&ret;
    target[0] = p[3];
    target[1] = p[2];
    target[2] = p[1];
    target[3] = p[0];
    id o = reinterpret_cast<id>((ret)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

id objectWithValue(double v)
{
    id o = reinterpret_cast<id>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

id objectWithValue(long v)
{
    id o = reinterpret_cast<id>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}
id objectWithValue(unsigned long v)
{
    id o = reinterpret_cast<id>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

id objectWithValue(long long v)
{
    id o = reinterpret_cast<id>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

id objectWithValue(unsigned long long v)
{
    id o = reinterpret_cast<id>(((uintptr_t)v)<<1 | TAGGED_POINTER_FLAG);
    return o;
}

void release_id(id obj)
{
    if (!((uintptr_t)obj & TAGGED_POINTER_FLAG)) {
        delete obj;
    }
}
