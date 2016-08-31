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

int main(int argc, const char * argv[]) {
    CHDemo *demo = new CHDemo;
    demo->_1() = 25453;
    
    auto ret =  propertyInvoke<int>(demo, "_1");
    printf("%d\n", ret);
    return 0;
}
