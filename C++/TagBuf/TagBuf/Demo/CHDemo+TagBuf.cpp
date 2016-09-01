//
//  CHDemo+TagBuf.cpp
//  TagBuf
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "CHDemo.hpp"
#include "runtime.hpp"
#include "cache.hpp"
#include <stdlib.h>
#include "TagBufDefines.h"

//------------------
// class CHDemo2
//------------------

const uint32_t property_count_CHDemo2 = 2;

static method_list property_list_CHDemo2[property_count_CHDemo2] = {
    // double _1
    {.method[0] = {__Property, CHTagBufEncodingTypeDouble, FunctionAddr<double&(CHDemo2::*)()>::addrValue(&CHDemo2::_1), "_1"} },
    // ::getClass
    {.method[0] = {__Static, 0, FunctionAddr<Class(*)(std::nullptr_t)>::addrValue(&CHDemo2::getClass), "getClass"} }
};

static Class ClassNamed(CHDemo2) = []() {
    class_t *instance = (Class)calloc(sizeof(class_t), 1);
    instance->name = selector(CHDemo2);
    instance->methodList = &property_list_CHDemo2[0];
    instance->property_count = property_count_CHDemo2;
    instance->cache = calloc(sizeof(cache_t), 1);
    instance->size = sizeof(CHDemo2);
    class_registerClass(instance);
    return instance;
}();

Implement(CHDemo2);

//------------------
// class CHDemo
//------------------

const uint32_t property_count_CHDemo = 3;

static method_list property_list_CHDemo[property_count_CHDemo] = {
    // int _1
    {.method[0] = {__Property, CHTagBufEncodingType32Bits, FunctionAddr<int&(CHDemo::*)()>::addrValue(&CHDemo::_1), "_1"} },
    // CHDemo2 _2
    {.method[0] = {__Property, CHTagBufEncodingTypeOtherObject, FunctionAddr<CHDemo2&(CHDemo::*)()>::addrValue(&CHDemo::_2), "_2"}},
    // ::getClass
    {.method[0] = {__Static, 0, FunctionAddr<Class(*)(std::nullptr_t)>::addrValue(&CHDemo::getClass), "getClass"} }
};

static Class ClassNamed(CHDemo) = []() {
    class_t *instance = (Class)calloc(sizeof(class_t), 1);
    instance->name = selector(CHDemo);
    instance->methodList = &property_list_CHDemo[0];
    instance->property_count = property_count_CHDemo;
    instance->cache = calloc(sizeof(cache_t), 1);
    instance->size = sizeof(CHDemo);
    class_registerClass(instance);
    return instance;
}();

Implement(CHDemo);
