//
//  CHTagBuf.hpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef CHTagBuf_hpp
#define CHTagBuf_hpp

#include <stdio.h>
#include "id.hpp"

class CHTagBuf : public CHObject
{
    friend struct CHTagBufFactor;
protected:
    CHTagBuf() :CHObject(){};
public:
    Class getClass() const override;
    static Class getClass(std::nullptr_t);
};

struct CHTagBufFactor
{
    template<typename T>
    static T *tagBuf()
    {
        return new T;
    }
};
#endif /* CHTagBuf_hpp */
