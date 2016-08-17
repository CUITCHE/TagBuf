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
 * that modify the NSNumber by protocol way represents contains a int32_t type
 * value. And when a NSNumber in array, it represents that array contains int32_t
 * type value. 
 *
 * @note The protocol NSNumberInt32 is default for NSNumber and NSArray.
 */
@protocol NSNumberInt32
@end

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

@protocol NSArrayNSData
@end

@protocol NSArrayNSString
@end

@protocol NSArrayNSArray
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
    /// For check use.
    CHTagBufEncodingTypeNone   = 0,
    /// cplusplus bool.
    CHTagBufEncodingTypeBool   = 1,
    /// 8 bits, such as char.
    CHTagBufEncodingType8Bits  = 2,
    /// 16 bits, such as short
    CHTagBufEncodingType16Bits = 3,
    /// 32 bits, such as int.
    CHTagBufEncodingType32Bits = 4,
    /// 64 bits, such as long long
    CHTagBufEncodingType64Bits = 5,
    /// float value
    CHTagBufEncodingTypeFloat  = 6,
    /// double value
    CHTagBufEncodingTypeDouble = 7,
    /// NSNumber value.
    CHTagBufEncodingTypeNSNumber,
    /// NSData object.
    CHTagBufEncodingTypeNSData,
    /// NSString object.
    CHTagBufEncodingTypeNSString,
    /// NSArray object.
    CHTagBufEncodingTypeNSArray,
    /// other object, such as custom class.
    CHTagBufEncodingTypeOtherObject
};

#ifndef tag_contact
#define tag_contact(x) @#x
#endif

#define printCHTagBufEncodingType(type) [@[tag_contact(CHTagBufEncodingTypeNone),\
tag_contact(CHTagBufEncodingTypeBool),\
tag_contact(CHTagBufEncodingType8Bits),\
tag_contact(CHTagBufEncodingType16Bits),\
tag_contact(CHTagBufEncodingType32Bits),\
tag_contact(CHTagBufEncodingType64Bits),\
tag_contact(CHTagBufEncodingTypeFloat),\
tag_contact(CHTagBufEncodingTypeDouble),\
tag_contact(CHTagBufEncodingTypeNSNumber),\
tag_contact(CHTagBufEncodingTypeNSData),\
tag_contact(CHTagBufEncodingTypeNSString),\
tag_contact(CHTagBufEncodingTypeNSArray),\
tag_contact(CHTagBufEncodingTypeOtherObject),\
] objectAtIndex:type]

#endif /* tagBuf_h */
