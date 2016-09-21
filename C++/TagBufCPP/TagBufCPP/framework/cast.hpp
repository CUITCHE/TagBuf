//
//  cast.hpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/9/16.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef cast_hpp
#define cast_hpp

#include <typeinfo>
#include <type_traits>
#include "TaggedPointer.h"

template<typename T> struct __encode__
{ static const char *type(){ return typeid(T).name(); } };

#ifndef __ENCODE_CONST__
#define __ENCODE_CONST__(type, T) template<T> struct __encode__<const type> : __encode__<type> {}
#endif

#ifndef __ENCODE_VOLATILE__
#define __ENCODE_VOLATILE__(type, T) template<T> struct __encode__<volatile type> : __encode__<type> {}
#endif

#ifndef __ENCODE_CV__
#define __ENCODE_CV__(type, T) __ENCODE_CONST__(type, T); __ENCODE_VOLATILE__(type, T)
#endif

#ifndef __ENCODE_TYPE_POINTER
#define __ENCODE_TYPE_POINTER(_type, t, T) template <T> struct __encode__<_type*> { static const char *type() { return #t; } }; template <T> struct __encode__<const _type*> { static const char *type() { return #t; } }
#endif

template <> struct __encode__<char>
{ static const char *type() { return "c"; } }; __ENCODE_CV__(char,);
__ENCODE_TYPE_POINTER(char, ^c,);

template <> struct __encode__<unsigned char>
{ static const char *type() { return "C"; } }; __ENCODE_CV__(unsigned char,);
__ENCODE_TYPE_POINTER(unsigned char, ^C,);

template <> struct __encode__<short>
{ static const char *type() { return "s"; } }; __ENCODE_CV__(short,);
__ENCODE_TYPE_POINTER(short, ^s,);

template <> struct __encode__<unsigned short>
{ static const char *type() { return "S"; } }; __ENCODE_CV__(unsigned short,);
__ENCODE_TYPE_POINTER(unsigned short, ^S,);

template <> struct __encode__<int>
{ static const char *type() { return "i"; } }; __ENCODE_CV__(int,);
__ENCODE_TYPE_POINTER(int, ^i,);


template <> struct __encode__<unsigned int>
{ static const char *type() { return "I"; } }; __ENCODE_CV__(unsigned int,);
__ENCODE_TYPE_POINTER(unsigned int, ^I,);

template <> struct __encode__<float>
{ static const char *type() { return "f"; } }; __ENCODE_CV__(float,);
__ENCODE_TYPE_POINTER(float, ^f,);

template <> struct __encode__<double>
{ static const char *type() { return "d"; } }; __ENCODE_CV__(double,);
__ENCODE_TYPE_POINTER(double, ^d,);

template <> struct __encode__<long>
{ static const char *type() { return "l"; } }; __ENCODE_CV__(long,);
__ENCODE_TYPE_POINTER(long, ^l,);

template <> struct __encode__<unsigned long>
{ static const char *type() { return "L"; } }; __ENCODE_CV__(unsigned long,);
__ENCODE_TYPE_POINTER(unsigned long, ^L,);

template <> struct __encode__<long long>
{ static const char *type() { return "q"; } }; __ENCODE_CV__(long long,);
__ENCODE_TYPE_POINTER(long long, ^q,);

template <> struct __encode__<unsigned long long>
{ static const char *type() { return "Q"; } }; __ENCODE_CV__(unsigned long long,);
__ENCODE_TYPE_POINTER(unsigned long long, ^Q,);

template <> struct __encode__<bool>
{ static const char *type() { return "B"; } }; __ENCODE_CV__(bool,);
__ENCODE_TYPE_POINTER(bool, ^B,);

template <> struct __encode__<void>
{ static const char *type() { return "v"; } }; __ENCODE_CV__(void,);
__ENCODE_TYPE_POINTER(void, ^v,);

class CHData;
class CHNumber;
class CHString;
class CHMutableString;
class CHTagBuf;
class CHMutableData;

template <> struct __encode__<CHData>
{ static const char *type() { return "#CHData"; } }; __ENCODE_CV__(CHData,);
__ENCODE_TYPE_POINTER(CHData, ^#CHData,);

template <> struct __encode__<CHMutableData>
{ static const char *type() { return "#CHMutableData"; } }; __ENCODE_CV__(CHMutableData,);
__ENCODE_TYPE_POINTER(CHMutableData, ^#CHMutableData,);

template <> struct __encode__<CHString>
{ static const char *type() { return "#CHString"; } }; __ENCODE_CV__(CHString,);
__ENCODE_TYPE_POINTER(CHString, ^#CHString,);

template <> struct __encode__<CHMutableString>
{ static const char *type() { return "#CHMutableString"; } }; __ENCODE_CV__(CHMutableString,);
__ENCODE_TYPE_POINTER(CHMutableString, ^#CHMutableString,);

template <> struct __encode__<CHNumber>
{ static const char *type() { return "#CHNumber"; } }; __ENCODE_CV__(CHNumber,);
__ENCODE_TYPE_POINTER(CHNumber, ^#CHNumber,);

#include <vector>

template <typename T> struct __encode__<std::vector<T>>
{ static const char *type() { return "#vector<T>"; } }; __ENCODE_CV__(std::vector<T>, typename T);
__ENCODE_TYPE_POINTER(std::vector<T>, ^#vector<T>, typename T);


template <typename T>
const char *encode()
{
    if (std::is_base_of<CHTagBuf, std::remove_pointer_t<T>>::value) {
        return std::is_pointer<T>::value ? "^#CHTagBuf" : "#CHTagBuf";
    } else {
        return __encode__<T>::type();
    }
}

template<typename T>
const char *encode(__unused T unused)
{
    return encode<std::remove_cv_t<T>>();
}

template <>
inline const char *encode<const char *>(const char *s)
{
    if (reinterpret_cast<uintptr_t>(&s[0]) <= MAX_CONSTANT_ADDRESS) {
        return "*";
    }
    return "^c";
}
#endif /* cast_hpp */
