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

struct runtimeclass(CHObject)
{
    static struct method_list_t *methods()
    {
        static method_list_t method[] = {
            {.method = {0, funcAddr(&CHObject::isTaggedPointer), selector(isTaggedPointer), __Member, 0, 1, 0} },
            {.method = {0, overloadFunc(Class(*)(std::nullptr_t),CHObject::getClass), selector(getClass), __Static|__Overload, 0, 1, 0} },
            {.method = {0, overloadFunc(Class(CHObject::*)()const, &CHObject::getClass), selector(getClass), __Member|__Overload, 0, 1, 0} },
            {.method = {0, funcAddr(&CHObject::setReserved), selector(setReserved), __Member, 0, 2, 0} },
            {.method = {0, funcAddr(&CHObject::reserved), selector(reserved), __Member, 0, 1, 0} },
            {.method = {0, funcAddr(&CHObject::setObjectType), selector(setObjectType), __Member, 0, 2, 0} },
        };
        return method;
    }
};

static class_t ClassNamed(CHObject) = {
    nullptr,
    selector(CHObject),
    runtimeclass(CHObject)::methods(),
    nullptr,
    allocateCache(),
    selector(^#CHObject),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHObject), CHObject::getClass(nullptr)), sizeof(CHObject))),
    0,
    6
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
