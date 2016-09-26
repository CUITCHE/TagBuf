//
//  CHLog.hpp
//  TagBufCPP
//
//  Created by hejunqiu on 2016/9/26.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef CHLog_hpp
#define CHLog_hpp

#include <stdio.h>

extern void CHLog(const char *fmt, ...) __attribute__((__format__ (__printf__, 1, 2)));

#endif /* CHLog_hpp */
