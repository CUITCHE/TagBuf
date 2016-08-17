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
#import "tagBuf.h"
#import "CHClassProperty.h"
#import "TagBufBitset.h"
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
#define container_stream_nsstring   0
#define container_stream_nsdata     1
#define container_object    2
#define container_container 3

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
    static NSCharacterSet *protocolCharacterSet;
    NSMutableData *buf;
    vector<char> *readBuffer = nullptr;
    union __tag_buffer_flag tag{0};
    BOOL accessToWriteTag = YES;
    CHTagBufferBuilderPrivate()
    {
    }

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

NSCharacterSet *CHTagBufferBuilderPrivate::protocolCharacterSet = [NSCharacterSet characterSetWithCharactersInString:@"\"<"];

const char *const kTagBufferPropertyKey = nullptr;

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

#ifdef DEBUG
#define debug_condition(cond, relate) cond relate
#else
#define debug_condition(cond, relate)
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
        while (debug_condition(*classname, &&) *++classname != '<') {
            continue;
        }
        NSCAssert(*classname, @"No such class. Info:%s", ivar_getTypeEncoding(ivar));
        start = ++classname;
        while (debug_condition(*classname, &&) *++classname != '>') {
            continue;
        }
        NSCAssert(*classname, @"No such class. Info:%s", ivar_getTypeEncoding(ivar));

        NSString *str = [[NSString alloc] initWithBytes:start length:classname - start encoding:NSUTF8StringEncoding];
        return str;
    }

    NS_INLINE CHTagBufObjectDetailType getNumberProtocolByPropertyAttributeType(NSString *typePart, NSScanner * _Nullable scanner)
    {
        if (typePart.length < 12) { // min length of 'NSNumberInt8'
            return CHTagBufObjectDetailTypeNSNumber32BitsInteger;
        }
        auto character = [typePart characterAtIndex:8];
        switch (character) {
            case 'I': {
                NSUInteger number = [typePart characterAtIndex:typePart.length-1];
                switch (number) {
                    case '2':
                        NSCAssert([typePart isEqualToString:@"NSNumberInt32"], @"Unexcepted protocol type:%@. May protocol NSNumberInt32 be your wanted?", typePart);
                        return CHTagBufObjectDetailTypeNSNumber32BitsInteger;
                    case '4':
                        NSCAssert([typePart isEqualToString:@"NSNumberInt64"], @"Unexcepted protocol type:%@. May protocol NSNumberInt64 be your wanted?", typePart);
                        return CHTagBufObjectDetailTypeNSNumber64BitsInteger;
                    case '6':
                        NSCAssert([typePart isEqualToString:@"NSNumberInt16"], @"Unexcepted protocol type:%@. May protocol NSNumberInt16 be your wanted?", typePart);
                        return CHTagBufObjectDetailTypeNSNumber16BitsInteger;
                    case '8':
                        NSCAssert([typePart isEqualToString:@"NSNumberInt8"], @"Unexcepted protocol type:%@. May protocol NSNumberInt8 be your wanted?", typePart);
                        return CHTagBufObjectDetailTypeNSNumber8BitsInteger;
                    default:  return CHTagBufObjectDetailTypeNSNumber32BitsInteger;
                }
            }
            case 'B':
                NSCAssert([typePart isEqualToString:@"NSNumberBoolean"], @"Unexcepted protocol type:%@. May protocol NSNumberBoolean be your wanted?", typePart);
                return CHTagBufObjectDetailTypeNSNumberBoolean;
            case 'F':
                NSCAssert([typePart isEqualToString:@"NSNumberFloat"], @"Unexcepted protocol type:%@. May protocol NSNumberFloat be your wanted?", typePart);
                return CHTagBufObjectDetailTypeNSNumberFloat;
            case 'D':
                NSCAssert([typePart isEqualToString:@"NSNumberDouble"], @"Unexcepted protocol type:%@. May protocol NSNumberDouble be your wanted?", typePart);
                return CHTagBufObjectDetailTypeNSNumberDouble;
            default:  return CHTagBufObjectDetailTypeNSNumber32BitsInteger;
        }
    }

    NS_INLINE CHTagBufObjectDetailType getNSArrayProtocolByPropertyAttributeType(NSString *typePart)
    {
        if (typePart.length < 13) { // min length of 'NSArrayNSData'
            return CHTagBufObjectDetailTypeOtherObject;
        }
        auto character = [typePart characterAtIndex:9];
        switch (character) {
            case 'A':
                NSCAssert([typePart isEqualToString:@"NSArrayNSArray"], @"Unexcepted protocol type:%@. May protocol NSArrayNSArray be your wanted?", typePart);
                return CHTagBufObjectDetailTypeNSArrayNSArray;
            case 'D':
                NSCAssert([typePart isEqualToString:@"NSArrayNSData"], @"Unexcepted protocol type:%@. May protocol NSArrayNSData be your wanted?", typePart);
                return CHTagBufObjectDetailTypeNSArrayNSData;
            case 'S':
                NSCAssert([typePart isEqualToString:@"NSArrayNSString"], @"Unexcepted protocol type:%@. May protocol NSArrayNSString be your wanted?", typePart);
                return CHTagBufObjectDetailTypeNSArrayNSString;
            default:  return CHTagBufObjectDetailTypeOtherObject;
        }
    }

    NS_INLINE CHTagBufObjectDetailType getNSArrayContainerTypeOnRuntime(id containedFirstObject, CHClassProperty *property)
    {
        if (property.detailType != CHTagBufObjectDetailTypeNone &&
            property.detailType != CHTagBufObjectDetailTypeNSArrayNSArray) {
            return property.detailType;
        }
        CHTagBufObjectDetailType type = CHTagBufObjectDetailTypeNone;
        if ([containedFirstObject isKindOfClass:[NSNumber class]]) {
            const char *t = [containedFirstObject objCType];
            switch (*t) {
                case 'd':
                    type = CHTagBufObjectDetailTypeNSNumberFloat;
                    break;
                case 'f':
                    type = CHTagBufObjectDetailTypeNSNumberDouble;
                    break;
                default:
                    type = CHTagBufObjectDetailTypeNSNumber32BitsInteger;
                    break;
            }
        } else if ([containedFirstObject isKindOfClass:[NSString class]]) {
            type = CHTagBufObjectDetailTypeNSArrayNSString;
        } else if ([containedFirstObject isKindOfClass:[NSData class]]) {
            type = CHTagBufObjectDetailTypeNSArrayNSData;
        } else if ([containedFirstObject isKindOfClass:[NSArray class]]) {
            type = CHTagBufObjectDetailTypeNSArrayNSArray;
        }
        return type;
    }

    NS_INLINE NSArray<CHClassProperty *>* getProperties(Class cls)
    {
        NSCAssert(cls, @"Class is nil.");
        NSMutableArray<CHClassProperty *> *properties = [NSMutableArray array];
        uint32_t count = 0;
        objc_property_t *pList = class_copyPropertyList(cls, &count);
        objc_property_t *p = pList;
        NSScanner *scanner = nil;
        for (uint32_t i=0; i<count; ++i) {
            objc_property_t property = *p++;
            const char *attrs = property_getAttributes(property);
            NSString *propertyAttributes = @(attrs);
            NSArray<NSString *> *attributeItems = [propertyAttributes componentsSeparatedByString:@","];
            if ([attributeItems containsObject:@"R"]) {
                continue;
            }
            CHClassProperty *cp = [CHClassProperty new];
            cp.propertyName = @(property_getName(property));

            scanner = [NSScanner scannerWithString:attributeItems.firstObject];
            NSString *propertyType = nil;
            if ([scanner scanString:@"T@\"" intoString:nil]) {
                [scanner scanUpToCharactersFromSet:CHTagBufferBuilderPrivate::protocolCharacterSet
                                       intoString:&propertyType];
                cp.propertyClassType = NSClassFromString(propertyType);
                /// If if-else structure has been over 10, change to switch-case structure by hash code.
                if ([propertyType isEqualToString:@"NSString"]) {
                    cp.encodingType = CHTagBufEncodingTypeNSString;
                } else if ([propertyType isEqualToString:@"NSArray"]) {
                    cp.encodingType = CHTagBufEncodingTypeNSArray;
                } else if ([propertyType isEqualToString:@"NSData"]) {
                    cp.encodingType = CHTagBufEncodingTypeNSData;
                } else if ([propertyType isEqualToString:@"NSNumber"]) {
                    cp.encodingType = CHTagBufEncodingTypeNSNumber;
                } else {
                    cp.encodingType = CHTagBufEncodingTypeOtherObject;
                }
                
                NSString *protocolName = nil;
                while ([scanner scanString:@"<" intoString:nil]) {
                    [scanner scanUpToString:@">" intoString:&protocolName];
                    if ([protocolName isEqualToString:@"optional"]) {
                        cp.isOptional = YES;
                    } else if ([protocolName containsString:@"ignore"]) {
                        cp.isIgnore = YES;
                    }  else if ([protocolName containsString:@"NSNumber"]) {
                        cp.detailType = getNumberProtocolByPropertyAttributeType(protocolName, nil);
                    } else if ([protocolName containsString:@"NSArray"]) {
                        cp.detailType = getNSArrayProtocolByPropertyAttributeType(protocolName);
                    } else {
                        cp.protocol = protocolName;
                        cp.detailType = CHTagBufObjectDetailTypeOtherObject;
                    }
                    [scanner scanString:@">" intoString:nil];
                }
            } else {
                CHTagBufEncodingType encodingType = CHTagBufEncodingTypeNone;
                decodeFromTypeEncoding(attrs + 1, encodingType);
                cp.encodingType = encodingType;
            }
            cp.fieldNumber = i;
            [properties addObject:cp];
        }
        free(pList);
        return properties;
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
                NSCAssert(NO, @"The judge of Objective-C has move to getProperties(...)");
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

#pragma mark - Write APIs

    WriteMemoryAPI(void) writeTagBuffer(id instance, NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        Class cls = [instance class];
        if (class_isMetaClass(cls)) {
            NSCAssert(NO, @"The Class is meta-class:%@", NSStringFromClass(cls));
            return;
        }

        NSArray<CHClassProperty *> *properties = objc_getAssociatedObject(cls, &kTagBufferPropertyKey);
        if (!properties) {
            properties = getProperties(cls);
            objc_setAssociatedObject(cls, &kTagBufferPropertyKey, properties, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        }

        NSUInteger count = properties.count;
        for (CHClassProperty *property in properties) {
            _tag.itag = 0;
            _tag.tag.fieldNumber = property.fieldNumber;
            if (--count == 0) {
                _tag.tag.next = 1;
            }
            writeByEncodingType(instance, property, _tag, buf);
        }
    }

    WriteMemoryAPI(void) writeByEncodingType(id instance, CHClassProperty *property, __tag_buffer_flag__ &tag, NSMutableData *buf)
    {
        if (property.encodingType == CHTagBufEncodingTypeNone) {
            NSCAssert(NO, @"");
            return;
        }
        id value = [instance valueForKey:property.propertyName];
        switch (property.encodingType) {
            case CHTagBufEncodingTypeBool: {
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                tag.tag.internalTag = varint_bool;
                tag.tag.placeholder12 = [value boolValue];
                writeTag(tag, buf);
            }
                break;
            case CHTagBufEncodingType8Bits: {
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                writeInteger([value charValue], buf, tag);
            }
                break;
            case CHTagBufEncodingType16Bits: {
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                writeInteger([value shortValue], buf, tag);
            }
                break;
            case CHTagBufEncodingType32Bits: {
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                writeInteger([value intValue], buf, tag);
            }
                break;
            case CHTagBufEncodingType64Bits: {
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                writeInteger([value longLongValue], buf, tag);
            }
                break;
            case CHTagBufEncodingTypeFloat: {
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                writeFloat([value floatValue], buf, tag);
            }
                break;
            case CHTagBufEncodingTypeDouble: {
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                writeDouble([value doubleValue], buf, tag);
            }
                break;
            case CHTagBufEncodingTypeNSNumber:
            case CHTagBufEncodingTypeNSData:
            case CHTagBufEncodingTypeNSString:
            case CHTagBufEncodingTypeNSArray:
            case CHTagBufEncodingTypeOtherObject: {
                if (value) {
                    writeObjcect(value, buf, tag, property);
                } else {
                    if (property.isOptional || property.isIgnore) {
                        tag.tag.writeType = CHTagBufferWriteTypeTagBuffer;
                        tag.tag.internalTag = object_is_nil;
                        writeTag(tag, buf);
                    } else {
                        @throw [NSException exceptionWithName:@"Object is nil"
                                                       reason:[NSString stringWithFormat:@"The value of %@'s property[%@]",[instance class], property.propertyName]
                                                     userInfo:nil];
                    }
                }
            }
                break;
            default:
                NSCAssert(NO, @"Logic error.");
                break;
        }

    }

    WriteMemoryAPI(void) writeObjcect(_Nonnull id obj, NSMutableData *buf, __tag_buffer_flag__ &_tag, CHClassProperty *property)
    {
        switch (property.encodingType) {
            case CHTagBufEncodingTypeNSNumber:
                _tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                writeNSNumber(obj, buf, _tag, property);
                break;
            case CHTagBufEncodingTypeNSData:
                _tag.tag.writeType = CHTagBufferWriteTypeblobStream;
                writeNSData(obj, buf, _tag);
                break;
            case CHTagBufEncodingTypeNSString: {
                _tag.tag.writeType = CHTagBufferWriteTypeblobStream;
                const char *_buf = [obj UTF8String];
                writeCString(_buf, buf, _tag);
            }
                break;
            case CHTagBufEncodingTypeNSArray:
                _tag.tag.writeType = CHTagBufferWriteTypeContainer;
                writeContainer(obj, buf, _tag, property);
                break;
            case CHTagBufEncodingTypeOtherObject:
                _tag.tag.writeType = CHTagBufferWriteTypeTagBuffer;
                writeTag(_tag, buf);
                _tag.itag = 0;
                writeTagBuffer(obj, buf, _tag);
                break;
            default:
                NSCAssert(NO, @"");
                break;
        }
    }

    WriteMemoryAPI(void) writeNSNumber(NSNumber *number,
                                       NSMutableData *buf, __tag_buffer_flag__ &_tag,
                                       CHClassProperty *property)
    {
        auto &tag = _tag.tag;
        switch (property.detailType) {
            case CHTagBufObjectDetailTypeNSNumberBoolean:
                tag.internalTag = varint_bool;
                tag.placeholder12 = number.boolValue;
                writeTag(_tag, buf);
                break;
            case CHTagBufObjectDetailTypeNSNumber8BitsInteger:
                tag.internalTag = varint_8bits;
                writeInteger(number.charValue, buf, _tag);
                break;
            case CHTagBufObjectDetailTypeNSNumber16BitsInteger:
                tag.internalTag = varint_16bits;
                writeInteger(number.shortValue, buf, _tag);
                break;
            case CHTagBufObjectDetailTypeNSNumber32BitsInteger:
                tag.internalTag = varint_32bits;
                writeInteger(number.intValue, buf, _tag);
                break;
            case CHTagBufObjectDetailTypeNSNumber64BitsInteger:
                tag.internalTag = varint_64bits;
                writeInteger(number.longLongValue, buf, _tag);
                break;
            case CHTagBufObjectDetailTypeNSNumberFloat:
                tag.internalTag = varint_float;
                writeFloat(number.floatValue, buf, _tag);
                break;
            case CHTagBufObjectDetailTypeNSNumberDouble:
                tag.internalTag = varint_double;
                writeDouble(number.doubleValue, buf, _tag);
                break;
            default:
                break;
        }
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

    WriteMemoryAPI(void) writeNSData(NSData *data, NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        uint64_t length = data.length;
        _tag.tag.internalTag = stream_nsdata;
        writeInteger<uint64_t, true, true, false, false>(length, buf, _tag);
        [buf appendData:data];
    }

    WriteMemoryAPI(void) writeContainer(NSArray *container, NSMutableData *buf, __tag_buffer_flag__ &_tag, CHClassProperty *property)
    {
        uint64_t count = (uint64_t)container.count;
        if (!count) {
            return;
        }

        CHTagBufObjectDetailType detailType = getNSArrayContainerTypeOnRuntime(container.firstObject, property);

        char _buf[8] = {0};
        auto &tag = _tag.tag;

        switch (detailType) {
            case CHTagBufObjectDetailTypeNSNumberBoolean: {
                TagBufBitset bitset(count);
                bitset.convertArrayToBitset(container);

                const char *bitData = nullptr;
                size_t dataCount = 0; // size of bit data.
                bitset.bitset(bitData, &dataCount);
                NSCAssert(bitData && dataCount, @"Bit set failed.");

                tag.internalTag = varint_bool;
                tag.placeholder12 = 1;

                writeCountIntegerToCharBuf(_buf, dataCount, tag);
                writeTag(_tag, buf);

                [buf appendBytes:_buf length:tag.lengthOfZigzag ?: sizeof(size_t)]; // write bit data count.
                [buf appendBytes:bitData length:dataCount]; // write bit data.
                [buf appendBytes:&count length:sizeof(count)]; // write size of bit.
            }
                break;
            case CHTagBufObjectDetailTypeNSNumber8BitsInteger:
                tag.internalTag = varint_8bits;
                tag.placeholder12 = 1;
                writeCountIntegerToCharBuf(_buf, count, tag);
                writeTag(_tag, buf);
                [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
                for (NSNumber *number in container) {
                    writeIntegerInList(number.unsignedCharValue, buf, _tag);
                }
                break;
            case CHTagBufObjectDetailTypeNSNumber16BitsInteger:
                tag.internalTag = varint_16bits;
                tag.placeholder12 = 1;
                writeCountIntegerToCharBuf(_buf, count, tag);
                writeTag(_tag, buf);
                [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
                for (NSNumber *number in container) {
                    writeIntegerInList(number.unsignedShortValue, buf, _tag);
                }
                break;
            case CHTagBufObjectDetailTypeNSNumber32BitsInteger:
                tag.internalTag = varint_32bits;
                tag.placeholder12 = 1;
                writeCountIntegerToCharBuf(_buf, count, tag);
                writeTag(_tag, buf);
                [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
                for (NSNumber *number in container) {
                    writeIntegerInList(number.unsignedIntValue, buf, _tag);
                }
                break;
            case CHTagBufObjectDetailTypeNSNumber64BitsInteger:
                tag.internalTag = varint_64bits;
                tag.placeholder12 = 1;
                writeCountIntegerToCharBuf(_buf, count, tag);
                writeTag(_tag, buf);
                [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
                for (NSNumber *number in container) {
                    writeIntegerInList(number.unsignedLongLongValue, buf, _tag);
                }
                break;
            case CHTagBufObjectDetailTypeNSNumberFloat:
                tag.internalTag = varint_float;
                tag.placeholder12 = 1;
                writeCountIntegerToCharBuf(_buf, count, tag);
                writeTag(_tag, buf);
                [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
                for (NSNumber *number in container) {
                    writeFloatInList(number.floatValue, buf, _tag);
                }
                break;
            case CHTagBufObjectDetailTypeNSNumberDouble:
                tag.internalTag = varint_double;
                tag.placeholder12 = 1;
                writeCountIntegerToCharBuf(_buf, count, tag);
                writeTag(_tag, buf);
                [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
                for (NSNumber *number in container) {
                    writeDoubleInList(number.doubleValue, buf, _tag);
                }
                break;
            case CHTagBufObjectDetailTypeNSArrayNSData:
                tag.internalTag = container_stream_nsdata;
                writeCountIntegerToCharBuf(_buf, count, tag);
                writeTag(_tag, buf);
                [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
                for (NSData *data in container) {
                    writeNSDataInList(data, buf, _tag);
                }
                break;
            case CHTagBufObjectDetailTypeNSArrayNSString:
                tag.internalTag = container_stream_nsstring;
                writeCountIntegerToCharBuf(_buf, count, tag);
                writeTag(_tag, buf);
                [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
                for (NSString *str in container) {
                    const char *_buf = [str UTF8String];
                    writeCStringInList(_buf, buf, _tag);
                }
                break;
            case CHTagBufObjectDetailTypeNSArrayNSArray:
                tag.internalTag = container_container;
                writeCountIntegerToCharBuf(_buf, count, tag);
                writeTag(_tag, buf);
                [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
                for (NSArray *a in container) {
                    _tag.itag = 0;
                    tag.writeType = CHTagBufferWriteTypeContainer;
                    writeContainer(a, buf, _tag, property);
                }
                break;
            case CHTagBufObjectDetailTypeOtherObject:
                tag.internalTag = container_object;
                writeCountIntegerToCharBuf(_buf, count, tag);
                writeTag(_tag, buf);
                [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
                _tag.itag = 0;
                for (id obj in container) {
                    writeTagBuffer(obj, buf, _tag);
                }
                break;
            default:
                NSCAssert(NO, @"Logic error.");
                break;
        }
    }

    template<typename T>
    WriteMemoryAPI(void) writeCountIntegerToCharBuf(char *dest, T count, __tag_detail__ &tag)
    {
        if (isWorthZigzag(count)) {
            (void) writeIntegerToZigzag(count, tag, (Byte *)dest);
        } else {
            integerToByte(count, (Byte *)dest);
        }
    }

    WriteMemoryAPI(void) writeNSDataInList(NSData *data, NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        writeInteger<uint64_t, false, false, false, false>(data.length, buf, _tag);
        [buf appendData:data];
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

#pragma mark - Read APIs

    template<typename T, bool accessToReadLength = false, bool readIntegerInList = false>
    ReadMemoryAPI(uint32_t) readInteger(T dest, __tag_detail__ tag, const char *buf)
    {
        uint32_t offset = 0;
        do {
            typedef typename std::remove_pointer<T>::type pod_type;
            if (sizeof(pod_type) < 4 || readIntegerInList) {
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

    ReadMemoryAPI(uint32_t) readContainer(NSMutableArray **_array, __tag_detail__ tag, const char *buf, CHClassProperty *property)
    {
        NSCAssert(_array, @"Param [_array] can't be nil!");
        uint64_t count = 0;
        uint32_t offset = readInteger<uint64_t *, true>(&count, tag, buf);
        NSCAssert(count, @"Logic error! count must not be 0.");
        NSMutableArray *array = [NSMutableArray arrayWithCapacity:count ?: 10];
        *_array = array;
        if (tag.placeholder12) {
            switch (tag.internalTag) {
                case varint_8bits: {
                    uint8_t value = 0;
                    for (uint64_t i=0; i<count; ++i) {
                        offset += readInteger<decltype(value)* ,false, true>(&value, tag, buf + offset);
                        [array addObject:@(value)];
                    }
                }
                    break;
                case varint_16bits: {
                    uint16_t value = 0;
                    for (uint64_t i=0; i<count; ++i) {
                        offset += readInteger<decltype(value)* ,false, true>(&value, tag, buf + offset);
                        [array addObject:@(value)];
                    }
                }
                    break;
                case varint_32bits: {
                    uint32_t value = 0;
                    for (uint64_t i=0; i<count; ++i) {
                        offset += readInteger<decltype(value)* ,false, true>(&value, tag, buf + offset);
                        [array addObject:@(value)];
                    }
                }
                    break;
                case varint_64bits: {
                    uint64_t value = 0;
                    for (uint64_t i=0; i<count; ++i) {
                        offset += readInteger<decltype(value)* ,false, true>(&value, tag, buf + offset);
                        [array addObject:@(value)];
                    }
                }
                    break;
                case varint_float: {
                    float value = 0;
                    for (uint64_t i=0; i<count; ++i) {
                        offset += readInteger<decltype(value)* ,false, true>(&value, tag, buf + offset);
                        [array addObject:@(value)];
                    }
                }
                    break;
                case varint_double: {
                    double value = 0;
                    for (uint64_t i=0; i<count; ++i) {
                        offset += readInteger<decltype(value)* ,false, true>(&value, tag, buf + offset);
                        [array addObject:@(value)];
                    }
                }
                    break;
                case varint_bool: {
                    const char *bitData = buf + offset; // bit data blob stream
                    offset += count;
                    uint64_t bitCount = count; // bit data size. var[count] represents bit size.
                    offset += byteToPodType(&count, buf + offset);
                    TagBufBitset bitset(bitData, count, bitCount);
                    bitset.convertToBooleanArray(array);
                }
                    break;
                default:
                    NSCAssert(NO, @"Logic error.");
                    break;
            }
        } else {
            switch (tag.internalTag) {
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
                    Class cls = NSClassFromString(property.protocol);
                    if (!cls) {
                        NSCAssert(NO, @"No such Class:%@ at property:%@", property.protocol, property.propertyName);
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
                    for (uint64_t i=0; i<count; ++i) {
                        offset += readTag(embeddeTag, buf + offset);
                        offset += readContainer(&embeddedArray, embeddeTag.tag, buf + offset, property);
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
        }
        return offset;
    }

    ReadMemoryAPI(uint32_t) readObject(const char *buf, id instance)
    {
        NSCAssert(instance, @"instance param must not be nil.");
        Class cls = [instance class];

        NSArray<CHClassProperty *> *properties = objc_getAssociatedObject(cls, &kTagBufferPropertyKey);
        if (!properties) {
            properties = getProperties(cls);
            objc_setAssociatedObject(cls, &kTagBufferPropertyKey, properties, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        }

        union __tag_buffer_flag _tag{0};
        auto &tag = _tag.tag;
        const char *cp = buf;
        NSUInteger count = properties.count;
        for (;;) {
            if (tag.next) {
                break;
            }
            cp += readTag(_tag, cp);
            if (tag.fieldNumber >= count) {
                NSCAssert(NO, @"field number is overflow.");
                break;
            }
            cp += readByWriteType(tag, cp, [properties objectAtIndex:tag.fieldNumber], instance);

        }

        return static_cast<uint32_t>(cp - buf);
    }

    ReadMemoryAPI(uint32_t) readByWriteType(__tag_detail__ tag, const char *buf, CHClassProperty *property, id instance)
    {
        uint32_t offset = 0;
        switch ((CHTagBufferWriteType)tag.writeType) {
            case CHTagBufferWriteTypeVarintFixed: {
                switch (tag.internalTag) {
                    case varint_bool:
                        [instance setValue:@(tag.placeholder12) forKey:property.propertyName];
                        break;
                    case varint_8bits: {
                        uint8_t value = 0;
                        offset = byteToPodType(&value, buf);
                        [instance setValue:@(value) forKey:property.propertyName];
                    }
                        break;
                    case varint_16bits: {
                        uint16_t value = 0;
                        offset = byteToPodType(&value, buf);
                        [instance setValue:@(value) forKey:property.propertyName];
                    }
                        break;
                    case varint_32bits: {
                        uint32_t value = 0;
                        offset = readInteger(&value, tag, buf);
                        [instance setValue:@(value) forKey:property.propertyName];
                    }
                        break;
                    case varint_64bits: {
                        uint64_t value = 0;
                        offset = readInteger(&value, tag, buf);
                        [instance setValue:@(value) forKey:property.propertyName];
                    }
                        break;
                    case varint_float: {
                        float value = 0;
                        offset = readFloat(&value, buf);
                        [instance setValue:@(value) forKey:property.propertyName];
                    }
                        break;
                    case varint_double: {
                        double value = 0;
                        offset = readDouble(&value, buf);
                        [instance setValue:@(value) forKey:property.propertyName];
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
                offset = readContainer(&array, tag, buf, property);
                if (array) {
                    [instance setValue:array forKey:property.propertyName];
                }
            }
                break;
            case CHTagBufferWriteTypeblobStream: {
                const char *dest = nullptr;
                uint32_t length = 0;
                offset = readBlobStream(dest, length, tag, buf);
                switch (tag.internalTag) {
                    case stream_nsstring: {
                        NSString *str = [[NSString alloc] initWithBytes:dest length:length encoding:NSUTF8StringEncoding];
                        [instance setValue:str forKey:property.propertyName];
                    }
                        break;
                    case stream_nsdata: {
                        NSData *data = [[NSData alloc] initWithBytes:dest length:length];
                        [instance setValue:data forKey:property.propertyName];
                    }
                    default:
                        break;
                }
            }
                break;
            case CHTagBufferWriteTypeTagBuffer: {
                if (tag.internalTag == object_is_exists) {
                    Class cls = property.propertyClassType;
                    if (!cls) {
                        @throw [NSException exceptionWithName:@"Class does not exist." reason:[NSString stringWithFormat:@"No such Class at property:%@", property.propertyName] userInfo:nil];
                    }
                    id value = [[cls alloc] init];
                    offset = readObject(buf, value);
                    [instance setValue:value forKey:property.propertyName];
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

NSData *CHTagBufferBuilder::buildedData() const
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

#ifdef DEBUG
    @try {
        auto final = cp + CHInternalHelper::readObject(cp, instance);
        NSCAssert(final = cp + data.length, @"");
    } @catch (NSException *exception) {
        NSLog(@"%@", exception);
    }
#else
    CHInternalHelper::readObject(cp, instance);
#endif

    return instance;
}