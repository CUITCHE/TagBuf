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
    int i = 1e8;
    while (i --> 0) {
        CHDemo *obj = CHTagBufFactor::tagBuf<CHDemo>();
        obj->_2 = CHTagBufFactor::tagBuf<CHDemo2>();
        release(obj);
    }
    return 0;
}
