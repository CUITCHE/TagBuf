//
//  id.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/9/16.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "id.hpp"
#include "types.h"
#include "CHTagBuf.hpp"

struct idPrivate
{
    void *obj;
    uint32_t valueType;
};

CHObject::CHObject()
:d(new idPrivate)
{
    ;
}

CHObject::~CHObject()
{
//    if (!((uintptr_t)this & TAGGED_POINTER_FLAG)) {
//        switch (d->valueType) {
//            case 0:
//                delete (CHTagBuf *)d->obj;
//                break;
//            default:
//                break;
//        }
//    }
}

CHObject::operator void *() const
{
    return d;
}

CHObject::operator CHTagBuf *() const
{
    return (CHTagBuf *)d;
}

void CHObject::setReserved(void *obj)
{
    d->obj = obj;
}

void *CHObject::reserved() const
{
    return d->obj;
}

// constructor
void release(id obj)
{
#if __LP64__
    if (!((uintptr_t)obj & TAGGED_POINTER_FLAG)) {
        delete obj;
    }
#else
    delete obj;
#endif
}
