//
//  CHTagBuf.cpp
//  TagBuf
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "CHTagBuf.hpp"
#include "runtime.hpp"
#include "TagBufDefines.h"
#include <stdlib.h>
#include "tagBuf.hpp"

//------------------
// class CHTagBuf
//------------------

const uint32_t method_count_CHTagBuf = 1;

static method_list method_list_CHTagBuf[method_count_CHTagBuf + 0] = {
    // ::getClass
    {.method[0] = {__Static, 0, FunctionAddr<Class(*)(std::nullptr_t)>::addrValue(&CHTagBuf::getClass), "getClass"} }
};

static Class ClassNamed(CHTagBuf) = []() {
    class_t *instance = (Class)calloc(1, sizeof(class_t));
    instance->name = selector(CHTagBuf);
    instance->methodList = &method_list_CHTagBuf[0];
    instance->methodCount = method_count_CHTagBuf;
    instance->methodOffset = method_count_CHTagBuf - 1;
    instance->cache = allocateCache();
    instance->size = sizeof(CHTagBuf);
    class_registerClass(instance);
    return instance;
}();

Implement(CHTagBuf);