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

static class_t ClassNamed(CHString) = {
    CHData::getClass(nullptr),
    selector(CHString),
    nullptr,
    nullptr,
    allocateCache(),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHString), CHString::getClass(nullptr)), sizeof(CHString))),
    0,
    selector(^#CHString)
};

Implement(CHString);

static class_t ClassNamed(CHMutableString) = {
    CHString::getClass(nullptr),
    selector(CHMutableString),
    nullptr,
    nullptr,
    allocateCache(),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHMutableString), CHMutableString::getClass(nullptr)), sizeof(CHMutableString))),
    0,
    selector(^#CHMutableString)
};

Implement(CHMutableString);

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
