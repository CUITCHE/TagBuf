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
