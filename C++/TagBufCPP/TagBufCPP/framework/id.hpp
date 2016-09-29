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
class CHString;

#ifndef runtimeclass
#define runtimeclass(classname) __##classname##RuntimeClass__
#endif

#ifndef __SUPPORTRUNTIME__
#define __SUPPORTRUNTIME__(classname) friend struct runtimeclass(classname)
#endif

#define protocolTo public
#define protocol class

class CHObjectProtocol
{
public:
    virtual bool equalTo(id anObject) const = 0;
    CHString *description() const { return nullptr; }

    virtual uint64_t hash() const = 0;
    virtual Class superclass() const = 0;

    bool isKindOfClass(Class aClass) const { return false; }
    bool isMemberOfClass(Class aClass) const { return false; }

    bool respondsToSelector(SEL selector) const { return false; }

    id retain();
    void release();
};

protocol CHCopying
{
protected:
    id copyWithZone(std::nullptr_t) const { return nullptr; };
};

protocol CHMutableCopying
{
protected:
    id mutableCopyWithZone(std::nullptr_t) const { return nullptr; };
};

CLASS_TAGGEDPOINTER_AVAILABLE class CHObject : protocolTo CHObjectProtocol
{
    __SUPPORTRUNTIME__(CHObject);
public:
    operator void*() const;
    operator CHTagBuf*() const;

    friend void release_outer(CHObject *obj);
    virtual ~CHObject();

    bool isTaggedPointer() const;
    const char *objectType() const;

    // copy
    id copy() const;
    id mutableCopy() const;

    // runtime
    Class getClass() const;
    static Class getClass(std::nullptr_t);

    // protocol
    bool equalTo(id anObject) const override;
    CHString *description() const;
    uint64_t hash() const override;
    Class superclass() const override;

    id retain(); // Will not be added to runtime.
    void release();

    bool isKindOfClass(Class aClass) const;
    bool isMemberOfClass(Class aClass) const;
    bool respondsToSelector(SEL selector) const;
protected:
    CHObject();

    struct idPrivate *d = 0;

    void setReserved(void *obj);
    void *reserved() const;
private:
    static id allocateInstance();
};

#endif /* id_hpp */
