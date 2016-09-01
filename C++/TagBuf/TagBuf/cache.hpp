//
//  cache.hpp
//  TagBuf
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef cache_hpp
#define cache_hpp

#include "runtime.hpp"

#if __LP64__
typedef uint32_t mask_t;  // x86_64 & arm64 asm are less efficient with 16-bits
#else
typedef uint16_t mask_t;
#endif
typedef uintptr_t cache_key_t;

struct bucket_t
{
private:
    cache_key_t _key;
    IMP _imp; // is method's address.
public:
    inline cache_key_t key() const { return _key; }
    inline IMP imp() const { return (IMP)_imp; }
    inline void setKey(cache_key_t newKey) { _key = newKey; }
    inline void setImp(IMP newImp) { _imp = newImp; }

    void set(cache_key_t newKey, IMP newImp) { setKey(newKey); setImp(newImp); }
};

struct cache_t
{
private:
    struct bucket_t *_buckets = nullptr;
    mask_t _mask = 0;
    mask_t _occupied = 0;
public:
    struct bucket_t *buckets();
    mask_t mask() const;
    mask_t occupied() const;
    void incrementOccupied();
    void setBucketsAndMask(struct bucket_t *newBuckets, mask_t newMask);
    void initializeToEmpty();

    mask_t capacity();
    static size_t bytesForCapacity(uint32_t cap);
    void reallocate(mask_t oldCapacity, mask_t newCapacity);

    struct bucket_t * find(cache_key_t key);
};

IMP cache_lookup_method(Class cls, SEL sel);
void cache_fill_method(Class cls, SEL sel, IMP imp);

#endif /* cache_hpp */
