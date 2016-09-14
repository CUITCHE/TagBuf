//
//  CHData.cpp
//  TagBuf
//
//  Created by hejunqiu on 16/9/2.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "CHData.hpp"
#include <stdlib.h>
#include <memory.h>
#include <memory>

/// Size of memory chunk
constexpr const uint32_t CHUNK_SIZE = 512;

#define __CHUNKS_SIZE__(capacity) (capacity) / CHUNK_SIZE + (uint32_t)(!!((capacity) % CHUNK_SIZE))
#define SYSTEM_RESERVE_CHUNK_COUNT 4
#define SYSTEM_GENERAL_BUFFER 128

struct CHDataPrivate
{
    uint32_t chunk_pos : 24; // chunk size max is 512 * (2**24 - 1);
    uint32_t chunk_pointer : 8;

    char **chunks = 0;
    uint32_t capacity;

    CHDataPrivate(uint32_t capacity)
    :capacity(capacity), chunk_pos(0), chunk_pointer(0)
    {
        if (capacity != 0) {
            uint32_t size = __CHUNKS_SIZE__(capacity);
            chunks = (char **)malloc(sizeof(char *) * size);
            char **p = chunks;
            while (size --> 0) {
                *p++ = (char *)calloc(1, sizeof(char) * CHUNK_SIZE);
            }
        }
    }

    void swap(CHDataPrivate &&other)
    {
        std::swap(chunks, other.chunks);
        std::swap(capacity, other.capacity);
        uint32_t tmp = chunk_pos;
        chunk_pos = other.chunk_pos;
        other.chunk_pos = tmp;

        tmp = chunk_pointer;
        chunk_pointer = other.chunk_pointer;
        other.chunk_pointer = tmp;
    }

    ~CHDataPrivate()
    {
        destructor();
    }

    uint32_t sizeOfChunks() const { return __CHUNKS_SIZE__(capacity); }

    uint32_t size () const { return chunk_pos * CHUNK_SIZE + chunk_pointer; }

    void checkMemory (uint32_t lengthOfWrite)
    {
        if (size() + lengthOfWrite > capacity) {
            uint32_t newSize = capacity + lengthOfWrite;
            if (lengthOfWrite >= capacity) { // Scary! Must multiply 2.
                newSize <<= 1;
            } else if (lengthOfWrite >= CHUNK_SIZE) {
                newSize += CHUNK_SIZE * SYSTEM_RESERVE_CHUNK_COUNT;
            } else if (lengthOfWrite >= SYSTEM_GENERAL_BUFFER) {
                newSize += CHUNK_SIZE;
            } else {
                newSize += lengthOfWrite;
            }
            resize(newSize);
        }
    }

    void write(const char *bytes, uint32_t length)
    {
        checkMemory(length);
        // Obtain current wirte point.
        char *dest = 0;
        char **src = chunks + chunk_pos - 1;
        uint32_t writeSize = 0;
    continueWrite:
        dest = *++src + chunk_pointer;
        if (length + chunk_pointer > CHUNK_SIZE) {
            writeSize = CHUNK_SIZE - chunk_pointer;
            memcpy(dest, bytes, writeSize);
            length -= writeSize;
            ++chunk_pos;
            chunk_pointer = 0;
            bytes += writeSize;
            goto continueWrite;
        } else {
            memcpy(dest, bytes, length);
            chunk_pointer += length;
            if (!chunk_pointer) { // May be 0, needs carry.
                ++chunk_pos;
            }
        }
    }
private:
    void resize(uint32_t size)
    {
        if (size > this->capacity) {
            uint32_t new_chunk_size = __CHUNKS_SIZE__(size);
            uint32_t old_chunk_size = this->sizeOfChunks();

            char **new_chunks = (char **)malloc(sizeof(char *) * new_chunk_size);
            if (chunks) {
                memcpy(new_chunks, chunks, sizeof(char *) * old_chunk_size);
                free(chunks);
            }
            chunks = new_chunks;

            new_chunks += old_chunk_size;
            uint32_t sz = new_chunk_size - old_chunk_size;
            while (sz --> 0) {
                *new_chunks++ = (char *)calloc(1, sizeof(char) * CHUNK_SIZE);
            }
        }
    }

    void destructor()
    {
        if (!chunks) {
            return;
        }
        uint32_t sz = sizeOfChunks();
        char **p = chunks;
        while (sz --> 0) {
            free(*p++);
        }
        free(chunks);
        chunks = 0;
    }
};

// CHData
CHData::CHData(uint32_t capacity) :d(new CHDataPrivate(capacity)) {}

CHData::~CHData() { delete d; }

CHData::CHData(CHData &&other)
:CHData(0)
{
    d->swap(std::move(*other.d));
}

void CHData::operator=(CHData &&right)
{
    d->swap(std::move(*right.d));
}

void CHData::appendBytes(const char *bytes, uint32_t length)
{
    d->write(bytes, length);
}

void CHData::enumerateByteUsingBlock(CHDataChunkCallback block) const
{
    uint32_t count = d->chunk_pos;
    bool stop = false;
    char **p = d->chunks;
    while (count --> 0) {
        block(*p++, CHUNK_SIZE, &stop);
        if (stop) {
            break;
        }
    }
    if (!stop && d->chunk_pointer) {
        block(*p, d->chunk_pointer, &stop);
    }
}

uint32_t CHData::length() const
{
    return d->size();
}

uint32_t CHData::capacity() const
{
    return d->capacity;
}
