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
    if (!is_tagged_pointer()) {
        delete d;
    }
}

CHObject::operator void *() const
{
    if (is_tagged_pointer()) {
        return nullptr;
    }
    return d;
}

CHObject::operator CHTagBuf *() const
{
    if (is_tagged_pointer()) {
        return nullptr;
    }
    return (CHTagBuf *)d;
}

void CHObject::setReserved(void *obj)
{
    if (!is_tagged_pointer()) {
        if (!d) {
            d = new idPrivate;
        }
        d->obj = obj;
    }
}

void *CHObject::reserved() const
{
    if (is_tagged_pointer()) {
        return nullptr;
    }
    return d->obj;
}

void CHObject::setObjectType(const char *type)
{
    if (!is_tagged_pointer()) {
        if (!d) {
            d = new idPrivate;
        }
        d->CType = type;
    }
}

const char *CHObject::objectType() const
{
    if (is_tagged_pointer()) {
        return nullptr;
    }
    return d->CType;
}

// constructor
void release(id obj)
{
#if __LP64__
    if (!((uintptr_t)obj & TAGGED_POINTER_FLAG)) {
        delete obj;
    }
#else
    delete obj;
#endif
}
