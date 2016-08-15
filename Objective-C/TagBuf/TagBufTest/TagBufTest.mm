//
//  TagBufTest.m
//  TagBufTest
//
//  Created by hejunqiu on 16/7/27.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "CHTagBufferBuilder.h"
#import "CHDataModel.h"

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
    static NSMutableData *s_data = [NSMutableData dataWithCapacity:4096];
//    [self measureBlock:^{
        CHDataModel *model = [CHDataModel new];
        model.obj0 = 0x1234567890ffeeddll;
        model.prority = 2344.54423;
        model.level = 335436343243.654743;
        model.str = @"abcdef___ddddd嘿嘿嘿哈哈哈哈哈";
        model.ch0 = 'A';
        model.sh0 = 353;
        model.t_b = YES;
        model.f_b = NO;
        model.obj1 = [CHDataModel1 new];
        model.obj1.str11 = @"feferwvefsvw frewfrew f";
//        model.array = (NSArray<CHDataModel1> *)(@[[CHDataModel1 new], [CHDataModel1 new], [CHDataModel1 new]]);
        NSData *data = model.toTagBuf;
        if (s_data.length == 0) {
            [s_data setData:data];
        } else {
            XCTAssertEqualObjects(s_data, data, @"data wrong.");
            [s_data setData:data];
        }

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
//        XCTAssertEqualObjects(model.array, model2.array);
//    }];
}
@end
