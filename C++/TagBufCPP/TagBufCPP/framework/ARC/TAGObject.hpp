//
//  TAGObject.hpp
//  TagBufCPP
//
//  Created by hejunqiu on 2016/9/27.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef TAGObject_hpp
#define TAGObject_hpp

#include <stdio.h>

/// ARC Container
class TAGObject
{
    struct TAGObjectPrivate *d;
public:
    
private:
    void *operator new (size_t t) noexcept { return nullptr; }
    void operator delete (void *ptr) {}
    void operator delete[] (void *ptr) {}
};

#endif /* TAGObject_hpp */
