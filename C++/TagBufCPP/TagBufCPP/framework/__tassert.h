//
//  __tassert.h
//  TagBufCPP
//
//  Created by hejunqiu on 2016/9/26.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef __tassert_h
#define __tassert_h

#include "CHString.hpp"
#include "tprintf.hpp"
#include <exception>

#define __tassert(cond, fmt, ...)  do {\
                if (!cond) {\
                    CHMutableString *log = CHMutableString::stringWithCapacity(128);\
                    log->appendFormat("<%p - %s:%d> ", this, __FILE__, __LINE__);\
                    log->appendFormat(fmt, ##__VA_ARGS__);\
                    tprintf_error("%p@\n", log); std::terminate();\
                }\
            } while(0)

#endif /* __tassert_h */
