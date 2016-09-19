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
#include <CHBase.hpp>

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
        if (chunk_pointer == 0) {
            *++src = allocate();
            dest = *src;
            *dest++ = Normal;
            ++chunk_pointer;
        } else {
            dest = *++src + chunk_pointer;
        }
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
        chunk_pointer = 0;
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

CHData *CHData::duplicate() const
{
    CHData *obj = new CHData(0);
    CHDataPrivate *d = (CHDataPrivate *)reserved();
    d_d(obj, chunk_pos) = d->chunk_pos;
    d_d(obj, chunk_pointer) = d->chunk_pointer;
    d_d(obj, lengthOfChunks) = d->lengthOfChunks;
    uint32_t count = d->lengthOfChunks;
    char **p = d->chunks - 1;
    char *sv = nullptr;

    d = (CHDataPrivate *)obj->reserved();
    d->chunks = (char **)malloc(sizeof(char *) * count);
    char **dest = d->chunks - 1;
    char *dv = nullptr;
    while (count --> 0) {
        sv = *++p;
        if (sv[0] == Normal) {
            dv =(char *)malloc(sizeof(char) * CHUNK_SIZE);
            memcpy(dv, sv, sizeof(char) * CHUNK_SIZE);
        } else {
            if (sv[0] == ChunkLengthNeedsCalculated) {
                short length = (sv[1] | sv[2] << 8) + 1 + 2;
                dv = (char *)malloc(sizeof(char) * length);
                memcpy(dv, sv, sizeof(char) * length);
            } else if (sv[0] & (NextIsNonCopyBuffer | NoCopy)) {
                uint64_t *bufferLength = (uint64_t *)malloc(sizeof(uint64_t));
                memcpy(bufferLength, sv, sizeof(uint64_t));

                bool needCopy = !!(sv[0] & FreeWhenDone);
                uint32_t *length = (uint32_t *)(sv + 1);

                *++dest = (char *)bufferLength;
                sv = *++p;
                --count;

                if (needCopy) {
                    dv = (char *)malloc(sizeof(char) * *length);
                    memcpy(dv, sv, sizeof(char) * *length);
                } else {
                    dv = sv;
                }
            } else {
                CHCAssert(false, "Logic error. It's impossible to go here.");
            }
        }
        *++dest = dv;
    }
    return obj;
}

CHData::~CHData()
{
    CHDataPrivate *d = (CHDataPrivate *)reserved();
    if (d) {
        delete d;
    }
}

//CHData::CHData(CHData &&other)
//:CHData(0)
//{
//    d_d(this, swap)(std::move(*(CHDataPrivate *)other.reserved()));
//}
//
//void CHData::operator=(CHData &&right)
//{
//    d_d(this, swap)(std::move(*(CHDataPrivate *)right.reserved()));
//}

void CHData::appendBytes(const char *bytes, uint32_t length)
{
    d_d(this, write)(bytes, length);
}

void CHData::appendBytesNoCopy(const char *bytes, uint32_t length, bool freeWhenDone /*= false*/)
{
    d_d(this, writeNoCopy)((char *)bytes, length, freeWhenDone);
}

void CHData::appendData(const CHData *other)
{
    other->enumerateByteUsingBlock([this](const char *bytes, uint32_t byteLength, bool *stop) {
        this->appendBytes(bytes, byteLength);
    });
}

void CHData::enumerateByteUsingBlock(CHDataChunkCallback block) const
{
    CHDataPrivate *d = (CHDataPrivate *)reserved();
    uint32_t count = d->lengthOfChunks;
    bool stop = false;
    char **p = d->chunks - 1;
    char *v = nullptr;
    while (count --> 0) {
        v = *++p;
        if (v[0] == Normal) {
            block(++v, CHUNK_SIZE - 1, &stop);
        } else {
            if (v[0] == ChunkLengthNeedsCalculated) {
                short length = v[1] | v[2] << 8;
                block(v + 3, length, &stop);
            } else if (v[0] & (NextIsNonCopyBuffer | NoCopy)) {
                uint32_t *length = (uint32_t *)(v + 1);
                --count;
                block(*++p, *length, &stop);
            } else {
                CHCAssert(false, "Logic error. It's impossible to go here.");
            }
        }

        if (stop) {
            break;
        }
    }
}

CHData *CHData::dataWithBytes(const char *bytes, uint32_t length)
{
    CHData *obj = nullptr;
    if (length < 8) {
        ;
    }
    return obj;
}

CHData *CHData::dataWithCapacity(uint32_t capacity)
{
    CHData *obj = new CHData(capacity);
    return obj;
}

CHData *CHData::dataWithData(const CHData *other)
{
    return other->duplicate();
}

uint32_t CHData::length() const
{
    return d_d(this, size)();
}

uint32_t CHData::capacity() const
{
    return d_d(this, capacity)();
}
