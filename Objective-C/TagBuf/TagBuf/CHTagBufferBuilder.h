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
    /// kinds of Integers, zigzag-int, zigzag-int64. Above also support unsinged type. double value, 8 bytes, float value, 4 bytes.
    CHTagBufferWriteTypeVarintFixed = 0,
    /// container, such as NSArray
    CHTagBufferWriteTypeContainer   = 1,
    /// such as NSString, c-style string, NSData
    CHTagBufferWriteTypeblobStream  = 2,
    /// data from the object which is kind of CHTagBuffer Class.
    CHTagBufferWriteTypeTagBuffer   = 3
};

#define FLOAT_FLAG (1 << 4)

typedef NS_ENUM(uint32_t, CHTagBufEncodingType) {
    CHTagBufEncodingTypeNone   = 0,
    CHTagBufEncodingType8Bits  = 1,
    CHTagBufEncodingType16Bits = 2,
    CHTagBufEncodingType32Bits = 3,
    CHTagBufEncodingType64Bits = 4,
    CHTagBufEncodingTypeFloat  = 5,
    CHTagBufEncodingTypeDouble = 6,
    CHTagBufEncodingTypeObject = 7
};

#define WriteAPI public
#define ReadAPI public

typedef struct objc_ivar *Ivar;
typedef struct objc_class *Class;

class CHTagBufferBuilder
{
    struct CHTagBufferBuilderPrivate *_d;
    friend struct CHInternalHelper;
public:
    CHTagBufferBuilder();
    ~CHTagBufferBuilder();
    void startBuildingWithObject(id instance);
    NSData *buildedData();
WriteAPI:
    void writeTagBuffer(id instance);
    void writeTag();
    void writeContainer(NSArray *container);

    void decodeFromTypeEncoding(const char *typeCoding, CHTagBufEncodingType &encodingType);
    void writeByEncodingType(CHTagBufEncodingType type, Ivar ivar, id instance);
    // Objc
    void writeObjcect(id obj);
ReadAPI:
    id readTagBuffer(NSData *data, Class cls);
};