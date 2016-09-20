//
//  id.hpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/9/16.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef id_hpp
#define id_hpp

#include <stdio.h>
#include "types.h"

class CHTagBuf;

CLASS_TAGGEDPOINTER_AVAILABLE class CHObject
{
public:
    operator void*() const;
    operator CHTagBuf*() const;

    friend void release(CHObject *obj);
    virtual ~CHObject();

    bool isTaggedPointer() const;
protected:
    CHObject();

    struct idPrivate *d = 0;

    void setReserved(void *obj);
    void *reserved() const;

    void setObjectType(const char *type);
    virtual const char *objectType() const;
};

#endif /* id_hpp */
