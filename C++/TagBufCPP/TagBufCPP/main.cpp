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
    long long i = 1e5;
    while (i-->0) {
        CHString *str = tstr("r346457");
        CHLog("%p@\t%d", str, str->isEqualToString(tstr("4325")));
        str->release();
    }
    return 0;
}
