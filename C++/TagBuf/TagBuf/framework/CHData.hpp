//
//  CHBuffer.hpp
//  TagBuf
//
//  Created by hejunqiu on 16/9/2.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef CHBuffer_hpp
#define CHBuffer_hpp

#include <stdio.h>
#include "types.h"

typedef void(*CHDataChunkCallback)(const char *bytes, unsigned long byteLength, bool *stop);

class CHData {
    struct CHDataPrivate *d;
public:
    CHData() = delete ;
    explicit CHData(uint32_t capacity);
    ~CHData();

    explicit CHData(CHData &&other);
    void operator=(CHData &&right);

    void appendBytes(const char *bytes, uint32_t length);
    void enumerateByteUsingBlock(CHDataChunkCallback block) const;

    uint32_t length() const;
    uint32_t capacity() const;
};

#endif /* CHBuffer_hpp */
