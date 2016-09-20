//
//  CHString.hpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/31.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef CHString_hpp
#define CHString_hpp

#include "CHData.hpp"

CLASS_TAGGEDPOINTER_AVAILABLE class CHString : protected CHData
{
    CHString();
    CHString(const CHString&) = delete;
    CHString& operator=(const CHString &) = delete;
public:
    uint32_t length() const;
    uint32_t capacity() const;
    void print() const;

    static CHString* stringWithCString(const char *str);
    static CHString* stringWithString(const CHString *other);
    static CHString* stringWithBytes(const void *bytes, uint32_t length);

    // runtime
    Class getClass() const override;
    static Class getClass(std::nullptr_t);
};

class CHMutableString : public CHString
{
public:
    CHString& appendString(const CHString *other);
    CHString& appendString(const char *str);
    CHString& appendString(const void *bytes, uint32_t length);

    void clear();

    // runtime
    Class getClass() const override;
    static Class getClass(std::nullptr_t);
};

#endif /* CHString_hpp */
