//
//  CHDataModel.h
//  TagBuf
//
//  Created by hejunqiu on 16/7/27.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import "CHTagBuffer.h"

@interface CHDataModel : CHTagBuffer

@property (nonatomic) NSInteger obj0;
@property float prority;
@property double level;

@property (nonatomic, strong) NSString *str;
@property (nonatomic) char ch0;
@property (nonatomic) short sh0;

@end
