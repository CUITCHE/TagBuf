//
//  tagBuf.h
//  TagBuf
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef tagBuf_h
#define tagBuf_h

#include <type_traits>

#ifndef variable_declare
#define variable_declare(name) _##name;
#endif

#include "CHTagBuf.hpp"
#ifndef Interface
#define Interface(classname) class classname final : public CHTagBuf { \
                                public:\
                                Class getClass() override;
#endif

#ifndef ClassNamed
#define ClassNamed(classname) class_##classname
#endif

#ifndef Implement
#define Implement(classname) Class classname::getClass(){ return class_getClass(ClassNamed(classname)->name); }
#endif

#ifndef tagbuf_class_check
#define tagbuf_class_check(type) static_assert(std::is_final<type>::value, "Your TagBuf class is not final class. Please use macro TagBufClass(classname) to declare TagBuf class.")
#endif

#ifndef tagbuf_class_check2
#define tagbuf_class_check2(type) static_assert(std::is_final<type>::value, "This property'type is not final class. Please use macro TagBufClass(classname) to declare TagBuf class.")
#endif

#ifndef property_pod
#define property_pod(name, pod_type) \
private: \
    pod_type variable_declare(name); \
public: \
    pod_type& name() { \
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
private: \
    object_type variable_declare(name); \
public: \
    object_type& name() { \
        tagbuf_class_check(std::remove_pointer<decltype(this)>::type); \
        tagbuf_class_check2(object_type); \
        static_assert(std::is_class<object_type>::value, "Unexcepted pod_type[not a class]"); \
        return variable_declare(name); \
    }
#endif

enum {
    /// kinds of Integers, zigzag-int, zigzag-int64. Above also support unsinged type. double value, 8 bytes, float value, 4 bytes. And about bool value, specially it only takes up 1 bit.
    CHTagBufferWriteTypeVarintFixed = 0,
    /// container, such as NSArray
    CHTagBufferWriteTypeContainer   = 1,
    /// such as NSString, c-style string, NSData
    CHTagBufferWriteTypeblobStream  = 2,
    /// data from the object which is kind of CHTagBuffer Class.
    CHTagBufferWriteTypeTagBuffer   = 3
};

enum {
    /// For check use.
    CHTagBufEncodingTypeNone   = 0,
    /// cplusplus bool.
    CHTagBufEncodingTypeBool   = 1,
    /// 8 bits, such as char.
    CHTagBufEncodingType8Bits  = 2,
    /// 16 bits, such as short
    CHTagBufEncodingType16Bits = 3,
    /// 32 bits, such as int.
    CHTagBufEncodingType32Bits = 4,
    /// 64 bits, such as long long
    CHTagBufEncodingType64Bits = 5,
    /// float value
    CHTagBufEncodingTypeFloat  = 6,
    /// double value
    CHTagBufEncodingTypeDouble = 7,
    /// NSNumber value.
    CHTagBufEncodingTypeNSNumber,
    /// NSData object.
    CHTagBufEncodingTypeNSData,
    /// NSString object.
    CHTagBufEncodingTypeNSString,
    /// NSArray object.
    CHTagBufEncodingTypeNSArray,
    /// other object, such as custom class.
    CHTagBufEncodingTypeOtherObject
};

#endif /* tagBuf_h */
