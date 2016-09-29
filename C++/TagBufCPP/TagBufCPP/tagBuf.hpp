//
//  tagBuf.h
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef tagBuf_h
#define tagBuf_h

#include <type_traits>

#ifndef variable_declare
#define variable_declare(name) name;
#endif

#include "CHTagBuf.hpp"

#ifndef runtimeclass
#define runtimeclass(classname) __##classname##RuntimeClass__
#endif

#ifndef __SUPPORTRUNTIME__
#define __SUPPORTRUNTIME__(classname) friend struct runtimeclass(classname)
#endif

#ifndef Interface
#define Interface(classname) class classname final : public CHTagBuf { \
                                friend struct CHTagBufFactor; \
                                __SUPPORTRUNTIME__(classname);\
                                protected:\
                                    classname() :CHTagBuf(){};\
                                    ~classname();\
                                public:\
                                    Class getClass() const; \
                                    static Class getClass(std::nullptr_t);\
                                private:\
                                    static id allocateInstance();
#endif

#ifndef ClassNamed
#define ClassNamed(classname) class_##classname
#endif

#ifndef IvarListNamed
#define IvarListNamed(classname) ivar_list_##classname
#endif

#ifndef Implement
#define Implement(classname) Class classname::getClass() const { if (isTaggedPointer()) { return object_getClass((id)this);} return &ClassNamed(classname); } \
    Class classname::getClass(std::nullptr_t) { return &ClassNamed(classname); } \
    id classname::allocateInstance() { return new classname; }
#endif

#ifndef ImplementTagBuf
#define ImplementTagBuf(classname) Implement(classname) \
    classname::~classname() { desctructor(this); }
#endif

#define RUNTIMECLASS(classname) struct runtimeclass(classname) { \
static struct method_list_t *methods() {\
static method_list_t method[] = {{.method = {0, overloadFunc(Class(*)(std::nullptr_t),classname::getClass), selector(getClass), __Static} },\
{.method = {0, overloadFunc(Class(classname::*)()const, &classname::getClass), selector(getClass), __Member} },\
{.method = {0, funcAddr(&classname::allocateInstance), selector(allocateInstance), __Static} },\
};\
return method;}}

#ifndef tagbuf_class_check
#define tagbuf_class_check(type) static_assert(std::is_final<type>::value, "Your TagBuf class is not final class. Please use macro TagBufClass(classname) to declare TagBuf class.")
#endif

#ifndef tagbuf_class_check2
#define tagbuf_class_check2(type) static_assert(std::is_final<type>::value, "This property'type is not final TagBuf class. Please use macro TagBufClass(classname) to declare TagBuf class.")
#endif

#ifndef property_pod
#define property_pod(name, pod_type) \
public: \
    pod_type variable_declare(name); \
private: \
    pod_type& private_##name() { \
        tagbuf_class_check(std::remove_pointer<decltype(this)>::type); \
        static_assert(std::is_pod<pod_type>::value, "Unexcepted pod_type"); \
        static_assert(!std::is_pointer<pod_type>::value, "Unexcepted pod_type[*]"); \
        static_assert(!std::is_class<pod_type>::value, "Unexcepted pod_type[class]"); \
        static_assert(!std::is_array<pod_type>::value, "Unexcepted pod_type[array]"); \
        static_assert(!std::is_void<pod_type>::value, "Unexcepted pod_type[void]"); \
        static_assert(!std::is_union<pod_type>::value, "Unexcepted pod_type[union]"); \
        static_assert(!std::is_function<pod_type>::value, "Unexcepted pod_type[function]"); \
        return variable_declare(name); \
    }
#endif

#ifndef property_class
#define property_class(name, object_type) \
public: \
    object_type variable_declare(name); \
private: \
    object_type& private##_name() { \
        using r_p_type = std::remove_pointer_t<object_type>;\
        tagbuf_class_check(std::remove_pointer_t<decltype(this)>); \
        tagbuf_class_check2(r_p_type); \
        static_assert(std::is_class<r_p_type>::value, "Unexcepted pod_type[not a class]"); \
        static_assert(std::is_pointer<object_type>::value && std::is_base_of<CHObject, r_p_type>::value, "Unexcepted pod_type[*]. Use kind of id type"); \
        return variable_declare(name); \
    }
#endif

#endif /* tagBuf_h */
