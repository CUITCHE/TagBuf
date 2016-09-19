//
//  CHBuffer.hpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/9/2.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef CHBuffer_hpp
#define CHBuffer_hpp

#include <stdio.h>
#include "id.hpp"

typedef void(*CHDataChunkCallback)(const char *bytes, unsigned long byteLength, bool *stop);

TAGGED_AVAILABLE class CHData : public CHObject
{
protected:
    ~CHData() override;
    explicit CHData(uint32_t capacity);
    CHData *duplicate() const;
public:
    CHData() = delete ;

//    explicit CHData(CHData &&other);
//    void operator=(CHData &&right);

    void appendBytes(const char *bytes, uint32_t length);
    void appendBytesNoCopy(const char *bytes, uint32_t length, bool freeWhenDone = false);
    void enumerateByteUsingBlock(CHDataChunkCallback block) const;

    uint32_t length() const;
    uint32_t capacity() const;

    static CHData *dataWithBytes(const char *bytes, uint32_t length);
    static CHData *dataWithCapacity(uint32_t capacity);
    static CHData *dataWithData(const CHData *other);
};

#endif /* CHBuffer_hpp */
