//
//  CHString.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/31.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "CHString.hpp"
#include "cast.hpp"
#include "TaggedPointer.h"

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

const char *CHString::objectType() const
{
    return encode(this);
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


const char *CHMutableString::objectType() const
{
    return encode(this);
}

