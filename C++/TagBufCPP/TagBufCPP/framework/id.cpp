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
#include "__tassert.h"

struct runtimeclass(CHObject)
{
    static struct method_list_t *methods()
    {
        static method_list_t method[13] = {
            {.method = {0, overloadFunc(Class(*)(std::nullptr_t),CHObject::getClass), selector(getClass), __Static} },
            {.method = {0, overloadFunc(Class(CHObject::*)()const, &CHObject::getClass), selector(getClass), __Member} },
            {.method = {0, funcAddr(&CHObject::allocateInstance), selector(allocateInstance), __Static} },
            {.method = {0, funcAddr(&CHObject::equalTo), selector(equalTo), __Member} },
            {.method = {0, funcAddr(&CHObject::description), selector(description), __Member} },
            {.method = {0, funcAddr(&CHObject::hash), selector(hash), __Member} },
            {.method = {0, funcAddr(&CHObject::superclass), selector(superclass), __Member} },

            {.method = {0, funcAddr(&CHObject::isTaggedPointer), selector(isTaggedPointer), __Member} },
            {.method = {0, funcAddr(&CHObject::setReserved), selector(setReserved), __Member} },
            {.method = {0, funcAddr(&CHObject::reserved), selector(reserved), __Member} },
            {.method = {0, funcAddr(&CHObject::setObjectType), selector(setObjectType), __Member} },
            // copy
            {.method = {0, funcAddr(&CHObject::copy), selector(copy), __Member} },
            {.method = {0, funcAddr(&CHObject::mutableCopy), selector(mutableCopy), __Member} },
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
    13
};

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

// copy
id CHObject::copy() const
{
    id cp = 0;
    if (this->respondsToSelector(selector(copyWithZone))) {
        cp = methodInvoke<id>((id)this, selector(copyWithZone), this->getClass(), nullptr);
    } else {
        __tassert(false, "Class:%s has not implement method:[copyWithZone]", object_getClassName((id)this));
    }
    return cp;
}

id CHObject::mutableCopy() const
{
    id cp = 0;
    if (this->respondsToSelector(selector(mutableCopyWithZone))) {
        cp = methodInvoke<id>((id)this, selector(mutableCopyWithZone), this->getClass(), nullptr);
    } else {
        __tassert(false, "Class:%s has not implement method:[mutableCopyWithZone]", object_getClassName((id)this));
    }
    return cp;
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

// protocol
bool CHObject::equalTo(id anObject) const
{
    return this == anObject;
}

CHString *CHObject::description() const
{
    return CHString::stringWithFormat("<%s:%p>", object_getClassName((id)this), this);
}

uint64_t CHObject::hash() const
{
    return (uint64_t)this >> 4;
}

Class CHObject::superclass() const
{
    return this->getClass()->super_class;
}

bool CHObject::isKindOfClass(Class aClass) const
{
    Class cls = 0;
    do {
        if (isTaggedPointer()) {
            uintptr_t dd = (((uintptr_t)this) & TAGGED_POINTER_DATA_FLAG);
            if (is_data(this) == TAGGED_POINTER_DATA_FLAG) {
                cls = CHData::getClass(nullptr);
                break;
            }
            if (is_string(this) == TAGGED_POINTER_STRING_FLAG) {
                cls = CHString::getClass(nullptr);
                break;
            }
            if (is_number(this) == TAGGED_POINTER_NUMBER_FLAG) {
                cls = CHNumber::getClass(nullptr);
                break;
            }
        }
        cls = this->getClass();
    } while (0);
    while (cls && cls != aClass) {
        cls = cls->super_class;
    }
    return cls != nullptr;
}

bool CHObject::isMemberOfClass(Class aClass) const
{
    return this->getClass() == aClass;
}

bool CHObject::respondsToSelector(SEL selector) const
{
    return runtime_lookup_method(this->getClass(), selector);
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
