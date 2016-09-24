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

class CHProtocol
{
public:
    virtual bool equalTo(id anObject) const = 0;
    virtual CHString *description() const = 0;

    virtual uint64_t hash() const = 0;
    virtual Class superclass() const = 0;

    virtual bool isKindOfClass(Class aClass) const = 0;
    virtual bool isMemberOfClass(Class aClass) const = 0;

    virtual bool respondsToSelector(SEL selector) const = 0;
};

CLASS_TAGGEDPOINTER_AVAILABLE class CHObject : protocolTo CHProtocol
{
    __SUPPORTRUNTIME__(CHObject);
public:
    operator void*() const;
    operator CHTagBuf*() const;

    friend void release(CHObject *obj);
    virtual ~CHObject();

    bool isTaggedPointer() const;
    const char *objectType() const;

    // runtime
    virtual Class getClass() const;
    static Class getClass(std::nullptr_t);

    // protocol
    bool equalTo(id anObject) const override;
    CHString *description() const override;
    uint64_t hash() const override;
    Class superclass() const override;
    bool isKindOfClass(Class aClass) const override;
    bool isMemberOfClass(Class aClass) const override;
    bool respondsToSelector(SEL selector) const override;
protected:
    CHObject();

    struct idPrivate *d = 0;

    void setReserved(void *obj);
    void *reserved() const;

    void setObjectType(const char *type);
private:
    static id allocateInstance();
};

#endif /* id_hpp */
