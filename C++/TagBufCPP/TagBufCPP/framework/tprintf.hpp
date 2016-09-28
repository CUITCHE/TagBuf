//
//  tprintf.hpp
//  TagBufCPP
//
//  Created by hejunqiu on 2016/9/23.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef tprintf_hpp
#define tprintf_hpp

#include "types.h"
#include <stdarg.h>

extern uint64_t tprintf(const char *format, ...) __attribute__((__format__ (__printf__, 1, 2)));

extern uint64_t tprintf_error(const char *format, ...) __attribute__((__format__ (__printf__, 1, 2)));

extern uint64_t tprintf_c(char *&outBuffer, uint32_t *capacity, const char *fmt, va_list ap, uint32_t objectOutputFlag);

#define OUTPUT_FLAG_LOG (uint32_t)1
#define OUTPUT_FLAG_DESCRIPTION (uint32_t)2
#endif /* tprintf_hpp */
