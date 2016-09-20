//
//  id.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/9/16.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "id.hpp"
#include "CHTagBuf.hpp"
#include "TaggedPointer.h"

struct idPrivate
{
    void *obj;
    const char *CType;
};

CHObject::CHObject() {}

CHObject::~CHObject()
{
    if (!isTaggedPointer()) {
        delete d;
    }
}

bool CHObject::isTaggedPointer() const
{
    return ((uintptr_t)this & TAG_MASK);
}

CHObject::operator void *() const
{
    if (isTaggedPointer()) {
        return nullptr;
    }
    return d;
}

CHObject::operator CHTagBuf *() const
{
    if (isTaggedPointer()) {
        return nullptr;
    }
    return (CHTagBuf *)d;
}

void CHObject::setReserved(void *obj)
{
    if (!isTaggedPointer()) {
        if (!d) {
            d = new idPrivate;
        }
        d->obj = obj;
    }
}

void *CHObject::reserved() const
{
    if (isTaggedPointer()) {
        return nullptr;
    }
    return d->obj;
}

void CHObject::setObjectType(const char *type)
{
    if (!isTaggedPointer()) {
        if (!d) {
            d = new idPrivate;
        }
        d->CType = type;
    }
}

const char *CHObject::objectType() const
{
    if (isTaggedPointer()) {
        return nullptr;
    }
    return d->CType;
}

// constructor
void release(id obj)
{
#if __LP64__
    if (!obj->isTaggedPointer()) {
        delete obj;
    }
#else
    delete obj;
#endif
}
