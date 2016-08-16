//
//  tagBuf
//  TagBuf
//
//  Created by hejunqiu on 16/8/16.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef tagBuf_h
#define tagBuf_h

#import <Foundation/Foundation.h>

/**
 * @author hejunqiu, 16-07-27 13:07:23
 *
 * Modify the target to optional, represent the target is optional.
 */
@protocol optional
@end

/**
 * @author hejunqiu, 16-07-27 13:07:08
 *
 * Modify the target to required, represent the target is required. If the target exists
 * in data stream, throws exception. The protocol required is default.
 */
//@protocol required
//@end

/**
 * @author hejunqiu, 16-08-16 16:08:50
 *
 * Modify the target to ignore, represent the target is be ignored. In this case:
 * you have more than one version in your project.
 */
@protocol ignore
@end

/**
 * @author hejunqiu, 16-07-27 17:07:07
 *
 * When A NSNumber in array, you should use the protocol to modify the NSArray.
 * That tell builder to choose which one way to build. The protocol NSNumberInt32
 * is default when this situation.
 */
@protocol NSNumberInt32
@end

/**
 * @author hejunqiu, 16-07-27 17:07:07
 *
 * When A NSNumber in array, you should use the protocol to modify the NSArray.
 * That tell builder to choose which one way to build. The protocol NSNumberInt32
 * is default when this situation.
 */
@protocol NSNumberInt64
@end

@protocol NSNumberBoolean
@end

@protocol NSNumberInt8
@end

@protocol NSNumberInt16
@end

@protocol NSNumberFloat
@end

@protocol NSNumberDouble
@end

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

#endif /* tagBuf_h */
