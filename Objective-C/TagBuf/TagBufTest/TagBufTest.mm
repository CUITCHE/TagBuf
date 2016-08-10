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
    CHDataModel *model = [CHDataModel new];
    model.obj0 = 0x100000;
    model.prority = 2344.54423;
    model.level = 335436343243.654743;
    NSData *data = model.toTagBuf;
    XCTAssertNotNil(data, @"");

    CHDataModel *model2 = [CHDataModel tagBufferWithTagBuf:data];
    XCTAssertEqual(model.obj0, model2.obj0);
    XCTAssertEqual(model.prority, model2.prority);
    XCTAssertEqual(model.level, model2.level);
}
@end
