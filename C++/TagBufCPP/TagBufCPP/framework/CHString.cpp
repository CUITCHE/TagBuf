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
            {.method = {0, overloadFunc(Class(*)(std::nullptr_t),CHString::getClass), selector(getClass), __Static|__Overload} },
            {.method = {0, overloadFunc(Class(CHString::*)()const, &CHString::getClass), selector(getClass), __Member|__Overload} },
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
    CHData::getClass(nullptr),
    selector(CHString),
    runtimeclass(CHString)::methods(),
    nullptr,
    allocateCache(),
    selector(^#CHString),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHString), CHString::getClass(nullptr)), sizeof(CHString))),
    0,
    7
};

Implement(CHString);

CHString::CHString() : CHData(0)
{}

uint32_t CHString::length() const
{
    return this->CHData::length();
}

uint32_t CHString::capacity() const
{
    return this->CHData::capacity();
}

CHString *CHString::stringWithCString(const char *str)
{
    const char *p = str;
    while (*++p) {
        continue;
    }
    return stringWithBytes(str, (uint32_t)(p - str));
}

CHString *CHString::stringWithBytes(const void *bytes, uint32_t length)
{
    CHData *data = dataWithBytes((const char *)bytes, length);
    return (CHString *)data;
}

CHString *CHString::stringWithString(const CHString *other)
{
    return (CHString *)other->duplicate();
}

struct runtimeclass(CHMutableString)
{
    static struct method_list_t *methods()
    {
        static method_list_t method[] = {
            {.method = {0, overloadFunc(Class(*)(std::nullptr_t),CHMutableString::getClass), selector(getClass), __Static|__Overload} },
            {.method = {0, overloadFunc(Class(CHMutableString::*)()const, &CHMutableString::getClass), selector(getClass), __Member|__Overload} },
            {.method = {0, overloadFunc(CHString&(CHMutableString::*)(const CHString *), &CHMutableString::appendString), selector(appendString), __Member|__Overload} },
            {.method = {0, overloadFunc(CHString&(CHMutableString::*)(const char *), &CHMutableString::appendString), selector(appendString), __Member|__Overload} },
            {.method = {0, overloadFunc(CHString&(CHMutableString::*)(const void *, uint32_t), &CHMutableString::appendString), selector(appendString), __Member|__Overload} },

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
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHMutableString), CHMutableString::getClass(nullptr)), sizeof(CHMutableString))),
    0,
    5
};

Implement(CHMutableString);

/// CHMutableString
CHString& CHMutableString::appendString(const CHString *other)
{
    appendData(other);
    return *this;
}

CHString& CHMutableString::appendString(const char *str)
{
    const char *p = str;
    while (*++p) {
        continue;
    }
    appendBytes(str, (uint32_t)(p - str));
    return *this;
}

CHString& CHMutableString::appendString(const void *bytes, uint32_t length)
{
    appendBytes((char *)bytes, length);
    return *this;
}
