//
//  CHTagBuffer.h
//  TagBuf
//
//  Created by hejunqiu on 16/7/22.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import <Foundation/Foundation.h>

/**
 * @author hejunqiu, 16-08-18 15:08:50
 *
 * Non Thread Safe. All threads have common builder to build data.
 * Your custom class should be inherited CHTagBuffer.
 */
@interface CHTagBuffer : NSObject

- (NSData *)toTagBuf;
+ (instancetype)tagBufferWithTagBuf:(NSData *)tagBuf;

@end

/**
 * @author hejunqiu, 16-08-18 15:08:27
 *
 * Thread Safe. Every thread has own builder to build data.
 * Just be inherited NSObject class. However because thread has own builder,
 * performance may be reduced.
 */
@interface NSObject (CHTagBuffer)

- (NSData *)toTagBuffer;
+ (instancetype)objectWithTagBuffer:(NSData *)tagBuffer;

@end
