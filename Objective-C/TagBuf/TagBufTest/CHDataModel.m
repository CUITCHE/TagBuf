//
//  CHDataModel.m
//  TagBuf
//
//  Created by hejunqiu on 16/7/27.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import "CHDataModel.h"

@implementation CHDataModel1
- (instancetype)init
{
    self = [super init];
    self.str11 = @"123243543";
    return self;
}

- (BOOL)isEqual:(CHDataModel1 *)object
{
    return [object.str11 isEqualToString:self.str11];
}
@end


@implementation CHDataModel


@end
