//
//  TagBufferBuilder.cpp
//  TagBuf
//
//  Created by hejunqiu on 16/8/31.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "TagBufferBuilder.hpp"
#include "runtime.hpp"
#include <assert.h>

using namespace std;

#define __tag32 0xFFE00000u
#define __tag64 0xFFE0000000000000ull

template <typename T>
inline bool isWorthZigzag(T integer)
{
    if (sizeof(T) == 4) {
        return !(integer & __tag32);
    } else if (sizeof(T) == 8) {
        return !(integer & __tag64);
    }
    assert((sizeof(T) == 4 || sizeof(T) == 8) && "Only support 4 bytes or 8 bytes integer");
    return false;
}

using __tag_output_type__ = uint32_t;

union __tag_buffer_flag {
    __tag_output_type__ itag;
    struct __tag_buffer_structure {
        uint32_t writeType          : 03; // [00~03]
        // if writeType is CHTagBufferWriteTypeVarintFixed
#define varint_8bits    0
#define varint_16bits   1
#define varint_32bits   2
#define varint_64bits   3
#define varint_float    4
#define varint_double   5
#define varint_bool     6

        // if writeType is CHTagBufferWriteTypeContainer
#define container_none      0
#define container_stream_nsstring   1
#define container_stream_nsdata     2
#define container_object    3
#define container_container 4


        // if writeType is CHTagBufferWriteTypeblobStream
#define stream_nsstring     0
#define stream_nsdata       1

        // if writeType is CHTagBufferWriteTypeTagBuffer

        // common defined
#define object_is_nil       7
        uint32_t internalTag        : 03; // [04~06]
        uint32_t next               : 01; // [07]
        uint32_t lengthOfZigzag     : 03; // [08~10]
        uint32_t lengthCompressed   : 01; // [11]
        uint32_t placeholder12      : 01; // [12]
        uint32_t fieldNumber        : 20; // [13~31]
    } tag;
};

using __tag_buffer_flag__ = __tag_buffer_flag;
using __tag_detail__ = __tag_buffer_flag::__tag_buffer_structure;

struct TagBufferBuilderPrivate
{
    vector<char> *writeBuffer = 0;
    vector<char> *readBuffer = 0;
    ~TagBufferBuilderPrivate()
    {
        delete writeBuffer;
        delete readBuffer;
    }
};

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