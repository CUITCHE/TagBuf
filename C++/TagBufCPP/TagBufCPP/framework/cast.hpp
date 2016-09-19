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

template <> struct __encode__<char>
{ static const char *type() { return "c"; } }; __ENCODE_CV__(char,);

template <> struct __encode__<unsigned char>
{ static const char *type() { return "C"; } }; __ENCODE_CV__(unsigned char,);

template <> struct __encode__<short>
{ static const char *type() { return "s"; } }; __ENCODE_CV__(short,);

template <> struct __encode__<unsigned short>
{ static const char *type() { return "S"; } }; __ENCODE_CV__(unsigned short,);

template <> struct __encode__<int>
{ static const char *type() { return "i"; } }; __ENCODE_CV__(int,);

template <> struct __encode__<unsigned int>
{ static const char *type() { return "I"; } }; __ENCODE_CV__(unsigned int,);

template <> struct __encode__<float>
{ static const char *type() { return "f"; } }; __ENCODE_CV__(float,);

template <> struct __encode__<double>
{ static const char *type() { return "d"; } }; __ENCODE_CV__(double,);

template <> struct __encode__<long>
{ static const char *type() { return "l"; } }; __ENCODE_CV__(long,);

template <> struct __encode__<unsigned long>
{ static const char *type() { return "L"; } }; __ENCODE_CV__(unsigned long,);

template <> struct __encode__<long long>
{ static const char *type() { return "q"; } }; __ENCODE_CV__(long long,);

template <> struct __encode__<unsigned long long>
{ static const char *type() { return "Q"; } }; __ENCODE_CV__(unsigned long long,);

template <> struct __encode__<bool>
{ static const char *type() { return "B"; } }; __ENCODE_CV__(bool,);

template <> struct __encode__<void>
{ static const char *type() { return "v"; } }; __ENCODE_CV__(void,);

template <> struct __encode__<char *>
{ static const char *type() { return "*"; } }; __ENCODE_CV__(char *,);

#include "CHData.hpp"
#include "CHString.hpp"
#include "CHNumber.hpp"

template <> struct __encode__<CHData>
{ static const char *type() { return "#\"CHData\""; } }; __ENCODE_CV__(CHData,);

template <> struct __encode__<CHString>
{ static const char *type() { return "#\"CHString\""; } }; __ENCODE_CV__(CHString,);

template <> struct __encode__<CHNumber>
{ static const char *type() { return "#\"CHNumber\""; } }; __ENCODE_CV__(CHNumber,);

#include <vector>

template <typename T> struct __encode__<std::vector<T>>
{ static const char *type() { return "#\"vector<T>\""; } }; __ENCODE_CV__(std::vector<T>, typename T);


template <typename T>
const char *encode()
{
    return std::is_base_of<CHTagBuf, typename std::remove_pointer<typename std::remove_cv<T>::type>::type>::value ? "#\"CHTagBuf\"" :  __encode__<T>::type();
}

template<typename T>
const char *encode(__unused T unused)
{
//    if (std::is_pointer<T>::value) {
//        typedef typename std::remove_pointer<typename std::remove_cv<T>::type>::type _T;
//        if (std::is_base_of<CHTagBuf, _T>::value) {
//            return runtimeClassName(unused);
//        }
//    }
    return std::is_base_of<CHTagBuf, typename std::remove_pointer<typename std::remove_cv<T>::type>::type>::value ? "#\"CHTagBuf\"" :  __encode__<T>::type();
}
#endif /* cast_hpp */
