//
//  main.m
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include <stdio.h>
#include "CHDemo.hpp"
#include "runtime.hpp"
#include "cast.hpp"
#include <map>
#include "CHNumber.hpp"
#include "TaggedPointer.h"

unsigned int RevBit(unsigned int x);
void test();

class TEST {
    virtual void qq(){};
public:
    int a;
    int b;
};

class TEST2 :  TEST {
    virtual void qqq(){}
public:
    int c;
};

int main(int argc, const char * argv[]) {
    printf("%s\n", encode("s"));
    printf("%s\n", encode<const char>());
    printf("%s\n", encode<TEST>());
    printf("%s\n", encode<CHDemo>());
    printf("%s\n", encode<std::__1::map<double, int>>());
    CHNumber *number = number('A');
    printf("%d\n", (int)*number);
    release(number);

    number = number(34.56);
    printf("%f\n", (float)*number);
    release(number);

    number = number(0x8FFFFFFFFFFFFFFFULL);
    printf("0x%llx\n", (long long)*number);
    printf("%f\n", (double)*number);
    release(number);

    CHDemo *demo = CHTagBufFactor::tagBuf<CHDemo>();
    Ivar *ivars = class_copyIvarList(demo->getClass(), 0);
    Ivar *p = ivars;
    while (*p) {
        id value = object_getIvar(demo, *p);
        printf("%p\n", value);
        object_setIvar(demo, *p, number(35264));
        ++p;
    }
    free(ivars);
    release(demo);
    return 0;
}

unsigned int RevBit(unsigned int x)
{
    if (x == 0) {
        x++;
    } else {
        x--;
    }
    x=((x&0x55555555)<<1)|((x>>1)&0x55555555);
    x=((x&0x33333333)<<2)|((x>>2)&0x33333333);
    x=((x&0x0f0f0f0f)<<4)|((x>>4)&0x0f0f0f0f);
    x=((x&0x00ff00ff)<<8)|((x>>8)&0x00ff00ff);
    x=x<<16|x>>16;
    return x;
}
