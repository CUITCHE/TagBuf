//
//  CHNumber.hpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/9/16.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef CHNumber_hpp
#define CHNumber_hpp

#include "id.hpp"

class CHNumber : public CHObject
{
    friend struct CHNumberHelper;
public:
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
protected:
    CHNumber();
    ~CHNumber() override;
};

CHNumber *objectWithValue(char v);
CHNumber *objectWithValue(unsigned char v);
CHNumber *objectWithValue(short v);
CHNumber *objectWithValue(unsigned short v);
CHNumber *objectWithValue(int v);
CHNumber *objectWithValue(unsigned int v);
CHNumber *objectWithValue(float v);
CHNumber *objectWithValue(double v);
CHNumber *objectWithValue(long v);
CHNumber *objectWithValue(unsigned long v);
CHNumber *objectWithValue(long long v);
CHNumber *objectWithValue(unsigned long long v);

#endif /* CHNumber_hpp */
