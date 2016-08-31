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

using Class = struct class_t *;

class CHTagBuf
{
public:
    virtual Class getClass() = 0;
};
#endif /* CHTagBuf_hpp */
