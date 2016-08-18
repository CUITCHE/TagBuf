//
//  CHTestModels.m
//  TagBuf
//
//  Created by hejunqiu on 16/8/18.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import "CHTestModels.h"

@implementation CHTestModels

@end

__implementation(NSNumber)

@implementation TestPodType

- (BOOL)isEqual:(TestPodType *)object
{
    return self._1==object._1 &&
           self._2==object._2 &&
           self._3==object._3 &&
    self._4==object._4 &&
    self._5==object._5 &&
    self._6==object._6 &&
    self._7==object._7 &&
    self._8==object._8;
}

@end

__implementation(NSString)

__implementation(NSData)

__implementation(NSArray)

__implementation(NSArrayMultiProtocol)

__implementation(NSArrayOtherObject)

__implementation(NSArrayEmpty)