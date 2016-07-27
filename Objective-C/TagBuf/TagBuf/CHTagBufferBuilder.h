//
//  CHTagBufferBuilder.h
//  TagBuf
//
//  Created by hejunqiu on 16/7/22.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import <Foundation/Foundation.h>

#pragma once

typedef NS_ENUM(uint32_t, CHTagBufferWriteType) {
    /// kinds of Integers, zigzag-int, zigzag-int64. Above also support unsinged type.
    CHTagBufferWriteTypeVarint      = 0,
    /// double value, 8 bytes
    CHTagBufferWriteTypeDouble      = 1,
    /// float value, 4 bytes.
    CHTagBufferWriteTypeFloat       = 2,
    /// container, such as NSArray
    CHTagBufferWriteTypeContainer   = 3,
    /// such as NSString, c-style string, NSData
    CHTagBufferWriteTypeblobStream  = 4,
    /// data from the object which is kind of CHTagBuffer Class.
    CHTagBufferWriteTypeTagBuffer   = 5
};

#define FLOAT_FLAG (1 << 4)

typedef NS_ENUM(uint32_t, CHTagBufEncodingType) {
    CHTagBufEncodingTypeNone   = 0,
    CHTagBufEncodingType8Bits  = 1,
    CHTagBufEncodingType16Bits = 2,
    CHTagBufEncodingType32Bits = 3,
    CHTagBufEncodingType64Bits = 4,
    CHTagBufEncodingTypeObject = 5,
    CHTagBufEncodingTypeFloat  = CHTagBufEncodingType32Bits | FLOAT_FLAG,
    CHTagBufEncodingTypeDouble = CHTagBufEncodingType64Bits | FLOAT_FLAG
};

#define WriteAPI public
#define ReadAPI public

typedef struct objc_ivar *Ivar;

class CHTagBufferBuilder
{
    struct CHTagBufferBuilderPrivate *_d;
    friend struct CHInternalHelper;
public:
    CHTagBufferBuilder();
    ~CHTagBufferBuilder();
    void maker(id instance);
WriteAPI:
    void writeTag();
    void writeCString(const char *str, uint32_t length);
    void writeInteger(long long integer);
    void writeFloat(float v);
    void writeDouble(double v);
    void writeContainer(NSArray *container);

    void decodeFromTypeEncoding(const char *typeCoding, CHTagBufEncodingType &encodingType, CHTagBufferWriteType &writeType);
    void writeByEncodingType(CHTagBufEncodingType type, Ivar ivar, id instance);
    // Objc
    void writeObjcect(id obj);
ReadAPI:
};