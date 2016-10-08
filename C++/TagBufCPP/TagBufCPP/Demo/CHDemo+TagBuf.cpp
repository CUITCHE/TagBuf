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

static ivar_list_t IvarListNamed(CHDemo2)[] = {
    {.ivar[0] = {.ivar_name = selector(_1), .ivar_type = encode<double>(), .ivar_offset = OFFSET(CHDemo2, _1)}}
};

RUNTIMECLASS(CHDemo2);

static class_t ClassNamed(CHDemo2) = {
    0,
    CHTagBuf::getClass(nullptr),
    selector(CHDemo2),
    runtimeclass(CHDemo2)::methods(),
    &ivar_list_CHDemo2[0],
    allocateCache(),
    selector(^#CHDemo2),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHDemo2)), sizeof(CHDemo2))),
    1,
    3
};

ImplementTagBuf(CHDemo2);

//------------------
// class CHDemo
//------------------

static ivar_list_t IvarListNamed(CHDemo)[] = {
    {.ivar[0] = {.ivar_name = selector(_1), .ivar_type = encode<decltype(((CHDemo *)0)->_1)>(), .ivar_offset = OFFSET(CHDemo, _1)}},
    {.ivar[0] = {.ivar_name = selector(_2), .ivar_type = "^#CHDemo2", .ivar_offset = OFFSET(CHDemo, _2)}},
    {.ivar[0] = {.ivar_name = selector(_3), .ivar_type = encode<decltype(((CHDemo *)0)->_3)>(), .ivar_offset = OFFSET(CHDemo, _3)}},
    {.ivar[0] = {.ivar_name = selector(_4), .ivar_type = encode<decltype(((CHDemo *)0)->_4)>(), .ivar_offset = OFFSET(CHDemo, _4)}},
    {.ivar[0] = {.ivar_name = selector(tableId), .ivar_type = encode<decltype(((CHDemo *)0)->tableId)>(), .ivar_offset = OFFSET(CHDemo, tableId)}}
};

RUNTIMECLASS(CHDemo);

static class_t ClassNamed(CHDemo) = {
    0,
    CHTagBuf::getClass(nullptr),
    selector(CHDemo),
    runtimeclass(CHDemo)::methods(),
    &ivar_list_CHDemo[0],
    allocateCache(),
    selector(^#CHDemo),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHDemo)), sizeof(CHDemo))),
    5,
    3
};

ImplementTagBuf(CHDemo);
