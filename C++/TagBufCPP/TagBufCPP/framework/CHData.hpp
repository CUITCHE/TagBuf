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
#include "CHObject.hpp"
#include <functional>

using CHDataChunkCallback = std::function<void(const char *bytes, uint32_t byteLength, bool *stop)>;

CLASS_TAGGEDPOINTER_AVAILABLE class CHData : public CHObject
{
    __SUPPORTRUNTIME__(CHData);
protected:
    explicit CHData(uint32_t capacity = 0);
public:
    CHData *duplicate() const;
    ~CHData() override;
//    explicit CHData(CHData &&other);
//    void operator=(CHData &&right);

    void enumerateByteUsingBlock(CHDataChunkCallback block) const;

    uint32_t length() const;
    uint32_t capacity() const;

    static CHData *dataWithBytes(const void *bytes, uint32_t length);
    static CHData *dataWithData(const CHData *other);
    static CHData *dataWithUTF8Data(const char *str);
    static CHData *dataWithBytesNoCopy(void *bytes, uint32_t length, bool freeWhenDone = false);

    // runtime
    Class getClass() const ;
    static Class getClass(std::nullptr_t);
private:
    static id allocateInstance();
};

class CHMutableData : public CHData
{
    __SUPPORTRUNTIME__(CHMutableData);
protected:
    explicit CHMutableData(uint32_t capacity = 0);
public:
    void appendBytes(const void *bytes, uint32_t length);
    void appendBytesNoCopy(const void *bytes, uint32_t length, bool freeWhenDone = false);
    void appendData(const CHData *other);

    static CHMutableData *dataWithBytes(const void *bytes, uint32_t length);
    static CHMutableData *dataWithData(const CHData *other);
    static CHMutableData *dataWithUTF8Data(const char *str);
    static CHMutableData *dataWithCapacity(uint32_t capacity);
    static CHMutableData *dataWithBytesNoCopy(void *bytes, uint32_t length, bool freeWhenDone = false);

    // runtime
    Class getClass() const ;
    static Class getClass(std::nullptr_t);
private:
    static id allocateInstance();
};
#endif /* CHBuffer_hpp */
