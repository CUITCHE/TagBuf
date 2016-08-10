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
 [08~10] represents the length of zigzag.
 [11] = 1 represents that the bytes of string or data's length has been
 compressed by zigzag.

  ---------
 |07~04 bit| tagBuf internal-tag
  ---------
 Especially, if [07] = 0 represents there is a tagBuf at next, 1 represents not.

  ---------
 |03~00 bit| tagBuf write type
  ---------
 If value = CHTagBufferWriteType for internal-tag[04~06]
 - CHTagBufferWriteTypeVarintFixed
    internal-tag = 0, represents 8bits integer.
    internal-tag = 1, represents 16bits integer.
    internal-tag = 2, represents 32bits integer.
    internal-tag = 3, represents 64bits integer.
    internal-tag = 4, represents 32bits float.
    internal-tag = 5, represents 64bits double.
 - CHTagBufferWriteTypeContainer
    internal-tag = 0, contains 32bits integer.
    internal-tag = 1, contains 64bits integer.
    internal-tag = 2, represents 32bits float.
    internal-tag = 3, represents 64bits double.
    internal-tag = 4, contains string or blob data.
    internal-tag = 5, contains tagBuffer object.
 - CHTagBufferWriteTypeblobStream
    internal-tag = 0, represents non-encrypted.
    internal-tag = 1, represents encrypted.
 - CHTagBufferWriteTypeTagBuffer
    none.
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
        uint32_t writeType          : 03; // [00~03]
// if writeType is CHTagBufferWriteTypeVarint
#define varint_8bits    0
#define varint_16bits   1
#define varint_32bits   2
#define varint_64bits   3
#define varint_float    4
#define varint_double   5

// if writeType is CHTagBufferWriteTypeContainer
#define container_32bits    0
#define container_64bits    1
#define container_float     2
#define container_double    3
#define container_stream    4
#define container_tagBuffer 5

// if writeType is CHTagBufferWriteTypeContainer
#define stream_nonecrypt    0
#define stream_encrypt      1
        uint32_t internalTag        : 03; // [04~06]
        uint32_t next               : 01; // [07]
        uint32_t lengthOfZigzag     : 03; // [08~10]
        uint32_t lengthCompressed   : 01; // [11]
        uint32_t compressedInArray  : 01; // [12]
        uint32_t fieldNumber        : 20; // [13~32]
    } tag;
};

using __tag_detail__ = __tag_buffer_flag::__tag_buffer_structure;

struct CHTagBufferBuilderPrivate
{
    static Class objc_class_tagbuffer;
    static Protocol *objc_proto_optional;
    static Protocol *objc_proto_required;
    static Protocol *objc_proto_NSNumberInt32;
    static Protocol *objc_proto_NSNumberInt64;
    NSMutableData *buf;
    vector<char> *readBuffer;
    union __tag_buffer_flag tag;
    BOOL accessToWriteTag;
    CHTagBufferBuilderPrivate()
    :readBuffer(new vector<char>(4098))
    {
        tag.itag = 0;
        buf = [NSMutableData dataWithCapacity:4098];
        accessToWriteTag = YES;
    }

    void resetTagWithFieldNumber(uint32_t fieldNumber)
    {
        tag.itag = 0;
        tag.tag.fieldNumber = fieldNumber;
    }
};

Class CHTagBufferBuilderPrivate::objc_class_tagbuffer = NSClassFromString(@"CHTagBuffer");
Protocol *CHTagBufferBuilderPrivate::objc_proto_optional = NSProtocolFromString(@"optional");
Protocol *CHTagBufferBuilderPrivate::objc_proto_required = NSProtocolFromString(@"required");
Protocol *CHTagBufferBuilderPrivate::objc_proto_NSNumberInt32 = NSProtocolFromString(@"NSNumberInt32");
Protocol *CHTagBufferBuilderPrivate::objc_proto_NSNumberInt64 = NSProtocolFromString(@"NSNumberInt64");

struct CHInternalHelper
{
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
    ReadMemoryAPI(T) read_from_buffer(Byte* buf, int max_size)
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

    template <typename T>
    WriteMemoryAPI(void) memoryCopyPod(T value, NSMutableData *buf)
    {
        static_assert(std::is_pod<T>::value == true, "Must not be a pointer");
        [buf appendBytes:&value length:sizeof(T)];
    }

    template <typename T>
    WriteMemoryAPI(void) memoryCopyPointer(T value, NSMutableData *buf)
    {
        static_assert(std::is_pointer<T>::value == true, "Must not be a pod");
        [buf appendBytes:&value length:sizeof(typename std::remove_pointer<T>::type)];
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
            builder->_d->tag.tag.internalTag = varint_float;
            writeFloat(number.floatValue, builder);
        } else if (objCType == 'd') { // double
            builder->_d->tag.tag.internalTag = varint_double;
            writeDouble(number.doubleValue, builder);
        } else {
            uint64_t _int64 = number.unsignedLongLongValue;
            uint32_t _int32 = number.unsignedIntValue;
            if (_int32 == _int64) {
                builder->_d->tag.tag.internalTag = varint_32bits;
                writeInteger(_int32, builder);
            } else {
                builder->_d->tag.tag.internalTag = varint_64bits;
                writeInteger(_int64, builder);
            }
        }
    }

    WriteMemoryAPI(void) writeNSData(NSData *data, CHTagBufferBuilder *builder)
    {
        auto &tag = builder->_d->tag.tag;
        tag.writeType = CHTagBufferWriteTypeblobStream;
        uint64_t length = data.length;
        if (isWorthZigzag(length)) {
            tag.lengthCompressed = 1;
            uint64_t compressed = integerToZigzag(length);
            Byte buf[8] = {0};
            tag.lengthOfZigzag = write_to_buffer(compressed, buf);
            [builder->_d->buf appendBytes:buf length:tag.lengthOfZigzag];
        } else {
            memoryCopyPod(length, builder->_d->buf);
        }
        [builder->_d->buf appendData:data];
    }
    WriteMemoryAPI(void) writeNSDataInList(NSData *data, CHTagBufferBuilder *builder)
    {
        memoryCopyPod(data.length, builder->_d->buf);
        [builder->_d->buf appendData:data];
    }

    /*
     c-style string's structure:
      -------------------------------
     |tag(4)|length(4)|string-data(N)|
      -------------------------------
     */
    WriteMemoryAPI(void) writeCString(const char *str, CHTagBufferBuilder *builder)
    {
        auto &tag = builder->_d->tag.tag;
        uint32_t length = str ? (uint32_t)strlen(str) : 0;
        if (isWorthZigzag(length)) {
            uint32_t compressed = integerToZigzag(length);
            Byte buf[4] = {0};
            tag.lengthCompressed = 1;
            tag.lengthOfZigzag = write_to_buffer(compressed, buf);
            [builder->_d->buf appendBytes:buf length:tag.lengthOfZigzag];
        } else {
            memoryCopyPod(length, builder->_d->buf);
        }
        builder->writeTag();
        [builder->_d->buf appendBytes:str length:sizeof(char) * length];
    }
    WriteMemoryAPI(void) writeCStringInList(const char *str, CHTagBufferBuilder *builder)
    {
        uint32_t length = str ? (uint32_t)strlen(str) : 0;
        if (length == 0) {
            NSCAssert(NO, @"The length is 0.");
            return;
        }
        memoryCopyPod(length, builder->_d->buf);
        [builder->_d->buf appendBytes:str length:sizeof(char) * length];
    }

    WriteMemoryAPI(void) writeFloat(float v, CHTagBufferBuilder *builder)
    {
        builder->writeTag();
        memoryCopyPod(v, builder->_d->buf);
    }
    WriteMemoryAPI(void) writeFloatInList(float v, CHTagBufferBuilder *builder)
    {
        memoryCopyPod(v, builder->_d->buf);
    }

    WriteMemoryAPI(void) writeDouble(double v, CHTagBufferBuilder *builder)
    {
        builder->writeTag();
        memoryCopyPod(v, builder->_d->buf);
    }
    WriteMemoryAPI(void) writeDoubleInList(double v, CHTagBufferBuilder *builder)
    {
        memoryCopyPod(v, builder->_d->buf);
    }

    template <typename T>
    WriteMemoryAPI(void) writeInteger(T integer, CHTagBufferBuilder *builder)
    {
        static_assert(std::is_pod<T>::value == true, "Must be a pod value");
        do {
            if (sizeof(T) < 4) {
                builder->writeTag();
                memoryCopyPod(integer, builder->_d->buf);
                break;
            }
            if (isWorthZigzag((typename make_unsigned<T>::type)integer)) {
                integer = integerToZigzag(integer);
                Byte buf[sizeof(T)] = {0};
                auto &tag = builder->_d->tag.tag;
                tag.lengthCompressed = 1;
                tag.lengthOfZigzag = write_to_buffer(integer, buf);
                builder->writeTag();
                [builder->_d->buf appendBytes:buf length:tag.lengthOfZigzag];
            } else {
                builder->writeTag();
                memoryCopyPod(integer, builder->_d->buf);
            }
        } while (0);
    }
    template <typename T>
    WriteMemoryAPI(void) writeIntegerInList(T integer, CHTagBufferBuilder *builder)
    {
        static_assert(std::is_pod<T>::value == true, "Must be a pod value");
        memoryCopyPod(integer, builder->_d->buf);
    }

    template<typename T>
    ReadMemoryAPI(uint32_t) pointerReadMemory(T dest, const char *buf)
    {
        uint32_t offset = sizeof(typename std::remove_pointer<T>::type);
        memcpy(dest, buf, offset);
        return offset;
    }

    template<typename T>
    ReadMemoryAPI(uint32_t) readInteger(T dest, __tag_detail__ tag, const char *buf)
    {
        uint32_t offset = 0;
        do {
            if (sizeof(typename std::remove_pointer<T>::type) < 4) {
                offset = pointerReadMemory(dest, buf);
                break;
            }
            if (tag.lengthCompressed) {
                offset = tag.lengthOfZigzag;
                if (tag.internalTag == varint_32bits) {
                    uint32_t t = read_from_buffer<uint32_t>((Byte *)buf, offset);
                    t = zigzagToInteger(t);
                    (void) pointerReadMemory(dest, (const char *)&t);
                } else if (tag.internalTag == varint_64bits) {
                    uint64_t t = read_from_buffer<uint64_t>((Byte *)buf, offset);
                    t = zigzagToInteger(t);
                    (void) pointerReadMemory(dest, (const char *)&t);
                }
            } else {
                offset = pointerReadMemory(dest, buf);
            }
        } while (0);
        return offset;
    }

    ReadMemoryAPI(uint32_t) readTag(const char *buf)
    {
        uint32_t *p = (uint32_t *)buf;
        return *p;
    }

    ReadMemoryAPI(uint32_t) readFloat(float *dest, const char *buf) {
        memcpy(dest, buf, sizeof(float));
        return 4;
    }

    ReadMemoryAPI(uint32_t) readDouble(double *dest, const char *buf) {
        memcpy(dest, buf, sizeof(double));
        return 8;
    }

};


CHTagBufferBuilder::CHTagBufferBuilder() :_d(new struct CHTagBufferBuilderPrivate)
{}

CHTagBufferBuilder::~CHTagBufferBuilder()
{
    delete _d;
}

#pragma mark - Write
void CHTagBufferBuilder::writeTag()
{
    if (_d->accessToWriteTag) {
        [_d->buf appendBytes:&_d->tag.itag length:sizeof(uint32_t)];
    }
}

void CHTagBufferBuilder::startBuildingWithObject(id instance)
{
    _d->buf.length = 0;
    writeTagBuffer(instance);
}

void CHTagBufferBuilder::writeTagBuffer(id instance)
{
    Class cls = [instance class];
    if (class_isMetaClass(cls)) {
        NSCAssert(NO, @"");
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
        this->decodeFromTypeEncoding(typeCoding, encodingType);
        _d->resetTagWithFieldNumber(i);
        if (i == count) {
            _d->tag.tag.next = 1;
        }
        this->writeByEncodingType(encodingType, *p, instance);
    }
    free(list);
}

NSData *CHTagBufferBuilder::buildedData()
{
    return _d->buf;
}

void CHTagBufferBuilder::writeContainer(NSArray *container)
{
    uint64_t count = (uint64_t)container.count;
    if (!count) {
        return;
    }
    if (isWorthZigzag(count)) {
        uint64_t ncount = CHInternalHelper::integerToZigzag(count);
        Byte buf[8] = {0};
        _d->tag.tag.lengthCompressed = 1;
        _d->tag.tag.lengthOfZigzag = CHInternalHelper::write_to_buffer(ncount, buf);
        [_d->buf appendBytes:buf length:_d->tag.tag.lengthOfZigzag];
    } else {
        [_d->buf appendBytes:&count length:sizeof(count)];
    }
    auto &tag = _d->tag.tag;
    tag.writeType = CHTagBufferWriteTypeContainer;
    id firstObject = container.firstObject;
    if ([firstObject isKindOfClass:[NSNumber class]]) {
        const char *type = [firstObject objCType];
        if (*type == 'f') {
            tag.internalTag = container_float;
            writeTag();
            for (NSNumber *number in container) {
                CHInternalHelper::writeFloatInList(number.floatValue, this);
            }
        } else if (*type == 'd') {
            tag.internalTag = container_double;
            writeTag();
            for (NSNumber *number in container) {
                CHInternalHelper::writeFloatInList(number.doubleValue, this);
            }
        } else {
            if (class_conformsToProtocol(container.class, CHTagBufferBuilderPrivate::objc_proto_NSNumberInt64)) {
                tag.internalTag = container_64bits;
                writeTag();
                for (NSNumber *number in container) {
                    CHInternalHelper::writeIntegerInList(number.unsignedLongLongValue, this);
                }
            } else {
                tag.internalTag = container_32bits;
                writeTag();
                for (NSNumber *number in container) {
                    CHInternalHelper::writeIntegerInList(number.unsignedIntValue, this);
                }
            }
        }
        return;
    } else if ([firstObject isKindOfClass:[NSString class]]) {
        tag.internalTag = container_stream;
        writeTag();
        for (NSString *str in container) {
            const char *buf = [str UTF8String];
            CHInternalHelper::writeCStringInList(buf, this);
        }
    } else if ([firstObject isKindOfClass:CHTagBufferBuilderPrivate::objc_class_tagbuffer]) {
        tag.internalTag = container_tagBuffer;
        writeTag();
        for (id obj in container) {
            writeTagBuffer(obj);
        }
    } else if ([firstObject isKindOfClass:[NSData class]]) {
        tag.internalTag = container_stream;
        writeTag();
        for (NSData *data in container) {
            CHInternalHelper::writeNSDataInList(data, this);
        }
    } else if ([firstObject isKindOfClass:[NSArray class]]) { // That can't support is stupid.
        NSCAssert(NO, @"Forbid to recursion NSArray.");
        return;
    }
}

void CHTagBufferBuilder::decodeFromTypeEncoding(const char *typeCoding, CHTagBufEncodingType &encodingType)
{
    char t = typeCoding[0];
    switch (t) {
        case 'c': // char
        case 'B': // C++ bool or C99 _Bool
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
        case 'f': // float
            encodingType = CHTagBufEncodingTypeFloat;
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

void CHTagBufferBuilder::writeObjcect(id obj)
{
    if ([obj isKindOfClass:[NSString class]]) {
        _d->tag.tag.writeType = CHTagBufferWriteTypeblobStream;
        const char *buf = [obj UTF8String];
        CHInternalHelper::writeCString(buf, this);
    } else if ([obj isKindOfClass:[NSArray class]]) {
        _d->tag.tag.writeType = CHTagBufferWriteTypeContainer;
        writeContainer(obj);
    } else if ([obj isKindOfClass:[NSNumber class]]) {
        _d->tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
        CHInternalHelper::writeNSNumber(obj, this);
    } else if ([obj isKindOfClass:[NSData class]]) {
        _d->tag.tag.writeType = CHTagBufferWriteTypeblobStream;
        CHInternalHelper::writeNSData(obj, this);
    } else if ([obj isKindOfClass:CHTagBufferBuilderPrivate::objc_class_tagbuffer]) {
        _d->tag.tag.writeType = CHTagBufferWriteTypeTagBuffer;
        this->writeTag();
        this->writeTagBuffer(obj);
    } else {
        NSCAssert(NO, @"tagBuf:Only support NSArray,NSString,NSNumber,NSData");
    }
}

using ptr8_t  = uint8_t  *;
using ptr16_t = uint16_t *;
using ptr32_t = uint32_t *;
using ptr64_t = uint64_t *;
using ptrf_t  = float    *;
using ptrd_t  = double   *;

void CHTagBufferBuilder::writeByEncodingType(CHTagBufEncodingType type, Ivar ivar, id instance)
{
    if (type == CHTagBufEncodingTypeNone) {
        NSCAssert(NO, @"");
        return;
    }
    switch (type) {
        case CHTagBufEncodingType8Bits:
        {
            auto p = reinterpret_cast<ptr8_t>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
            _d->tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
            _d->tag.tag.internalTag = varint_8bits;
            CHInternalHelper::memoryCopyPointer(p, _d->buf);
        }
            break;
        case CHTagBufEncodingType16Bits:
        {
            auto p = reinterpret_cast<ptr16_t>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
            _d->tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
            _d->tag.tag.internalTag = varint_16bits;
            CHInternalHelper::memoryCopyPointer(p, _d->buf);
        }
            break;
        case CHTagBufEncodingType32Bits:
        {
            auto p = reinterpret_cast<ptr32_t>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
            _d->tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
            _d->tag.tag.internalTag = varint_32bits;
            CHInternalHelper::writeInteger(*p, this);
        }
            break;
        case CHTagBufEncodingType64Bits:
        {
            auto p = reinterpret_cast<ptr64_t>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
            _d->tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
            _d->tag.tag.internalTag = varint_64bits;
            CHInternalHelper::writeInteger(*p, this);
        }
            break;
        case CHTagBufEncodingTypeFloat:
        {
            auto p = reinterpret_cast<ptrf_t>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
            _d->tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
            _d->tag.tag.internalTag = varint_float;
            CHInternalHelper::writeFloat(*p, this);
        }
            break;
        case CHTagBufEncodingTypeDouble:
        {
            auto p = reinterpret_cast<ptrd_t>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
            _d->tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
            _d->tag.tag.internalTag = varint_double;
            CHInternalHelper::writeDouble(*p, this);
        }
            break;
        case CHTagBufEncodingTypeObject:
        {
            char *name = (char *)ivar_getName(ivar);
            if (name[0] == '_') {
                ++name;
            }
            NSString *key = [NSString stringWithUTF8String:name];
            id value = [instance valueForKey:key];
            this->writeObjcect(value);
        }
            break;
        default:
            break;
    }

}

#pragma mark - Read

uint32_t readByWriteType(__tag_detail__ tag, const char *buf, Ivar ivar, id instance)
{
    uint32_t offset = 0;
    switch ((CHTagBufferWriteType)tag.writeType) {
        case CHTagBufferWriteTypeVarintFixed: {
            auto instance_ptr = reinterpret_cast<void *>(reinterpret_cast<NSInteger>(instance) + ivar_getOffset(ivar));
            switch (tag.internalTag) {
                case varint_8bits: {
                    auto p = (ptr8_t)instance_ptr;
                    offset = CHInternalHelper::readInteger(p, tag, buf);
                }
                    break;
                case varint_16bits: {
                    auto p = (ptr16_t)instance_ptr;
                    offset = CHInternalHelper::readInteger(p, tag, buf);
                }
                    break;
                case varint_32bits: {
                    auto p = (ptr32_t)instance_ptr;
                    offset = CHInternalHelper::readInteger(p, tag, buf);
                }
                    break;
                case varint_64bits: {
                    auto p = (ptr64_t)instance_ptr;
                    offset = CHInternalHelper::readInteger(p, tag, buf);
                }
                    break;
                case varint_float: {
                    auto p = (ptrf_t)instance_ptr;
                    offset = CHInternalHelper::readFloat(p, buf);
                }
                    break;
                case varint_double: {
                    auto p = (ptrd_t)instance_ptr;
                    offset = CHInternalHelper::readDouble(p, buf);
                }
                    break;
                default:
                    break;
            }
        }
            break;
        case CHTagBufferWriteTypeContainer:
            break;
        case CHTagBufferWriteTypeblobStream:
            break;
        case CHTagBufferWriteTypeTagBuffer:
            break;
        default:
            break;
    }
    return offset;
}

id CHTagBufferBuilder::readTagBuffer(NSData *data, id instance)
{
    if (!data || !instance) {
        return nil;
    }

    Class cls = [instance class];
    uint32_t count = 0;
    Ivar *list = class_copyIvarList(cls, &count);
    if (count == 0) {
        return nil;
    }

    _d->readBuffer->clear();
    _d->readBuffer->resize(data.length);
    char *cp = _d->readBuffer->data();
    [data enumerateByteRangesUsingBlock:^(const void * _Nonnull bytes, NSRange byteRange, BOOL * _Nonnull stop) {
        memcpy(cp + byteRange.location, bytes, sizeof(char) * byteRange.length);
    }];

    char *end = cp + _d->readBuffer->size();

    union __tag_buffer_flag tag{0};
    auto tag_p = &tag.tag;
    for (;;) {
        if (cp >= end || tag_p->next) {
            break;
        }
        tag.itag = CHInternalHelper::readTag(cp);
        cp += sizeof(tag.itag);
        if (tag_p->fieldNumber >= count) {
            NSCAssert(NO, @"");
            break;
        }
        Ivar p = list[tag_p->fieldNumber];
        cp += readByWriteType(*tag_p, cp, p, instance);
    }
    free(list);

    return instance;
}