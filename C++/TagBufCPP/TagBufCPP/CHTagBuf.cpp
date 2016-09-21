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

RUNTIMECLASS(CHTagBuf);

static class_t ClassNamed(CHTagBuf) = {
    CHObject::getClass(nullptr),
    selector(CHTagBuf),
    runtimeclass(CHTagBuf)::methods(),
    nullptr,
    allocateCache(),
    selector(^#CHTagBuf),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHTagBuf)), sizeof(CHTagBuf))),
    0,
    3
};

Implement(CHTagBuf);
