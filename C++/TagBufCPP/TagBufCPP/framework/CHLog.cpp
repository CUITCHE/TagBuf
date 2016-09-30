//
//  CHLog.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 2016/9/26.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "CHLog.hpp"
#include "tprintf.hpp"
#include <stdlib.h>
#include "TaggedPointer.h"

void CHLog(const char *fmt, ...)
{
    char *buffer = nullptr;
    va_list ap;
    va_start(ap, fmt);
    (void) tprintf_c(buffer, nullptr, fmt, ap, OUTPUT_FLAG_LOG);
    va_end(ap);
    puts(buffer);
    free(buffer);
}
