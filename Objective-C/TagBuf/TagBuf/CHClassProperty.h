//
//  CHClassProperty.h
//  TagBuf
//
//  Created by hejunqiu on 16/8/16.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import "tagBuf.h"

typedef NS_ENUM(uint32_t, CHTagBufObjectDetailType) {
    CHTagBufObjectDetailTypeNone,
    /// NSNumber<NSNumberBoolean> value.
    CHTagBufObjectDetailTypeNSNumberBoolean,
    /// NSNumber<NSNumberInt8> value.
    CHTagBufObjectDetailTypeNSNumber8BitsInteger,
    /// NSNumber<NSNumberInt16> value.
    CHTagBufObjectDetailTypeNSNumber16BitsInteger,
    /// NSNumber<NSNumberInt32> value.
    CHTagBufObjectDetailTypeNSNumber32BitsInteger,
    /// NSNumber<NSNumberInt64> value.
    CHTagBufObjectDetailTypeNSNumber64BitsInteger,
    /// NSNumber<NSNumberFloat> value.
    CHTagBufObjectDetailTypeNSNumberFloat,
    /// NSNumber<NSNumberDouble> value.
    CHTagBufObjectDetailTypeNSNumberDouble,
    CHTagBufObjectDetailTypeNSArrayNSData,
    CHTagBufObjectDetailTypeNSArrayNSString,
    CHTagBufObjectDetailTypeNSArrayNSArray,
    CHTagBufObjectDetailTypeOtherObject
};

@interface CHClassProperty : NSObject

@property (nonatomic, strong) NSString *propertyName;

@property (nonatomic) CHTagBufferWriteType writeType;
@property (nonatomic) CHTagBufEncodingType encodingType;
/// Specify the string of protocol.
@property (nonatomic) CHTagBufObjectDetailType detailType;

@property (nonatomic) Class propertyClassType;

@property (nonatomic, strong) NSString *protocol;
@property (nonatomic) BOOL isOptional;
@property (nonatomic) BOOL isIgnore;

@property (nonatomic) uint32_t fieldNumber;

@end

#define printCHTagBufObjectDetailType(type) [@[tag_contact(CHTagBufObjectDetailTypeNone),\
tag_contact(CHTagBufObjectDetailTypeNSNumberBoolean),\
tag_contact(CHTagBufObjectDetailTypeNSNumber8BitsInteger),\
tag_contact(CHTagBufObjectDetailTypeNSNumber16BitsInteger),\
tag_contact(CHTagBufObjectDetailTypeNSNumber32BitsInteger),\
tag_contact(CHTagBufObjectDetailTypeNSNumber64BitsInteger),\
tag_contact(CHTagBufObjectDetailTypeNSNumberFloat),\
tag_contact(CHTagBufObjectDetailTypeNSNumberDouble),\
tag_contact(CHTagBufObjectDetailTypeNSArrayNSData),\
tag_contact(CHTagBufObjectDetailTypeNSArrayNSString),\
tag_contact(CHTagBufObjectDetailTypeNSArrayNSArray),\
tag_contact(CHTagBufObjectDetailTypeOtherObject),\
] objectAtIndex:type]