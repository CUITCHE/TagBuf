//
//  CHTagBuffer.h
//  TagBuf
//
//  Created by hejunqiu on 16/7/22.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface CHTagBuffer : NSObject

- (NSData *)toTagBuf;

+ (instancetype)tagBufferWithTagBuf:(NSData *)tagBuf;

@end

@interface NSObject (CHTagBuffer)

- (NSData *)toTagBuffer;
+ (instancetype)objectWithTagBuffer:(NSData *)tagBuffer;

@end
