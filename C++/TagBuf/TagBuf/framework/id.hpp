//
//  id.hpp
//  TagBuf
//
//  Created by hejunqiu on 16/9/16.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef id_hpp
#define id_hpp

#include <stdio.h>
class CHTagBuf;
class id_t;

using id = id_t *;

class id_t
{
    struct idPrivate *d;
public:
    ~id_t();
    operator char() const;
    operator unsigned char() const;
    operator short() const;
    operator unsigned short() const;
    operator int() const;
    operator unsigned int() const;
    operator float() const;
    operator long() const;
    operator unsigned long() const;
    operator long long() const;
    operator unsigned long long() const;
    operator double() const;
    operator void*() const;
    operator CHTagBuf*() const;

    friend id objectWithValue(double v);
    friend id objectWithValue(long v);
    friend id objectWithValue(unsigned long v);
    friend id objectWithValue(long long v);
    friend id objectWithValue(unsigned long long v);
private:
    id_t();
};

id objectWithValue(char value);
id objectWithValue(unsigned char v);
id objectWithValue(short v);
id objectWithValue(unsigned short v);
id objectWithValue(int v);
id objectWithValue(unsigned int v);
id objectWithValue(float v);

void release_id(id obj);
#endif /* id_hpp */
