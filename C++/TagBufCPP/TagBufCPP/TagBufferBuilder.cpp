//
//  TagBufferBuilder.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/31.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "TagBufferBuilder.hpp"
#include "runtime.hpp"
#include "CHData.hpp"
#include <CHBase.hpp>
#include <assert.h>

using namespace std;


#pragma mark TagBufferBuilder
TagBufferBuilder::TagBufferBuilder()
:d(new TagBufferBuilderPrivate)
{
}

TagBufferBuilder::~TagBufferBuilder()
{
    delete d;
}

void TagBufferBuilder::startBuildingWithObject(CHTagBuf *object)
{
    if (!d->writeBuffer) {
        d->writeBuffer = CHMutableData::dataWithCapacity(0);
    }
}

void *TagBufferBuilder::readTagBuffer(const CHData *data, Class cls)
{
    do {
        if (!data->length()) {
            break;
        }
        if (!d->readBuffer) {
            d->readBuffer = CHMutableData::dataWithCapacity(0);
        }
        d->readBuffer = data;
    } while (0);
    return nullptr;
}


#pragma public interface

void objectToTagBuffer(CHTagBuf *object, CHData &outdata)
{
    ;
}

void *objectWithTagBuffer(const CHData &data, Class cls)
{
    if (!data.length()) {
        return 0;
    }
    return 0;
}
