//
//  main.m
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include <stdio.h>
#include "CHString.hpp"
#include "tprintf.hpp"

int main(int argc, const char * argv[]) {
    const char *text = "123456789012asdfgh12";
    CHString *string = CHString::stringWithUTF8String(text);
//    CHString *pattern = CHString::stringWithUTF8String("12");
    CHString *replacement = CHString::stringWithUTF8String("zxcvbnm,.");
    CHString *mod = string->stringByReplacingCharactersInRange(CHMakeRange(3, 4), replacement);
    tprintf("%@\n", mod);
    return 0;
}
