//
//  main.m
//  TagBuf
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include <stdio.h>
#include "CHDemo.hpp"
#include "runtime.hpp"
#include "cast.hpp"
#include <map>
#include "id.hpp"

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
    id obj = objectWithValue('A');
    printf("%d\n", (int)*obj);
    obj = objectWithValue(34.56f);
    printf("%f\n", (float)*obj);
    release_id(obj);
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
