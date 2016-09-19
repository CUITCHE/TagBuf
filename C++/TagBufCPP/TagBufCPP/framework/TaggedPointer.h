//
//  TaggedPointer.h
//  TagBufCPP
//
//  Created by hejunqiu on 16/9/19.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef TaggedPointer_h
#define TaggedPointer_h


#define TAGGED_POINTER_FLAG 0x8000000000000001
#define is_tagged_pointer() ((uintptr_t)this & TAGGED_POINTER_FLAG)
#define MAX_INDICATE_FLAG 0xE000000000000000ULL

#define TAGGED_POINTER_NUMBER_FLAG TAGGED_POINTER_FLAG

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
