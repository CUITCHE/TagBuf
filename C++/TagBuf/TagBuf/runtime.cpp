//
//  runtime.cpp
//  TagBuf
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

struct method_list *class_getPropertyList(Class cls, uint32_t *outCount)
{
    assert(cls);
    assert(outCount);
    *outCount = cls->methodOffset;
    return cls->methodList;
}

struct method_list *class_getMethodList(Class cls, uint32_t *outCount)
{
    assert(cls);
    assert(outCount);
    *outCount = cls->methodCount - cls->methodOffset;
    return cls->methodList + cls->methodOffset;
}

IMP runtime_lookup_method(Class cls, SEL selector)
{
    IMP imp = cache_lookup_method(cls, selector);
    if (imp == (IMP)0) {
        uint32_t outcount = 0;
        method_list *list = class_getPropertyList(cls, &outcount);
    Retry:
        while (outcount-->0) {
            if (!strcmp(list->method[0].name, selector)) {
                imp = reinterpret_cast<IMP>(&list->method);
                break;
            }
            ++list;
        }
        if (imp != (IMP)0) {
            cache_fill_method(cls, selector, imp);
        } else {
            list = class_getMethodList(cls, &outcount);
            goto Retry;
        }
    }
    return imp;
}

void *allocateInstance(Class cls)
{
    assert(cls);
    void *s = calloc(cls->size, 1);
    return s;
}