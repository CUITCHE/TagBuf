//
//  types.h
//  TagBuf
//
//  Created by hejunqiu on 16/8/31.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef types_h
#define types_h

#include <type_traits>

using uint32_t = unsigned int;
using uint16_t = unsigned short;

#if __LP64__
using IMP = unsigned long;
using uintptr_t = unsigned long;
#else
using IMP = unsigned int;
using uintptr_t = unsigned int;
#endif

using SEL = const char *;

using Class = struct class_t *;
using id = Class;

#endif /* types_h */
