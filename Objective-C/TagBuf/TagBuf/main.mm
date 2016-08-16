//
//  main.m
//  TagBuf
//
//  Created by hejunqiu on 16/7/22.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import <Foundation/Foundation.h>
#include <objc/runtime.h>
#import "CHTagBufferBuilder.h"
@interface Test : NSObject
{
    int abd;
}

@property (nonatomic, copy) NSString *str;
@property (nonatomic, copy, readonly) NSString *str2;
@property (nonatomic, assign) double num;
@property double num2;
@property NSArray<NSCopying> *ttt;
@property (nonatomic) char ch0;
@property (nonatomic) short s0;
@property (nonatomic) BOOL t_b;
@property (nonatomic, setter=setTb:) bool t_b_cpp;

@property (nonatomic, copy) NSArray<NSCopying, NSCoding> *numbers;
@end

@implementation Test

- (void)setTb:(bool)b
{
    self.t_b_cpp = b;
}

@end

void maker(id instance)
{
    Class cls = [instance class];
    if (class_isMetaClass(cls)) {
        return;
    }
    uint32_t count = 0;
    objc_property_t *list = class_copyPropertyList(cls, &count);
    for (uint32_t i=0; i<count; ++i) {
        objc_property_t property = list[i];
        const char *attribute = property_getAttributes(property);
        const char *name = property_getName(property);
        printf("attribute:%s, name:%s\n", attribute, name);
        id value = [instance valueForKey:[NSString stringWithUTF8String:name]];
//        NSLog(@"%@", value);
    }
}

void func();
void func1();

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        maker([Test new]);
        uint32_t integer = 0x12345678;
        auto integer_ptr = &integer;
        auto p = (const char *)(integer_ptr);
        printf("%x %x %x %x\n", p[0], p[1], p[2], p[3]);
    }
    return 0;
}

int int_to_zigzag(int n)
{
    return (n << 1 ) ^ (n >> 31 );
}

int zigzag_to_int(int n)
{
    return ((( unsigned int )n) >> 1 ) ^ -(n & 1 );
}

int64_t int64_to_zigzag(int64_t n)
{
    return (n << 1 ) ^ (n >> 63 );
}

int64_t zigzag_to_int64(int64_t n)
{
    return (((uint64_t)n) >> 1 ) ^ -(n & 1 );
}

void func()
{
    int a = -127;
    int t = int_to_zigzag(a);
    printf("zigzag:%08x\n", t);
    int ori = zigzag_to_int(t);
    printf("original:%08x, Ten:%d\n", ori, ori);
}

void func1()
{
    int64_t a = -1;
    while (scanf("%lld", &a) != EOF) {
        int64_t t = int64_to_zigzag(a);
        printf("zigzag:%016llX\n", t);
        int64_t ori = zigzag_to_int64(t);
        printf("original:%016llx, Ten:%lld\n", ori, ori);
    }
}