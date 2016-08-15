//
//  CHTagBufferBuilder.m
//  TagBuf
//
//  Created by hejunqiu on 16/7/22.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import "CHTagBufferBuilder.h"
#include <objc/runtime.h>
#include <vector>
using namespace std;

#define CHTAGBUFFER_CATEGORY 1
#define CHTAGBUFFER_VERSION  1
#define WriteMemoryAPI(returnType) NS_INLINE returnType
#define ReadMemoryAPI(returnType) NS_INLINE returnType


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
 - CHTagBufferWriteTypeContainer
    internal-tag = 0, contains 32bits integer.
    internal-tag = 1, contains 64bits integer.
    internal-tag = 2, contains 32bits float.
    internal-tag = 3, contains 64bits double.
    internal-tag = 4, contains string or blob data.
    internal-tag = 5, contains custome object.
    internal-tag = 6, contains container;
 - CHTagBufferWriteTypeblobStream
    internal-tag = 0, represents NSString.
    internal-tag = 1, represents NSData.
 - CHTagBufferWriteTypeTagBuffer
    internal-tag = 1, represents object is nil.
 *
 * How about to support structure, sounds interesting.
 */

#define __tag32 0xFFE00000u
#define __tag64 0xFFE0000000000000ull

template <typename T>
NS_INLINE bool isWorthZigzag(T integer)
{
    if (sizeof(T) == 4) {
        return !(integer & __tag32);
    } else if (sizeof(T) == 8) {
        return !(integer & __tag64);
    }
    NSCAssert(sizeof(T) == 4 || sizeof(T) == 8, @"Only support 4 bytes or 8 bytes integer");
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
#define container_32bits    0
#define container_64bits    1
#define container_float     2
#define container_double    3
#define container_stream_nsstring   4
#define container_stream_nsdata     5
#define container_object    6
#define container_container 7

// if writeType is CHTagBufferWriteTypeblobStream
#define stream_nsstring     0
#define stream_nsdata       1
// if writeType is CHTagBufferWriteTypeTagBuffer
#define object_is_exists    0
#define object_is_nil       1
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

struct CHTagBufferBuilderPrivate
{
    static Protocol *objc_proto_optional;
    static Protocol *objc_proto_required;
    static Protocol *objc_proto_NSNumberInt32;
    static Protocol *objc_proto_NSNumberInt64;
    NSMutableData *buf;
    vector<char> *readBuffer = nullptr;
    union __tag_buffer_flag tag{0};
    BOOL accessToWriteTag = YES;
    CHTagBufferBuilderPrivate()
    {}

    ~CHTagBufferBuilderPrivate()
    {
        delete readBuffer;
    }

    void resetTagWithFieldNumber(uint32_t fieldNumber)
    {
        NSCAssert(fieldNumber <= 0xFFFFF, @"Too many variable. [%ud]th. The max is %d", fieldNumber, 0xFFFFF);
        tag.itag = 0;
        tag.tag.fieldNumber = fieldNumber;
    }
};

Protocol *CHTagBufferBuilderPrivate::objc_proto_optional = NSProtocolFromString(@"optional");
Protocol *CHTagBufferBuilderPrivate::objc_proto_required = NSProtocolFromString(@"required");
Protocol *CHTagBufferBuilderPrivate::objc_proto_NSNumberInt32 = NSProtocolFromString(@"NSNumberInt32");
Protocol *CHTagBufferBuilderPrivate::objc_proto_NSNumberInt64 = NSProtocolFromString(@"NSNumberInt64");


using ptr8_t  = uint8_t  *;
using ptr16_t = uint16_t *;
using ptr32_t = uint32_t *;
using ptr64_t = uint64_t *;
using ptrf_t  = float    *;
using ptrd_t  = double   *;

#ifdef _OBJC_OBJC_H_
using tag_bool = BOOL;
#elif __cplusplus
using tag_bool = bool;
#endif

struct CHInternalHelper
{
private:
    NS_INLINE NSString *getClassNameByIvar(Ivar ivar)
    {
        const char *classname = ivar_getTypeEncoding(ivar) + 2;
        uint32_t length = (uint32_t)strlen(classname) - 1;
        NSString *str = [[NSString alloc] initWithBytes:classname length:length encoding:NSUTF8StringEncoding];
        return str;
    }

    NS_INLINE NSString *getClassNameInContainerByIvar(Ivar ivar)
    {
        const char *classname = ivar_getTypeEncoding(ivar);
        const char *start = nullptr;
        while (*++classname != '<') {
            continue;
        }
        start = ++classname;
        const char *end = nullptr;
        while (*++classname != '>') {
            continue;
        }
        end = classname;
        NSString *str = [[NSString alloc] initWithBytes:start length:end - start encoding:NSUTF8StringEncoding];
        return str;
    }

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
            _buf[7] = ((integer & 0xff));
        } else {
            NSCAssert(sizeof(T) > 8, @"Larger integer not supported");
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
                NSCAssert(sizeof(pod_type) > 8, @"Larger integer not supported");
            }
        }
        return 0;
    }
public:
    WriteMemoryAPI(void) decodeFromTypeEncoding(const char *typeCoding, CHTagBufEncodingType &encodingType)
    {
        char t = typeCoding[0];
        switch (t) {
            case 'B': // C++ bool or C99 _Bool
                encodingType = CHTagBufEncodingTypeBool;
                break;
            case 'c': // char
            case 'C': // uchar
                encodingType = CHTagBufEncodingType8Bits;
                break;
            case 's': // short
            case 'S': // ushort
                encodingType = CHTagBufEncodingType16Bits;
                break;
            case 'l': // long
            case 'L': // ulong
            case 'i': // int
            case 'I': // uint
                encodingType = CHTagBufEncodingType32Bits;
                break;
            case 'f': // float
                encodingType = CHTagBufEncodingTypeFloat;
                break;
            case 'd': // double
                encodingType = CHTagBufEncodingTypeDouble;
                break;
            case 'q': // long long
            case 'Q': // ulong long
                encodingType = CHTagBufEncodingType64Bits;
                break;
            case '@': // Objective-C object
                encodingType = CHTagBufEncodingTypeObject;
                break;
            default:
                NSCAssert(NO, @"Unsupport type:%s", typeCoding);
                break;
        }
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
                zz = ((typename make_unsigned<T>::type)zz) >> 7;
            }
        }
        return i+1;
    }

    template <typename T>
    WriteMemoryAPI(T) integerToZigzag(T number)
    {
        NSCAssert(sizeof(T) == 4 || sizeof(T) == 8, @"Param must be a int32_t or uint32_t or int64_t or uint64_t!");
        return (number << 1 ) ^ (number >> (sizeof(T) * 8 - 1) );
    }

    template <typename T>
    ReadMemoryAPI(T) zigzagToInteger(T number)
    {
        static_assert(sizeof(T) == 4 || sizeof(T) == 8, "Param must be a int32_t or uint32_t or int64_t or uint64_t!");
        return (((typename make_unsigned<T>::type)number) >> 1 ) ^ -(number & 1 );
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

    WriteMemoryAPI(void) writeTag(__tag_buffer_flag__ tag, NSMutableData *buf)
    {
        Byte _buf[4] = {0};
        integerToByte(tag.itag, _buf);
        [buf appendBytes:_buf length:sizeof(_buf)];
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

    template<typename WriteType, bool shouldCompressLength = true, bool shouldWriteTag = true, bool accessToExempt = false, bool accessToModifyInternalOfTag = true>
    WriteMemoryAPI(uint32_t) writeInteger(WriteType integer, NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        static_assert(std::is_pod<WriteType>::value, "WriteType Must be a integer");
        static_assert(std::is_integral<WriteType>::value, "WriteType must be a integer");
        static_assert(!(shouldCompressLength && !shouldWriteTag && !accessToExempt), "If you want to compress the length of integer, you must set shouldWriteTag to true! Please check your code, the invoke:[writeInteger<T, true, false>(...);] is illegal! If you really want to do that, please set accessToExempt to true.");
        Byte _buf[sizeof(WriteType)] = {0};
        auto &tag = _tag.tag;
        if (sizeof(WriteType) == 1) {
            if (shouldWriteTag) {
                if (accessToModifyInternalOfTag) {
                    tag.internalTag = varint_8bits;
                }
                writeTag(_tag, buf);
            }
            integerToByte(integer, _buf);
            [buf appendBytes:_buf length:1];
            return 1;
        } else if (sizeof(WriteType) == 2) {
            if (shouldWriteTag) {
                if (accessToModifyInternalOfTag) {
                    tag.internalTag = varint_16bits;
                }
                writeTag(_tag, buf);
            }
            integerToByte(integer, _buf);
            [buf appendBytes:_buf length:2];
            return 2;
        } else {
            if (shouldCompressLength && isWorthZigzag(integer)) {
                (void) writeIntegerToZigzag(integer, tag, _buf);
                if (shouldWriteTag) {
                    if (accessToModifyInternalOfTag) {
                        tag.internalTag = sizeof(WriteType) == 4 ? varint_32bits : varint_64bits;
                    }
                    writeTag(_tag, buf);
                }
                [buf appendBytes:_buf length:tag.lengthOfZigzag];
                return tag.lengthOfZigzag;
            } else {
                integerToByte(integer, _buf);
                if (shouldWriteTag) {
                    if (accessToModifyInternalOfTag) {
                        tag.internalTag = sizeof(WriteType) == 4 ? varint_32bits : varint_64bits;
                    }
                    writeTag(_tag, buf);
                }
                [buf appendBytes:_buf length:sizeof(WriteType)];
                return sizeof(WriteType);
            }
        }
        return 0;
    }

    WriteMemoryAPI(void) writeNSNumber(NSNumber *number,
                                       NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        char objCType = number.objCType[0];
        if (objCType == 'f') { // float
            _tag.tag.internalTag = varint_float;
            writeFloat(number.floatValue, buf, _tag);
        } else if (objCType == 'd') { // double
            _tag.tag.internalTag = varint_double;
            writeDouble(number.doubleValue, buf, _tag);
        } else {
            uint64_t _int64 = number.unsignedLongLongValue;
            uint32_t _int32 = number.unsignedIntValue;
            if (_int32 == _int64) {
                writeInteger(_int32, buf, _tag);
            } else {
                writeInteger(_int64, buf, _tag);
            }
        }
    }

    WriteMemoryAPI(void) writeNSData(NSData *data, NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        uint64_t length = data.length;
        _tag.tag.internalTag = stream_nsdata;
        writeInteger<uint64_t, true, true, false, false>(length, buf, _tag);
        [buf appendData:data];
    }
    WriteMemoryAPI(void) writeNSDataInList(NSData *data, NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        writeInteger<uint64_t, false, false, false, false>(data.length, buf, _tag);
        [buf appendData:data];
    }

    /*
     c-style string's structure:
      -------------------------------------------------
     |tag(4)|length(4) may be compressed|string-data(N)|
      -------------------------------------------------
     */
    WriteMemoryAPI(void) writeCString(const char *str, NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        size_t length = str ? strlen(str) : 0;
        _tag.tag.internalTag = stream_nsstring;
        writeInteger<size_t, true, true, false, false>(length, buf, _tag);
        [buf appendBytes:str length:sizeof(char) * length];
    }
    WriteMemoryAPI(void) writeCStringInList(const char *str, NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        size_t length = str ? strlen(str) : 0;
        if (length == 0) {
            NSCAssert(NO, @"The length is 0.");
            return;
        }
        writeInteger<size_t, false, false, false, false>(length, buf, _tag);
        [buf appendBytes:str length:sizeof(char) * length];
    }
    template<bool shouldWriteTag = true>
    WriteMemoryAPI(void) writeFloat(float v, NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        char *ptr = (char *)&v;
        char _buf[4] = {ptr[3], ptr[2], ptr[1], ptr[0]};
        if (shouldWriteTag) {
            _tag.tag.internalTag = varint_float;
            writeTag(_tag, buf);
        }
        [buf appendBytes:_buf length:4];
    }
    WriteMemoryAPI(void) writeFloatInList(float v, NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        writeFloat<false>(v, buf, _tag);
    }

    template<bool shouldWriteTag = true>
    WriteMemoryAPI(void) writeDouble(double v, NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        char *ptr = (char *)&v;
        char _buf[8] = {ptr[7], ptr[6], ptr[5], ptr[4], ptr[3], ptr[2], ptr[1], ptr[0]};
        if (shouldWriteTag) {
            _tag.tag.internalTag = varint_double;
            writeTag(_tag, buf);
        }
        [buf appendBytes:_buf length:8];
    }
    WriteMemoryAPI(void) writeDoubleInList(double v, NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        writeFloat<false>(v, buf, _tag);
    }

    template <typename T>
    WriteMemoryAPI(void) writeIntegerInList(T integer, NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        static_assert(std::is_pod<T>::value == true, "Must be a pod value");
        writeInteger<T, false, false>(integer, buf, _tag);
    }

    WriteMemoryAPI(void) writeByEncodingType(CHTagBufEncodingType type, id instance, Ivar ivar, __tag_buffer_flag__ &tag, NSMutableData *buf)
    {
        if (type == CHTagBufEncodingTypeNone) {
            NSCAssert(NO, @"");
            return;
        }
        switch (type) {
            case CHTagBufEncodingTypeBool: {
                auto p = reinterpret_cast<tag_bool *>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                tag.tag.internalTag = varint_bool;
                tag.tag.placeholder12 = *p;
                writeTag(tag, buf);
            }
                break;
            case CHTagBufEncodingType8Bits: {
                auto p = reinterpret_cast<ptr8_t>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                writeInteger(*p, buf, tag);
            }
                break;
            case CHTagBufEncodingType16Bits: {
                auto p = reinterpret_cast<ptr16_t>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                writeInteger(*p, buf, tag);
            }
                break;
            case CHTagBufEncodingType32Bits: {
                auto p = reinterpret_cast<ptr32_t>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                writeInteger(*p, buf, tag);
            }
                break;
            case CHTagBufEncodingType64Bits: {
                auto p = reinterpret_cast<ptr64_t>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                writeInteger(*p, buf, tag);
            }
                break;
            case CHTagBufEncodingTypeFloat: {
                auto p = reinterpret_cast<ptrf_t>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                writeFloat(*p, buf, tag);
            }
                break;
            case CHTagBufEncodingTypeDouble: {
                auto p = reinterpret_cast<ptrd_t>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                writeDouble(*p, buf, tag);
            }
                break;
            case CHTagBufEncodingTypeObject: {
                id value = object_getIvar(instance, ivar);
                if (value) {
                    writeObjcect(value, buf, tag);
                } else {
                    // May remove blow all of code in the future.
                    tag.tag.writeType = CHTagBufferWriteTypeTagBuffer;
                    tag.tag.internalTag = object_is_nil;
                    writeTag(tag, buf);
                }
            }
                break;
            default:
                break;
        }
        
    }

    WriteMemoryAPI(void) writeObjcect(_Nonnull id obj, NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        if ([obj isKindOfClass:[NSString class]]) {
            _tag.tag.writeType = CHTagBufferWriteTypeblobStream;
            const char *_buf = [obj UTF8String];
            writeCString(_buf, buf, _tag);
        } else if ([obj isKindOfClass:[NSArray class]]) {
            _tag.tag.writeType = CHTagBufferWriteTypeContainer;
            writeContainer(obj, buf, _tag);
        } else if ([obj isKindOfClass:[NSNumber class]]) {
            _tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
            writeNSNumber(obj, buf, _tag);
        } else if ([obj isKindOfClass:[NSData class]]) {
            _tag.tag.writeType = CHTagBufferWriteTypeblobStream;
            writeNSData(obj, buf, _tag);
        } else {
            _tag.tag.writeType = CHTagBufferWriteTypeTagBuffer;
            writeTag(_tag, buf);
            _tag.itag = 0;
            writeTagBuffer(obj, buf, _tag);
        }
    }

    WriteMemoryAPI(void) writeTagBuffer(id instance, NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        Class cls = [instance class];
        if (class_isMetaClass(cls)) {
            NSCAssert(NO, @"The Class is meta-class:%@", NSStringFromClass(cls));
            return;
        }

        uint32_t count = 0;
        Ivar *list = class_copyIvarList(cls, &count);
        if (count == 0) {
            return;
        }
        Ivar *p = list;
        --count;
        CHTagBufEncodingType encodingType;
        for (uint32_t i=0; i<=count; ++i, ++p) {
            const char *typeCoding = ivar_getTypeEncoding(*p);
            decodeFromTypeEncoding(typeCoding, encodingType);
            NSLog(@"%s", ivar_getName(*p));
            _tag.itag = 0;
            _tag.tag.fieldNumber = i;
            if (i == count) {
                _tag.tag.next = 1;
            }
            writeByEncodingType(encodingType, instance, *p, _tag, buf);
        }
        free(list);
    }

    WriteMemoryAPI(void) writeContainer(NSArray *container, NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        uint64_t count = (uint64_t)container.count;
        if (!count) {
            return;
        }

        char _buf[8] = {0};
        if (isWorthZigzag(count)) {
            (void) writeIntegerToZigzag(count, _tag.tag, (Byte *)_buf);
        } else {
            integerToByte(count, (Byte *)_buf);
        }

        auto &tag = _tag.tag;
        id firstObject = container.firstObject;
        if ([firstObject isKindOfClass:[NSNumber class]]) {
            const char *type = [firstObject objCType];
            if (*type == 'f') {
                tag.internalTag = container_float;
                writeTag(_tag, buf);
                [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
                for (NSNumber *number in container) {
                    writeFloatInList(number.floatValue, buf, _tag);
                }
            } else if (*type == 'd') {
                tag.internalTag = container_double;
                writeTag(_tag, buf);
                [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
                for (NSNumber *number in container) {
                    writeDoubleInList(number.doubleValue, buf, _tag);
                }
            } else {
                if (class_conformsToProtocol(container.class, CHTagBufferBuilderPrivate::objc_proto_NSNumberInt64)) {
                    tag.internalTag = container_64bits;
                    writeTag(_tag, buf);
                    [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
                    for (NSNumber *number in container) {
                        writeIntegerInList(number.unsignedLongLongValue, buf, _tag);
                    }
                } else {
                    tag.internalTag = container_32bits;
                    writeTag(_tag, buf);
                    [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
                    for (NSNumber *number in container) {
                        writeIntegerInList(number.unsignedIntValue, buf, _tag);
                    }
                }
            }
        } else if ([firstObject isKindOfClass:[NSString class]]) {
            tag.internalTag = container_stream_nsstring;
            writeTag(_tag, buf);
            [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
            for (NSString *str in container) {
                const char *_buf = [str UTF8String];
                writeCStringInList(_buf, buf, _tag);
            }
        } else if ([firstObject isKindOfClass:[NSData class]]) {
            tag.internalTag = container_stream_nsdata;
            writeTag(_tag, buf);
            [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
            for (NSData *data in container) {
                writeNSDataInList(data, buf, _tag);
            }
        } else if ([firstObject isKindOfClass:[NSArray class]]) {
            tag.internalTag = container_container;
            writeTag(_tag, buf);
            [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
            _tag.itag = 0;
            for (NSArray *a in container) {
                writeContainer(a, buf, _tag);
            }
        } else {
            tag.internalTag = container_object;
            writeTag(_tag, buf);
            [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
            _tag.itag = 0;
            for (id obj in container) {
                writeTagBuffer(obj, buf, _tag);
            }
        }
    }

    template<typename T, bool accessToReadLength = false>
    ReadMemoryAPI(uint32_t) readInteger(T dest, __tag_detail__ tag, const char *buf)
    {
        uint32_t offset = 0;
        do {
            typedef typename std::remove_pointer<T>::type pod_type;
            if (sizeof(pod_type) < 4) {
                offset = byteToPodType(dest, buf);
                break;
            }
            if (tag.lengthCompressed) {
                offset = tag.lengthOfZigzag;
                if (accessToReadLength) {
                    uint64_t t = read_from_buffer<uint64_t>((const Byte *)buf, offset);
                    t = zigzagToInteger(t);
                    *dest = t;
                } else {
                    if (tag.internalTag == varint_32bits) {
                        uint32_t t = read_from_buffer<uint32_t>((const Byte *)buf, offset);
                        t = zigzagToInteger(t);
                        *dest = t;
                    } else if (tag.internalTag == varint_64bits) {
                        uint64_t t = read_from_buffer<uint64_t>((const Byte *)buf, offset);
                        t = zigzagToInteger(t);
                        *dest = (pod_type)t;
                    }
                }
            } else {
                offset = byteToPodType(dest, buf);
            }
        } while (0);
        return offset;
    }

    ReadMemoryAPI(uint32_t) readFloat(ptrf_t dest, const char *buf)
    {
        return byteToPodType(dest, buf);
    }

    ReadMemoryAPI(uint32_t) readDouble(ptrd_t dest, const char *buf)
    {
        return byteToPodType(dest, buf);
    }

    ReadMemoryAPI(uint32_t) readBlobStream(const char *&dest, uint32_t &length, __tag_detail__ tag, const char *buf)
    {
        uint32_t offset = 0;
        if (tag.lengthCompressed) {
            offset = tag.lengthOfZigzag;
            length = read_from_buffer<uint32_t>((Byte *)buf, offset);
            length = zigzagToInteger(length);
        } else {
            offset = byteToPodType(&length, buf);
        }
        dest = buf + offset;
        return offset + length;
    }

    ReadMemoryAPI(uint64_t) readBlobStreamInList(const char *&dest, uint64_t &length, const char *buf)
    {
        byteToPodType(&length, buf);
        dest = buf + 8;
        return 8 + length;
    }

    ReadMemoryAPI(uint32_t) readContainer(NSMutableArray **_array, __tag_detail__ tag, const char *buf, Ivar ivar)
    {
        NSCAssert(_array, @"Param [_array] can't be nil!");
        uint64_t count = 0;
        uint32_t offset = readInteger<uint64_t *, true>(&count, tag, buf);
        NSMutableArray *array = [NSMutableArray arrayWithCapacity:count ?: 10];
        *_array = array;
        switch (tag.internalTag) {
            case container_32bits: {
                uint32_t value = 0;
                for (uint64_t i=0; i<count; ++i) {
                    offset += byteToPodType(&value, buf + offset);
                    [array addObject:@(value)];
                }
            }
                break;
            case container_64bits: {
                uint64_t value = 0;
                for (uint64_t i=0; i<count; ++i) {
                    offset += byteToPodType(&value, buf + offset);
                    [array addObject:@(value)];
                }
            }
                break;
            case container_float: {
                float value = 0;
                for (uint64_t i=0; i<count; ++i) {
                    offset += byteToPodType(&value, buf + offset);
                    [array addObject:@(value)];
                }
            }
                break;
            case container_double: {
                double value = 0;
                for (uint64_t i=0; i<count; ++i) {
                    offset += byteToPodType(&value, buf + offset);
                    [array addObject:@(value)];
                }
            }
                break;
            case container_stream_nsstring: {
                NSString *str = nil;
                uint64_t length = 0;
                const char *dest = nullptr;
                for (uint64 i=0; i<count; ++i) {
                    offset += readBlobStreamInList(dest, length, buf + offset);
                    str = [[NSString alloc] initWithBytes:dest length:length encoding:NSUTF8StringEncoding];
                    [array addObject:str];
                }
            }
                break;
            case container_stream_nsdata: {
                NSData *data = nil;
                uint64_t length = 0;
                const char *dest = nullptr;
                for (uint64 i=0; i<count; ++i) {
                    offset += readBlobStreamInList(dest, length, buf + offset);
                    data = [[NSData alloc] initWithBytes:dest length:length];
                    [array addObject:data];
                }
            }
                break;
            case container_object: {
                NSString *classname = getClassNameInContainerByIvar(ivar);
                Class cls = NSClassFromString(classname);
                if (!cls) {
                    NSCAssert(NO, @"No such Class:(%@)", classname);
                    break;
                }
                id value = nil;
                for (uint64_t i=0; i<count; ++i) {
                    value = [[cls alloc] init];
                    offset += readObject(buf + offset, value);
                    [array addObject:value];
                }
            }
                break;
            case container_container: {
                NSMutableArray *embeddedArray = nil;
                __tag_buffer_flag embeddeTag = {0};
                offset += readTag(embeddeTag, buf + offset);
                for (uint64_t i=0; i<count; ++i) {
                    offset += readContainer(&embeddedArray, tag, buf + offset, ivar);
                    if (embeddedArray) {
                        [array addObject:embeddedArray];
                    }
                }
            }
                break;
            default:
                NSCAssert(NO, @"Logic error.");
                break;
        }
        return offset;
    }

    ReadMemoryAPI(uint32_t) readObject(const char *buf, id instance)
    {
        NSCAssert(instance, @"instance param must not be nil.");
        Class cls = [instance class];
        uint32_t count = 0;
        Ivar *list = class_copyIvarList(cls, &count);
        if (count == 0) {
            return 0;
        }

        union __tag_buffer_flag _tag{0};
        auto &tag = _tag.tag;
        const char *cp = buf;
        Ivar p = nullptr;
        for (;;) {
            if (tag.next) {
                break;
            }
            cp += readTag(_tag, cp);
            if (tag.fieldNumber >= count) {
                NSCAssert(NO, @"field number is overflow.");
                break;
            }
            p = list[tag.fieldNumber];
#ifdef DEBUG
            NSLog(@"%s", ivar_getName(p));
#endif
            cp += readByWriteType(tag, cp, p, instance);
        }
        free(list);
        return static_cast<uint32_t>(cp - buf);
    }

    ReadMemoryAPI(uint32_t) readByWriteType(__tag_detail__ tag, const char *buf, Ivar ivar, id instance)
    {
        uint32_t offset = 0;
        switch ((CHTagBufferWriteType)tag.writeType) {
            case CHTagBufferWriteTypeVarintFixed: {
                auto instance_ptr = reinterpret_cast<void *>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
                switch (tag.internalTag) {
                    case varint_bool: {
                        tag_bool *b = (tag_bool *)instance_ptr;
                        *b = tag.placeholder12;
                    }
                        break;
                    case varint_8bits: {
                        auto p = (ptr8_t)instance_ptr;
                        offset = byteToPodType(p, buf);
                    }
                        break;
                    case varint_16bits: {
                        auto p = (ptr16_t)instance_ptr;
                        offset = byteToPodType(p, buf);
                    }
                        break;
                    case varint_32bits: {
                        auto p = (ptr32_t)instance_ptr;
                        offset = readInteger(p, tag, buf);
                    }
                        break;
                    case varint_64bits: {
                        auto p = (ptr64_t)instance_ptr;
                        offset = readInteger(p, tag, buf);
                    }
                        break;
                    case varint_float: {
                        auto p = (ptrf_t)instance_ptr;
                        offset = readFloat(p, buf);
                    }
                        break;
                    case varint_double: {
                        auto p = (ptrd_t)instance_ptr;
                        offset = readDouble(p, buf);
                    }
                        break;
                    default:
                        NSCAssert(NO, @"Logic error.");
                        break;
                }
            }
                break;
            case CHTagBufferWriteTypeContainer: {
                NSMutableArray *array = nil;
                offset = readContainer(&array, tag, buf, ivar);
                object_setIvar(instance, ivar, array);
            }
                break;
            case CHTagBufferWriteTypeblobStream: {
                const char *dest = nullptr;
                uint32_t length = 0;
                offset = readBlobStream(dest, length, tag, buf);
                switch (tag.internalTag) {
                    case stream_nsstring: {
                        NSString *str = [[NSString alloc] initWithBytes:dest length:length encoding:NSUTF8StringEncoding];
                        object_setIvar(instance, ivar, str);
                    }
                        break;
                    case stream_nsdata: {
                        NSData *data = [[NSData alloc] initWithBytes:dest length:length];
                        object_setIvar(instance, ivar, data);
                    }
                    default:
                        break;
                }
            }
                break;
            case CHTagBufferWriteTypeTagBuffer: {
                if (tag.internalTag == object_is_exists) {
                    NSString *classname = getClassNameByIvar(ivar);
                    Class cls = NSClassFromString(classname);
                    if (!cls) {
                        @throw [NSException exceptionWithName:@"Class does not exist." reason:[NSString stringWithFormat:@"No such Class:%@", classname] userInfo:nil];
                    }
                    id value = [[cls alloc] init];
                    offset = readObject(buf, value);
                    object_setIvar(instance, ivar, value);
                }
            }
                break;
            default:
                break;
        }
        return offset;
    }
};


CHTagBufferBuilder::CHTagBufferBuilder() :_d(new struct CHTagBufferBuilderPrivate)
{}

CHTagBufferBuilder::~CHTagBufferBuilder()
{
    delete _d;
}

void CHTagBufferBuilder::startBuildingWithObject(id instance)
{
    __tag_buffer_flag__ tag = {0};
    if (!_d->buf) {
        _d->buf = [NSMutableData dataWithCapacity:4096];
    } else {
        _d->buf.length = 0;
    }

    CHInternalHelper::writeTagBuffer(instance, _d->buf, tag);
}

NSData *CHTagBufferBuilder::buildedData()
{
    return _d->buf;
}

id CHTagBufferBuilder::readTagBuffer(NSData *data, id instance)
{
    if (!data || !instance || object_isClass(instance)) {
        return nil;
    }
    if (!_d->readBuffer) {
        _d->readBuffer = new vector<char>(data.length);
    } else {
        _d->readBuffer->clear();
        _d->readBuffer->resize(data.length);
    }
    char *cp = _d->readBuffer->data();
    [data enumerateByteRangesUsingBlock:^(const void * _Nonnull bytes, NSRange byteRange, BOOL * _Nonnull stop) {
        memcpy(cp + byteRange.location, bytes, sizeof(char) * byteRange.length);
    }];

    auto final = cp + CHInternalHelper::readObject(cp, instance);
    NSCAssert(final = cp + data.length, @"");

    return instance;
}