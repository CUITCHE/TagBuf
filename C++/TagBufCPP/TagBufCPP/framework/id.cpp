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
#include "TagBufDefines.h"
#include "tagBuf.hpp"
#include "runtime.hpp"

static class_t ClassNamed(CHObject) = {
    nullptr,
    selector(CHObject),
    nullptr,
    nullptr,
    allocateCache(),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHObject), CHObject::getClass(nullptr)), sizeof(CHObject))),
    0,
    selector(^#CHTagBuf)
};

Implement(CHObject);

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
        return (void *)this;
    }
    return d;
}

CHObject::operator CHTagBuf *() const
{
    if (isTaggedPointer()) {
        return (CHTagBuf *)this;
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

#include "CHNumber.hpp"
const char *CHObject::objectType() const
{
    if (isTaggedPointer()) {
        if (is_number(this)) {
            return CHNumber::getClass(nullptr)->typeName;
        }
        return nullptr;
    }
    return d->CType;
}

// destructor
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
