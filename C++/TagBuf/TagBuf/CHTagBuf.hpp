//
//  CHTagBuf.hpp
//  TagBuf
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef CHTagBuf_hpp
#define CHTagBuf_hpp

#include <stdio.h>
#include <cstddef>

using Class = struct class_t *;

class CHTagBuf
{
public:
    virtual Class getClass() = 0;
    static Class getClass(std::nullptr_t);
};
#endif /* CHTagBuf_hpp */
