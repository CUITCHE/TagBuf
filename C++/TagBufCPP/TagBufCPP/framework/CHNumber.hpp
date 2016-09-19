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

CHNumber *numberWithValue(char v);
CHNumber *numberWithValue(unsigned char v);
CHNumber *numberWithValue(short v);
CHNumber *numberWithValue(unsigned short v);
CHNumber *numberWithValue(int v);
CHNumber *numberWithValue(unsigned int v);
CHNumber *numberWithValue(float v);
CHNumber *numberWithValue(double v);
CHNumber *numberWithValue(long v);
CHNumber *numberWithValue(unsigned long v);
CHNumber *numberWithValue(long long v);
CHNumber *numberWithValue(unsigned long long v);

#define number(v) numberWithValue(v)
#endif /* CHNumber_hpp */
