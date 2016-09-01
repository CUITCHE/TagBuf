//
//  CHString.cpp
//  TagBuf
//
//  Created by hejunqiu on 16/8/31.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "CHString.hpp"
#include <stdlib.h>
#include <memory>

struct CHStringPrivate
{
    uint32_t canbeFreed : 1;
    uint32_t length     : 31;
    void *buffer;
    void tryToFree()
    {
        if (canbeFreed) {
            free(buffer);
        }
    }
};

CHString::CHString()
:d((struct CHStringPrivate *)calloc(sizeof(struct CHString), 1))
{
    ;
}

CHString::~CHString()
{
    d->tryToFree();
    free(d);
}

CHString::CHString(CHString&& other)
:CHString()
{
    memcpy(d, other.d, sizeof(struct CHStringPrivate));
    memset(other.d, 0, sizeof(struct CHStringPrivate));
}

void CHString::operator=(CHString &&right)
{
    d->tryToFree();
    memcpy(d, right.d, sizeof(struct CHStringPrivate));
    memset(right.d, 0, sizeof(struct CHStringPrivate));
}

CHString&& CHString::stringWithConstantString(const char *str, uint32_t length /*= -1*/)
{
    return stringWithString((char *)str, length);
}

CHString&& CHString::stringWithString(char *str, uint32_t length, bool freeWhenDone /*= true*/)
{
    CHString string;
    string.d->buffer = (void *)str;
    string.d->length = length;
    string.d->canbeFreed = freeWhenDone;
    return std::move(string);
}