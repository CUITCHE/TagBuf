//
//  CHString.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/31.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "CHString.hpp"
#include "CHData.hpp"
#include <string.h>

CHString::CHString() {}

uint32_t CHString::length() const
{
    return ((CHData *)this)->length();
}
