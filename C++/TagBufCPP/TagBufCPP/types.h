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

/// Tag the class to indicate the class support tagged-pointer.
#ifndef CLASS_TAGGEDPOINTER_AVAILABLE
#define CLASS_TAGGEDPOINTER_AVAILABLE
#endif

class CHObject;
using id = CHObject *;

const constexpr uint32_t CHNotFound = (uint32_t)-1;

#define ARRAY_CONTAINS_TYPE(objType, containsType) objType
#define ARRAY_CONTAINS(type) CHArray *

#endif /* types_h */
