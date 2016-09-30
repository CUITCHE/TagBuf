//
//  arc_ptr.hpp
//  TagBufCPP
//
//  Created by hejunqiu on 2016/9/27.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef TAGObject_hpp
#define TAGObject_hpp

#include <stdio.h>
#include "types.h"
#include "TaggedPointer.h"

union ARCPointer
{
    void *ptr;
    struct {
        uintptr_t ptr : 38;
        uintptr_t count : 22;
        uintptr_t flag : 4;
    } counter;

    ARCPointer(void *ptr) :ptr(ptr) {}
};

/// ARC Container
template<typename _T>
class arc_ptr
{
    using element_type = _T;
    using element_pointer_type = _T *;

    ARCPointer *__impl__;

    uintptr_t operator &() const = delete;
public:
    explicit arc_ptr(element_pointer_type ptr);
    explicit arc_ptr(const arc_ptr&);

    ~arc_ptr();

    arc_ptr& operator = (const arc_ptr &right);


    operator bool() const { return __impl__->counter.ptr; }

    element_pointer_type operator->() { return (element_pointer_type)__impl__->counter.ptr; }
    const element_pointer_type operator->() const { return (element_pointer_type)__impl__->counter.ptr; }

    uint32_t use_count() const { return __impl__->counter.count; }

    void swap(const arc_ptr &other);
protected:
    void retain()
    {
        ++__impl__->counter.count;
    }
    void release()
    {
        if (__impl__->counter.count == 1) {
            __impl__->counter.count = 0;
            element_pointer_type p = (element_pointer_type)__impl__->ptr;
            p->release();
            delete __impl__;
        } else {
            --__impl__->counter.count;
        }
    }
private:
    void *operator new (size_t t) noexcept { return nullptr; }
    void operator delete (void *ptr) {}
    void operator delete[] (void *ptr) {}
};

template<typename _T>
arc_ptr<_T>::arc_ptr(element_pointer_type ptr) :__impl__(new ARCPointer(reinterpret_cast<element_pointer_type>((uintptr_t)ptr | TAGGED_POINTER_ARC_INIT)))
{}

template<typename _T>
arc_ptr<_T>::arc_ptr(const arc_ptr& other) :__impl__(other.__impl__)
{
    retain();
}

template<typename _T>
arc_ptr<_T>::~arc_ptr()
{
    release();
}



#endif /* TAGObject_hpp */
