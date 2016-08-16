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
    CHTagBufObjectDetailTypeNSString = 1,
    CHTagBufObjectDetailTypeNSArray,
    CHTagBufObjectDetailTypeNSNumberBoolean,
    CHTagBufObjectDetailTypeNSNumber8BitsInteger,
    CHTagBufObjectDetailTypeNSNumber16BitsInteger,
    CHTagBufObjectDetailTypeNSNumber32BitsInteger,
    CHTagBufObjectDetailTypeNSNumber64BitsInteger,
    CHTagBufObjectDetailTypeNSNumberFloat,
    CHTagBufObjectDetailTypeNSNumberDouble,
    CHTagBufObjectDetailTypeNSData,
    CHTagBufObjectDetailTypeOtherObject
};

@interface CHClassProperty : NSObject

@property (nonatomic, strong) NSString *propertyName;

@property (nonatomic) CHTagBufferWriteType writeType;
@property (nonatomic) CHTagBufEncodingType encodingType;
@property (nonatomic) CHTagBufObjectDetailType detailType;

@property (nonatomic) Class propertyClassType;

@property (nonatomic, strong) NSString *protocol;
@property (nonatomic) BOOL isOptional;
@property (nonatomic) BOOL isIgnore;

@property (nonatomic) uint32_t fieldNumber;
@end
