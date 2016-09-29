//
//  CHString.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/31.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "CHString.hpp"
#include "TaggedPointer.h"
#include "TagBufDefines.h"
#include "tagBuf.hpp"
#include "runtime.hpp"
#include <stdlib.h>
#include <string.h>
#include "tprintf.hpp"
#include "Algorithm.hpp"
#include "CHArray.hpp"
#include "CHData.hpp"
#include "CHException.hpp"

typedef enum : uint32_t {
    CHStringBufferTypeImmutable,
    CHStringBufferTypeMutable
} CHStringBufferType;

struct runtimeclass(CHString)
{
    static struct method_list_t *methods()
    {
        static method_list_t method[40] = {
            // runtime
            {.method = {0, overloadFunc(Class(*)(std::nullptr_t),CHString::getClass), selector(getClass), __Static} },
            {.method = {0, overloadFunc(Class(CHString::*)()const, &CHString::getClass), selector(getClass), __Member} },
            {.method = {0, funcAddr(&CHString::allocateInstance), selector(allocateInstance), __Static} },
            // copy
            {.method = {0, funcAddr(&CHString::copyWithZone), selector(copyWithZone), __Member} },
            {.method = {0, funcAddr(&CHString::mutableCopyWithZone), selector(mutableCopyWithZone), __Member} },
            // protocol
            {.method = {0, funcAddr(&CHString::equalTo), selector(equalTo), __Member} },
            {.method = {0, funcAddr(&CHString::description), selector(description), __Member} },
            {.method = {0, funcAddr(&CHString::hash), selector(hash), __Member} },
            // memeber method
            {.method = {0, funcAddr(&CHString::length), selector(length), __Member} },
            {.method = {0, funcAddr(&CHString::capacity), selector(capacity), __Member} },
            {.method = {0, funcAddr(&CHString::substringFromIndex), selector(substringFromIndex), __Member} },
            {.method = {0, funcAddr(&CHString::substringToIndex), selector(substringToIndex), __Member} },
            {.method = {0, funcAddr(&CHString::substring), selector(substring), __Member} },
            {.method = {0, funcAddr(&CHString::compare), selector(compare), __Member} },
            {.method = {0, funcAddr(&CHString::caseInsensitiveCompare), selector(caseInsensitiveCompare), __Member} },
            {.method = {0, funcAddr(&CHString::isEqualToString), selector(isEqualToString), __Member} },
            {.method = {0, funcAddr(&CHString::hasPrefix), selector(hasPrefix), __Member} },
            {.method = {0, funcAddr(&CHString::hasSuffix), selector(hasSuffix), __Member} },
            {.method = {0, funcAddr(&CHString::containsString), selector(containsString), __Member} },
            {.method = {0, funcAddr(&CHString::rangeOfString), selector(rangeOfString), __Member} },
            {.method = {0, funcAddr(&CHString::stringByAppendingString), selector(stringByAppendingString), __Member} },
            {.method = {0, funcAddr(&CHString::stringByAppendingFormat), selector(stringByAppendingFormat), __Member} },
            {.method = {0, funcAddr(&CHString::stringByReplacingOccurrencesOfStringWithString), selector(stringByReplacingOccurrencesOfStringWithString), __Member} },
            {.method = {0, funcAddr(&CHString::stringByReplacingCharactersInRange), selector(stringByReplacingCharactersInRange), __Member} },
            {.method = {0, funcAddr(&CHString::componentsSeparatedByString), selector(componentsSeparatedByString), __Member} },
            {.method = {0, funcAddr(&CHString::doubleValue), selector(doubleValue), __Member} },
            {.method = {0, funcAddr(&CHString::floatValue), selector(floatValue), __Member} },
            {.method = {0, funcAddr(&CHString::intValue), selector(intValue), __Member} },
            {.method = {0, funcAddr(&CHString::longLongValue), selector(longLongValue), __Member} },
            {.method = {0, funcAddr(&CHString::boolValue), selector(boolValue), __Member} },
            {.method = {0, funcAddr(&CHString::uppercaseString), selector(uppercaseString), __Member} },
            {.method = {0, funcAddr(&CHString::lowercaseString), selector(lowercaseString), __Member} },
            {.method = {0, funcAddr(&CHString::dataUsingEncoding), selector(dataUsingEncoding), __Member} },
            {.method = {0, funcAddr(&CHString::getBytes), selector(getBytes), __Member} },
            {.method = {0, funcAddr(&CHString::stringWithString), selector(stringWithString), __Static} },
            {.method = {0, funcAddr(&CHString::stringWithBytes), selector(stringWithBytes), __Static} },
            {.method = {0, funcAddr(&CHString::stringWithBytesNoCopy), selector(stringWithBytesNoCopy), __Static} },
            {.method = {0, funcAddr(&CHString::stringWithUTF8String), selector(stringWithUTF8String), __Static} },
            {.method = {0, overloadFunc(CHString*(*)(const char *, ...),&CHString::stringWithFormat), selector(stringWithFormat), __Static|__Overload} },
            {.method = {0, overloadFunc(CHString*(*)(const char *, va_list),&CHString::stringWithFormat), selector(stringWithFormat), __Static|__Overload} },
        };
        return method;
    }
};

static class_t ClassNamed(CHString) = {
    CHObject::getClass(nullptr),
    selector(CHString),
    runtimeclass(CHString)::methods(),
    nullptr,
    allocateCache(),
    selector(^#CHString),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHString)), sizeof(CHString))),
    0,
    40
};

struct CHStringPrivate
{
    void *buffer = 0;
    uint32_t length = 0;
    uint32_t capacity = 0;
    uint64_t hash = 0;

    CHStringPrivate(uint32_t capacity) :capacity(capacity)
    {
        if (capacity > 0) {
            buffer = malloc(capacity);
        }
    }

    CHStringPrivate(void *buffer, uint32_t length, bool isNoCopy)
    : length(length), capacity(length), buffer(buffer)
    {
        if (!isNoCopy) {
            this->buffer = malloc(length);
            memcpy(this->buffer, buffer, length);
        }
    }

    CHStringPrivate(const char *utf8String)
    {
        length = capacity = (uint32_t)strlen(utf8String);
        buffer = malloc(length);
        memcpy(buffer, utf8String, length);
    }

    ~CHStringPrivate() { destructor(); }

    CHStringPrivate *duplicate() const
    {
        CHStringPrivate *d = new CHStringPrivate(buffer, length, false);
        d->hash = hash;
        return d;
    }

    void *substring(CHRange range) const
    {
        if (CHMaxRange(range) > length) {
            return nullptr;
        }
        void *m = malloc(range.length);
        memcpy(m, (char *)buffer + range.location, range.length);
        return m;
    }

    static CHStringPrivate *substring(const char *str, uint32_t length, CHRange range)
    {
        if (CHMaxRange(range) > length) {
            return nullptr;
        }
        void *m = malloc(range.length);
        memcpy(m, str + range.location, range.length);
        CHStringPrivate *d = new CHStringPrivate(m, length, true);
        return d;
    }

    bool caseInsensitiveCompare(const CHStringPrivate *other) const
    {
        if (length != other->length) {
            return false;
        }
        return caseInsensitiveCompare((const char *)this->buffer, (const char *)other->buffer, length);
    }

    static bool caseInsensitiveCompare(const char *_1, const char *_2, int64_t lengthCommon)
    {
        char c1 = 0, c2 = 0;
        ++lengthCommon;
        while (--lengthCommon) {
            c1 = *_1++;
            c2 = *_2++;
            if (c1 >= 'A' && c1 <= 'Z') {
                c1 += 32;
            }
            if (c2 >= 'A' && c2 <= 'Z') {
                c2 += 32;
            }
            if (c1 != c2) {
                break;
            }
        }
        return lengthCommon == 0;
    }

    bool compare(const CHStringPrivate *other) const
    {
        if (length != other->length) {
            return false;
        }
        const char *_1 = (const char *)buffer;
        const char *_2 = (const char *)other->buffer;
        return compare(_1, _2, length);
    }

    static bool compare(const char *_1, const char *_2, int64_t lengthCommon)
    {
        ++lengthCommon;
        while (--lengthCommon) {
            if (*_1++ != *_2++) {
                break;
            }
        }
        return lengthCommon == 0;
    }

    bool hasPrefix(const CHStringPrivate *other) const
    {
        const char *_1 = (const char *)buffer;
        const char *_2 = (const char *)other->buffer;
        uint32_t length1 = length;
        uint32_t length2 = other->length;
        return  hasPrefix(_1, length1, _2, length2);
    }

    static bool hasPrefix(const char *str1, int64_t length1, const char *str2, int64_t length2)
    {
        if (length1 < length2) {
            return false;
        }
        ++length1, ++length2;
        while (--length2 > 0 && *str1++ == *str2) {
            continue;
        }
        return length2 == 0;
    }

    bool hasSuffix(const CHStringPrivate *other) const
    {
        const char *_1 = (const char *)buffer;
        const char *_2 = (const char *)other->buffer;
        uint32_t length1 = length;
        uint32_t length2 = other->length;
        return hasSuffix(_1, length1, _2, length2);
    }

    static bool hasSuffix(const char *str1, int64_t length1, const char *str2, int64_t length2)
    {
        if (length1 < length2) {
            return false;
        }
        str1 += length1;
        str2 += length2;
        ++length1, ++length2;
        while (--length2 > 0 && *--str1 == *--str2) {
            continue;
        }
        return length2 == 0;
    }

    CHRange rangeOfString(const CHStringPrivate *other) const
    {
        return BMContainsString((const char *)buffer, length, (const char *)other->buffer, other->length);
    }

    void appendBytes(const void *bytes, uint32_t length, CHStringBufferType bufferType)
    {
        if (bytes == 0) {
            throwException(CHInvalidArgumentException, "Argument bytes must not be nil.");
        }
        checkMemory(length, bufferType);
        memcpy((char *)buffer + this->length, bytes, length);
        this->length += length;
        hash = 0;
    }

    static CHStringPrivate *stringByReplacingOccurrencesOfStringWithString(const char *src,
                                                                           uint32_t srcLength,
                                                                           const char *target,
                                                                           uint32_t targetLength,
                                                                           const char *replacement,
                                                                           uint32_t replacementLength)
    {
        vector<CHRange> result;
        searchAllOfOccurrencesOfString(src, srcLength, target, targetLength, result);
        if (result.empty()) {
            return nullptr;
        }
        char *destBuffer = nullptr;
        const char *srcBuffer = src;
        uint32_t buffersize = srcLength;
        if (targetLength == replacementLength) {
            destBuffer = (char *)malloc(srcLength);
            memcpy(destBuffer, srcBuffer, srcLength);
            for (auto &range : result) { // do replace
                memcpy(destBuffer + range.location, replacement, replacementLength);
            }
        } else {
            buffersize = srcLength + (int32_t)(replacementLength - targetLength) * (uint32_t)result.size();
            destBuffer = (char *)malloc(buffersize);
            uint32_t offset = 0;
            char *dest = destBuffer;
            for (auto &range : result) {
                // copy src buffer
                int location = range.location - targetLength;
                for (int i=offset; i<location; ++i) {
                    *dest++ = *srcBuffer++;
                }
                // copy replacement
                memcpy(dest, replacement, replacementLength);
                dest += replacementLength;
                srcBuffer += targetLength;
                offset = range.location;
            }
        }
        CHStringPrivate *d = new CHStringPrivate(destBuffer, buffersize, true);
        return d;
    }

    static CHStringPrivate *stringByReplacingCharactersInRange(const char *src,
                                                               uint32_t length,
                                                               CHRange range,
                                                               const char *replacement,
                                                               uint32_t replacementLength)
    {
        uint32_t offset = CHMaxRange(range);
        if (offset > length) {
            return nullptr;
        }
        uint32_t buffersize = length - range.length + replacementLength;
        char *buf = (char *)malloc(buffersize);
        memcpy(buf, src, range.location);
        memcpy(buf + range.location, replacement, replacementLength);
        memcpy(buf + range.location + replacementLength, src + offset, length - offset);
        CHStringPrivate *d = new CHStringPrivate(buf , buffersize, true);
        return d;
    }

    void componentsSeparatedByString(const char *separaotr, uint32_t separaotrLength, vector<CHStringPrivate *> &components)
    {
        vector<CHRange> result;
        searchAllOfOccurrencesOfString((const char *)buffer, length, separaotr, separaotrLength, result);
        if (result.empty()) {
            components.push_back(duplicate());
            return;
        }
        uint32_t offset = 0;
        char *buf = (char *)buffer;
        for (auto &range : result) {
            CHStringPrivate *d = new CHStringPrivate(buf + offset, range.location - offset, false);
            components.push_back(d);
            offset += CHMaxRange(range);
        }
    }

    static double doubleValue(const char *buffer, uint32_t length)
    {
        double v = 0.0;
        double base = 10;
        unsigned char ch = 0;
        const char *src = buffer - 1;
        const char *end = src + length;
        while ((ch = *++src, src) < end && ch < 128) {
            if (ch == '-') {
                if (src == buffer) {
                    v = 0.0;
                    break;
                }
                base = -base;
                continue;
            }
            if (ch == '.') {
                if (base < 1) {
                    v = 0.0;
                    break;
                }
                base = 0.1;
                continue;
            }
            if (ch >= '0' && ch <= '9') {
                v = v * base + ch - '0';
            } else {
                break;
            }
        }
        return v;
    }

    static long long longLongValue(const char *buffer, uint32_t length)
    {
        long long v = 0;
        int base = 10;
        unsigned char ch = 0;
        const char *src = buffer - 1;
        const char *end = src + length;
        while ((ch = *++src, src) < end && ch < 128) {
            if (ch == '-') {
                if (src == buffer) {
                    v = 0;
                    break;
                }
                base = -base;
                continue;
            }
            if (ch >= '0' && ch <= '9') {
                v = v * base + ch - '0';
            } else {
                break;
            }
        }

        return v;
    }

    static CHStringPrivate *uppercaseString(const char *buffer, uint32_t length)
    {
        CHStringPrivate *d = new CHStringPrivate((char *)buffer, length, false);
        char *begin = (char *)d->buffer - 1;
        const char *end = begin + length;
        while (++begin < end) {
            if (*begin >= 'a' && *end <= 'z') {
                *begin += 32;
            }
        }
        return d;
    }

    static CHStringPrivate *lowercaseString(const char *buffer, uint32_t length)
    {
        CHStringPrivate *d = new CHStringPrivate((char *)buffer, length, false);
        char *begin = (char *)d->buffer - 1;
        const char *end = begin + length;
        while (++begin < end) {
            if (*begin >= 'A' && *end <= 'Z') {
                *begin -= 32;
            }
        }
        return d;
    }

    // mutable string
    void insertString(const void *aString, uint32_t length, uint32_t index)
    {
        if (length == 0) {
            return;
        }
        checkMemory(length, CHStringBufferTypeMutable);
        if (index == this->length) {
            memcpy((char *)buffer + this->length, aString, length);
        } else {
            memcpy((char *)buffer + index, (char *)buffer + index + length, length);
            memcpy((char *)buffer + index, aString, length);
        }
        hash = 0;
    }

    void deleteCharactersInRange(CHRange range) throw()
    {
        uint32_t max = CHMaxRange(range);
        if (max > length) {
            throwException(CHOutOfRangeException, "Argument:range's max(%u) is over CHString'length(%u)", max, length);
        }
        memcpy((char *)buffer + range.location, (char *)buffer + max, length - max);
        length -= range.length;
        hash = 0;
    }

    void setString(void *bytes, uint32_t length)
    {
        this->length = length;
        this->capacity = length;
        free(buffer);
        buffer = malloc(length);
        memcpy(buffer, bytes, length);
    }

private:
    inline void destructor()
    {
        if (buffer) {
            free(buffer);
        }
    }

    inline void checkMemory(uint32_t addLength, CHStringBufferType bufferType)
    {
        if (addLength + length > capacity) {
            capacity = (addLength + length);
            if (bufferType == CHStringBufferTypeMutable) {
                capacity *= 2;
            }
            buffer = realloc(buffer, capacity);
        }
    }
};

Implement(CHString);

CHString::CHString() : CHObject()
{}

CHString::~CHString()
{
    if (reserved()) {
        delete (CHStringPrivate *)reserved();
    }
}

#define d_d(obj, field) ((CHStringPrivate *)(obj)->reserved())->field
#define TAGGED_STRING_POINTER_RETURN(obj) reinterpret_cast<const char *>(((uintptr_t)obj ^ TAGGED_POINTER_STRING_FLAG) >> 1)

uint32_t CHString::length() const
{
    if (isTaggedPointer()) {
        const char *ptr = reinterpret_cast<const char *>(((uintptr_t)this ^ TAGGED_POINTER_STRING_FLAG) >> 1);
        return (uint32_t)strlen(ptr);
    }
    return d_d(this, length);
}

uint32_t CHString::capacity() const
{
    if (isTaggedPointer()) {
        return 0;
    }
    return d_d(this, capacity);
}

// CHStringExtensionMethods
CHString *CHString::substringFromIndex(uint32_t index) const
{
    return CHString::substring(CHMakeRange(index, length() - index));
}

CHString *CHString::substringToIndex(uint32_t index) const
{
    return CHString::substring(CHMakeRange(0, index + 1));
}

CHString *CHString::substring(CHRange range) const
{
    CHString *string = new CHString();
    CHStringPrivate *d = nullptr;
    const char *str = nullptr;
    if (isTaggedPointer()) {
        str = TAGGED_STRING_POINTER_RETURN(this);
    } else {
        str = (const char *)d_d(this, buffer);
    }

    d = CHStringPrivate::substring(str, this->length(), range);
    if (!d) {
        d = new CHStringPrivate(0u);
    }
    string->setReserved(d);
    return string;
}

bool CHString::compare(CHString *aString) const
{
    if (this == aString) {
        return true;
    }
    uint32_t length1 = this->length();
    uint32_t length2 = aString->length();
    if (length1 != length2) {
        return false;
    }
    const char *str1 = nullptr;
    const char *str2 = nullptr;
    if (isTaggedPointer()) {
        str1 = TAGGED_STRING_POINTER_RETURN(this);
    } else {
        str1 = (const char *)d_d(this, buffer);
    }

    if (aString->isTaggedPointer()) {
        str2 = TAGGED_STRING_POINTER_RETURN(aString);
    } else {
        str2 = (const char *)d_d(aString, buffer);
    }
    return CHStringPrivate::compare(str1, str2, length1);
}

bool CHString::caseInsensitiveCompare(CHString *aString) const
{
    if (this == aString) {
        return true;
    }
    uint32_t length1 = this->length();
    uint32_t length2 = aString->length();
    if (length1 != length2) {
        return false;
    }
    const char *str1 = nullptr;
    const char *str2 = nullptr;
    if (isTaggedPointer()) {
        str1 = TAGGED_STRING_POINTER_RETURN(this);
    } else {
        str1 = (const char *)d_d(this, buffer);
    }

    if (aString->isTaggedPointer()) {
        str2 = TAGGED_STRING_POINTER_RETURN(aString);
    } else {
        str2 = (const char *)d_d(aString, buffer);
    }
    return CHStringPrivate::caseInsensitiveCompare(str1, str2, length1);
}

bool CHString::isEqualToString(CHString *aString) const
{
    return caseInsensitiveCompare(aString);
}

bool CHString::hasPrefix(CHString *aString) const
{
    if (this == aString) {
        return true;
    }
    uint32_t length1 = this->length();
    uint32_t length2 = aString->length();
    if (length1 < length2) {
        return false;
    }
    const char *str1 = nullptr;
    const char *str2 = nullptr;
    if (isTaggedPointer()) {
        str1 = TAGGED_STRING_POINTER_RETURN(this);
    } else {
        str1 = (const char *)d_d(this, buffer);
    }

    if (aString->isTaggedPointer()) {
        str2 = TAGGED_STRING_POINTER_RETURN(aString);
    } else {
        str2 = (const char *)d_d(aString, buffer);
    }
    return CHStringPrivate::hasPrefix(str1, length1, str2, length2);
}

bool CHString::hasSuffix(CHString *aString) const
{
    if (this == aString) {
        return true;
    }
    uint32_t length1 = this->length();
    uint32_t length2 = aString->length();
    if (length1 < length2) {
        return false;
    }
    const char *str1 = nullptr;
    const char *str2 = nullptr;
    if (isTaggedPointer()) {
        str1 = TAGGED_STRING_POINTER_RETURN(this);
    } else {
        str1 = (const char *)d_d(this, buffer);
    }

    if (aString->isTaggedPointer()) {
        str2 = TAGGED_STRING_POINTER_RETURN(aString);
    } else {
        str2 = (const char *)d_d(aString, buffer);
    }
    return CHStringPrivate::hasSuffix(str1, length1, str2, length2);
}

bool CHString::containsString(CHString *aString) const
{
    return rangeOfString(aString).location != CHNotFound;
}

CHRange CHString::rangeOfString(CHString *aString) const
{
    if (this == aString) {
        return CHMakeRange(0, length());
    }

    uint32_t length1 = this->length();
    uint32_t length2 = aString->length();
    if (length1 < length2) {
        return CHMakeRange(CHNotFound, 0);
    }
    const char *str1 = nullptr;
    const char *str2 = nullptr;
    if (isTaggedPointer()) {
        str1 = TAGGED_STRING_POINTER_RETURN(this);
    } else {
        str1 = (const char *)d_d(this, buffer);
    }

    if (aString->isTaggedPointer()) {
        str2 = TAGGED_STRING_POINTER_RETURN(aString);
    } else {
        str2 = (const char *)d_d(aString, buffer);
    }
    return BMContainsString(str1, length1, str2, length2);
}

CHString *CHString::stringByAppendingString(CHString *aString) const
{
    uint32_t otherLength = aString->length();
    if (otherLength == 0) {
        return (CHString *)this;
    }
    bool thisIsTagPointer = isTaggedPointer();
    bool otherIsTagPointer = isTaggedPointer();
    uint32_t thisLength = this->length();
    CHString *str = new CHString();
    if (thisIsTagPointer && otherIsTagPointer) {
        char *m = (char *)malloc(otherLength + thisLength);
        memcpy(m, TAGGED_STRING_POINTER_RETURN(this), thisLength);
        memcpy(m + thisLength, TAGGED_STRING_POINTER_RETURN(aString), otherLength);
        CHStringPrivate *d = new CHStringPrivate(m, thisLength + otherLength, true);
        str->setReserved(d);
        return str;
    }
    const CHString *heapString = nullptr;
    const void *buffer = nullptr;
    if (!thisIsTagPointer) {
        heapString = this;
        if (otherIsTagPointer) {
            buffer = TAGGED_STRING_POINTER_RETURN(aString);
        } else {
            buffer = d_d(aString, buffer);
        }
    } else if (!otherIsTagPointer) {
        heapString = aString;
        if (thisIsTagPointer) {
            buffer = TAGGED_STRING_POINTER_RETURN(this);
        } else {
            buffer = d_d(this, buffer);
        }
        otherLength = thisLength;
    }

    CHStringPrivate *d = d_d(heapString, duplicate());
    d->appendBytes(buffer, otherLength, CHStringBufferTypeImmutable);
    str->setReserved(d);
    return str;
}

CHString *CHString::stringByAppendingFormat(const char *format, ...) const
{
    if (!*format) {
        return (CHString *)this;
    }
    va_list ap;
    va_start(ap, format);
    char *buffer = nullptr;
    uint32_t capacity = 0;
    uint32_t length = (uint32_t)tprintf_c(buffer, &capacity, format, ap, OUTPUT_FLAG_DESCRIPTION);
    va_end(ap);

    CHStringPrivate *d = 0;
    if (isTaggedPointer()) {
        d = new CHStringPrivate((void *)TAGGED_STRING_POINTER_RETURN(this), this->length(), false);
    } else {
        d = d_d(this, duplicate());
    }

    d->appendBytes(buffer, length, CHStringBufferTypeImmutable);
    CHString *str = new CHString();
    str->setReserved(d);
    free(buffer);
    return str;
}

CHString *CHString::stringByReplacingOccurrencesOfStringWithString(CHString *target, CHString *replacement) const
{
    const char *src = (const char *)(isTaggedPointer() ? TAGGED_STRING_POINTER_RETURN(this) : d_d(this, buffer));
    const char *tat = (const char *)(isTaggedPointer() ? TAGGED_STRING_POINTER_RETURN(target) : d_d(target, buffer));
    const char *rpm = (const char *)(isTaggedPointer() ? TAGGED_STRING_POINTER_RETURN(replacement) : d_d(replacement, buffer));
    CHStringPrivate *d = CHStringPrivate::stringByReplacingOccurrencesOfStringWithString(src,
                                                                                         length(),
                                                                                         tat,
                                                                                         target->length(),
                                                                                         rpm, replacement->length());
    if (!d) {
        return (CHString *)this;
    }
    CHString *string = new CHString();
    string->setReserved(d);
    return string;
}

CHString *CHString::stringByReplacingCharactersInRange(CHRange range, CHString *replacement) const
{
    const char *src = (const char *)(isTaggedPointer() ? TAGGED_STRING_POINTER_RETURN(this) : d_d(this, buffer));
    const char *rpm = (const char *)(isTaggedPointer() ? TAGGED_STRING_POINTER_RETURN(replacement) : d_d(replacement, buffer));
    CHStringPrivate *d = CHStringPrivate::stringByReplacingCharactersInRange(src, length(), range, rpm, replacement->length());
    if (!d) {
        return (CHString *)this;
    }
    CHString *string = new CHString();
    string->setReserved(d);
    return string;
}

ARRAY_CONTAINS_TYPE(CHArray *, CHString *) CHString::componentsSeparatedByString(CHString *separaotr) const
{
    vector<CHStringPrivate *> components;
    d_d(this, componentsSeparatedByString((const char *)d_d(separaotr, buffer), separaotr->length(), components));
    // TODO: needs to implement CHMutableArray
    CHArray *array = nullptr;
    return array;
}

double CHString::doubleValue() const
{
    const char *src = (const char *)(isTaggedPointer() ? TAGGED_STRING_POINTER_RETURN(this) : d_d(this, buffer));
    return CHStringPrivate::doubleValue(src, length());
}

float CHString::floatValue() const
{
    const char *src = (const char *)(isTaggedPointer() ? TAGGED_STRING_POINTER_RETURN(this) : d_d(this, buffer));
    return CHStringPrivate::doubleValue(src, length());
}

long long CHString::longLongValue() const
{
    const char *src = (const char *)(isTaggedPointer() ? TAGGED_STRING_POINTER_RETURN(this) : d_d(this, buffer));
    return CHStringPrivate::longLongValue(src, length());
}

int CHString::intValue() const
{
    const char *src = (const char *)(isTaggedPointer() ? TAGGED_STRING_POINTER_RETURN(this) : d_d(this, buffer));
    return (int)CHStringPrivate::longLongValue(src, length());
}

bool CHString::boolValue() const
{
    return intValue() != 0;
}

CHString::operator double() const
{
    return doubleValue();
}

CHString::operator float() const
{
    return floatValue();
}

CHString::operator int() const
{
    return intValue();
}

CHString::operator bool() const
{
    return boolValue();
}

CHString::operator long long() const
{
    return longLongValue();
}

uint32_t CHString::getBytes(void *buffer, uint32_t bufferLength) const
{
    uint32_t copyLength = bufferLength;
    if (copyLength > length()) {
        copyLength = length();
    }
    const char *src = (const char *)(isTaggedPointer() ? TAGGED_STRING_POINTER_RETURN(this) : d_d(this, buffer));
    memcpy(buffer, src, copyLength);
    return copyLength;
}

CHString *CHString::uppercaseString() const
{
    const char *src = (const char *)(isTaggedPointer() ? TAGGED_STRING_POINTER_RETURN(this) : d_d(this, buffer));
    CHStringPrivate *d = CHStringPrivate::uppercaseString(src, length());
    CHString *str = new CHString();
    str->setReserved(d);
    return str;
}

CHString *CHString::lowercaseString() const
{
    const char *src = (const char *)(isTaggedPointer() ? TAGGED_STRING_POINTER_RETURN(this) : d_d(this, buffer));
    CHStringPrivate *d = CHStringPrivate::lowercaseString(src, length());
    CHString *str = new CHString();
    str->setReserved(d);
    return str;
}

CHData *CHString::dataUsingEncoding() const
{
    const char *src = (const char *)(isTaggedPointer() ? TAGGED_STRING_POINTER_RETURN(this) : d_d(this, buffer));
    CHData *data = CHData::dataWithBytes(src, length());
    return data;
}

/// creation
CHString *CHString::stringWithString(const CHString *other)
{
    return (CHString *)other;
}

CHString *CHString::stringWithBytes(const void *bytes, uint32_t length)
{
    if ((uintptr_t)bytes < MAX_CONSTANT_ADDRESS) {
        CHString *str = reinterpret_cast<CHString *>((uintptr_t)bytes << 1 | TAGGED_POINTER_STRING_FLAG);
        return str;
    }
    CHString *str = new CHString();
    str->setReserved(new CHStringPrivate((void *)bytes, length, false));
    return str;
}

CHString *CHString::stringWithBytesNoCopy(void *bytes, uint32_t length)
{
    if ((uintptr_t)bytes < MAX_CONSTANT_ADDRESS) {
        CHString *str = reinterpret_cast<CHString *>((uintptr_t)bytes << 1 | TAGGED_POINTER_STRING_FLAG);
        return str;
    }
    CHString *str = new CHString();
    str->setReserved(new CHStringPrivate((void *)bytes, length, true));
    return str;
}

CHString *CHString::stringWithUTF8String(const char *nullTerminatedCString)
{
    if ((uintptr_t)nullTerminatedCString < MAX_CONSTANT_ADDRESS) {
        CHString *str = reinterpret_cast<CHString *>((uintptr_t)nullTerminatedCString << 1 | TAGGED_POINTER_STRING_FLAG);
        return str;
    }
    CHString *str = new CHString();
    str->setReserved(new CHStringPrivate(nullTerminatedCString));
    return str;
}

CHString *CHString::stringWithFormat(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    va_end(ap);
    CHString *str = CHString::stringWithFormat(format, ap);
    return str;
}

CHString *CHString::stringWithFormat(const char *format, va_list argList)
{
    char *buffer = nullptr;
    uint32_t capacity = 0;
    uint32_t length = (uint32_t)tprintf_c(buffer, &capacity,format, argList, OUTPUT_FLAG_DESCRIPTION);
    CHString *str = CHString::stringWithBytesNoCopy(buffer, length);
    d_d(str, capacity) = capacity;
    return str;
}

// protocol
bool CHString::equalTo(id anObject) const
{
    if (!anObject->isKindOfClass(CHString::getClass(nullptr))) {
        return false;
    }
    return this->isEqualToString((CHString *)anObject);
}

CHString *CHString::description() const
{
    CHString *str = CHString::stringWithFormat("%p@", this);
    return str;
}

uint64_t CHString::hash() const
{
    uint64_t &hash = d_d(this, hash);
    if (hash == 0) {
        hash = bkdr_hash((const char *)d_d(this, buffer), d_d(this, length));
    }
    return hash;
}

// copy
id CHString::copyWithZone(std::nullptr_t) const
{
    return (id)this;
}

id CHString::mutableCopyWithZone(std::nullptr_t) const
{
    CHString *string = new CHString();
    string->setReserved(d_d(this, duplicate()));
    return string;
}

// CHMutableString
struct runtimeclass(CHMutableString)
{
    static struct method_list_t *methods()
    {
        static method_list_t method[12] = {
            {.method = {0, overloadFunc(Class(*)(std::nullptr_t),CHMutableString::getClass), selector(getClass), __Static} },
            {.method = {0, overloadFunc(Class(CHMutableString::*)()const, &CHMutableString::getClass), selector(getClass), __Member} },
            {.method = {0, funcAddr(&CHMutableString::allocateInstance), selector(allocateInstance), __Static} },
            // copy
            {.method = {0, funcAddr(&CHMutableString::copyWithZone), selector(copyWithZone), __Member} },
            {.method = {0, funcAddr(&CHMutableString::insertString), selector(insertString), __Member} },
            {.method = {0, funcAddr(&CHMutableString::deleteCharactersInRange), selector(deleteCharactersInRange), __Member} },
            {.method = {0, funcAddr(&CHMutableString::appendString), selector(appendString), __Member} },
            {.method = {0, funcAddr(&CHMutableString::appendFormat), selector(appendFormat), __Member} },
            {.method = {0, funcAddr(&CHMutableString::setString), selector(setString), __Member} },
            {.method = {0, funcAddr(&CHMutableString::replaceOccurrencesOfStringWithString), selector(replaceOccurrencesOfStringWithString), __Member} },
            {.method = {0, funcAddr(&CHMutableString::replaceCharactersInRangeWithString), selector(replaceCharactersInRangeWithString), __Member} },
            {.method = {0, funcAddr(&CHMutableString::stringWithCapacity), selector(stringWithCapacity), __Member} },
        };
        return method;
    }
};

static class_t ClassNamed(CHMutableString) = {
    CHString::getClass(nullptr),
    selector(CHMutableString),
    nullptr,
    nullptr,
    allocateCache(),
    selector(^#CHMutableString),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHMutableString)), sizeof(CHMutableString))),
    0,
    12
};

Implement(CHMutableString);

CHMutableString::CHMutableString() :CHString(){}

// craetion
CHMutableString *CHMutableString::stringWithCapacity(uint32_t capacity)
{
    CHMutableString *str = new CHMutableString();
    str->setReserved(new CHStringPrivate(capacity));
    return str;
}

void CHMutableString::insertString(CHString *aString, uint32_t index)
{
    auto p = d_d((CHMutableString *)aString, buffer);
    d_d(this, insertString(p, aString->length(), index));
}

void CHMutableString::deleteCharactersInRange(CHRange range) throw()
{
    d_d(this, deleteCharactersInRange(range));
}

void CHMutableString::appendString(const CHString *other)
{
    d_d(this, appendBytes(((CHStringPrivate *)((CHMutableString *)other))->buffer, other->length(), CHStringBufferTypeMutable));
}

void CHMutableString::appendFormat(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char *buffer = nullptr;
    uint32_t length = (uint32_t)tprintf_c(buffer, nullptr, format, ap, OUTPUT_FLAG_DESCRIPTION);
    if (buffer) {
        d_d(this, appendBytes(buffer, length, CHStringBufferTypeMutable));
    }
    va_end(ap);
}

void CHMutableString::setString(CHString *aString)
{
    d_d(this, setString(((CHStringPrivate *)((CHMutableString *)aString))->buffer, aString->length()));
}

// copy
id CHMutableString::copyWithZone(std::nullptr_t) const
{
    CHMutableString *str = new CHMutableString();
    str->setReserved(d_d(this, duplicate()));
    return str;
}

id CHMutableString::mutableCopyWithZone(std::nullptr_t) const
{
    return CHMutableString::copyWithZone(nullptr);
}
