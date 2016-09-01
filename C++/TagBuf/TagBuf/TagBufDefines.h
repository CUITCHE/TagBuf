//
//  TagBufDefines.h
//  TagBuf
//
//  Created by hejunqiu on 16/8/31.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef TagBufDefines_h
#define TagBufDefines_h

enum {
    /// kinds of Integers, zigzag-int, zigzag-int64. Above also support unsinged type. double value, 8 bytes, float value, 4 bytes. And about bool value, specially it only takes up 1 bit.
    CHTagBufferWriteTypeVarintFixed = 0,
    /// vector<T>(T is like integer, float, double, bool, class inherts from CHTagBuf) in C++.
    CHTagBufferWriteTypeContainer   = 1,
    /// such as string
    CHTagBufferWriteTypeblobStream  = 2,
    /// data from the object which is kind of CHTagBuf Class.
    CHTagBufferWriteTypeTagBuffer   = 3
};

enum {
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

enum {
    CHTagBufObjectDetailTypeNone,
    /// vector<NSNumberBoolean> value.
    CHTagBufObjectDetailTypeNSNumberBoolean,
    /// vector<NSNumberInt8> value.
    CHTagBufObjectDetailTypeNSNumber8BitsInteger,
    /// vector<NSNumberInt16> value.
    CHTagBufObjectDetailTypeNSNumber16BitsInteger,
    /// vector<NSNumberInt32> value.
    CHTagBufObjectDetailTypeNSNumber32BitsInteger,
    /// vector<NSNumberInt64> value.
    CHTagBufObjectDetailTypeNSNumber64BitsInteger,
    /// vector<NSNumberFloat> value.
    CHTagBufObjectDetailTypeNSNumberFloat,
    /// vector<NSNumberDouble> value.
    CHTagBufObjectDetailTypeNSNumberDouble,
    CHTagBufObjectDetailTypeNSArrayNSData,
    CHTagBufObjectDetailTypeNSArrayNSString,
    CHTagBufObjectDetailTypeNSArrayNSArray,
    CHTagBufObjectDetailTypeOtherObject
};

#endif /* TagBufDefines_h */
