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

unsigned int RevBit(unsigned int x);
void test();

int main(int argc, const char * argv[]) {
    CHDemo *demo = new CHDemo;
    demo->_1() = 0x25453;

    auto ret =  propertyInvoke<int>(demo, selector(_1));
    printf("0x%x\n", ret);
    auto cls = methodInvoke<Class>(demo, selector(getClass), nullptr);
    printf("%s\n", cls->name);
    test();
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

#include <assert.h>
void test()
{
    CHDemo *demo = new CHDemo;
    demo->_1() = 0x25453;
    demo->_3() = 0x243;
    demo->_4() = 334;
    demo->tableId() = 35372343;
    int i = 1e6;
    while (i-->0) {
        {
            auto ret =  propertyInvoke<CHDemo2>(demo, selector(_2));
        }
        {
            auto ret =  propertyInvoke<int>(demo, selector(_1));
            assert(ret == demo->_1());
            ret =  propertyInvoke<int>(demo, selector(_3));
            assert(ret == demo->_3());
            ret =  propertyInvoke<int>(demo, selector(_4));
            assert(ret == demo->_4());
            ret =  propertyInvoke<int>(demo, selector(tableId));
            assert(ret == demo->tableId());
        }
    }
}