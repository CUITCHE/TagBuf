//
//  CHString.hpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/31.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef CHString_hpp
#define CHString_hpp

#include "id.hpp"
#include "CHRange.hpp"
#include <stdarg.h>

class CHArray;
class CHData;

CLASS_TAGGEDPOINTER_AVAILABLE class CHString : public CHObject,
                                               protocolTo CHCopying,
                                               protocolTo CHMutableCopying
{
    __SUPPORTRUNTIME__(CHString);
protected:
    CHString();
    CHString(const CHString&) = delete;
    CHString& operator=(const CHString &) = delete;
public:
    ~CHString() override;
    uint32_t length() const;
    uint32_t capacity() const;

    // CHStringExtensionMethods
    CHString *substringFromIndex(uint32_t index) const;
    CHString *substringToIndex(uint32_t index) const;
    CHString *substring(CHRange range) const;

    bool compare(CHString *aString) const;
    bool caseInsensitiveCompare(CHString *aString) const;

    bool isEqualToString(CHString *aString) const;

    bool hasPrefix(CHString *str) const;
    bool hasSuffix(CHString *str) const;

    bool containsString(CHString *aString) const;
    CHRange rangeOfString(CHString *aString) const;

    CHString *stringByAppendingString(CHString *aString) const;
    CHString *stringByAppendingFormat(const char *format, ...) const __printflike(2, 3);
    CHString *stringByReplacingOccurrencesOfStringWithString(CHString *target, CHString *replacement) const;
    CHString *stringByReplacingCharactersInRange(CHRange range, CHString *replacement) const;
    ARRAY_CONTAINS_TYPE(CHArray *, CHString *) componentsSeparatedByString(CHString *separaotr) const;

    double doubleValue() const;
    float floatValue() const;
    int intValue() const;
    long long longLongValue() const;
    bool boolValue() const;

    operator double() const;
    operator float() const;
    operator int() const;
    operator long long() const;
    operator bool() const;

    CHString *uppercaseString() const;
    CHString *lowercaseString() const;

    CHData *dataUsingEncoding() const;
    uint32_t getBytes(void *buffer, uint32_t bufferLength) const;

    // creation
    static CHString* stringWithString(const CHString *other);
    static CHString* stringWithBytes(const void *bytes, uint32_t length);
    static CHString* stringWithBytesNoCopy(void *bytes, uint32_t length);
    static CHString* stringWithUTF8String(const char *nullTerminatedCString);
    static CHString* stringWithFormat(const char *format, ...) __printflike(1, 2);
    static CHString* stringWithFormat(const char *format, va_list argList);

    // runtime
    Class getClass() const override;
    static Class getClass(std::nullptr_t);

    // protocol
    bool equalTo(id anObject) const override;
    CHString *description() const;
    uint64_t hash() const override;
private:
    static id allocateInstance();
protected:
    id copyWithZone(std::nullptr_t) const;
    id mutableCopyWithZone(std::nullptr_t) const;
};

class CHMutableString : public CHString
{
    __SUPPORTRUNTIME__(CHMutableString);
protected:
    CHMutableString();
public:
    void insertString(CHString *aString, uint32_t index);
    void deleteCharactersInRange(CHRange range) throw();
    void appendString(const CHString *other);
    void appendFormat(const char *format, ...) __printflike(2,3);
    void setString(CHString *aString);

    void replaceOccurrencesOfStringWithString(CHString *target, CHString *replacement);
    void replaceCharactersInRangeWithString(CHRange range, CHString *aString);

    // craetion
    static CHMutableString* stringWithCapacity(uint32_t capacity);

    // runtime
    Class getClass() const override;
    static Class getClass(std::nullptr_t);
private:
    static id allocateInstance();
protected:
    id copyWithZone(std::nullptr_t) const;
    id mutableCopyWithZone(std::nullptr_t) const;
};

#define tstr(s) CHString::stringWithUTF8String(s)
#endif /* CHString_hpp */
