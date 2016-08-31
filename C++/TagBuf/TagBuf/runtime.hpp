//
//  runtime.hpp
//  TagBuf
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef runtime_hpp
#define runtime_hpp

#if defined(__OBJC__)
#error This code just support cplusplus
#endif

#include <stdio.h>

using Class = struct class_t *;
using id = Class;

#define selector(method) #method

/**
 * @author hejunqiu, 16-08-30 14:08:14
 *
 * Simple and short runtime library.
 */

enum {
    __Property  = 1,
    __Overload  = 1 << 1, // Not implement
    __Static    = 1 << 2, // Not implement
};

using uint32_t = unsigned int;
using uint16_t = unsigned short;

#if __LP64__
using IMP = unsigned long;
using uintptr_t = unsigned long;
#else
using IMP = unsigned int;
using uintptr_t = unsigned int;
#endif

using SEL = const char *;

struct property_t
{
    uint32_t varEncodeType;
    IMP imp;
    SEL name;
};

struct property_list
{
    struct property_t method[1];
};

struct class_t
{
    uint32_t property_count;
    Class super_class;
    const char *name;
    struct property_list *methodList;
    void *cache;
};

#ifndef _func_addr_
template<typename Func>
union _func_addr {
    uintptr_t addr;
    Func func;
};
#define _func_addr_
#endif // !_func_addr

template<typename Function> struct FunctionAddr
{
    static uintptr_t addrValue(Function f) {
        _func_addr<Function> addr;
        addr.func = f;
        return addr.addr;
    }
    static Function funcValue(uintptr_t _addr) {
        _func_addr<Function> addr;
        addr.addr = _addr;
        return addr.func;
    }
};

/**
 * @author hejunqiu, 16-08-30 21:08:00
 *
 * Obtain a Class by classname.
 *
 * @param classname Class name.
 *
 * @return NULL if classname is NULL or the class is not exites.
 */
Class class_getClass(const char *classname);

/**
 * @author hejunqiu, 16-08-30 22:08:03
 *
 * Register a Class to runtime. If there has a Class which its name is equal to 
 * cls's, will return false and register failed.
 *
 * @param cls A class object. Must not be NULL.
 *
 * @return true if success, otherwise is false.
 */
bool class_registerClass(Class cls);

/**
 * @author hejunqiu, 16-08-30 23:08:19
 *
 * Obtain property list of the cls and count of property.
 *
 * @param cls      Class structure.
 * @param outCount Return property count.
 *
 * @return Return a pointer to property_list pointer.
 */
struct property_list *class_getPropertyList(Class cls, uint32_t *outCount);

IMP runtime_lookup_property(Class cls, SEL selector);

#include "CHTagBuf.hpp"
template<typename _T>
_T& propertyInvoke(CHTagBuf *self, SEL propertyName)
{
    typedef _T&(*Function)();
    Function f = (Function)runtime_lookup_property(self->getClass(), propertyName);
    uintptr_t object_addr = (uintptr_t)self;
#ifdef __GNUC__ // GCC Compiler
#if __LP64__
    __asm__ __volatile__ (
                          "movq %0, %%rdi"
                          : "=r"(object_addr)
                          );
#else
    __asm__ __volatile__ (
                          "movl %0, %%rdi"
                          : "=r"(object_addr)
                          );
#endif
#elif _MSC_VER_
#if __LP64__
#error cl.exe compiler don't support 64 bit inline asm.
#endif
    __asm {
        mov ecx, object_addr
    }
#endif
    return f();
}

enum {
    CHTagBufObjectDetailTypeNone,
    /// NSNumber<NSNumberBoolean> value. Or NSArray<NSNumberBoolean> value.
    CHTagBufObjectDetailTypeNSNumberBoolean,
    /// NSNumber<NSNumberInt8> value. Or NSArray<NSNumberInt8> value.
    CHTagBufObjectDetailTypeNSNumber8BitsInteger,
    /// NSNumber<NSNumberInt16> value. Or NSArray<NSNumberInt16> value.
    CHTagBufObjectDetailTypeNSNumber16BitsInteger,
    /// NSNumber<NSNumberInt32> value. Or NSArray<NSNumberInt32> value.
    CHTagBufObjectDetailTypeNSNumber32BitsInteger,
    /// NSNumber<NSNumberInt64> value. Or NSArray<NSNumberInt64> value.
    CHTagBufObjectDetailTypeNSNumber64BitsInteger,
    /// NSNumber<NSNumberFloat> value. Or NSArray<NSNumberFloat> value.
    CHTagBufObjectDetailTypeNSNumberFloat,
    /// NSNumber<NSNumberDouble> value. Or NSArray<NSNumberDouble> value.
    CHTagBufObjectDetailTypeNSNumberDouble,
    CHTagBufObjectDetailTypeNSArrayNSData,
    CHTagBufObjectDetailTypeNSArrayNSString,
    CHTagBufObjectDetailTypeNSArrayNSArray,
    CHTagBufObjectDetailTypeOtherObject
};

#endif /* runtime_hpp */
