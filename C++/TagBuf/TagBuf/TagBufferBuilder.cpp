//
//  TagBufferBuilder.cpp
//  TagBuf
//
//  Created by hejunqiu on 16/8/31.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "TagBufferBuilder.hpp"
#include "runtime.hpp"
#include "CHBase.hpp"
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
        d->writeBuffer = new vector<char>(1024 * 4);
    }
}

void *TagBufferBuilder::readTagBuffer(std::vector<char> &data, id cls)
{
    do {
        if (data.empty()) {
            break;
        }
        if (!d->readBuffer) {
            d->readBuffer = new vector<char>;
        }
        *d->readBuffer = std::move(data);
    } while (0);
    return nullptr;
}


#pragma public interface

void objectToTagBuffer(CHTagBuf *object, vector<char> &outdata)
{
    ;
}

void *objectWithTagBuffer(const vector<char> &data, id cls)
{
    if (data.empty()) {
        return 0;
    }
    return 0;
}
