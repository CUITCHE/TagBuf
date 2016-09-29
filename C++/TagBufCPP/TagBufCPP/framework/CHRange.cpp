//
//  CHRange.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 2016/9/23.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "CHRange.hpp"
#include "CHString.hpp"

extern CHString *showRange(CHRange range)
{
    CHString *str = CHString::stringWithFormat("{.location=%u, .length=%u}",
                                               range.location, range.length);
    return str;
}
