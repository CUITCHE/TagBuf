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
#include <functional>

using CHDataChunkCallback = std::function<void(const char *bytes, uint32_t byteLength, bool *stop)>;

CLASS_TAGGEDPOINTER_AVAILABLE class CHData : public CHObject
{
    __SUPPORTRUNTIME__(CHData);
protected:
    explicit CHData(uint32_t capacity = 0);
    CHData *duplicate() const;
public:
    ~CHData() override;
//    explicit CHData(CHData &&other);
//    void operator=(CHData &&right);

    void appendBytes(const char *bytes, uint32_t length);
    void appendBytesNoCopy(const char *bytes, uint32_t length, bool freeWhenDone = false);
    void appendData(const CHData *other);

    void enumerateByteUsingBlock(CHDataChunkCallback block) const;

    uint32_t length() const;
    uint32_t capacity() const;

    static CHData *dataWithBytes(const char *bytes, uint32_t length);
    static CHData *dataWithCapacity(uint32_t capacity);
    static CHData *dataWithData(const CHData *other);

    // runtime
    Class getClass() const override;
    static Class getClass(std::nullptr_t);
private:
    static id allocateInstance();
};

#endif /* CHBuffer_hpp */
