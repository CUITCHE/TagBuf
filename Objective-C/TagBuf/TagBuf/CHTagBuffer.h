//
//  CHTagBuffer.h
//  TagBuf
//
//  Created by hejunqiu on 16/7/22.
//  Copyright © 2016年 CHE. All rights reserved.
//

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
@protocol required
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

@interface CHTagBuffer : NSObject

- (NSData *)toTagBuf;

+ (instancetype)tagBufferWithTagBuf:(NSData *)tagBuf;

@end

@interface NSObject (CHTagBuffer)

- (NSData *)toTagBuffer;
+ (instancetype)objectWithTagBuffer:(NSData *)tagBuffer;

@end
