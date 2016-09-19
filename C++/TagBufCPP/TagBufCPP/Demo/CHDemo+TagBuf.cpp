//
//  CHDemo+TagBuf.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "CHDemo.hpp"
#include "runtime.hpp"
#include <stdlib.h>
#include "TagBufDefines.h"
#include "cast.hpp"

//------------------
// class CHDemo2
//------------------

static ivar_list IvarListNamed(CHDemo2)[] = {
    {.ivar[0] = {.ivar_name = selector(_1), .ivar_type = encode<double>(), .ivar_offset = OFFSET(CHDemo2, _1)}}
};

static class_t ClassNamed(CHDemo2) = {
    nullptr,
    selector(CHDemo2),
    nullptr,
    &ivar_list_CHDemo2[0],
    allocateCache(),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHDemo2), CHTagBuf::getClass(nullptr)), sizeof(CHDemo2))),
    1,
    selector(^#CHDemo2)
};

Implement(CHDemo2);

//------------------
// class CHDemo
//------------------

static ivar_list IvarListNamed(CHDemo)[] = {
    {.ivar[0] = {.ivar_name = selector(_1), .ivar_type = encode<int>(), .ivar_offset = OFFSET(CHDemo, _1)}},
    {.ivar[0] = {.ivar_name = selector(_2), .ivar_type = "^#CHDemo2", .ivar_offset = OFFSET(CHDemo, _2)}},
    {.ivar[0] = {.ivar_name = selector(_3), .ivar_type = encode<int>(), .ivar_offset = OFFSET(CHDemo, _3)}},
    {.ivar[0] = {.ivar_name = selector(_4), .ivar_type = encode<int>(), .ivar_offset = OFFSET(CHDemo, _4)}},
    {.ivar[0] = {.ivar_name = selector(tableId), .ivar_type = encode<int>(), .ivar_offset = OFFSET(CHDemo, tableId)}}
};

static class_t ClassNamed(CHDemo) = {
    nullptr,
    selector(CHDemo),
    nullptr,
    &ivar_list_CHDemo[0],
    allocateCache(),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHDemo), CHTagBuf::getClass(nullptr)), sizeof(CHDemo))),
    5,
    selector(^#CHDemo)
};

Implement(CHDemo);
