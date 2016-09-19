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

void *allocateInstance(Class cls)
{
    assert(cls);
    void *s = calloc(cls->size, 1);
    return s;
}

Ivar object_getIvar(CHTagBuf *self, const char *name)
{
    auto ivarList = self->getClass()->ivarList - 1;
    uint32_t count = self->getClass()->ivarCount;
    while (count --> 0) {
        if (!strcmp((++ivarList)->ivar[0].ivar_name, name)) {
            return ivarList->ivar;
        }
    }
    return nullptr;
}

#include "CHNumber.hpp"

void object_setIvar(CHTagBuf *self, const Ivar ivar, id value)
{
    if (!strcmp(ivar->ivar_type, "i")) {
        int *dst = (int *)((char *)self + ivar->ivar_offset);
        *dst = *(CHNumber *)value;
    } else if (strstr(ivar->ivar_type, "^#") == ivar->ivar_type) {
        id *dst = (id *)((char *)self + ivar->ivar_offset);
        *dst = value;
    }
}
