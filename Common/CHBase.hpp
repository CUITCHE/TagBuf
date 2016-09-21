//
//  CHBase.hpp
//  TagBuf
//
//  Created by hejunqiu on 16/9/9.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef CHBase_hpp
#define CHBase_hpp

/*
 * What is tagBuffer structure:
 -------------------------------
 | tag | data | tag | data | ... |
 -------------------------------
 tag+data  = 1 tagBuf.
 tagBuffer = N tagBuf.
 *
 * tag structure:
 * total size: 4bytes.
 -------------------------------------------------------------------------------------
 |31|30|29|28|27|26|25|24|23|21|21|20|19|18|17|16|15|14|13|12|11|10|9|8|7|6|5|4|3|2|1|0|
 -------------------------------------------------------------------------------------
 ---------
 |31~13 bit| {field number}
 ---------

 ---------
 |12~08 bit| {tagBuf reserved}
 ---------
 [08~10] represents the length of zigzag.
 [11] = 1 represents that the bytes of string or data's length has been compressed by zigzag.
 [12] = 1 represents that the blob stream has been encrypted, otherwise has not.
 = 1 represents that there store a bool [true] value, 0 represents [false] value when
 write tpye is CHTagBufferWriteTypeVarintFixed and internal-tag is varint_bool.
 = 1 represents that container contains number value(bool, 8~64bits, values
 by CHTagBufferWriteTypeVarintFixed's internal values).

 ---------
 |07~04 bit| {tagBuf internal-tag}
 ---------
 Especially, if [07] = 0 represents there is a tagBuf at next, 1 represents not.

 ---------
 |03~00 bit| {tagBuf write type}
 ---------
 If value = CHTagBufferWriteType for internal-tag[04~06]
 - CHTagBufferWriteTypeVarintFixed
 internal-tag = 0, represents 8bits integer.
 internal-tag = 1, represents 16bits integer.
 internal-tag = 2, represents 32bits integer.
 internal-tag = 3, represents 64bits integer.
 internal-tag = 4, represents 32bits float.
 internal-tag = 5, represents 64bits double.
 internal-tag = 6, represents bool or BOOL.
 internal-tag = 7, represents object is nil if this is a NSNumber object.
 - CHTagBufferWriteTypeContainer
 internal-tag = 0, contains 32bits integer.
 internal-tag = 1, contains 64bits integer.
 internal-tag = 2, contains 32bits float.
 internal-tag = 3, contains 64bits double.
 internal-tag = 4, contains string or blob data.
 internal-tag = 5, contains custome object.
 internal-tag = 6, contains container;
 internal-tag = 7, represents object is nil
 - CHTagBufferWriteTypeblobStream
 internal-tag = 0, represents NSString.
 internal-tag = 1, represents NSData.
 internal-tag = 7, represents object is nil
 - CHTagBufferWriteTypeTagBuffer
 internal-tag = 7, represents object is nil
 *
 * How about to support structure, sounds interesting.
 */

#include <vector>

using std::vector;

template <typename IOType>
struct CHBufferIOType {
    typedef void(*CHBufferWriteFunction)(IOType buf, uint32_t length);
};

#if defined(__OBJC__)
    #import <Foundation/Foundation.h>
    #define CHCAssert(cond, ...) NSCAssert(cond, ##__VA_ARGS__)
    #define CH_INLINE NS_INLINE
#elif defined(__cplusplus)
    #include <assert.h>
    #define CHCAssert(cond, ...) assert(cond)
    #define CH_INLINE static inline
class CHMutableData;
class CHData;
#else
#error This code only supports C++ or Objective-C.
#endif

#define WriteMemoryAPI(returnType) CH_INLINE returnType
#define ReadMemoryAPI(returnType) CH_INLINE returnType

#define __tag32 0xFFE00000u
#define __tag64 0xFFE0000000000000ull

template <typename T>
CH_INLINE bool isWorthZigzag(T integer)
{
    if (sizeof(T) == 4) {
        return !(integer & __tag32);
    } else if (sizeof(T) == 8) {
        return !(integer & __tag64);
    }
    CHCAssert(sizeof(T) == 4 || sizeof(T) == 8, @"Only support 4 bytes or 8 bytes integer");
    return false;
}

using __tag_output_type__ = uint32_t;

union __tag_buffer_flag {
    __tag_output_type__ itag;
    struct __tag_buffer_structure {
        uint32_t writeType          : 03; // [00~03]
        // if writeType is CHTagBufferWriteTypeVarintFixed
#define varint_8bits    0
#define varint_16bits   1
#define varint_32bits   2
#define varint_64bits   3
#define varint_float    4
#define varint_double   5
#define varint_bool     6

        // if writeType is CHTagBufferWriteTypeContainer
#define container_none      0
#define container_stream_nsstring   1
#define container_stream_nsdata     2
#define container_object    3
#define container_container 4


        // if writeType is CHTagBufferWriteTypeblobStream
#define stream_nsstring     0
#define stream_nsdata       1

        // if writeType is CHTagBufferWriteTypeTagBuffer

        // common defined
#define object_is_nil       7
        uint32_t internalTag        : 03; // [04~06]
        uint32_t next               : 01; // [07]
        uint32_t lengthOfZigzag     : 03; // [08~10]
        uint32_t lengthCompressed   : 01; // [11]
        uint32_t placeholder12      : 01; // [12]
        uint32_t fieldNumber        : 20; // [13~31]
    } tag;
};

using __tag_buffer_flag__ = __tag_buffer_flag;
using __tag_detail__ = __tag_buffer_flag::__tag_buffer_structure;

#if defined(__OBJC__)
struct CHTagBufferBuilderPrivate
{

    static NSCharacterSet *protocolCharacterSet;
    NSMutableData *writeBuf = nullptr;
    vector<char> *extraBuffer;
    vector<char> *readBuffer = nullptr;

    CHTagBufferBuilderPrivate()
    :extraBuffer(new vector<char>) {}

    ~CHTagBufferBuilderPrivate() {
        delete readBuffer;
        delete extraBuffer;
    }
};
NSCharacterSet *CHTagBufferBuilderPrivate::protocolCharacterSet = [NSCharacterSet characterSetWithCharactersInString:@"\"<"];
#elif defined(__cplusplus)
class CHObject;
extern void release(CHObject *);

struct TagBufferBuilderPrivate
{
    CHMutableData *writeBuffer = 0;
    const CHData *readBuffer = 0;

    ~TagBufferBuilderPrivate()
    {
        release(writeBuffer);
    }
};
#endif

const char *const kTagBufferPropertyKey = nullptr;

using ptr8_t  = unsigned char  *;
using ptr16_t = unsigned short *;
using ptr32_t = unsigned int *;
using ptr64_t = unsigned long long *;
using ptrf_t  = float    *;
using ptrd_t  = double   *;
using Byte    = unsigned char;

template<typename T>
WriteMemoryAPI(void) integerToByte(T integer, Byte *_buf)
{
    static_assert(std::is_pod<T>::value == true, "T Must be a integer");
    static_assert(std::is_integral<T>::value, "T must be a integer");
    if (sizeof(T) == 1) {
        _buf[0] = integer;
    } else if (sizeof(T) == 2) {
        _buf[0] = ((integer & 0xff00) >> 8);
        _buf[1] = ((integer & 0x00ff));
    } else if (sizeof(T) == 4) {
        _buf[0] = ((integer & 0xff000000) >> 24);
        _buf[1] = ((integer & 0x00ff0000) >> 16);
        _buf[2] = ((integer & 0x0000ff00) >>  8);
        _buf[3] = ((integer & 0x000000ff));
    } else if (sizeof(T) == 8) {
        _buf[0] = ((integer & 0xff00000000000000ll) >> 56);
        _buf[1] = ((integer & 0x00ff000000000000ll) >> 48);
        _buf[2] = ((integer & 0x0000ff0000000000ll) >> 40);
        _buf[3] = ((integer & 0x000000ff00000000ll) >> 32);
        _buf[4] = ((integer & 0x00000000ff000000ll) >> 24);
        _buf[5] = ((integer & 0x0000000000ff0000ll) >> 16);
        _buf[6] = ((integer & 0x000000000000ff00ll) >>  8);
        _buf[7] = ((integer & 0x00000000000000ffll));
    } else {
        CHCAssert(sizeof(T) > 8, @"Larger integer not supported");
    }
}

template <typename T>
ReadMemoryAPI(uint32_t) byteToPodType(T pod_ptr,
                                      const char *_buf)
{
    static_assert(std::is_pointer<T>::value == true, "T Must be a pointer");
    typedef typename std::remove_pointer<T>::type pod_type;
    if (sizeof(pod_type) == 1) {
        *pod_ptr = _buf[0];
        return 1;
    } else if (sizeof(pod_type) == 2) {
        auto ptr = (char *)(pod_ptr);
        ptr[1] = _buf[0];
        ptr[0] = _buf[1];
        return 2;
    } else {
        auto ptr = (char *)pod_ptr;
        if (sizeof(pod_type) == 4) {
            ptr[3] = _buf[0];
            ptr[2] = _buf[1];
            ptr[1] = _buf[2];
            ptr[0] = _buf[3];
            return 4;
        } else if (sizeof(pod_type) == 8) {
            ptr[7] = _buf[0];
            ptr[6] = _buf[1];
            ptr[5] = _buf[2];
            ptr[4] = _buf[3];
            ptr[3] = _buf[4];
            ptr[2] = _buf[5];
            ptr[1] = _buf[6];
            ptr[0] = _buf[7];
            return 8;
        } else {
            CHCAssert(sizeof(pod_type) > 8, @"Larger integer not supported");
        }
    }
    return 0;
}

template <typename T>
WriteMemoryAPI(int) write_to_buffer(T zz, Byte* buf)
{
    int i = 0;
    for (; i<sizeof(zz); ++i) {
        if (!(zz & (~ 0x7f))) {
            buf[i] = (Byte)zz;
            break;
        } else {
            buf[i] = (Byte)((zz & 0x7f) | 0x80);
            zz = ((typename std::make_unsigned<T>::type)zz) >> 7;
        }
    }
    return i+1;
}

template <typename T>
WriteMemoryAPI(T) integerToZigzag(T number)
{
    CHCAssert(sizeof(T) == 4 || sizeof(T) == 8, @"Param must be a int32_t or uint32_t or int64_t or uint64_t!");
    return (number << 1 ) ^ (number >> (sizeof(T) * 8 - 1) );
}

template <typename T>
ReadMemoryAPI(T) zigzagToInteger(T number)
{
    static_assert(sizeof(T) == 4 || sizeof(T) == 8, "Param must be a int32_t or uint32_t or int64_t or uint64_t!");
    return (((typename std::make_unsigned<T>::type)number) >> 1 ) ^ -(number & 1 );
}

template <typename T>
ReadMemoryAPI(T) read_from_buffer(const Byte* buf, int max_size)
{
    T ret = 0;
    Byte n = 0;
    for (int i=0, offset = 0; i<max_size; ++i, offset += 7) {
        n = buf[i];
        if ((n & 0x80 ) != 0x80) {
            ret |= (n << offset);
            break;
        } else {
            ret |= ((n & 0x7f) << offset);
        }
    }
    return ret;
}

template<typename IOType>
WriteMemoryAPI(void) writeTag(__tag_buffer_flag__ tag, IOType buf)
{
    Byte _buf[4] = {0};
    integerToByte(tag.itag, _buf);
#if defined(__OBJC__)
    [buf appendBytes:_buf length:sizeof(_buf)];
#elif defined(__cplusplus)
    buf->appendBytes(_buf, sizeof(_buf));
#endif
}

ReadMemoryAPI(uint32_t) readTag(__tag_buffer_flag__ &tag, const char *buf)
{
    // 暂时不考虑tag的压缩
    byteToPodType(&tag.itag, buf);
    return 4;
}

template<typename T>
WriteMemoryAPI(uint32_t) writeIntegerToZigzag(T value, __tag_detail__ &tag, Byte *buf)
{
    value = integerToZigzag(value);
    tag.lengthCompressed = 1;
    tag.lengthOfZigzag = write_to_buffer(value, buf);
    return tag.lengthOfZigzag;
}

#endif /* CHBase_hpp */





