//
//  CHTagBuf.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "CHTagBuf.hpp"
#include "runtime.hpp"
#include "TagBufDefines.h"
#include "tagBuf.hpp"
#include <memory.h>

//------------------
// class CHTagBuf
//------------------
static class_t ClassNamed(CHTagBuf) = {
    CHObject::getClass(nullptr),
    selector(CHTagBuf),
    nullptr,
    nullptr,
    allocateCache(),
    selector(^#CHTagBuf),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHTagBuf), CHTagBuf::getClass(nullptr)), sizeof(CHTagBuf))),
    0,
    0
};

Implement(CHTagBuf);
