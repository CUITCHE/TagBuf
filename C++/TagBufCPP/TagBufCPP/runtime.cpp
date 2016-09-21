//
//  runtime.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "runtime.hpp"
#include "cache.hpp"
#include <assert.h>

#include <unordered_map>

using namespace::std;

size_t bkdr_hash(const char *str)
{
    const unsigned char *s = (const unsigned char *)str;
    --s;
    size_t hash = 0;
    while (*++s) {
        hash = 31 * hash + *s;
    }
    return hash & (0x7FFFFFFFFFFFFFFFULL);
}

struct _hash_
{
    size_t operator()(const char *str) const
    { return bkdr_hash(str); }
};

/**
 * @author hejunqiu, 16-08-30 21:08:48
 *
 * The key [const char *] whose lifetime must be long.
 */
static unordered_map<const char *, Class/*, struct _hash_*/> runtime_class_hashmap;

extern void *allocateCache()
{
    return new struct cache_t();
}

Class class_getClass(const char *classname)
{
    do {
        if (!classname) {
            break;
        }
        auto iter = runtime_class_hashmap.find(classname);
        if (iter != runtime_class_hashmap.end()) {
            return iter->second;
        }
    } while (0);
    return nullptr;
}

bool class_registerClass(Class cls, Class superClass)
{
    cls->super_class = superClass;
    return runtime_class_hashmap.emplace(cls->name, cls).second;
}

IMP runtime_lookup_method(Class cls, SEL selector)
{
    IMP imp = cache_lookup_method(cls, selector);
    if (imp == (IMP)0) {
        do {
            Class _cls = cls;
        ReTry:
            uint32_t outcount = _cls->methodCount;
            method_list_t *list = _cls->methodList;
            while (outcount-->0) {
                if (!strcmp(list->method[0].name, selector)) {
                    imp = reinterpret_cast<IMP>(&list->method);
                    break;
                }
                ++list;
            }
            if (imp != (IMP)0) {
                cache_fill_method(cls, selector, imp);
                break;
            }
            _cls = _cls->super_class;
            if (!_cls) {
                break;
            } else {
                goto ReTry;
            }
        } while (0);
    }
    return imp;
}

id allocateInstance(Class cls)
{
    struct idPrivate
    {
        void *obj;
        const char *CType;
    };
    assert(cls);
    id instance = methodInvoke<id>(nullptr, selector(allocateInstance), cls);
    Ivar ivar = class_getIvar(CHObject::getClass(nullptr), selector(d));
    int offset = ivar_getOffset(ivar);
    struct idPrivate **s = (struct idPrivate **)((char *)instance + offset);
    (*s)->CType = cls->typeName;
    return instance;
}

int ivar_getOffset(Ivar ivar)
{
    return ivar->ivar_offset;
}

const char *ivar_getName(Ivar ivar)
{
    return ivar->ivar_name;
}

const char *ivar_getTypeEncoding(Ivar ivar)
{
    return ivar->ivar_type;
}

id object_getIvar(id obj, Ivar ivar)
{
    if (obj && ivar && !obj->isTaggedPointer()) {
        int offset = ivar_getOffset(ivar);
        id *idx = (id *)((char *)obj + offset);
        return *idx;
    }
    return nullptr;
}

#include "CHNumber.hpp"

void object_setIvar(id obj, const Ivar ivar, id value)
{
    if (!strcmp(ivar->ivar_type, "i")) {
        int *dst = (int *)((char *)obj + ivar->ivar_offset);
        *dst = *(CHNumber *)value;
    } else if (strstr(ivar->ivar_type, "^#") == ivar->ivar_type) {
        id *dst = (id *)((char *)obj + ivar->ivar_offset);
        *dst = value;
    }
}

Ivar *class_copyIvarList(Class cls, uint32_t *outCount)
{
    if (!cls) {
        if (outCount) {
            *outCount = 0;
        }
        return nullptr;
    }
    Ivar *result = nullptr;
    ivar_list_t *ivars = cls->ivarList;
    uint32_t count = cls->ivarCount;
    if (outCount) {
        *outCount = count;
    }
    if (ivars && count) {
        result = (Ivar *)malloc(sizeof(Ivar) * (count + 1));
        result[count] = nullptr;
        Ivar *dst = result - 1;
        ivar_list_t *p = ivars - 1;
        while (count --> 0) {
            *++dst = (++p)->ivar;
        }
    }
    return result;
}

Ivar class_getIvar(Class cls, SEL ivarName)
{
    ivar_list_t *list = cls->ivarList - 1;
    uint32_t count = cls->ivarCount;
    Ivar ivar = 0;
    while (count-->0) {
        if (!strcmp((++list)->ivar[0].ivar_name, ivarName)) {
            ivar = list->ivar;
        }
    }
    return ivar;
}

IMP method_getImplementation(Method m)
{
    return m ? m->imp : (IMP)nullptr;
}

SEL method_getName(Method m)
{
    return m ? m->name : nullptr;
}

Method *class_copyMethodList(Class cls, unsigned int *outCount)
{
    if (!cls) {
        if (outCount) {
            *outCount = 0;
        }
        return nullptr;
    }
    Method *result = nullptr;
    method_list_t *methods = cls->methodList;
    int count = cls->methodCount;
    if (outCount) {
        *outCount = count;
    }
    if (methods && count) {
        result = (Method *)malloc(sizeof(Method) * (count + 1));
        result[count] = nullptr;
        Method *dst = result - 1;
        method_list_t *p = methods - 1;
        while (count --> 0) {
            *++dst = (++p)->method;
        }
    }
    return result;
}
