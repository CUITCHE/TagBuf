//
//  CHClassProperty.m
//  TagBuf
//
//  Created by hejunqiu on 16/8/16.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import "CHClassProperty.h"

@implementation CHClassProperty

- (NSString *)description
{
    NSMutableArray *properties = [NSMutableArray arrayWithCapacity:8];
    [properties addObject:@(self.fieldNumber)];
    [properties addObject:printCHTagBufEncodingType(self.encodingType)];
    [properties addObject:printCHTagBufObjectDetailType(self.detailType)];
    if (self.isOptional) {
        [properties addObject:@"optional"];
    }
    if (self.isIgnore) {
        [properties addObject:@"ignore"];
    }
    NSString *propertiesString = [NSString stringWithFormat:@"(%@)", [properties componentsJoinedByString:@", "]];
    NSString *desc = [NSString stringWithFormat:@"<property[%@], protocol[%@], type[%@]\n%@>", self.propertyName, self.protocol, self.propertyClassType, propertiesString];
    return desc;
}

@end
