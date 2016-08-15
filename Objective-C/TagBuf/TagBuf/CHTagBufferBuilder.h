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
    /// kinds of Integers, zigzag-int, zigzag-int64. Above also support unsinged type. double value, 8 bytes, float value, 4 bytes. And about bool value, specially it only takes up 1 bit.
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
    CHTagBufEncodingTypeBool   = 1,
    CHTagBufEncodingType8Bits  = 2,
    CHTagBufEncodingType16Bits = 3,
    CHTagBufEncodingType32Bits = 4,
    CHTagBufEncodingType64Bits = 5,
    CHTagBufEncodingTypeFloat  = 6,
    CHTagBufEncodingTypeDouble = 7,
    CHTagBufEncodingTypeObject = 8
};


class CHTagBufferBuilder
{
    struct CHTagBufferBuilderPrivate *_d;
public:
    CHTagBufferBuilder();
    ~CHTagBufferBuilder();

    void startBuildingWithObject(id instance);
    id readTagBuffer(NSData *data, id instance);

    NSData *buildedData();
};