//
//  TaggedPointer.h
//  TagBufCPP
//
//  Created by hejunqiu on 16/9/19.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef TaggedPointer_h
#define TaggedPointer_h

/// Its value is the size of program.
extern const uintptr_t MAX_CONSTANT_ADDRESS;

#define TAG_MASK 0x8000000000000001ull
#define ISTAGGEDPOINTER() (uintptr_t)this & TAG_MASK

#define is_typeTaggedPointer(obj, typeFlag) ((((uintptr_t)obj) & 0xF000000000000001ull) == typeFlag)

#define TAGGED_POINTER_NUMBER_FLAG TAG_MASK
#define is_number(obj) is_typeTaggedPointer(obj, TAGGED_POINTER_NUMBER_FLAG)
#define MAX_INDICATE_NUMBER 0xF000000000000000ULL


#define TAGGED_POINTER_STRING_FLAG 0x9000000000000001ull
#define is_string(obj) is_typeTaggedPointer(obj, TAGGED_POINTER_STRING_FLAG)
#define MAX_INDICATE_STRING_DATA_LENGTH 0xFFFFFF


#define TAGGED_POINTER_DATA_FLAG 0xA000000000000001ull
#define is_data(obj) is_typeTaggedPointer(obj, TAGGED_POINTER_DATA_FLAG)
#define TAGGED_POINTER_DATA_LENGTH_OFFSET TAGGED_POINTER_STRING_LENGTH_OFFSET
#define TAGGED_POINTER_DATA_LENGTH_MASK TAGGED_POINTER_STRING_LENGTH_MASK

#define TAGGED_POINTER_STRING_DATA_FREE_FLAG 0x3FFFFFFFFull

union _float {
    float f;
    struct {
        uint32_t m : 23; // 小数位
        uint32_t e :  8; // 指数位
        uint32_t S :  1; // 符号位
    } s;
    uint32_t ff;
};

union _double {
    double d;
    struct {
        uint64_t m : 52; // 小数位
        uint64_t e : 11; // 指数位
        uint64_t S :  1; // 符号位
    } s;
    uint64_t dd;
};

#endif /* TaggedPointer_h */
