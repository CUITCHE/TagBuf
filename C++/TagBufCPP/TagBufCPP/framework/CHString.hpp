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

CLASS_TAGGEDPOINTER_AVAILABLE class CHString : protected CHMutableData
{
    __SUPPORTRUNTIME__(CHString);
protected:
    CHString();
    CHString(const CHString&) = delete;
    CHString& operator=(const CHString &) = delete;
public:
    uint32_t length() const;
    uint32_t capacity() const;
//    void print() const;

    static CHString* stringWithCString(const char *str);
    static CHString* stringWithString(const CHString *other);
    static CHString* stringWithBytes(const void *bytes, uint32_t length);

    // runtime
    Class getClass() const override;
    static Class getClass(std::nullptr_t);
private:
    static id allocateInstance();
};

class CHMutableString : public CHString
{
    __SUPPORTRUNTIME__(CHMutableString);
protected:
    CHMutableString();
public:
    CHMutableString& appendString(const CHString *other);
    CHMutableString& appendString(const char *str);
    CHMutableString& appendString(const void *bytes, uint32_t length);

//    void clear();

    // runtime
    Class getClass() const override;
    static Class getClass(std::nullptr_t);
private:
    static id allocateInstance();
};

#endif /* CHString_hpp */
