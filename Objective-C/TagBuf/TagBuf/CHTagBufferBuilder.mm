//
//  CHTagBufferBuilder.m
//  TagBuf
//
//  Created by hejunqiu on 16/7/22.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import "CHTagBufferBuilder.h"
#include <objc/runtime.h>
#import "tagBuf.h"
#import "CHClassProperty.h"
#import "TagBufBitset.h"
#include "CFStringRefTools.h"
#include "CHBase.hpp"

using namespace std;

typedef struct __storageParametersWrite {
    NSMutableData *writeBuffer;
    vector<char> *extraBuffer;
    __tag_buffer_flag__ tag;
    CHClassProperty *property;
} __storageParametersWrite;

struct CHInternalHelper
{
private:
    NS_INLINE void getBufferAndLength(NSString *str, const char *&dest, CFIndex &length, vector<char> *extraBuffer)
    {
        CHCFStringGetBuffer((__bridge CFStringRef)str, dest, length);
        if (!dest) {
            NSCAssert(NO, @"Foot into unexpected branch-code."); // In fact, we don't wish go here.
            if (extraBuffer->size() < length) {
                extraBuffer->resize(length);
            }
            Boolean suc = CFStringGetCString((__bridge CFStringRef)str,
                                             extraBuffer->data(),
                                             length,
                                             NSUTF8StringEncoding);
            if (suc) {
                dest = extraBuffer->data();
            } else {
                dest = [str UTF8String];
            }
        }
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

    NS_INLINE Ivar getIvarWithPropertyName(const char *propertyName, Ivar *iList, uint32_t count)
    {
        if (iList == nullptr) {
            NSCAssert(NO, @"Must not be NULL.");
            return nullptr;
        }
        Ivar *p = iList;
        ++count;
        const char *i = nullptr;
        while (--count) {
            i = ivar_getName(*p);
            if (!strcmp(propertyName, ++i)) {
                break;
            }
            ++p;
        }
        return count ? *p : nullptr;
    }

    NS_INLINE NSArray<CHClassProperty *>* getProperties(Class cls)
    {
        NSCAssert(cls, @"Class is nil.");
        NSMutableArray<CHClassProperty *> *properties = [NSMutableArray array];
        uint32_t increment = 0;
        while (cls) {
            if (!strcmp(class_getName(cls), "NSObject")) {
                break;
            }
            if (!strcmp(class_getName(cls), "CHTagBuffer")) {
                break;
            }
            uint32_t count = 0;
            objc_property_t *pList = class_copyPropertyList(cls, &count);
            objc_property_t *p = pList;

            uint32_t ivarCount = 0;
            Ivar *ivarList = class_copyIvarList(cls, &ivarCount); // memory leak.
            NSCAssert(ivarList, @"Logic error.");
            NSCAssert(count == ivarCount, @"Count of property and count of ivar must be equal.");

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
                cp.needFreeIvar = !i;
                cp.fieldNumber = i + increment;
                const char *propertyName = property_getName(property);
                cp.propertyName = @(propertyName);
                cp.ivar = getIvarWithPropertyName(propertyName, ivarList, ivarCount);

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
                            cp.protocolClassType = NSClassFromString(protocolName);
                            NSCAssert(cp.protocolClassType, @"No such Class:%@", protocolName);
                            cp.detailType = CHTagBufObjectDetailTypeOtherObject;
                        }
                        [scanner scanString:@">" intoString:nil];
                    }
                } else {
                    CHTagBufEncodingType encodingType = CHTagBufEncodingTypeNone;
                    decodeFromTypeEncoding(attrs + 1, encodingType);
                    cp.encodingType = encodingType;
                }
                [properties addObject:cp];
            }
            free(pList);
            cls = class_getSuperclass(cls);
            increment = count;
        }
        return properties;
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

    WriteMemoryAPI(void) writeTagBuffer(id instance, NSMutableData *buf, vector<char> *extraBuffer)
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
        __storageParametersWrite pw{buf, extraBuffer};
        for (CHClassProperty *property in properties) {
            pw.tag.itag = 0;
            pw.tag.tag.fieldNumber = property.fieldNumber;
            if (--count == 0) {
                pw.tag.tag.next = 1;
            }
            pw.property = property;
            writeByEncodingType(instance, pw);
        }
    }

    WriteMemoryAPI(void) writeByEncodingType(id instance, __storageParametersWrite &pw)
    {
        if (pw.property.encodingType == CHTagBufEncodingTypeNone) {
            NSCAssert(NO, @"");
            return;
        }
        auto property = pw.property;
        auto &tag = pw.tag;
        auto buf = pw.writeBuffer;
        switch (pw.property.encodingType) {
            case CHTagBufEncodingTypeBool: {
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                tag.tag.internalTag = varint_bool;
                bool *p = (bool *)(reinterpret_cast<NSUInteger>((__bridge void *)(instance)) + ivar_getOffset(property.ivar));
                tag.tag.placeholder12 = *p;
                writeTag(tag, buf);
            }
                break;
            case CHTagBufEncodingType8Bits: {
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                ptr8_t p = (ptr8_t)(reinterpret_cast<NSUInteger>((__bridge void *)(instance)) + ivar_getOffset(property.ivar));
                writeInteger(*p, buf, tag);
            }
                break;
            case CHTagBufEncodingType16Bits: {
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                ptr16_t p = (ptr16_t)(reinterpret_cast<NSUInteger>((__bridge void *)(instance)) + ivar_getOffset(property.ivar));
                writeInteger(*p, buf, tag);
            }
                break;
            case CHTagBufEncodingType32Bits: {
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                ptr32_t p = (ptr32_t)(reinterpret_cast<NSUInteger>((__bridge void *)(instance)) + ivar_getOffset(property.ivar));
                writeInteger(*p, buf, tag);
            }
                break;
            case CHTagBufEncodingType64Bits: {
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                ptr64_t p = (ptr64_t)(reinterpret_cast<NSUInteger>((__bridge void *)(instance)) + ivar_getOffset(property.ivar));
                writeInteger(*p, buf, tag);
            }
                break;
            case CHTagBufEncodingTypeFloat: {
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                ptrf_t p = (ptrf_t)(reinterpret_cast<NSUInteger>((__bridge void *)(instance)) + ivar_getOffset(property.ivar));
                writeFloat(*p, buf, tag);
            }
                break;
            case CHTagBufEncodingTypeDouble: {
                tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                ptrd_t p = (ptrd_t)(reinterpret_cast<NSUInteger>((__bridge void *)(instance)) + ivar_getOffset(property.ivar));
                writeDouble(*p, buf, tag);
            }
                break;
            case CHTagBufEncodingTypeNSNumber:
            case CHTagBufEncodingTypeNSData:
            case CHTagBufEncodingTypeNSString:
            case CHTagBufEncodingTypeNSArray:
            case CHTagBufEncodingTypeOtherObject: {
                id value = object_getIvar(instance, property.ivar);
                if (!value ) {
                    if (property.isIgnore || property.isOptional) {
                        tag.tag.internalTag = object_is_nil;
                        writeTag(tag, buf);
                    } else {
                        @throw [NSException exceptionWithName:@"Object which is not optional or ignore is nil"
                                                       reason:[NSString stringWithFormat:@"The value of %@'s property[%@]",[instance class], property.propertyName]
                                                     userInfo:nil];
                    }
                    return;
                }
                writeObjcect(value, pw);
            }
                break;
            default:
                NSCAssert(NO, @"Logic error.");
                break;
        }

    }

    WriteMemoryAPI(void) writeObjcect(_Nonnull id obj, __storageParametersWrite &pw)
    {
        auto buf = pw.writeBuffer;
        auto &_tag = pw.tag;
        auto property = pw.property;
        switch (property.encodingType) {
            case CHTagBufEncodingTypeNSNumber:
                _tag.tag.writeType = CHTagBufferWriteTypeVarintFixed;
                writeNSNumber(obj, pw);
                break;
            case CHTagBufEncodingTypeNSData:
                _tag.tag.writeType = CHTagBufferWriteTypeblobStream;
                writeNSData(obj, buf, _tag);
                break;
            case CHTagBufEncodingTypeNSString: {
                _tag.tag.writeType = CHTagBufferWriteTypeblobStream;
                writeNSString(obj, pw);
            }
                break;
            case CHTagBufEncodingTypeNSArray:
                _tag.tag.writeType = CHTagBufferWriteTypeContainer;
                writeContainer(obj, pw);
                break;
            case CHTagBufEncodingTypeOtherObject:
                _tag.tag.writeType = CHTagBufferWriteTypeTagBuffer;
                writeTag(_tag, buf);
                _tag.itag = 0;
                writeTagBuffer(obj, buf, pw.extraBuffer);
                break;
            default:
                NSCAssert(NO, @"");
                break;
        }
    }

    WriteMemoryAPI(void) writeNSNumber(NSNumber *number, __storageParametersWrite &pw)
    {
        auto &tag = pw.tag.tag;
        auto &_tag = pw.tag;
        auto buf = pw.writeBuffer;
        auto property = pw.property;
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
    WriteMemoryAPI(void) writeNSString(NSString *str, __storageParametersWrite &pw)
    {
        CFIndex length = 0;
        const char *buf = nullptr;
        getBufferAndLength(str, buf, length, pw.extraBuffer);
        pw.tag.tag.internalTag = stream_nsstring;
        writeInteger<size_t, true, true, false, false>(length, pw.writeBuffer, pw.tag);
        [pw.writeBuffer appendBytes:buf length:sizeof(char) * length];
    }

    WriteMemoryAPI(void) writeNSData(NSData *data, NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        uint64_t length = data.length;
        _tag.tag.internalTag = stream_nsdata;
        writeInteger<uint64_t, true, true, false, false>(length, buf, _tag);
        [buf appendData:data];
    }

    WriteMemoryAPI(void) writeContainer(NSArray *container, __storageParametersWrite &pw)
    {
        auto &tag = pw.tag.tag;
        auto buf = pw.writeBuffer;
        uint64_t count = (uint64_t)container.count;
        if (!count) {
            tag.internalTag = container_none;
            writeTag(pw.tag, buf);
            return;
        }
        auto &_tag = pw.tag;
        auto property = pw.property;

        CHTagBufObjectDetailType detailType = getNSArrayContainerTypeOnRuntime(container.firstObject, property);

        char _buf[8] = {0};
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
                integerToByte(count, (Byte *)_buf);
                [buf appendBytes:_buf length:sizeof(count)]; // write size of bit.
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
                    writeNSStringInList(str, pw);
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
                    writeContainer(a, pw);
                }
                break;
            case CHTagBufObjectDetailTypeOtherObject:
                tag.internalTag = container_object;
                writeCountIntegerToCharBuf(_buf, count, tag);
                writeTag(_tag, buf);
                [buf appendBytes:_buf length:tag.lengthOfZigzag ?: 8];
                _tag.itag = 0;
                for (id obj in container) {
                    writeTagBuffer(obj, buf, pw.extraBuffer);
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

    WriteMemoryAPI(void) writeNSStringInList(NSString *str, __storageParametersWrite &pw)
    {
        CFIndex length = 0;
        const char *buf = nullptr;
        getBufferAndLength(str, buf, length, pw.extraBuffer);
        writeInteger<size_t, false, false, false, false>(length, pw.writeBuffer, pw.tag);
        [pw.writeBuffer appendBytes:buf length:sizeof(char) * length];
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
        writeDouble<false>(v, buf, _tag);
    }

    template <typename T>
    WriteMemoryAPI(void) writeIntegerInList(T integer, NSMutableData *buf, __tag_buffer_flag__ &_tag)
    {
        static_assert(std::is_pod<T>::value == true, "Must be a pod value");
        writeInteger<T, false, false>(integer, buf, _tag);
    }

#pragma mark - Read APIs

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
        if (tag.internalTag == object_is_nil) {
            if (property.isOptional || property.isIgnore) {
                return 0;
            } else {
                @throw [NSException exceptionWithName:@"Transmitted failed."
                                               reason:[NSString stringWithFormat:@"Transmitted data represents nil. But the property[%@] is not [optional] or [ignore].", property.propertyName]
                                             userInfo:nil];
            }
        }
        uint32_t offset = 0;
        switch ((CHTagBufferWriteType)tag.writeType) {
            case CHTagBufferWriteTypeVarintFixed: {
                switch (tag.internalTag) {
                    case varint_bool: {
                        if (property.encodingType == CHTagBufEncodingTypeNSNumber) {
                            object_setIvar(instance, property.ivar, @(!!tag.placeholder12));
                        } else {
                            bool *p = (bool *)(reinterpret_cast<NSUInteger>((__bridge void *)(instance)) + ivar_getOffset(property.ivar));
                            *p = tag.placeholder12;
                        }
                    }
                        break;
                    case varint_8bits: {
                        uint8_t value = 0;
                        offset = byteToPodType(&value, buf);
                        if (property.encodingType == CHTagBufEncodingTypeNSNumber) {
                            object_setIvar(instance, property.ivar, @(value));
                        } else {
                            ptr8_t p = (ptr8_t)(reinterpret_cast<NSUInteger>((__bridge void *)(instance)) + ivar_getOffset(property.ivar));
                            *p = value;
                        }
                    }
                        break;
                    case varint_16bits: {
                        uint16_t value = 0;
                        offset = byteToPodType(&value, buf);
                        if (property.encodingType == CHTagBufEncodingTypeNSNumber) {
                            object_setIvar(instance, property.ivar, @(value));
                        } else {
                            ptr16_t p = (ptr16_t)(reinterpret_cast<NSUInteger>((__bridge void *)(instance)) + ivar_getOffset(property.ivar));
                            *p = value;
                        }
                    }
                        break;
                    case varint_32bits: {
                        uint32_t value = 0;
                        offset = readInteger(&value, tag, buf);
                        if (property.encodingType == CHTagBufEncodingTypeNSNumber) {
                            object_setIvar(instance, property.ivar, @(value));
                        } else {
                            ptr32_t p = (ptr32_t)(reinterpret_cast<NSUInteger>((__bridge void *)(instance)) + ivar_getOffset(property.ivar));
                            *p = value;
                        }
                    }
                        break;
                    case varint_64bits: {
                        uint64_t value = 0;
                        offset = readInteger(&value, tag, buf);
                        if (property.encodingType == CHTagBufEncodingTypeNSNumber) {
                            object_setIvar(instance, property.ivar, @(value));
                        } else {
                            ptr64_t p = (ptr64_t)(reinterpret_cast<NSUInteger>((__bridge void *)(instance)) + ivar_getOffset(property.ivar));
                            *p = value;
                        }
                    }
                        break;
                    case varint_float: {
                        float value = 0;
                        offset = readFloat(&value, buf);
                        if (property.encodingType == CHTagBufEncodingTypeNSNumber) {
                            object_setIvar(instance, property.ivar, @(value));
                        } else {
                            ptrf_t p = (ptrf_t)(reinterpret_cast<NSUInteger>((__bridge void *)(instance)) + ivar_getOffset(property.ivar));
                            *p = value;
                        }
                    }
                        break;
                    case varint_double: {
                        double value = 0;
                        offset = readDouble(&value, buf);
                        if (property.encodingType == CHTagBufEncodingTypeNSNumber) {
                            object_setIvar(instance, property.ivar, @(value));
                        } else {
                            ptrd_t p = (ptrd_t)(reinterpret_cast<NSUInteger>((__bridge void *)(instance)) + ivar_getOffset(property.ivar));
                            *p = value;
                        }
                    }
                        break;
                    default:
                        NSCAssert(NO, @"Logic error.");
                        break;
                }
            }
                break;
            case CHTagBufferWriteTypeContainer: {
                NSArray *array = nil;
                offset = readContainer(&array, tag, buf, property);
                if (array) {
                    object_setIvar(instance, property.ivar, array);
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
                        object_setIvar(instance, property.ivar, str);
                    }
                        break;
                    case stream_nsdata: {
                        NSData *data = [[NSData alloc] initWithBytes:dest length:length];
                        object_setIvar(instance, property.ivar, data);
                    }
                    default:
                        break;
                }
            }
                break;
            case CHTagBufferWriteTypeTagBuffer: {
                Class cls = property.propertyClassType;
                if (!cls) {
                    @throw [NSException exceptionWithName:@"Class does not exist." reason:[NSString stringWithFormat:@"No such Class at property:%@", property.propertyName] userInfo:nil];
                }
                id value = [[cls alloc] init];
                offset = readObject(buf, value);
                object_setIvar(instance, property.ivar, value);
            }
                break;
            default:
                break;
        }
        return offset;
    }

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

    ReadMemoryAPI(uint32_t) readContainer(NSArray **_array, __tag_detail__ tag, const char *buf, CHClassProperty *property)
    {
        NSCAssert(_array, @"Param [_array] can't be nil!");
        if (tag.placeholder12 != 1 && tag.internalTag == container_none) {
            *_array = @[];
            return 0;
        }
        uint64_t count = 0;
        uint32_t offset = readInteger<uint64_t *, true>(&count, tag, buf);
        NSCAssert(count, @"Logic error! count must not be 0.");
        NSMutableArray *array = [NSMutableArray arrayWithCapacity:count];
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
                        offset += readFloat(&value, buf + offset);
                        [array addObject:@(value)];
                    }
                }
                    break;
                case varint_double: {
                    double value = 0;
                    for (uint64_t i=0; i<count; ++i) {
                        offset += readDouble(&value, buf + offset);
                        [array addObject:@(value)];
                    }
                }
                    break;
                case varint_bool: {
                    const char *bitData = buf + offset; // bit data blob stream
                    offset += count;
                    uint64_t bitCount = 0; // bit data size. var[count] represents bit size.
                    offset += byteToPodType(&bitCount, buf + offset);
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
                    Class cls = property.protocolClassType;
                    if (!cls) {
                        NSCAssert(NO, @"No such Class at property:%@", property.propertyName);
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
                    NSArray *embeddedArray = nil;
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


};

#pragma mark - public
CHTagBufferBuilder::CHTagBufferBuilder() :_d(new struct CHTagBufferBuilderPrivate)
{}

CHTagBufferBuilder::~CHTagBufferBuilder()
{
    delete _d;
}

void CHTagBufferBuilder::startBuildingWithObject(id instance)
{
    if (!_d->writeBuf) {
        _d->writeBuf = [NSMutableData dataWithCapacity:4096];
    } else {
        _d->writeBuf.length = 0;
    }

    CHInternalHelper::writeTagBuffer(instance, _d->writeBuf, _d->extraBuffer);
}

NSData *CHTagBufferBuilder::buildedData() const
{
    return _d->writeBuf;
}

id CHTagBufferBuilder::readTagBuffer(NSData *data, id instance)
{
    if (!data || !instance || object_isClass(instance)) {
        return nil;
    }
    if (!_d->readBuffer) {
        _d->readBuffer = new vector<char>(data.length);
    } else {
        _d->readBuffer->resize(data.length);
    }
    char *cp = _d->readBuffer->data();
    [data enumerateByteRangesUsingBlock:^(const void * _Nonnull bytes, NSRange byteRange, BOOL * _Nonnull stop) {
        memcpy(cp + byteRange.location, bytes, sizeof(char) * byteRange.length);
    }];

#ifdef DEBUG
    @try {
        auto final = cp + CHInternalHelper::readObject(cp, instance);
        NSCAssert(final == cp + data.length, @"Logic error.");
    } @catch (NSException *exception) {
        NSLog(@"%@", exception);
    }
#else
    CHInternalHelper::readObject(cp, instance);
#endif

    return instance;
}
