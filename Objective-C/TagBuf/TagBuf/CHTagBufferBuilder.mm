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
 |31~13 bit| field number
  ---------
  ---------
 |12~08 bit| tagBuf reserved
  ---------
 {
 If [04] = 1 and [08~10] is vaild only that.
 08~10: The length of zigzag.
 }
  ---------
 |07~04 bit| tagBuf internal-tag
  ---------
 04: 1 represents integer was compressed by zigzag if write-type is about integer 
     or string's length. And the placeholder is valid only write-type is about
     integer or string. 0 represents not.
 05: 1 represents there is a data of container like array at next. 0 represents not.
 06: 1 represents there is a tagBuf cell at next. 0 represents not.
 07: 1 represents there is a tagBuf object at next. 0 represents not.
  ---------
 |03~00 bit| tagBuf write type
  ---------
 */

#define __tag32 0xFFE00000u
#define __tag64 0xFFE0000000000000ull

NS_INLINE bool isWorthZigzag(uint32_t integer)
{
    return !(integer & __tag32);
}

NS_INLINE bool isWorthZigzag(uint64_t integer)
{
    return !(integer & __tag64);
}

union __tag_buffer_flag {
    uint32_t itag;
    struct __tag_buffer_structure {
        uint32_t writeType          : 03; // [0~3]
        uint32_t isZigzag           : 01; // [04]
        uint32_t nextIsDataOfList   : 01; // [05]
        uint32_t next               : 01; // [06]
        uint32_t isTagBufObject     : 01; // [07]
        uint32_t lengthOfZigzag     : 03; // [08~10]
        uint32_t reserved           : 02; // [11~12]
        uint32_t field_number       : 19; // [13~31]
    }tag;
};

struct CHTagBufferBuilderPrivate
{
    vector<char> *buf = new vector<char>();
    char *offset = 0;
    union __tag_buffer_flag tag;
    static Class objc_class_tagbuffer;
    CHTagBufferBuilderPrivate()
    {
        tag.itag = 0;
        buf->resize(4098);
    }
    ~CHTagBufferBuilderPrivate()
    {delete buf;}

    void resetTagWithFieldNumber(uint32_t fieldNumber)
    {
        tag.itag = 0;
        tag.tag.field_number = fieldNumber;
        tag.tag.next = 1;
    }
};

Class CHTagBufferBuilderPrivate::objc_class_tagbuffer = NSClassFromString(@"CHTagBuffer");

struct CHInternalHelper
{
    template <typename T>
    WriteMemoryAPI(T) write_to_buffer(T zz,
                                      Byte* buf,
                                      CHTagBufferBuilder *builder)
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
        return i;
    }

    template <typename T>
    ReadMemoryAPI(T) read_from_buffer(Byte* buf,
                                      int max_size,
                                      CHTagBufferBuilder *builder)
    {
        T ret = 0;
        Byte n = 0;
        for (int i=0, offset = 0; i<max_size; ++i, offset += 7 ){
            n = buf[i];
            if ((n & 0x80 ) != 0x80){
                ret |= (n << offset);
                break;
            } else {
                ret |= ((n & 0x7f) << offset);
            }
        }
        return ret;
    }

    template <typename T>
    WriteMemoryAPI(char*) memoryCopyPod(char *dst, T value)
    {
        static_assert(std::is_pointer<T>::value == false, "Must not be a pointer");
        memcpy(dst, &value, sizeof(T));
        return dst + sizeof(T);
    }

    template <typename T>
    WriteMemoryAPI(char*) memoryCopyPointer(char *dst, T value)
    {
        static_assert(std::is_pointer<T>::value == true, "Must not be a pod");
        memcpy(dst, value, sizeof(typename std::remove_pointer<T>::type));
        return dst + sizeof(typename std::remove_pointer<T>::type);
    }

    template <typename T>
    WriteMemoryAPI(T) integerToZigzag(T number)
    {
        static_assert(sizeof(T) == 4 || sizeof(T) == 8, "Param must be a int32_t or uint32_t or int64_t or uint64_t!");
        return (number << 1 ) ^ (number >> (sizeof(T) * 8 - 1) );
    }

    template <typename T>
    ReadMemoryAPI(T) zigzagToInteger(T number)
    {
        static_assert(sizeof(T) == 4 || sizeof(T) == 8, "Param must be a int32_t or uint32_t or int64_t or uint64_t!");
        return (((typename make_unsigned<T>::type)number) >> 1 ) ^ -(number & 1 );
    }

    WriteMemoryAPI(void) writeNSNumber(NSNumber *number,
                                      CHTagBufferBuilder *builder)
    {
        char objCType = number.objCType[0];
        if (objCType == 'f') { // float
            builder->writeFloat(number.floatValue);
        } else if (objCType == 'd') { // double
            builder->writeDouble(number.doubleValue);
        } else {
            uint64_t _int64 = number.unsignedLongLongValue;
            builder->writeInteger(_int64);
        }
    }

    WriteMemoryAPI(void) writeNSData(NSData *data, CHTagBufferBuilder *builder)
    {
        auto &tag = builder->_d->tag.tag;
        tag.writeType = CHTagBufferWriteTypeblobStream;
        uint64_t length = data.length;
        if (isWorthZigzag(length)) {
            tag.isZigzag = 1;
            length = integerToZigzag(length);
        }
        char *&offset = builder->_d->offset;
        offset = memoryCopyPod(offset, length);
        [data enumerateByteRangesUsingBlock:^(const void *bytes, NSRange byteRange, BOOL *stop) {
            memcpy(offset, bytes, sizeof(char) * byteRange.length);
            offset += byteRange.length;
        }];
    }

    WriteMemoryAPI(void) writeNSString(NSString *aString, CHTagBufferBuilder *builder)
    {
        auto &tag = builder->_d->tag.tag;
        tag.writeType = CHTagBufferWriteTypeblobStream;
        const char *buf = [aString UTF8String];
        uint32_t length = buf ? (uint32_t)strlen(buf) : 0;
        if (isWorthZigzag(length)) {
            tag.isZigzag = 1;
            length = integerToZigzag(length);
        }
        builder->writeTag();
        char *&offset = builder->_d->offset;
        offset = memoryCopyPod(offset, length);
        memcpy(offset, buf, sizeof(char) * length);
        offset += length;
    }
};



CHTagBufferBuilder::CHTagBufferBuilder() :_d(new struct CHTagBufferBuilderPrivate)
{}

CHTagBufferBuilder::~CHTagBufferBuilder()
{
    delete _d;
}

void CHTagBufferBuilder::writeTag()
{
    memcpy(_d->offset, &_d->tag.itag, sizeof(uint32_t));
}

void CHTagBufferBuilder::maker(id instance)
{
    Class cls = [instance class];
    if (class_isMetaClass(cls)) {
        NSCAssert(NO, @"");
        return;
    }

    uint32_t count = 0;
    Ivar *list = class_copyIvarList(cls, &count);
    Ivar *p = list;
    --count;
    CHTagBufEncodingType encodingType;
    CHTagBufferWriteType writeType;
    for (uint32_t i=0; i<count; ++i, ++p) {
        const char *typeCoding = ivar_getTypeEncoding(*p);
        this->decodeFromTypeEncoding(typeCoding, encodingType, writeType);
        _d->resetTagWithFieldNumber(i);
        this->writeByEncodingType(encodingType, *p, instance);
    }
}

/*
 c-style string's structure:
  -------------------------------
 |tag(4)|length(4)|string-data(N)|
  -------------------------------
 */
void CHTagBufferBuilder::writeCString(const char *str, uint32_t length)
{
    _d->tag.tag.writeType = CHTagBufferWriteTypeblobStream;
    if (isWorthZigzag(length)) {
        _d->tag.tag.isZigzag = 1;
        length = CHInternalHelper::integerToZigzag(length);
    }
    writeTag();
    _d->offset = CHInternalHelper::memoryCopyPod(_d->offset, length);
    memcpy(_d->offset, str, sizeof(char) * length);
    _d->offset += length;
}

void CHTagBufferBuilder::writeInteger(long long integer)
{
    _d->tag.tag.writeType = CHTagBufferWriteTypeVarint;
    if (isWorthZigzag((make_unsigned<long long>::type)integer)) {
        _d->tag.tag.isZigzag = 1;
        integer = CHInternalHelper::integerToZigzag(integer);
        Byte buf[8] = {0};
        (void) CHInternalHelper::write_to_buffer(integer, buf, this);
        writeTag();
        char *src = (char *)buf;
        while (*src) {
            *_d->offset++ = *src++;
        }
    } else {
        writeTag();
        _d->offset = CHInternalHelper::memoryCopyPod(_d->offset, integer);
    }
}

void CHTagBufferBuilder::writeFloat(float v)
{
    _d->tag.tag.writeType = CHTagBufferWriteTypeFloat;
    writeTag();
    _d->offset = CHInternalHelper::memoryCopyPod(_d->offset, v);
}

void CHTagBufferBuilder::writeDouble(double v)
{
    _d->tag.tag.writeType = CHTagBufferWriteTypeDouble;
    writeTag();
    _d->offset = CHInternalHelper::memoryCopyPod(_d->offset, v);
}

void CHTagBufferBuilder::writeContainer(NSArray *container)
{
    _d->tag.tag.writeType = CHTagBufferWriteTypeContainer;
    for (id obj in container) {
        this->writeObjcect(obj);
    }
}

/*
 R:readOnly
 C:copy
 &:strong or retain
 N:nonatomic
 结构 T{type}[{类名}],[{R},{C},{N},{&}],V{变量的名字}
 */
void CHTagBufferBuilder::decodeFromTypeEncoding(const char *typeCoding, CHTagBufEncodingType &encodingType, CHTagBufferWriteType &writeType)
{
    char t = typeCoding[0];
    encodingType = CHTagBufEncodingTypeNone;
    switch (t) {
        case 'c': // char
        case 'B': // C++ bool or C99 _Bool
        case 'C': // uchar
            encodingType = CHTagBufEncodingType8Bits;
            writeType = CHTagBufferWriteTypeVarint;
            break;
        case 's': // short
        case 'S': // ushort
            encodingType = CHTagBufEncodingType16Bits;
            writeType = CHTagBufferWriteTypeVarint;
            break;
        case 'l': // long
        case 'L': // ulong
        case 'i': // int
        case 'I': // uint
            writeType = CHTagBufferWriteTypeVarint;
        case 'f': // float
            encodingType = CHTagBufEncodingType32Bits;
            writeType = CHTagBufferWriteTypeFloat;
        case 'd': // double
            encodingType = CHTagBufEncodingType64Bits;
            writeType = CHTagBufferWriteTypeDouble;
            break;
        case 'q': // long long
        case 'Q': // ulong long
            encodingType = CHTagBufEncodingType64Bits;
            writeType = CHTagBufferWriteTypeVarint;
            break;
        case '@': // Objective-C object
            encodingType = CHTagBufEncodingTypeObject;
            writeType = CHTagBufferWriteTypeblobStream;
            break;
        default:
            NSCAssert(NO, @"Unsupport type:%s", typeCoding);
            break;
    }
}

void CHTagBufferBuilder::writeObjcect(id obj)
{
    if ([obj isKindOfClass:[NSString class]]) {
        const char *buf = [obj UTF8String];
        uint32_t length = buf ? (uint32_t)strlen(buf) : 0;
        writeCString(buf, length);
    } else if ([obj isKindOfClass:[NSArray class]]) {
        writeContainer(obj);
    } else if ([obj isKindOfClass:[NSNumber class]]) {
        CHInternalHelper::writeNSNumber(obj, this);
    } else if ([obj isKindOfClass:[NSData class]]) {
        CHInternalHelper::writeNSData(obj, this);
    } else if ([obj isKindOfClass:CHTagBufferBuilderPrivate::objc_class_tagbuffer]) {
        this->maker(obj);
    } else {
        NSCAssert(NO, @"tagBuf:Only support NSArray,NSString,NSNumber,NSData");
    }
}

void CHTagBufferBuilder::writeByEncodingType(CHTagBufEncodingType type, Ivar ivar, id instance)
{
    if (type == CHTagBufEncodingTypeNone) {
        NSCAssert(NO, @"");
        return;
    }
    using ptr8_t  = uint8_t  *;
    using ptr16_t = uint16_t *;
    using ptr32_t = uint32_t *;
    using ptr64_t = uint64_t *;
    switch (type) {
        case CHTagBufEncodingType8Bits:
        {
            auto p = reinterpret_cast<ptr8_t>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
            _d->offset = CHInternalHelper::memoryCopyPointer(_d->offset, p);
        }
            break;
        case CHTagBufEncodingType16Bits:
        {
            auto p = reinterpret_cast<ptr16_t>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
            _d->offset = CHInternalHelper::memoryCopyPointer(_d->offset, p);
        }
            break;
        case CHTagBufEncodingType32Bits:
        {
            auto p = reinterpret_cast<ptr32_t>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
            writeInteger(*p);
        }
            break;
        case CHTagBufEncodingType64Bits:
        {
            auto p = reinterpret_cast<ptr64_t>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
            writeInteger(*p);
        }
            break;
        case CHTagBufEncodingTypeObject:
        {
            char *name = (char *)ivar_getName(ivar);
            NSString *key = [NSString stringWithUTF8String:name];
            id value = [instance valueForKey:key];
            this->writeObjcect(value);
        }
            break;
        default:
            break;
    }

}