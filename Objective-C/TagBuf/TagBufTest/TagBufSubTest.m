//
//  TagBufSubTest.m
//  TagBuf
//
//  Created by hejunqiu on 16/8/18.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "CHTestModels.h"

@interface TagBufSubTest : XCTestCase

@end

@implementation TagBufSubTest

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testNSNumber
{
    TestNSNumber *model = TestNSNumber.new;
    model._1 = (NSNumber<NSNumberBoolean> *)@(true);
    model._2 = (NSNumber<NSNumberInt8> *)@('R');
    model._3 = (NSNumber<NSNumberInt16> *)@((uint16_t)456345);
    model._4 = (NSNumber<NSNumberInt32> *)@(3467633547);
    model._5 = (NSNumber<NSNumberInt64> *)@(0x1122334455667788ll);
    model._6 = (NSNumber<NSNumberFloat> *)@(5434.654);
    model._7 = (NSNumber<NSNumberDouble> *)@(543523432642.6547567);

    NSData *data = model.toTagBuf;
    TestNSNumber *target = [TestNSNumber tagBufferWithTagBuf:data];

    XCTAssertEqualObjects(model._1, target._1);
    XCTAssertEqualObjects(model._2, target._2);
    XCTAssertEqualObjects(model._3, target._3);
    XCTAssertEqualObjects(model._4, target._4);
    XCTAssertEqualObjects(model._5, target._5);
    XCTAssertEqual(model._6.floatValue - target._6.floatValue, 0.00);
    XCTAssertEqualObjects(model._7, target._7);
}

- (void)testPodType
{
    TestPodType *model = TestPodType.new;
    model._1 = YES;
    model._2 = '3';
    model._3 = 35265;
    model._4 = 3465474245;
    model._5 = 0x1122334455667788ll;
    model._6 = 356.4;
    model._7 = 2353464897894.6523543;
    model._8 = false;

    NSData *data = model.toTagBuf;
    TestPodType *target = [TestPodType tagBufferWithTagBuf:data];
    XCTAssertEqual(model._1, target._1);
    XCTAssertEqual(model._2, target._2);
    XCTAssertEqual(model._3, target._3);
    XCTAssertEqual(model._4, target._4);
    XCTAssertEqual(model._5, target._5);
    XCTAssertEqual(model._6, target._6);
    XCTAssertEqual(model._7, target._7);
    XCTAssertEqual(model._8, target._8);
}

- (void)testNSString
{
    TestNSString *model = TestNSString.new;
    model._1 = @"fewfwiuevb43892578234vb43 432v582394v32tvt3289 g342v8y5382yt324y5b83trewjkf nrem woih84325v";

    NSData *data = model.toTagBuf;
    XCTAssert(data.length == model._1.length + 4 + 2 + 8);

    TestNSString *target = [TestNSString tagBufferWithTagBuf:data];
    XCTAssertEqualObjects(model._1, target._1);
    XCTAssertEqualObjects(model._3, target._2);
    XCTAssertEqualObjects(model._3, target._3);

    model._2 = (NSString<optional>*)@"12423543 eafhvr342534 j943ww";
    data = model.toTagBuf;
    target = [TestNSString tagBufferWithTagBuf:data];
    XCTAssertEqualObjects(model._1, target._1);
    XCTAssertEqualObjects(model._2, target._2);
    XCTAssertEqualObjects(model._3, target._3);
}

- (void)testNSData
{
    TestNSData *model = TestNSData.new;
    model._1 = [NSData dataWithBytes:"1234567890" length:11];

    NSData *data = model.toTagBuf;
    TestNSData *target = [TestNSData tagBufferWithTagBuf:data];
    XCTAssertEqualObjects(model._1, target._1);
    XCTAssertEqualObjects(model._2, target._2);
    XCTAssertEqualObjects(model._3, target._3);
}

#define CHAssertEqualObjects(var) XCTAssertEqualObjects(model.var, target.var);

- (void)testNSArrayEmpty
{
    TestNSArrayEmpty *model = TestNSArrayEmpty.new;
    model._1 = @[];

    NSData *data = model.toTagBuf;

    TestNSArrayEmpty *target = [TestNSArrayEmpty tagBufferWithTagBuf:data];
    CHAssertEqualObjects(_1);
}

- (void)testNSArray
{
    TestNSArray *model = TestNSArray.new;
    model._1 = @[@"fefer463f34",@"fefer463f34",@"fefer463f34",@"fefer463f34",@"fefer463f34",@"fefer463f34"];
    model._4 = (NSArray<NSArrayNSArray> *)@[@[@"fewfew535555", @"ferfgreg", @"ferwvt5 "],
                 @[@"fewfew535555", @"ferfgreg", @"ferwvt5 "],
                 @[@"fewfew535555", @"ferfgreg", @"ferwvt5 "],
                 @[@"fewfew535555", @"ferfgreg", @"ferwvt5 "],
                 @[@"fewfew535555", @"ferfgreg", @"ferwvt5 "],
                 @[@"fewfew535555", @"ferfgreg", @"ferwvt5 "],
                 @[@"fewfew535555", @"ferfgreg", @"ferwvt5 "]];
    model._5 = model._1.copy;
    model._6 = @[[NSData dataWithBytes:@"1234567890" length:11],
                 [NSData dataWithBytes:@"1234567890" length:11],
                 [NSData dataWithBytes:@"1234567890" length:11],
                 [NSData dataWithBytes:@"1234567890" length:11],
                 [NSData dataWithBytes:@"1234567890" length:11],
                 [NSData dataWithBytes:@"1234567890" length:11],
                 [NSData dataWithBytes:@"1234567890" length:11],
                 [NSData dataWithBytes:@"1234567890" length:11]].copy;
    model._7 = @[@(YES),@(NO),@(YES),@(YES),@(YES),@(NO),@(YES),@(NO),@(NO),
                 @(YES),@(YES),@(YES),@(NO),@(YES),@(YES),@(NO),@(YES),@(YES),].copy;
    model._8 = @[@('A'),@('B'),@('C'),@('D'),@('A'),@('E'),@('G'),@('D'),@('3'),
                 @('u'),@('.'),@('f'),@('j'),@('~'),@('['),@(' '),@('7'),@('5'),].copy;
    model._9 = @[@((uint16_t)4654),@((uint16_t)4654),@((uint16_t)4654),@((uint16_t)4654),
                 @((uint16_t)4654),@((uint16_t)4654),@((uint16_t)4654),@((uint16_t)4654),].copy;
    model._10 = @[@(0x343acd),@(0x343acd),@(0x343acd),@(0x343acd),@(0x343acd),@(0x343acd),
                  @(0x343acd),@(0x343acd),@(0x343acd),@(0x343acd),@(0x343acd),].copy;
    model._11 = @[@(0x1122334455667788ll),@(0x1122334455667788ll),@(0x1122334455667788ll),
                  @(0x1122334455667788ll),@(0x1122334455667788ll),@(0x1122334455667788ll),
                  @(0x1122334455667788ll),@(0x1122334455667788ll),@(0x1122334455667788ll),
                  @(0x1122334455667788ll),@(0x1122334455667788ll),@(0x1122334455667788ll),
                  @(0x1122334455667788ll),@(0x1122334455667788ll),@(0x1122334455667788ll),].copy;
    model._12 = @[@(343.2),@(343.2),@(343.2),@(343.2),@(343.2),@(343.2),@(343.2),
                  @(343.2),@(343.2),@(343.2),@(343.2),@(343.2),@(343.2),@(343.2),].copy;
    model._13 = @[@(354546353.54632),@(354546353.54632),@(354546353.54632),@(354546353.54632),
                  @(354546353.54632),@(354546353.54632),@(354546353.54632),@(354546353.54632),].copy;

    NSData *data = model.toTagBuf;
    TestNSArray *target = [TestNSArray tagBufferWithTagBuf:data];
    CHAssertEqualObjects(_1);
    CHAssertEqualObjects(_2);
    CHAssertEqualObjects(_3);
    CHAssertEqualObjects(_4);
    CHAssertEqualObjects(_5);
    CHAssertEqualObjects(_6);
    CHAssertEqualObjects(_7);
    CHAssertEqualObjects(_8);
    CHAssertEqualObjects(_9);
    CHAssertEqualObjects(_10);
    CHAssertEqualObjects(_11);
    for (NSUInteger i=0; i<model._12.count; ++i) {
        XCTAssertEqual([model._12[i] floatValue] - [target._12[i] floatValue] , 0);
    }
    CHAssertEqualObjects(_13);
}

@end
