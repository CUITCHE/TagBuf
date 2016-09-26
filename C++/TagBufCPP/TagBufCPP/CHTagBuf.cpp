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

void CHTagBuf::desctructor(CHTagBuf *obj)
{
    Class cls = obj->getClass();
    Class endCls = &class_CHTagBuf;
    do {
        if (!cls || cls == endCls) {
            break;
        }
        ivar_list_t *list = cls->ivarList - 1;
        int count = cls->ivarCount;
        const char *encodeType = 0;
        while (count --> 0) {
            encodeType = ivar_getTypeEncoding((++list)->ivar);
            if (encodeType[0] == '^') {
                if (encodeType[1] == '#') {
                    id value = object_getIvar(this, list->ivar);
                    release(value);
                }
            }
        }
        cls = cls->super_class;
    } while (true);
}
