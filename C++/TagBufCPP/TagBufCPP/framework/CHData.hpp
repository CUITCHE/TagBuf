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
#include "types.h"
#include "id.hpp"

typedef void(*CHDataChunkCallback)(const char *bytes, unsigned long byteLength, bool *stop);

class CHData : public CHObject
{
protected:
    ~CHData() override;
public:
    CHData() = delete ;
    explicit CHData(uint32_t capacity);

    explicit CHData(CHData &&other);
    void operator=(CHData &&right);

    void appendBytes(const char *bytes, uint32_t length);
    void appendBytesNoCopy(const char *bytes, uint32_t length, bool freeWhenDone = false);
    void enumerateByteUsingBlock(CHDataChunkCallback block) const;

    uint32_t length() const;
    uint32_t capacity() const;

    static CHData *dataWithData(const char *bytes, uint32_t length);
};

#endif /* CHBuffer_hpp */
