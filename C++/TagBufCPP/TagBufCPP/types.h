//
//  types.h
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/31.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef types_h
#define types_h

#include <type_traits>

using uint32_t = unsigned int;
using uint16_t = unsigned short;
using uint64_t = unsigned long long;

#if __LP64__
using IMP = unsigned long long;
using uintptr_t = unsigned long;
#else
using IMP = unsigned int;
using uintptr_t = unsigned int;
#endif

using SEL = const char *;

using Class = struct class_t *;

using Ivar   = struct ivar_t *;
using Method = struct method_t *;

#define TAGGED_POINTER_FLAG 0x8000000000000001
#define is_tagged_pointer() ((uintptr_t)this & TAGGED_POINTER_FLAG)
#define MAX_INDICATE_FLAG 0xE000000000000000ULL

#endif /* types_h */
