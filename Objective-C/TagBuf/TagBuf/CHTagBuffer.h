//
//  CHTagBuffer.h
//  TagBuf
//
//  Created by hejunqiu on 16/7/22.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol optional
@end

@protocol require
@end

@interface CHTagBuffer : NSObject

+ (NSData *)toTagBuf;

+ (instancetype)instanceWillTagBuf:(NSData *)tagBuf;

@end
