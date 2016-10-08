//
//  runtime.hpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef runtime_hpp
#define runtime_hpp

#if defined(__OBJC__)
#error This code only support cplusplus
#endif

#if !defined(__LP64__)
#error This code only support 64 bit machine.
#endif

#include <stdio.h>
#include "CHObject.hpp"
#include <stdlib.h>
#include "CHException.hpp"

#define selector(method) #method

/**
 * @author hejunqiu, 16-08-30 14:08:14
 *
 * Simple and short runtime library.
 */

enum {
    __Member    = 1,
    __Overload  = 1 << 1,
    __Static    = 1 << 2,
};

struct method_t
{
    const char *const description; // method signature
    IMP imp;
    SEL name;
    uint32_t flag;
#ifdef __LP64__
    int space;
#endif
};

#define OFFSET(structure, member) ((int)(reinterpret_cast<uintptr_t>(&((structure *)0)->member)))

struct ivar_t
{
    const char *const ivar_name;
    const char *const ivar_type;
    int ivar_offset;
    int type_code;
};

struct method_list_t
{
    struct method_t method[1];
};

struct ivar_list_t
{
    struct ivar_t ivar[1];
};

struct class_t final
{
    Class isa;
    Class super_class;
    const char *const name;
    struct method_list_t *const methodList; // In this version, Not Implement.
    struct ivar_list_t *const ivarList;
    void *cache;
    const char *typeName;
    uint32_t size; // size of class
    uint32_t ivarCount   : 16;
    uint32_t methodCount : 16;

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

#define funcAddr(func) FunctionAddr<decltype(func)>::addrValue(func)
#define overloadFunc(type, func) FunctionAddr<type>::addrValue(func)

/**
 * @author hejunqiu, 16-08-30 21:08:00
 *
 * Obtain a Class by classname.
 *
 * @param classname Class name.
 *
 * @note Must be local variable.
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

IMP runtime_lookup_method(Class cls, SEL selector, IMP ignoreIMP = (IMP)-1);

id allocateInstance(Class cls);

template <typename _T, typename... Args>
_T methodInvoke(id self, SEL selector, Class cls, Args... args)
{
    if (!cls) {
        if (!self) {
            throwException(CHInvalidArgumentException, "Firstly. If 'cls' is null, 'self' must not be nil!");
        }
        cls = self->getClass();
    }
    if (!cls || !selector) {
        throwException(CHInvalidArgumentException, "Param must not be nil!(cls:%p\nselector:%p)", cls, selector);
    }
    struct method_t *method = reinterpret_cast<struct method_t *>(runtime_lookup_method(cls, selector));
    if (!method) {
        throwException(CHInvalidArgumentException, "<Class:%s>Unrecognized selector:%s",cls->name ,selector);
    }
    typedef _T(*Function)(Args...);
    Function f = (Function)method->imp;
    if (!self) { // This is a static-member method
        if (method->flag & __Static) {
            return f(std::forward<Args>(args)...);
        }
    }

    if (!self) {
        throwException(CHInvalidArgumentException, "self must not be nil!");
    }
    uintptr_t object_addr = (uintptr_t)self;
#ifdef __GNUC__ // GCC Compiler
#if __LP64__
    __asm__ __volatile__ (
                          "movq %0, %%rdi"
                          : "=r"(object_addr)
                          :
                          :"%rdi"
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
    return f(std::forward<Args>(args)...);
}

template<typename _T>
_T& propertyInvoke(id self, SEL propertyName)
{
    return std::forward<_T&>(methodInvoke<_T&>(self, propertyName, self->getClass()));
}

extern size_t bkdr_hash(const char *str);
extern uint64_t bkdr_hash(const char *str, uint32_t length);

int ivar_getOffset(Ivar ivar);
const char *ivar_getName(Ivar ivar);
const char *ivar_getTypeEncoding(Ivar ivar);
id object_getIvar(const id obj, Ivar ivar);
void object_setIvar(id obj, const Ivar ivar, id value);

Ivar *class_copyIvarList(const Class cls, uint32_t *outCount);
Ivar class_getIvar(const Class cls, const SEL ivarName);


IMP method_getImplementation(Method m);
SEL method_getName(Method m);
//const char *method_getTypeEncoding(Method m);
//void method_getReturnType(Method m, char *dst, size_t dst_len);
//Method class_getInstanceMethod(Class cls, SEL sel);
Method *class_copyMethodList(Class cls, unsigned int *outCount);

const char *object_getClassName(const id object);
Class object_getClass(const id object);


#endif /* runtime_hpp */
