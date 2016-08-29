//
//  CHPerformanceTestModel.m
//  TagBuf
//
//  Created by hejunqiu on 16/8/18.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import "CHPerformanceTestModel.h"

@implementation CHPerformanceTestModel

- (void)fillTestData
{
    self.name = 0x23546;
    self.prority = 5346.65473;
    self.number = 35;
    self.opened = true;
    NSMutableString *str = [NSMutableString stringWithCapacity:100];
    int i=100;
    while (i-->0) {
        [str appendString:@"1"];
    }
    self.str100Length = str;

    self._10StringContent = @[str,str,str,str,str,str,str,str,str,str];
}

- (void)fillTestLongStringData
{
    self.name = 0x23546;
    self.prority = 5346.65473;
    self.number = 35;
    self.opened = true;
    NSMutableString *str = [NSMutableString stringWithCapacity:100];
    int i=100;
    while (i-->0) {
        [str appendString:@"14524"];
    }
    i=1000;
    while (i-->0) {
        [str appendString:@"14524fdsav435dc"];
    }
    self.str100Length = str;

    self._10StringContent = @[str,str,str,str,str,str,str,str,str,str];
}

- (BOOL)isEqual:(CHPerformanceTestModel *)object
{
    return self.name == object.name &&
    self.prority - object.prority == 0 &&
    self.number == object.number &&
    self.opened == object.opened &&
    [self.str100Length isEqualToString:object.str100Length] &&
    [self._10StringContent isEqualToArray:object._10StringContent];
}

@end

@implementation CHPerformanceTestJSONModel

- (void)fillTestData
{
    self.name = 0x23546;
    self.prority = 5346.65473;
    self.number = 35;
    self.opened = true;
    NSMutableString *str = [NSMutableString stringWithCapacity:100];
    int i=100;
    while (i-->0) {
        [str appendString:@"1"];
    }
    self.str100Length = str;

    self._10StringContent = @[str,str,str,str,str,str,str,str,str,str];
}

@end
