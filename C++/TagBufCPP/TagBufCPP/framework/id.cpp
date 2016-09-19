//
//  id.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/9/16.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "id.hpp"
#include "types.h"
#include "CHTagBuf.hpp"
#include "TaggedPointer.h"

struct idPrivate
{
    void *obj;
    const char *CType;
};

CHObject::CHObject()
:d(new idPrivate)
{
    ;
}

CHObject::~CHObject()
{
    delete d;
}

CHObject::operator void *() const
{
    return d;
}

CHObject::operator CHTagBuf *() const
{
    return (CHTagBuf *)d;
}

void CHObject::setReserved(void *obj)
{
    d->obj = obj;
}

void *CHObject::reserved() const
{
    return d->obj;
}

void CHObject::setObjectType(const char *type)
{
    d->CType = type;
}

const char *CHObject::objectType() const
{
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
