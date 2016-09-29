//
//  runtime.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "runtime.hpp"
#include "cache.hpp"
#include "CHNumber.hpp"
#include "CHString.hpp"
#include "CHData.hpp"
#include <assert.h>
#include "TaggedPointer.h"
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

uint64_t bkdr_hash(const char *str, uint32_t length)
{
    const unsigned char *s = (const unsigned char *)str;
    --s;
    uint64_t hash = 0;
    while (length --> 0) {
        hash = 31 * hash + *++s;
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
//    cls->super_class = superClass;
    return runtime_class_hashmap.emplace(cls->name, cls).second;
}

IMP runtime_lookup_method(Class cls, SEL selector)
{
    IMP imp = cache_lookup_method(cls, selector);
    if (imp == (IMP)0) {
        do {
            Class _cls = cls;
        ReTry:
            int outcount = _cls->methodCount;
            method_list_t *list = _cls->methodList;
            while (outcount --> 0) {
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
    assert(cls);
    id instance = methodInvoke<id>(nullptr, selector(allocateInstance), cls);
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
        const char *encodeType = ivar_getTypeEncoding(ivar);
        switch (encodeType[0]) {
            case 'C':
            case 'c': {
                auto v = (char *)obj + offset;
                return number(*v);
            }
            case 'i':
            case 'I': {
                auto v = (int *)((char *)obj + offset);
                return number(*v);
            }
            case 'l':
            case 'L': {
                auto v = (long *)((char *)obj + offset);
                return number(*v);
            }
            case 'q':
            case 'Q': {
                auto *v = (long long *)((char *)obj + offset);
                return number(*v);
            }
            case 'f': {
                auto v = (float *)((char *)obj + offset);
                return number(*v);
            }
            case 'd': {
                auto v = (double *)((char *)obj + offset);
                return number(*v);
            }
            case 'B': {
                auto v = (bool *)((char *)obj + offset);
                return number(*v);
            }
            case '^': {
                switch (encodeType[1]) {
                    case '#': {
                        id *idx = (id *)((char *)obj + offset);
                        return *idx;
                    }
                    case 'C':
                    case 'c': {
                        auto v = (char **)obj + offset;
                        return (id)*v;
                    }
                    case 'i':
                    case 'I': {
                        auto v = (int **)((char *)obj + offset);
                        return number(*v);
                    }
                    case 'l':
                    case 'L': {
                        auto v = (long **)((char *)obj + offset);
                        return (id)*v;
                    }
                    case 'q':
                    case 'Q': {
                        auto *v = (long long **)((char *)obj + offset);
                        return (id)*v;
                    }
                    case 'f': {
                        auto v = (float **)((char *)obj + offset);
                        return (id)*v;
                    }
                    case 'd': {
                        auto v = (double **)((char *)obj + offset);
                        return (id)*v;
                    }
                    case 'B': {
                        auto v = (bool **)((char *)obj + offset);
                        return (id)*v;
                    }
                    case 'v': {
                        auto v = (void **)((char *)obj + offset);
                        return (id)*v;
                    }
                    default:
                        break;
                }
            }
            case ':': {
                auto v = (SEL *)((char *)obj + offset);
                CHString *str = CHString::stringWithUTF8String(*v);
                return (id)str;
            }

            default:
                break;
        }
    }
    return nullptr;
}

void object_setIvar(id obj, const Ivar ivar, id value)
{
    if (obj && ivar && !obj->isTaggedPointer()) {
        const char *encodeType = ivar_getTypeEncoding(ivar);
        int offset = ivar_getOffset(ivar);
        switch (encodeType[0]) {
            case 'C':
            case 'c': {
                auto v = (char *)obj + offset;
                *v = *(CHNumber *)value;
            }
            case 'i':
            case 'I': {
                auto v = (int *)((char *)obj + offset);
                *v = *(CHNumber *)value;
            }
            case 'l':
            case 'L': {
                auto v = (long *)((char *)obj + offset);
                *v = *(CHNumber *)value;
            }
            case 'q':
            case 'Q': {
                auto *v = (long long *)((char *)obj + offset);
                *v = *(CHNumber *)value;
            }
            case 'f': {
                auto v = (float *)((char *)obj + offset);
                *v = *(CHNumber *)value;
            }
            case 'd': {
                auto v = (double *)((char *)obj + offset);
                *v = *(CHNumber *)value;
            }
            case 'B': {
                auto v = (bool *)((char *)obj + offset);
                *v = *(CHNumber *)value;
            }
            case '^': {
                switch (encodeType[1]) {
                    case '#': {
                        id *idx = (id *)((char *)obj + offset);
                        *idx = value;
                    }
                    case 'C':
                    case 'c': {
                        auto v = (char **)obj + offset;
                        *v = (char *)value;
                    }
                    case 'i':
                    case 'I': {
                        auto v = (int **)((char *)obj + offset);
                        *v = (int *)value;
                    }
                    case 'l':
                    case 'L': {
                        auto v = (long **)((char *)obj + offset);
                        *v = (long *)value;
                    }
                    case 'q':
                    case 'Q': {
                        auto *v = (long long **)((char *)obj + offset);
                        *v = (long long *)value;
                    }
                    case 'f': {
                        auto v = (float **)((char *)obj + offset);
                        *v = (float *)value;
                    }
                    case 'd': {
                        auto v = (double **)((char *)obj + offset);
                        *v = (double *)value;
                    }
                    case 'B': {
                        auto v = (bool **)((char *)obj + offset);
                        *v = (bool *)value;
                    }
                    case 'v': {
                        auto v = (void **)((char *)obj + offset);
                        *v = (void *)value;
                    }
                    default:
                        break;
                }
            }
            case ':': {
                // do nothing...
            }
                
            default:
                break;
        }
    }
}

Ivar *class_copyIvarList(const Class cls, uint32_t *outCount)
{
    if (!cls) {
        if (outCount) {
            *outCount = 0;
        }
        return nullptr;
    }
    Ivar *result = nullptr;
    ivar_list_t *ivars = cls->ivarList;
    int count = cls->ivarCount;
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

Ivar class_getIvar(const Class cls, const SEL ivarName)
{
    ivar_list_t *list = cls->ivarList - 1;
    int count = cls->ivarCount;
    Ivar ivar = 0;
    while (count --> 0) {
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

Method *class_copyMethodList(const Class cls, unsigned int *outCount)
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

const char *object_getClassName(const id object)
{
    if (object->isTaggedPointer()) {
        if (is_number(object)) {
            return CHNumber::getClass(nullptr)->name;
        } else if (is_string(object)) {
            return CHString::getClass(nullptr)->name;
        } else if (is_data(object)) {
            return CHData::getClass(nullptr)->name;
        }
        return nullptr;
    }
    return object->getClass()->name;
}

Class object_getClass(const id object)
{
    Class cls = 0;
    do {
        if (object->isTaggedPointer()) {
            if (is_data(object)) {
                cls = CHData::getClass(nullptr);
                break;
            }
            if (is_string(object)) {
                cls = CHString::getClass(nullptr);
                break;
            }
            if (is_number(object)) {
                cls = CHNumber::getClass(nullptr);
                break;
            }
        }
        cls = object->getClass();
    } while (0);
    return cls;
}
