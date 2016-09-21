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
#include "cast.hpp"

struct runtimeclass(CHObject)
{
    static struct method_list_t *methods()
    {
        static method_list_t method[] = {
            {.method = {0, funcAddr(&CHObject::isTaggedPointer), selector(isTaggedPointer), __Member} },
            {.method = {0, overloadFunc(Class(*)(std::nullptr_t),CHObject::getClass), selector(getClass), __Static} },
            {.method = {0, funcAddr(&CHObject::allocateInstance), selector(allocateInstance), __Static} },
            {.method = {0, overloadFunc(Class(CHObject::*)()const, &CHObject::getClass), selector(getClass), __Member|__Overload} },
            {.method = {0, funcAddr(&CHObject::setReserved), selector(setReserved), __Member} },
            {.method = {0, funcAddr(&CHObject::reserved), selector(reserved), __Member} },
            {.method = {0, funcAddr(&CHObject::setObjectType), selector(setObjectType), __Member} },
        };
        return method;
    }
    static struct ivar_list_t *ivars()
    {
        static ivar_list_t ivar[] = {
            {.ivar[0] = {.ivar_name = selector(d), .ivar_type = encode<char *>(), .ivar_offset = OFFSET(CHObject, d)}}
        };
        return ivar;
    }
};

static class_t ClassNamed(CHObject) = {
    nullptr,
    selector(CHObject),
    runtimeclass(CHObject)::methods(),
    runtimeclass(CHObject)::ivars(),
    allocateCache(),
    selector(^#CHObject),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHObject)), sizeof(CHObject))),
    1,
    7
};

//Implement(CHObject);

Class CHObject::getClass() const
{
    return &class_CHObject;
}

Class CHObject::getClass(std::nullptr_t)
{
    return &class_CHObject;
}

struct idPrivate
{
    void *obj = 0;
    const char *CType = 0;
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
#include "CHString.hpp"
#include "CHData.hpp"
const char *CHObject::objectType() const
{
    if (isTaggedPointer()) {
        if (is_number(this)) {
            return encode<CHNumber *>();
        } else if (is_string(this)) {
            return encode<CHString *>();
        } else if (is_data(this)) {
            return encode<CHData *>();
        }
        return nullptr;
    }
    if (d) {
        return d->CType ?: this->getClass()->typeName;
    } else {
        return this->getClass()->typeName;
    }
}

id CHObject::allocateInstance()
{
    return new CHObject();
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
