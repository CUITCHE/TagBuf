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
template<typename _Tp>
class arc_ptr
{
    using element_type = _Tp;
    using element_pointer_type = _Tp *;

    ARCPointer *__impl__;
public:
    explicit arc_ptr(element_pointer_type ptr);
    explicit arc_ptr(const arc_ptr&);
    explicit arc_ptr() :__impl__(nullptr) {}

    ~arc_ptr();

    arc_ptr& operator = (const arc_ptr &right);

    template<typename _Yp>
    arc_ptr<_Tp>& operator = (const arc_ptr<_Yp> &right);


    operator bool() const { return __impl__->counter.ptr; }

    element_pointer_type operator->() { return (element_pointer_type)__impl__->counter.ptr; }
    const element_pointer_type operator->() const { return (element_pointer_type)__impl__->counter.ptr; }

    uint32_t use_count() const { return __impl__->counter.count; }

    template<typename _R>
    void swap(const arc_ptr<_R> &other);

    uintptr_t operator &() const;
protected:
    void retain();
    void release();
private:
    void *operator new (size_t t) noexcept { return nullptr; }
    void operator delete (void *ptr) {}
    void operator delete[] (void *ptr) {}
};

template<typename _Tp>
arc_ptr<_Tp>::arc_ptr(element_pointer_type ptr) :__impl__(new ARCPointer(reinterpret_cast<element_pointer_type>((uintptr_t)ptr | TAGGED_POINTER_ARC_INIT)))
{}

template<typename _Tp>
arc_ptr<_Tp>::arc_ptr(const arc_ptr& other) :__impl__(other.__impl__)
{
    retain();
}

template<typename _Tp>
arc_ptr<_Tp>::~arc_ptr()
{
    release();
}

template<typename _Tp> template<typename _R>
void arc_ptr<_Tp>::swap(const arc_ptr<_R> &other)
{
    ARCPointer **arcp = reinterpret_cast<ARCPointer **>(&other);
    std::swap(__impl__, *arcp);
}

template<typename _Tp>
uintptr_t arc_ptr<_Tp>::operator&() const
{
    return (uintptr_t)&__impl__;
}

template<typename _Tp>
arc_ptr<_Tp>& arc_ptr<_Tp>::operator=(const arc_ptr &right)
{
    arc_ptr(right).swap(*this);
    return *this;
}

template<typename _Tp>
void arc_ptr<_Tp>::retain()
{
    if (__impl__) {
        ++__impl__->counter.count;
    }
}

template<typename _Tp>
void arc_ptr<_Tp>::release()
{
    if (__impl__) {
        if (__impl__->counter.count == 1) {
            element_pointer_type p = reinterpret_cast<element_pointer_type>(__impl__->counter.ptr);
            p->release();
            delete __impl__;
        } else {
            --__impl__->counter.count;
        }
    }
}

template<typename _Tp>
template<typename _Yp>
arc_ptr<_Tp>& arc_ptr<_Tp>::operator=(const arc_ptr<_Yp> &right)
{
    static_assert(!std::is_abstract<_Yp>::value, "_Yp is a abstract class.");
    static_assert(std::is_base_of<_Tp, _Yp>::value, "Class _Tp and _Yp are not in same inhert chain or a reasonable conversion.");
    arc_ptr<_Yp>(right).swap(*this);
    return *this;
}
#endif /* TAGObject_hpp */
