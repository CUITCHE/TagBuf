//
//  cache.cpp
//  TagBuf
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "cache.hpp"
#include <assert.h>

static inline mask_t cache_hash(cache_key_t key, mask_t mask)
{
    return (mask_t)(key & mask);
}

cache_t *getCache(Class cls)
{
    return (cache_t *)&cls->cache;
}

cache_key_t getKey(SEL sel)
{
    assert(sel);
    return (cache_key_t)sel;
}

static inline mask_t cache_next(mask_t i, mask_t mask) {
    return (i+1) & mask;
}


// cache_t

struct bucket_t *cache_t::buckets()
{
    return _buckets;
}

mask_t cache_t::mask() const
{
    return _mask;
}

mask_t cache_t::occupied() const
{
    return _occupied;
}

void cache_t::incrementOccupied()
{
    ++_occupied;
}

void cache_t::setBucketsAndMask(struct bucket_t *newBuckets, mask_t newMask)
{
    free(_buckets);
    _buckets = newBuckets;
    _mask = newMask;
    _occupied = 0;
}

void cache_t::initializeToEmpty()
{
    free(_buckets);
    _buckets = nullptr;
    _mask = _occupied = 0;
}

mask_t cache_t::capacity()
{
    return mask() ? _mask + 1 : 0;
}

size_t cache_t::bytesForCapacity(uint32_t cap)
{
    return sizeof(bucket_t) * cap;
}

bucket_t *allocateBuckets(mask_t newCapacity)
{
    return (bucket_t *)calloc(cache_t::bytesForCapacity(newCapacity), 1);
}

void cache_t::reallocate(mask_t oldCapacity, mask_t newCapacity)
{
    bucket_t *newBuckets = allocateBuckets(newCapacity);

    assert(newCapacity > 0);
    assert((uintptr_t)(mask_t)(newCapacity-1) == newCapacity-1);

    setBucketsAndMask(newBuckets, newCapacity - 1);
}

struct bucket_t *cache_t::find(cache_key_t key)
{
    assert(key != 0);

    bucket_t *b = buckets();
    mask_t m = mask();
    mask_t begin = cache_hash(key, m);
    mask_t i = begin;
    do {
        if (b[i].key() == 0  ||  b[i].key() == key) {
            return &b[i];
        }
    } while ((i = cache_next(i, m)) != begin);
    return nullptr;
}

IMP cache_lookup_method(Class cls, SEL sel)
{
    cache_key_t key = getKey(sel);
    cache_t *cache = getCache(cls);
    bucket_t *bucket = cache->find(key);
    if (bucket) {
        return bucket->key() == key ? bucket->imp() : (IMP)0;
    }
    return (IMP)0;
}

void cache_fill_method(Class cls, SEL sel, IMP imp)
{
    cache_t *cache = getCache(cls);
    cache_key_t key = getKey(sel);

    mask_t newOccupied = cache->occupied() + 1;
    mask_t capacity = cache->capacity();
    if (newOccupied > capacity) {
        cache->reallocate(capacity, newOccupied);
    }

    bucket_t *bucket = cache->find(key);
    if (bucket->key() == 0) {
        cache->incrementOccupied();
    }
    bucket->set(key, imp);
}