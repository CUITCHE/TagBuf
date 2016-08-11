//
//  CHTagBuffer.m
//  TagBuf
//
//  Created by hejunqiu on 16/7/22.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import "CHTagBuffer.h"
#include <objc/runtime.h>
#import "CHTagBufferBuilder.h"

static CHTagBufferBuilder *___builder = new CHTagBufferBuilder();

@implementation CHTagBuffer

- (NSData *)toTagBuf
{
    ___builder->startBuildingWithObject(self);
    return ___builder->buildedData();
}

+ (instancetype)tagBufferWithTagBuf:(NSData *)tagBuf
{
    id instance = [[self alloc] init];
    ___builder->readTagBuffer(tagBuf, instance);
    return instance;
}

@end

@implementation NSObject (CHTagBuffer)

- (NSData *)toTagBuffer
{
    CHTagBufferBuilder *builder = new CHTagBufferBuilder();
    return builder->buildedData();
}

+ (instancetype)objectWithTagBuffer:(NSData *)tagBuffer
{
    CHTagBufferBuilder *builder = new CHTagBufferBuilder();
    id instance = [[self alloc] init];
    builder->readTagBuffer(tagBuffer, instance);
    return instance;
}

@end