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

int main(int argc, const char * argv[]) {
    auto cls = CHObject::getClass(nullptr);
    uint32_t count = 0;
    auto method = class_copyMethodList(cls, &count);
    return 0;
}
