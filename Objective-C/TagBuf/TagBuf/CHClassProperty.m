//
//  CHClassProperty.m
//  TagBuf
//
//  Created by hejunqiu on 16/8/16.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import "CHClassProperty.h"

struct ___internalFlags {
    uint32_t isOptional : 1;
    uint32_t isIgnore : 1;
    uint32_t needFree : 1;
};

@interface CHClassProperty ()
{
    struct ___internalFlags flags;
}
@end

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
    NSString *desc = [NSString stringWithFormat:@"<property[%@], protocol[%@], type[%@]\n%@>", self.propertyName, self.protocolClassType, self.propertyClassType, propertiesString];
    return desc;
}

- (void)dealloc
{
    if (self.needFreeIvar) {
        if (self.ivar) {
            free(_ivar);
        }
    }
}

#pragma mark - property
- (void)setIsOptional:(BOOL)isOptional
{
    flags.isOptional = isOptional;
}

- (BOOL)isOptional
{
    return flags.isOptional;
}

- (void)setIsIgnore:(BOOL)isIgnore
{
    flags.isIgnore = isIgnore;
}

- (BOOL)isIgnore
{
    return flags.isIgnore;
}

- (void)setNeedFreeIvar:(BOOL)needFreeIvar
{
    flags.needFree = needFreeIvar;
}

- (BOOL)needFreeIvar
{
    return flags.needFree;
}
@end
