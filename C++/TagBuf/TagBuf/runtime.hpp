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
#include "types.h"

#define selector(method) #method

/**
 * @author hejunqiu, 16-08-30 14:08:14
 *
 * Simple and short runtime library.
 */

enum {
    __Property  = 1,
    __Overload  = 1 << 1, // Not implement
    __Static    = 1 << 2, // Not implement. Only support 'getClass'. Custome method is not supported.
};

struct method_t
{
    uint32_t flag : 3;
    uint32_t varEncodeType : 28;
    IMP imp;
    SEL name;
};

struct method_list
{
    struct method_t method[1];
};

struct class_t final
{
    uint32_t methodCount;
    uint32_t methodOffset;
    uint32_t size; // size of class
    Class super_class;
    const char *name;
    struct method_list *methodList;
    void *cache;
};

extern void *allocateCache();

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
 * @author hejunqiu, 16-09-02 09:09:54
 *
 * Register a Class to runtime. If there has a Class which its name is equal to
 * cls's, will return false and register failed.
 *
 * @param cls        A class object. Must not be NULL.
 * @param superClass The super of cls. May be NULL if cls has not a super.
 *
 * @return true if success, otherwise is false.
 */
bool class_registerClass(Class cls, Class superClass = nullptr);

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
struct method_list *class_getPropertyList(Class cls, uint32_t *outCount);

IMP runtime_lookup_method(Class cls, SEL selector);

void *allocateInstance(Class cls);

#include "CHTagBuf.hpp"

template <typename _T, typename... Args>
_T methodInvoke(CHTagBuf *self, SEL selector, Class cls, Args&&... args)
{
    struct method_t *method = reinterpret_cast<struct method_t *>(runtime_lookup_method(cls ?: self->getClass(), selector));
    typedef _T(*Function)(Args...);
    Function f = (Function)method->imp;
    if (method->flag == __Static) {
        return f(std::forward<Args...>(args)...);
    }
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
    return f(std::forward<Args...>(args)...);
}

template<typename _T>
_T& propertyInvoke(CHTagBuf *self, SEL propertyName)
{
    return std::forward<_T&>(methodInvoke<_T&>(self, propertyName, self->getClass()));
}

extern size_t bkdr_hash(const char *str);
#endif /* runtime_hpp */
