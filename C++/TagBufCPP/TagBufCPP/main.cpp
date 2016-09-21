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
#include "CHNumber.hpp"
#include "TaggedPointer.h"
#include "id.hpp"
#include "CHData.hpp"

int main(int argc, const char * argv[]) {
    int i = 1e7;
    while (i --> 0) {
        CHDemo *obj = CHTagBufFactor::tagBuf<CHDemo>();
        obj->_2 = CHTagBufFactor::tagBuf<CHDemo2>();
        release(obj);
    }
    char *bytes = new char[1024]{1,2,3,4};
    printf("%p\n", bytes);
    return 0;
}
