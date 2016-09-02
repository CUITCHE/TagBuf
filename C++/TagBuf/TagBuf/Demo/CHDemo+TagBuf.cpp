//
//  CHDemo+TagBuf.cpp
//  TagBuf
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "CHDemo.hpp"
#include "runtime.hpp"
#include <stdlib.h>
#include "TagBufDefines.h"

//------------------
// class CHDemo2
//------------------

const uint32_t method_count_CHDemo2 = 2;

static method_list property_list_CHDemo2[method_count_CHDemo2] = {
    /*-------property start-------*/
    // double _1
    {.method[0] = {__Property, CHTagBufEncodingTypeDouble, FunctionAddr<double&(CHDemo2::*)()>::addrValue(&CHDemo2::_1), "_1"} },
    /*-------property start-------*/
    // ::getClass
    {.method[0] = {__Static, 0, FunctionAddr<Class(*)(std::nullptr_t)>::addrValue(&CHDemo2::getClass), "getClass"} }
};

static Class ClassNamed(CHDemo2) = []() {
    class_t *instance = (Class)calloc(1, sizeof(class_t));
    instance->name = selector(CHDemo2);
    instance->methodList = &property_list_CHDemo2[0];
    instance->methodCount = method_count_CHDemo2;
    instance->methodOffset = method_count_CHDemo2 - 1;
    instance->cache = allocateCache();
    instance->size = sizeof(CHDemo2);
    class_registerClass(instance, CHTagBuf::getClass(nullptr));
    return instance;
}();

Implement(CHDemo2);

//------------------
// class CHDemo
//------------------

const uint32_t method_count_CHDemo = 6;

static method_list property_list_CHDemo[method_count_CHDemo] = {
    /*-------property start-------*/
    // int _1
    {.method[0] = {__Property, CHTagBufEncodingType32Bits, FunctionAddr<int&(CHDemo::*)()>::addrValue(&CHDemo::_1), "_1"} },
    // CHDemo2 _2
    {.method[0] = {__Property, CHTagBufEncodingTypeOtherObject, FunctionAddr<CHDemo2&(CHDemo::*)()>::addrValue(&CHDemo::_2), "_2"}},
    // int _3
    {.method[0] = {__Property, CHTagBufEncodingType32Bits, FunctionAddr<int&(CHDemo::*)()>::addrValue(&CHDemo::_3), "_3"} },
    // int _4
    {.method[0] = {__Property, CHTagBufEncodingType32Bits, FunctionAddr<int&(CHDemo::*)()>::addrValue(&CHDemo::_4), "_4"} },
    // int tableId
    {.method[0] = {__Property, CHTagBufEncodingType32Bits, FunctionAddr<int&(CHDemo::*)()>::addrValue(&CHDemo::tableId), "tableId"} },
    /*-------property start-------*/
    // ::getClass
    {.method[0] = {__Static, 0, FunctionAddr<Class(*)(std::nullptr_t)>::addrValue(&CHDemo::getClass), "getClass"} }
};

static Class ClassNamed(CHDemo) = []() {
    class_t *instance = (Class)calloc(1, sizeof(class_t));
    instance->name = selector(CHDemo);
    instance->methodList = &property_list_CHDemo[0];
    instance->methodCount = method_count_CHDemo;
    instance->methodOffset = method_count_CHDemo - 1;
    instance->cache = allocateCache();
    instance->size = sizeof(CHDemo);
    class_registerClass(instance, CHTagBuf::getClass(nullptr));
    return instance;
}();

Implement(CHDemo);
