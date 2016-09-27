//
//  CHExceptioncpp
//  TagBufCPP
//
//  Created by hejunqiu on 2016/9/26.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "CHException.hpp"
#include "runtime.hpp"
#include "tagBuf.hpp"
#include "CHString.hpp"
#include <execinfo.h>
#include "CHLog.hpp"
#include "CHArray.hpp"
#include "CHNumber.hpp"
#include <exception>

using namespace std;

CHString *const CHRangeException = tstr("CHRangeException");
CHString *const CHInvalidArgumentException = tstr("CHInvalidArgumentException");
CHString *const CHOutOfRangeException = tstr("CHOutOfRangeException");

static inline void exceptionHandler()
{
    exception_ptr ptr = current_exception();
    try {
        rethrow_exception(ptr);
    } catch (CHException *e) {
        CHLog("%p@", e);
        e->callStackSymbols();
        rethrow_exception(ptr);
    } catch (exception *e) {
        CHLog("%s", e->what());
        rethrow_exception(ptr);
    } catch (void *p) {
        printf("%p", p);
        rethrow_exception(ptr);
    }
}

__attribute__((constructor)) void load_exception_handler()
{
    set_unexpected(exceptionHandler);
}

struct runtimeclass(CHException)
{
    static struct method_list_t *methods()
    {
        static method_list_t method[14] = {
            // runtime
            {.method = {0, overloadFunc(Class(*)(std::nullptr_t),CHException::getClass), selector(getClass), __Static} },
            {.method = {0, overloadFunc(Class(CHException::*)()const, &CHException::getClass), selector(getClass), __Member} },
            {.method = {0, funcAddr(&CHException::allocateInstance), selector(allocateInstance), __Static} },
            // protocol
            {.method = {0, funcAddr(&CHException::equalTo), selector(equalTo), __Member} },
            {.method = {0, funcAddr(&CHException::description), selector(description), __Member} },
            // memeber method
            {.method = {0, overloadFunc(CHException*(*)(CHString*, CHString*, id),&CHException::exceptionWithExceptionName), selector(exceptionWithExceptionName), __Static|__Overload} },
            {.method = {0, overloadFunc(CHException*(*)(CHString*, const char*, ...),&CHException::exceptionWithExceptionName), selector(exceptionWithExceptionName), __Static|__Overload} },
            {.method = {0, funcAddr(&CHException::exceptionName), selector(exceptionName), __Member} },
            {.method = {0, funcAddr(&CHException::reason), selector(reason), __Member} },
            {.method = {0, funcAddr(&CHException::userInfo), selector(userInfo), __Member} },
            {.method = {0, overloadFunc(void(*)(CHString*,const char*,...), &CHException::raise), selector(raise), __Static|__Overload} },
            {.method = {0, overloadFunc(void(*)(CHString*,const char*,va_list), &CHException::raise), selector(raise), __Static|__Overload} },
            {.method = {0, funcAddr(&CHException::callStackSymbols), selector(callStackSymbols), __Member} },
            {.method = {0, funcAddr(&CHException::callStackReturnAddresses), selector(callStackReturnAddresses), __Member} },
            
        };
        return method;
    }
};

static class_t ClassNamed(CHException) = {
    CHObject::getClass(nullptr),
    selector(CHString),
    runtimeclass(CHException)::methods(),
    nullptr,
    allocateCache(),
    selector(^#CHException),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHException)), sizeof(CHException))),
    0,
    14
};

Implement(CHException);

struct CHExceptionPrivate
{
    CHString *name;
    CHString *reason;
    id userInfo;
    id reserved;
    CHExceptionPrivate(CHString *exceptionName, CHString *reason, id userInfo)
    : name(exceptionName), reason(reason), userInfo(userInfo) {}
};

#define d_d(obj, field) ((CHExceptionPrivate *)obj->reserved())->field

CHException::CHException() : CHObject() {}

CHException::CHException(CHString *exceptionName, CHString *reason, id userInfo) : CHException()
{
    this->setReserved(new CHExceptionPrivate(exceptionName, reason, userInfo));
}

// creation
CHException *CHException::exceptionWithExceptionName(CHString *exceptionName, CHString *reason, id userInfo)
{
    CHException *exception = new CHException(exceptionName, reason, userInfo);
    return exception;
}

CHException *CHException::exceptionWithExceptionName(CHString *exceptionName, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    CHString *reason = CHString::stringWithFormat(fmt, ap);
    va_end(ap);
    CHException *e = CHException::exceptionWithExceptionName(exceptionName, reason, nullptr);
    return e;
}


void CHException::raise(CHString *exceptionName, const char *format, ...) throw()
{
    va_list ap;
    va_start(ap, format);
    raise(exceptionName, format, ap);
}

void CHException::raise(CHString *exceptionName, const char *format, va_list argList) throw()
{
    CHString *reason = CHString::stringWithFormat(format, argList);
    va_end(argList);
    CHException *e = CHException::exceptionWithExceptionName(exceptionName, reason, nullptr);
    throw e;
}

// protocol
bool CHException::equalTo(id anObject) const
{
    if (!anObject->isKindOfClass(getClass(nullptr))) {
        return false;
    }
    if (this == anObject) {
        return true;
    }
    CHException *e = (CHException *)anObject;
    return exceptionName()->isEqualToString(e->exceptionName()) &&
    reason()->isEqualToString(e->reason()) && userInfo() == e->userInfo();
}

CHString *CHException::description() const
{
    CHString *string = CHString::stringWithFormat("<%p@:%p@>:userInfo:%p@", exceptionName(), reason(), userInfo());
    return string;
}

// method
CHString *CHException::exceptionName() const
{
    return d_d(this, name);
}

CHString *CHException::reason() const
{
    return d_d(this, reason);
}

id CHException::userInfo() const
{
    return d_d(this, userInfo);
}

CHArray *CHException::callStackReturnAddresses() const
{
    void *d[10];
    int count = backtrace(d, 10);
    id objects[10] = {0};
    id *p = objects;
    for (int i=0; i<count; ++i) {
        *p++ = number((uintptr_t)(d[i]));
    }
    CHArray *array = CHArray::arrayWithObjects(objects, count);
    return array;
}

CHArray *CHException::callStackSymbols() const
{
    void *array[10];
    int size;
    char **strings;
    size = backtrace (array, 10);
    strings = backtrace_symbols (array, size);
    for (int i = 0; i < size; i++)
        printf ("%s\n", strings[i]);

    free (strings);
    return 0;
}
