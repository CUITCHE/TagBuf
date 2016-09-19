//
//  CHData.cpp
//  TagBufCPP
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

typedef enum : unsigned char {
    Normal,
    NoCopy = 1,
    FreeWhenDone = 1 << 1,
    ChunkLengthNeedsCalculated = 1 << 2, // If Nocopy is 0, length occupies 2 bytes. Otherwise occupies 4bytes.
    NextIsNonCopyBuffer = 1 << 3 // This list member is point to a length(uint32_t) flag.
} DataOption; /// For first byte.

struct CHDataPrivate
{
    char **chunks = 0;

    uint32_t chunk_pos : 24; // chunk size max is 512 * (2**24 - 1);
    uint32_t chunk_pointer : 8;

    uint32_t lengthOfChunks;

    CHDataPrivate(uint32_t capacity)
    :lengthOfChunks(0), chunk_pos(0), chunk_pointer(0)
    {
        if (capacity != 0) {
            uint32_t size = __CHUNKS_SIZE__(capacity);
            chunks = (char **)malloc(sizeof(char *) * size);
            lengthOfChunks = size;
        }
    }

    void swap(CHDataPrivate &&other)
    {
        std::swap(chunks, other.chunks);
        std::swap(lengthOfChunks, other.lengthOfChunks);
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

    uint32_t sizeOfChunks() const { return lengthOfChunks; }

    uint32_t size() const { return chunk_pos * CHUNK_SIZE + chunk_pointer; }

    uint32_t capacity() const { return CHUNK_SIZE * lengthOfChunks; }

    void checkMemory (uint32_t lengthOfWrite)
    {
        uint32_t newLengthOfChunks = __CHUNKS_SIZE__(lengthOfWrite);
        if (newLengthOfChunks + chunk_pos > lengthOfChunks) {
            resize(newLengthOfChunks + chunk_pos);
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
        if (chunk_pointer == 0) {
            *src = allocate();
            dest = *src;
        }
        *++dest = Normal;
        ++chunk_pointer;
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

    /**
     In order to reallocte a new space to point non-copy memory space, we just
     adjust list memory.

     @note Whatever the non-copy memory space is, the chunk is always added a
     CHUNK_SIZE memory space.

     @param bytes        Bytes ara to be wrote.
     @param length       The length of bytes.
     @param freeWhenDone Whether the bytes memory should be free or not if CHData
     needs to be freed.
     */
    void writeNoCopy(char *bytes, uint32_t length, bool freeWhenDone)
    {
        checkMemory(CHUNK_SIZE * 2);
        char **p = chunks + chunk_pos;
        char *src = *p;
        if (chunk_pointer == CHUNK_SIZE - 1) {
            // Do nothing...
        } else if (chunk_pointer <= CHUNK_SIZE - 2 - 1) { // chunk_pointer + 2 + 1 <= CHUNK_SIZE
            memcpy(src + 3, src + 1, chunk_pointer);
            src[0] = ChunkLengthNeedsCalculated;
            src[1] = (char)chunk_pointer;
            src[2] = (char)(chunk_pointer >> 8);
        } else {
            char *newSpace = (char *)malloc(sizeof(char) * (chunk_pointer + 1 + 2));
            memcpy(newSpace + 3, src + 1, chunk_pointer);
            newSpace[0] = ChunkLengthNeedsCalculated;
            newSpace[1] = (char)chunk_pointer;
            newSpace[2] = (char)(chunk_pointer >> 8);
            free(src);
            *p = newSpace;
        }
        // 1.Write length.
        char flag = NextIsNonCopyBuffer | NoCopy;
        if (freeWhenDone) {
            flag |= FreeWhenDone;
        }
        uint64_t *bufferLength = (uint64_t *)malloc(sizeof(uint64_t));
        memcpy(bufferLength, &flag, sizeof(char));
        memcpy((char *)bufferLength + 1, &length, sizeof(length));
        *++p = (char *)bufferLength;

        // 2.Associate to non-copy buffer.
        *++p = bytes;
        chunk_pos += 2;
    }
private:
    void resize(uint32_t new_chunk_size)
    {
        if (new_chunk_size > this->lengthOfChunks) {
            uint32_t old_chunk_size = this->sizeOfChunks();
            char **new_chunks = (char **)malloc(sizeof(char *) * new_chunk_size);
            if (chunks) {
                memcpy(new_chunks, chunks, sizeof(char *) * old_chunk_size);
                free(chunks);
            }
            chunks = new_chunks;
        }
        lengthOfChunks = new_chunk_size;
    }

    void resizeToNonCopy()
    {
        // 1.Indicate length of non-copy buffer.
        // 2.pointer to non-copy buffer.
        uint32_t chunk_size = this->sizeOfChunks();
        char **new_chunks = (char **)malloc(sizeof(char *) * (chunk_size + 2));
        if (chunks) {
            memcpy(new_chunks, chunks, sizeof(char *) * chunk_size);
            free(chunks);
        }
        chunks = new_chunks;
    }

    void destructor()
    {
        if (!chunks) {
            return;
        }
        uint32_t sz = sizeOfChunks();
        char **p = chunks - 1;
        char *v = nullptr;
        while (sz --> 0) {
            v = *++p;
            if (v[0] == Normal || v[0] == FreeWhenDone) {
                free(*p);
            }
        }
        free(chunks);
        chunks = 0;
    }

    static char *allocate()
    {
        return (char *)malloc(sizeof(char) * CHUNK_SIZE);
    }
};

#define d_d(obj, field) ((CHDataPrivate *)obj->reserved())->field

// CHData
CHData::CHData(uint32_t capacity) : CHObject()
{
    setReserved(new CHDataPrivate(capacity));
}

CHData::~CHData()
{
    CHDataPrivate *d = (CHDataPrivate *)reserved();
    if (d) {
        delete d;
    }
}

CHData::CHData(CHData &&other)
:CHData(0)
{
    d_d(this, swap)(std::move(*(CHDataPrivate *)other.reserved()));
}

void CHData::operator=(CHData &&right)
{
    d_d(this, swap)(std::move(*(CHDataPrivate *)right.reserved()));
}

void CHData::appendBytes(const char *bytes, uint32_t length)
{
    d_d(this, write)(bytes, length);
}

void CHData::appendBytesNoCopy(const char *bytes, uint32_t length, bool freeWhenDone /*= false*/)
{
    d_d(this, writeNoCopy)((char *)bytes, length, freeWhenDone);
}

void CHData::enumerateByteUsingBlock(CHDataChunkCallback block) const
{
    CHDataPrivate *d = (CHDataPrivate *)reserved();
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

CHData *CHData::dataWithData(const char *bytes, uint32_t length)
{
    CHData *obj = nullptr;
    if (length < 8) {
        ;
    }
    return obj;
}

uint32_t CHData::length() const
{
    return d_d(this, size)();
}

uint32_t CHData::capacity() const
{
    return d_d(this, capacity)();
}
