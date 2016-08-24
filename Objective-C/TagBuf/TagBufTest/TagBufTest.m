//
//  TagBufTest.m
//  TagBufTest
//
//  Created by hejunqiu on 16/7/27.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "CHDataModel.h"
#import "CHPerformanceTestModel.h"

@interface TagBufTest : XCTestCase

@end

@implementation TagBufTest

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testExample {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

- (void)test1
{
    [self measureBlock:^{
        CHDataModel *model = [CHDataModel new];
        model.obj0 = 0x1234567890ffeeddll;
        model.prority = 2344.54423;
        model.level = 335436343243.654743;
        model.str = @"abcdef___dddddfergew34534efewedddddddmmmmmmmmmm";
        model.ch0 = 'A';
        model.sh0 = 353;
        model.t_b = YES;
        model.f_b = NO;
        model.obj1 = [CHDataModel1 new];
        model.obj1.str11 = @"feferwvefsvw frewfrew f";
        model.arrayString = @[@"12432543", @"dewf34ffw"];
        model.array = (NSArray<CHDataModel1> *)(@[[CHDataModel1 new], [CHDataModel1 new], [CHDataModel1 new]]);
        model.strEmbedArray = (NSArray<NSArrayNSArray> *)@[@[@"fefer", @"fert45t4", @"fergert"], @[@"freger", @"dewfre"], @[@"fersvt54 4t34_d34d3"]];
        NSData *data = model.toTagBuf;

        CHDataModel *model2 = [CHDataModel tagBufferWithTagBuf:data];
        XCTAssertEqual(model.obj0, model2.obj0);
        XCTAssertEqual(model.prority, model2.prority);
        XCTAssertEqual(model.level, model2.level);
        XCTAssertEqual(model.ch0, model2.ch0);
        XCTAssertEqual(model.sh0, model2.sh0);
        XCTAssertEqual(model.t_b, model2.t_b);
        XCTAssertEqual(model.f_b, model2.f_b);
        XCTAssertEqualObjects(model.str, model2.str);
        XCTAssertEqualObjects(model.obj1.str11, model2.obj1.str11);
        XCTAssertEqualObjects(model.arrayString, model2.arrayString);
        XCTAssertEqualObjects(model.array, model2.array);
    }];
}

- (void)testTagBufPerformanceGenrateObject1Million
{
    CHPerformanceTestModel *model = [CHPerformanceTestModel new];
    [model fillTestData]; // genrated 1220 bytes data.

    [self measureBlock:^{
        int i = 1e5;
        while (i-->0) {
            @autoreleasepool {
                (void) [model toTagBuf];
            }
        }
    }];
    NSLog(@"END.");
}

- (void)testTagBufPerformanceToBuffer1Million
{
    CHPerformanceTestModel *model = [CHPerformanceTestModel new];
    [model fillTestData]; // genrated 1220 bytes data.
    NSData *data = model.toTagBuf;

    [self measureBlock:^{
        int i = 1e5;
        while (i-->0) {
            @autoreleasepool {
                (void) [CHPerformanceTestModel tagBufferWithTagBuf:data];
            }
        }
    }];
    NSLog(@"END.");
}

- (void)testJSONModelPerformanceGenrateObject1Million
{
    CHPerformanceTestJSONModel *model = [CHPerformanceTestJSONModel new];
    [model fillTestData];

    [self measureBlock:^{
        int i = 1e5;
        while (i-->0) {
            @autoreleasepool {
                (void) [model toJSONString];
            }
        }
    }];
}

- (void)testJSONModelPerformanceToBuffer1Million
{
    CHPerformanceTestJSONModel *model = [CHPerformanceTestJSONModel new];
    [model fillTestData];
    NSString *json = [model toJSONString]; // genrated 1231 bytes data.

    [self measureBlock:^{
        int i = 1e5;
        while (i-->0) {
            @autoreleasepool {
                (void)[[CHPerformanceTestJSONModel alloc] initWithString:json error:nil];
            }
        }
    }];
}

- (void)testCFStringToTagBuf
{
    CHPerformanceTestModel *model = CHPerformanceTestModel.new;
    [model fillTestData];

    NSData *data = model.toTagBuf;
    CHPerformanceTestModel *target = [CHPerformanceTestModel tagBufferWithTagBuf:data];
    XCTAssertEqualObjects(model, target);
}
@end
