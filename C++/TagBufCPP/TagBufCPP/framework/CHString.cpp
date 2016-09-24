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

typedef enum : uint32_t {
    CHStringBufferTypeImmutable,
    CHStringBufferTypeMutable
} CHStringBufferType;

struct runtimeclass(CHString)
{
    static struct method_list_t *methods()
    {
        static method_list_t method[] = {
            // runtime
            {.method = {0, overloadFunc(Class(*)(std::nullptr_t),CHString::getClass), selector(getClass), __Static} },
            {.method = {0, overloadFunc(Class(CHString::*)()const, &CHString::getClass), selector(getClass), __Member|__Overload} },
            {.method = {0, funcAddr(&CHString::allocateInstance), selector(allocateInstance), __Static} },
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
            {.method = {0, funcAddr(&CHString::stringWithString), selector(stringWithString), __Member} },
            {.method = {0, funcAddr(&CHString::stringWithBytes), selector(stringWithBytes), __Member} },
            {.method = {0, funcAddr(&CHString::stringWithBytesNoCopy), selector(stringWithBytesNoCopy), __Member} },
            {.method = {0, funcAddr(&CHString::stringWithUTF8String), selector(stringWithUTF8String), __Member} },
            {.method = {0, funcAddr(&CHString::stringWithFormat), selector(stringWithFormat), __Member} },
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
    37
};

struct CHStringPrivate
{
    void *buffer = 0;
    uint32_t length = 0;
    uint32_t capacity = 0;

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
        CHStringPrivate *d = new CHStringPrivate(this->length);
        memcpy(d->buffer, buffer, length);
        d->length = length;
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

    bool caseInsensitiveCompare(const CHStringPrivate *other) const
    {
        if (length != other->length) {
            return false;
        }
        const char *_1 = (const char *)buffer;
        const char *_2 = (const char *)other->buffer;
        uint32_t length = this->length;
        while (length --> 0) {
            if (*_1 != *_2) {
                break;
            }
            ++_1, ++_2;
        }
        return *_1 == *_2;
    }

    bool compare(const CHStringPrivate *other) const
    {
        if (length != other->length) {
            return false;
        }
        const char *_1 = (const char *)buffer;
        const char *_2 = (const char *)other->buffer;
        char c1 = 0, c2 = 0;
        uint32_t length = this->length;
        while (length --> 0) {
            c1 = *_1;
            c2 = *_2;
            if (c1 >= 'A' && c1 <= 'Z') {
                c1 += 32;
            }
            if (c2 >= 'A' && c2 <= 'Z') {
                c2 += 32;
            }
            if (c1 != c2) {
                break;
            }
            ++_1, ++_2;
        }
        return c1 == c2;
    }

    bool hasPrefix(const CHStringPrivate *other) const
    {
        const char *_1 = (const char *)buffer - 1;
        const char *_2 = (const char *)other->buffer - 1;
        uint32_t length1 = length;
        uint32_t length2 = other->length;
        while (--length1 > 0 && --length2 > 0 && *++_1 == *++_2) {
            continue;
        }
        return length2 == 0 && *_1 == *_2;
    }

    bool hasSuffix(const CHStringPrivate *other) const
    {
        const char *_1 = (const char *)buffer + length;
        const char *_2 = (const char *)other->buffer + other->length;
        uint32_t length1 = length;
        uint32_t length2 = other->length;
        while (--length1 > 0 && --length2 > 0 && *--_1 == *--_2) {
            continue;
        }
        return length2 == 0 && *_1 == *_2;
    }

    CHRange rangeOfString(const CHStringPrivate *other) const
    {
        return BMContainsString((const char *)buffer, length, (const char *)other->buffer, other->length);
    }

    void appendBytes(const void *bytes, uint32_t length, CHStringBufferType bufferType)
    {
        checkMemory(length, bufferType);
        memcpy((char *)buffer + this->length, bytes, length);
        this->length += length;
    }

    CHStringPrivate *stringByReplacingOccurrencesOfStringWithString(const char *target,
                                                                    uint32_t targetLength,
                                                                    const char *replacement,
                                                                    uint32_t replacementLength)
    {
        vector<CHRange> result;
        searchAllOfOccurrencesOfString((const char *)buffer, length, target, targetLength, result);
        if (result.empty()) {
            return nullptr;
        }
        char *destBuffer = nullptr;
        const char *srcBuffer = (const char *)buffer;
        uint32_t buffersize = length;
        if (targetLength == replacementLength) {
            destBuffer = (char *)malloc(length);
            memcpy(destBuffer, srcBuffer, length);
            for (auto &range : result) { // do replace
                memcpy(destBuffer + range.location, replacement, replacementLength);
            }
        } else {
            buffersize = length + (int32_t)(replacementLength - targetLength) * (uint32_t)result.size();
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

    CHStringPrivate *stringByReplacingCharactersInRange(CHRange range,
                                                        const char *replacement,
                                                        uint32_t replacementLength) const
    {
        uint32_t offset = CHMaxRange(range);
        if (offset > length) {
            return this->duplicate();
        }
        uint32_t buffersize = length - range.length + replacementLength;
        char *buf = (char *)malloc(buffersize);
        memcpy(buf, buffer, range.location);
        memcpy(buf + range.location, replacement, replacementLength);
        memcpy(buf + range.location + replacementLength, (const char *)buffer + offset, length - offset);
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

    double doubleValue() const
    {
        double v = 0.0;
        double base = 10;
        unsigned char ch = 0;
        const char *src = (const char *)buffer - 1;
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

    long long longLongValue() const
    {
        long long v = 0;
        int base = 10;
        unsigned char ch = 0;
        const char *src = (const char *)buffer - 1;
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

    CHStringPrivate *uppercaseString() const
    {
        CHStringPrivate *d = duplicate();
        char *begin = (char *)d->buffer - 1;
        const char *end = begin + length;
        while (++begin < end) {
            if (*begin >= 'a' && *end <= 'z') {
                *begin += 32;
            }
        }
        return d;
    }

    CHStringPrivate *lowercaseString() const
    {
        CHStringPrivate *d = duplicate();
        char *begin = (char *)d->buffer - 1;
        const char *end = begin + length;
        while (++begin < end) {
            if (*begin >= 'A' && *end <= 'Z') {
                *begin -= 32;
            }
        }
        return d;
    }

private:
    void destructor()
    {
        if (buffer) {
            free(buffer);
        }
    }

    void checkMemory(uint32_t addLength, CHStringBufferType bufferType)
    {
        if (addLength + length > capacity) {
            capacity = (addLength + length);
            if (bufferType == CHStringBufferTypeMutable) {
                capacity *= 2;
            }
            buffer = realloc(buffer, capacity);
        }
    }

    void resize(uint32_t newSize)
    {
        ;
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

#define d_d(obj, field) ((CHStringPrivate *)obj->reserved())->field

uint32_t CHString::length() const
{
    return d_d(this, length);
}

uint32_t CHString::capacity() const
{
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
    void *buffer = d_d(this, substring(range));
    CHString *str = new CHString();
    if (buffer) {
        str->setReserved(new CHStringPrivate(buffer, range.length, true));
    }
    return str;
}

bool CHString::compare(CHString *aString) const
{
    if (this == aString) {
        return true;
    }
    return d_d(this, compare((const CHStringPrivate *)aString->reserved()));
}

bool CHString::caseInsensitiveCompare(CHString *aString) const
{
    if (this == aString) {
        return true;
    }
    return d_d(this, caseInsensitiveCompare((const CHStringPrivate *)aString->reserved()));
}

bool CHString::isEqualToString(CHString *aString) const
{
    return caseInsensitiveCompare(aString);
}

bool CHString::hasPrefix(CHString *str) const
{
    if (this == str) {
        return true;
    }
    return d_d(this, hasPrefix((const CHStringPrivate *)str->reserved()));
}

bool CHString::hasSuffix(CHString *str) const
{
    if (this == str) {
        return true;
    }
    return d_d(this, hasSuffix((const CHStringPrivate *)str->reserved()));
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
    return d_d(this, rangeOfString((const CHStringPrivate *)aString->reserved()));
}

CHString *CHString::stringByAppendingString(CHString *aString) const
{
    CHStringPrivate *d = d_d(this, duplicate());
    d->appendBytes(d_d(aString, buffer), d_d(aString, length), CHStringBufferTypeImmutable);
    CHString *str = new CHString();
    str->setReserved(d);
    return str;
}

CHString *CHString::stringByAppendingFormat(const char *format, ...) const
{
    va_list ap;
    va_start(ap, format);
    char *buffer = nullptr;
    uint32_t capacity = 0;
    uint32_t length = (uint32_t)tprintf(buffer, capacity,format, ap);
    va_end(ap);

    CHStringPrivate *d = d_d(this, duplicate());
    d->appendBytes(buffer, length, CHStringBufferTypeImmutable);
    CHString *str = new CHString();
    str->setReserved(d);
    free(buffer);
    return str;
}

CHString *CHString::stringByReplacingOccurrencesOfStringWithString(CHString *target, CHString *replacement) const
{
    CHStringPrivate *d = d_d(this, stringByReplacingOccurrencesOfStringWithString((const char *)d_d(target, buffer), d_d(target, length), (const char *)d_d(replacement, buffer), d_d(replacement, length)));
    CHString *string = new CHString();
    string->setReserved(d);
    return string;
}

CHString *CHString::stringByReplacingCharactersInRange(CHRange range, CHString *replacement) const
{
    CHString *string = new CHString();
    string->setReserved(d_d(this, stringByReplacingCharactersInRange(range, (const char *)d_d(replacement, buffer), replacement->length())));
    return string;
}

ARRAY_CONTAINS_TYPE(CHArray *, CHString *) CHString::componentsSeparatedByString(CHString *separaotr) const
{
    vector<CHStringPrivate *> components;
    d_d(this, componentsSeparatedByString((const char *)d_d(separaotr, buffer), separaotr->length(), components));
    // TODO: needs to implement CHMutableArray
    CHArray *array;
    return array;
}

double CHString::doubleValue() const
{
    return d_d(this, doubleValue());
}

float CHString::floatValue() const
{
    return d_d(this, doubleValue());
}

long long CHString::longLongValue() const
{
    return d_d(this, longLongValue());
}

int CHString::intValue() const
{
    return (int)d_d(this, longLongValue());
}

bool CHString::boolValue() const
{
    return !!*(const char *)d_d(this, buffer);
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
    memcpy(buffer, d_d(this, buffer), copyLength);
    return copyLength;
}

CHString *CHString::uppercaseString() const
{
    CHString *str = new CHString();
    str->setReserved(d_d(this, uppercaseString()));
    return str;
}

CHString *CHString::lowercaseString() const
{
    CHString *str = new CHString();
    str->setReserved(d_d(this, lowercaseString()));
    return str;
}

CHData *CHString::dataUsingEncoding() const
{
    CHData *data = CHData::dataWithBytes(d_d(this, buffer), length());
    return data;
}

/// creation
CHString *CHString::stringWithString(const CHString *other)
{
    CHString *string = new CHString();
    string->setReserved(d_d(other, duplicate()));
    return string;
}

CHString *CHString::stringWithBytes(const void *bytes, uint32_t length)
{
    CHString *str = new CHString();
    str->setReserved(new CHStringPrivate((void *)bytes, length, false));
    return str;
}

CHString *CHString::stringWithBytesNoCopy(void *bytes, uint32_t length)
{
    CHString *str = new CHString();
    str->setReserved(new CHStringPrivate((void *)bytes, length, true));
    return str;
}

CHString *CHString::stringWithUTF8String(const char *nullTerminatedCString)
{
    CHString *str = new CHString();
    str->setReserved(new CHStringPrivate(nullTerminatedCString));
    return str;
}

CHString *CHString::stringWithFormat(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char *buffer = nullptr;
    uint32_t capacity = 0;
    uint32_t length = (uint32_t)tprintf(buffer, capacity,format, ap);
    va_end(ap);
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
    CHString *str = CHString::stringWithFormat("<%s:%p>%@", object_getClassName((id)this), this, this);
    return str;
}

uint64_t CHString::hash() const
{
    return bkdr_hash((const char *)d_d(this, buffer), d_d(this, length));
}

// CHMutableString
struct runtimeclass(CHMutableString)
{
    static struct method_list_t *methods()
    {
        static method_list_t method[] = {
            {.method = {0, overloadFunc(Class(*)(std::nullptr_t),CHMutableString::getClass), selector(getClass), __Static} },
            {.method = {0, overloadFunc(Class(CHMutableString::*)()const, &CHMutableString::getClass), selector(getClass), __Member} },
            {.method = {0, funcAddr(&CHMutableString::allocateInstance), selector(allocateInstance), __Static} },

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
    6
};

Implement(CHMutableString);

CHMutableString::CHMutableString() :CHString(){}
