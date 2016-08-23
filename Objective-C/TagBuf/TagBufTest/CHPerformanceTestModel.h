//
//  CHPerformanceTestModel.h
//  TagBuf
//
//  Created by hejunqiu on 16/8/18.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import "CHTagBuffer.h"
#import "JSONModel.h"

@interface CHPerformanceTestModel : CHTagBuffer

@property (nonatomic) NSUInteger name;
@property (nonatomic) double prority;
@property (nonatomic) short number;
@property (nonatomic) bool opened;

@property (nonatomic, strong) NSString *str100Length;
@property (nonatomic, strong) NSArray *_10StringContent;

- (void)fillTestData;

@end

@interface CHPerformanceTestJSONModel : JSONModel

@property (nonatomic) NSUInteger name;
@property (nonatomic) double prority;
@property (nonatomic) short number;
@property (nonatomic) bool opened;

@property (nonatomic, strong) NSString *str100Length;
@property (nonatomic, strong) NSArray *_10StringContent;

- (void)fillTestData;

@end
