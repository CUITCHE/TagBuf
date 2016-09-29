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
    __SUPPORTRUNTIME__(CHTagBuf);
protected:
    CHTagBuf() :CHObject(){};
public:
    Class getClass() const ;
    static Class getClass(std::nullptr_t);
    void desctructor(CHTagBuf *obj);
private:
    static id allocateInstance();
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
