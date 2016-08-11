//
//  CHDataModel.h
//  TagBuf
//
//  Created by hejunqiu on 16/7/27.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import "CHTagBuffer.h"

@interface CHDataModel1 : CHTagBuffer

@property (nonatomic, strong) NSString *str;

@end

@interface CHDataModel : CHTagBuffer

@property (nonatomic) NSInteger obj0;
@property float prority;
@property double level;

@property (nonatomic, strong) NSString *str;
@property (nonatomic) char ch0;
@property (nonatomic) short sh0;

@property (nonatomic) BOOL t_b;
@property (nonatomic) BOOL f_b;
@property (nonatomic, strong) CHDataModel1 *obj1;
@end
