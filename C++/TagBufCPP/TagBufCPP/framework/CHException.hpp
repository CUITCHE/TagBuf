//
//  CHException.hpp
//  TagBufCPP
//
//  Created by hejunqiu on 2016/9/26.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef _exception_hpp
#define _exception_hpp

#include <stdarg.h>

#include "CHObject.hpp"

class CHString;
class CHArray;
class CHException : public CHObject
{
    __SUPPORTRUNTIME__(CHException);
    CHException(CHString *exceptionName, CHString *reason, id userInfo);
    CHException();
public:
    CHString *exceptionName() const;
    CHString *reason() const;
    id userInfo() const;

    ARRAY_CONTAINS_TYPE(CHArray *, CHNumber *) callStackReturnAddresses() const;
    ARRAY_CONTAINS_TYPE(CHArray *, CHString *) callStackSymbols() const;

    // creation
    static CHException *exceptionWithExceptionName(CHString *exceptionName, CHString *reason, id userInfo);
    static CHException *exceptionWithExceptionName(CHString *exceptionName, const char *fmt, ...) __printflike(2, 3);
    static void raise(CHString *exceptionName, const char *format, ...) throw() __printflike(2, 3);
    static void raise(CHString *exceptionName, const char *format, va_list argList) throw();

    // runtime
    Class getClass() const ;
    static Class getClass(std::nullptr_t);

    // protocol
    bool equalTo(id anObject) const override;
    CHString *description() const;
private:
    static id allocateInstance();
};

extern CHString *const CHRangeException;
extern CHString *const CHInvalidArgumentException;
extern CHString *const CHOutOfRangeException;

#define throwException(name, format, ...) throw CHException::exceptionWithExceptionName(name, format, ##__VA_ARGS__)
#endif /* _exception_hpp */
