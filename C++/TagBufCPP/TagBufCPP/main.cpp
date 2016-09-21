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
    auto cls = CHData::getClass(nullptr);
    id instance = allocateInstance(cls);
    printf("%s\n", instance->objectType());
    return 0;
}
