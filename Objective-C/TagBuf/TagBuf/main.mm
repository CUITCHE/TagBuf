//
//  main.m
//  TagBuf
//
//  Created by hejunqiu on 16/7/22.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CHPerformanceTestModel.h"

void test()
{
    CHPerformanceTestModel *model = [CHPerformanceTestModel new];
    [model fillTestData]; // genrated 1220 bytes data.
    NSData *data = model.toTagBuf;

    int i = 2e6;
    while (i-->0) {
        @autoreleasepool {
            (void) [CHPerformanceTestModel tagBufferWithTagBuf:data];
        }
    }
    NSLog(@"END.");
}

void test2()
{
    CHPerformanceTestModel *model = [CHPerformanceTestModel new];
    [model fillTestData]; // genrated 1220 bytes data.

    int i = 2e6;
    while (i-->0) {
        @autoreleasepool {
            (void) model.toTagBuf;
        }
    }
    NSLog(@"END.");
}

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        test();
        test2();
    }
    return 0;
}