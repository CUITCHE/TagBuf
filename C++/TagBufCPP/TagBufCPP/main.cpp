//
//  main.m
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include <stdio.h>
#include "CHString.hpp"
#include "CHLog.hpp"

int main(int argc, const char * argv[]) {
    CHString *str = tstr("r346457");
    id obj = dynamic_cast<id>(str);
    auto cls = obj->getClass();
    CHLog("%p@", obj->description());
    return 0;
}
