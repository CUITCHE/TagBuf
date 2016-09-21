//
//  CHString.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/31.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "CHString.hpp"
#include "TaggedPointer.h"

#include "TagBufDefines.h"
#include "tagBuf.hpp"
#include "runtime.hpp"

struct runtimeclass(CHString)
{
    static struct method_list_t *methods()
    {
        static method_list_t method[] = {
            {.method = {0, overloadFunc(Class(*)(std::nullptr_t),CHString::getClass), selector(getClass), __Static} },
            {.method = {0, overloadFunc(Class(CHString::*)()const, &CHString::getClass), selector(getClass), __Member|__Overload} },
            {.method = {0, funcAddr(&CHString::allocateInstance), selector(allocateInstance), __Static} },
            {.method = {0, funcAddr(&CHString::length), selector(length), __Member} },
            {.method = {0, funcAddr(&CHString::capacity), selector(length), __Member} },
            {.method = {0, funcAddr(&CHString::stringWithCString), selector(stringWithCString), __Member} },
            {.method = {0, funcAddr(&CHString::stringWithString), selector(stringWithString), __Member} },
            {.method = {0, funcAddr(&CHString::stringWithBytes), selector(stringWithBytes), __Member} },
        };
        return method;
    }
};

static class_t ClassNamed(CHString) = {
    CHMutableData::getClass(nullptr),
    selector(CHString),
    runtimeclass(CHString)::methods(),
    nullptr,
    allocateCache(),
    selector(^#CHString),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHString)), sizeof(CHString))),
    0,
    8
};

Implement(CHString);

CHString::CHString() : CHMutableData(0)
{}

uint32_t CHString::length() const
{
    if (isTaggedPointer()) {
        uint32_t len = (uint32_t)((((uintptr_t)this ^ TAGGED_POINTER_STRING_FLAG) >> 1) >> TAGGED_POINTER_STRING_LENGTH_OFFSET);
        return len;
    }
    return this->CHData::length();
}

uint32_t CHString::capacity() const
{
    if (isTaggedPointer()) {
        return -1;
    }
    return this->CHData::capacity();
}

CHString *CHString::stringWithCString(const char *str)
{
    const char *p = str;
    uint32_t length = -1;
    while (++length, *++p) {
        continue;
    }
    uintptr_t ptr = (uintptr_t)str;
    if (ptr <= MAX_CONSTANT_ADDRESS) {
        ptr |= ((uintptr_t)length << TAGGED_POINTER_STRING_LENGTH_OFFSET);
        CHString *o = reinterpret_cast<CHString *>(ptr << 1 | TAGGED_POINTER_STRING_FLAG);
        return o;
    }
    return stringWithBytes(str, length);
}

CHString *CHString::stringWithBytes(const void *bytes, uint32_t length)
{
    CHData *data = dataWithBytes((const char *)bytes, length);
    return (CHString *)data;
}

CHString *CHString::stringWithString(const CHString *other)
{
    if (other->isTaggedPointer()) {
        const char *str = reinterpret_cast<const char *>((((uintptr_t)other ^ TAGGED_POINTER_STRING_FLAG) >> 1)
                                                         & ~TAGGED_POINTER_STRING_LENGTH_MASK);
        return CHString::stringWithBytes(str, other->length());
    }
    return (CHString *)other->duplicate();
}

struct runtimeclass(CHMutableString)
{
    static struct method_list_t *methods()
    {
        static method_list_t method[] = {
            {.method = {0, overloadFunc(Class(*)(std::nullptr_t),CHMutableString::getClass), selector(getClass), __Static} },
            {.method = {0, overloadFunc(Class(CHMutableString::*)()const, &CHMutableString::getClass), selector(getClass), __Member|__Overload} },
            {.method = {0, funcAddr(&CHMutableString::allocateInstance), selector(allocateInstance), __Static} },
            {.method = {0, overloadFunc(CHMutableString&(CHMutableString::*)(const CHString *), &CHMutableString::appendString), selector(appendString), __Member|__Overload} },
            {.method = {0, overloadFunc(CHMutableString&(CHMutableString::*)(const char *), &CHMutableString::appendString), selector(appendString), __Member|__Overload} },
            {.method = {0, overloadFunc(CHMutableString&(CHMutableString::*)(const void *, uint32_t), &CHMutableString::appendString), selector(appendString), __Member|__Overload} },

        };
        return method;
    }
};

static class_t ClassNamed(CHMutableString) = {
    CHString::getClass(nullptr),
    selector(CHMutableString),
    nullptr,
    nullptr,
    allocateCache(),
    selector(^#CHMutableString),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHMutableString)), sizeof(CHMutableString))),
    0,
    6
};

Implement(CHMutableString);

/// CHMutableString
CHMutableString::CHMutableString() :CHString(){}

CHMutableString& CHMutableString::appendString(const CHString *other)
{
    appendData(other);
    return *this;
}

CHMutableString& CHMutableString::appendString(const char *str)
{
    const char *p = str;
    while (*++p) {
        continue;
    }
    appendBytes(str, (uint32_t)(p - str));
    return *this;
}

CHMutableString& CHMutableString::appendString(const void *bytes, uint32_t length)
{
    appendBytes((char *)bytes, length);
    return *this;
}
