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
    CHLog("%p@", str);
    return 0;
}
