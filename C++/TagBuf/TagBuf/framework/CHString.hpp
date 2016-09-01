//
//  CHString.hpp
//  TagBuf
//
//  Created by hejunqiu on 16/8/31.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef CHString_hpp
#define CHString_hpp

#include <stdio.h>
#include "types.h"

class CHString
{
    struct CHStringPrivate *d;

private:
    CHString();
public:
    ~CHString();

    CHString(const CHString&) = delete;
    CHString& operator=(const CHString &) = delete;

    CHString(CHString&& other);
    void operator=(CHString &&right);

    static CHString&& stringWithConstantString(const char *str, uint32_t length);
    static CHString&& stringWithString(char *str, uint32_t length, bool freeWhenDone = true);
};

#endif /* CHString_hpp */
