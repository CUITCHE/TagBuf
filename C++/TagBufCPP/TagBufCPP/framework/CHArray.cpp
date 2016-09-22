//
//  CHArray.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 2016/9/22.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "CHArray.hpp"
#include "TagBufDefines.h"
#include "tagBuf.hpp"
#include "runtime.hpp"
#include "CHString.hpp"
#include "TaggedPointer.h"
#include <stdarg.h>
#include <algorithm>

struct runtimeclass(CHArray)
{
    static struct method_list_t *methods()
    {
        static method_list_t method[] = {
            {.method = {0, overloadFunc(Class(*)(std::nullptr_t),CHArray::getClass), selector(getClass), __Static} },
            {.method = {0, overloadFunc(Class(CHArray::*)()const, &CHArray::getClass), selector(getClass), __Member} },
            {.method = {0, funcAddr(&CHArray::allocateInstance), selector(allocateInstance), __Static} },

            {.method = {0, overloadFunc(const id(CHArray::*)(uint32_t)const, &CHArray::objectAtIndex), selector(objectAtIndex), __Member} },
            {.method = {0, overloadFunc(id(CHArray::*)(uint32_t), &CHArray::objectAtIndex), selector(objectAtIndex), __Member} },
            {.method = {0, overloadFunc(CHArray*(CHArray::*)()const, &CHArray::duplicate), selector(duplicate), __Member} },

            {.method = {0, funcAddr(&CHArray::arrayWithObject), selector(arrayWithObject), __Static} },
            {.method = {0, overloadFunc(CHArray *(*)(const id[], uint32_t), &CHArray::arrayWithObjects), selector(arrayWithObjects), __Static} },
            {.method = {0, overloadFunc(CHArray *(*)(id, ...), &CHArray::arrayWithObjects), selector(arrayWithObjects), __Static} },
            {.method = {0, funcAddr(&CHArray::arrayWithArray), selector(arrayWithArray), __Static} },

            {.method = {0, funcAddr(&CHArray::arrayByAddiObject), selector(arrayByAddiObject), __Member} },
            {.method = {0, funcAddr(&CHArray::arrayByAddingObjectsFromArray), selector(arrayByAddingObjectsFromArray), __Member} },
            {.method = {0, funcAddr(&CHArray::componentsJoinedByString), selector(componentsJoinedByString), __Member} },
            {.method = {0, funcAddr(&CHArray::containsObject), selector(containsObject), __Member} },
            {.method = {0, funcAddr(&CHArray::indexOfObject), selector(indexOfObject), __Member} },
            {.method = {0, funcAddr(&CHArray::firstObject), selector(firstObject), __Member} },
            {.method = {0, funcAddr(&CHArray::lastObject), selector(lastObject), __Member} },
            {.method = {0, funcAddr(&CHArray::enumerateObjectsUsingBlock), selector(enumerateObjectsUsingBlock), __Member} },
            {.method = {0, funcAddr(&CHArray::sortedArrayUsingComparator), selector(sortedArrayUsingComparator), __Member} },
        };
        return method;
    }
};

static class_t ClassNamed(CHArray) = {
    CHObject::getClass(nullptr),
    selector(CHArray),
    runtimeclass(CHArray)::methods(),
    nullptr,
    allocateCache(),
    selector(^#CHArray),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHArray)), sizeof(CHArray))),
    0,
    18
};

Implement(CHArray);

struct CHArrayPrivate
{
    id *_begin = 0;
    uint32_t size = 0;
    uint32_t capacity;

    CHArrayPrivate(uint32_t capacity) : capacity(capacity)
    {
        resize(capacity);
    }

    void insertObjectAtIndex(id obj, uint32_t index)
    {
        checkMemoryWithInsertCount(count() + 1);
        _begin[size++] = obj;
    }

    void resize(uint32_t newSize)
    {
        if (newSize < size) {
            id *_end = end();
            id *begin = _begin + newSize - 1;
            while (++begin < _end) {
                release(*begin);
            }
            size = newSize;
        } else if (newSize > size) {
            if (size > capacity) {
                id *_new = (id *)malloc(sizeof(id) * newSize);
                memcpy(_new, _begin, size);
                free(_begin);
                _begin = _new;
            }
        }
    }

    id *end() const { return _begin + size; }
    uint32_t count() const { return size; }

    bool containsObject(id anObject) const
    {
        // TODO: id需要实现EqualTo函数
        id *_end = end();
        id *begin = _begin - 1;
        while (++begin < _end) {
            if (anObject == *begin) {
                return true;
            }
        }
        return false;
    }

    uint32_t indexOfObject(id anObject) const
    {
        // TODO: id需要实现EqualTo函数
        id *_end = end();
        id *begin = _begin - 1;
        while (++begin < _end) {
            if (anObject == *begin) {
                return (uint32_t)(begin - _begin);
            }
        }
        return -1;
    }

    id firstObject() const
    {
        return *_begin;
    }

    id lastObject() const
    {
        return *(_begin + size - 1);
    }

    void enumerateObjectsUsingBlock(CHArrayObjectCallback block) const
    {
        id *_end = end();
        id *begin = _begin - 1;
        bool stop = false;
        while (++begin < _end) {
            block(*begin, (uint32_t)(begin - _begin), &stop);
            if (stop) {
                break;
            }
        }
    }

    void sortedArrayUsingComparator(CHArraySortedComparator cmptr)
    {
        std::sort(_begin, end(), cmptr);
    }

private:
    void checkMemoryWithInsertCount(uint32_t insertCount)
    {
        if (size + insertCount > capacity) {
            resize(size + insertCount * 2);
        }
    }
};

#define d_d(obj,field) ((CHArrayPrivate *)obj->reserved())->field

CHArray::CHArray() :CHObject()
{
    ;
}

CHArray::CHArray(uint32_t capacity) :CHObject()
{
    this->setReserved(new CHArrayPrivate(capacity));
}

CHArray::~CHArray()
{
    if (reserved()) {
        delete (CHArrayPrivate *)reserved();
    }
}

uint32_t CHArray::count() const
{
    return d_d(this, count)();
}

id CHArray::objectAtIndex(uint32_t index)
{
    return d_d(this, _begin)[index];
}

const id CHArray::objectAtIndex(uint32_t index) const
{
    return d_d(this, _begin)[index];
}

CHArray *CHArray::duplicate() const
{
    uint32_t size = count();
    CHArray *array = new CHArray(size);
    memcpy(d_d(array, _begin), d_d(this, _begin), sizeof(id) * size);
    d_d(array, size) = size;
    return array;
}

// creation
CHArray *CHArray::arrayWithObject(id obj)
{
    CHArray *array = new CHArray(1);
    d_d(array, insertObjectAtIndex)(obj , 0);
    return array;
}

CHArray *CHArray::arrayWithObjects(const id *objects, uint32_t count)
{
    CHArray *array = new CHArray(count);
    memcpy(d_d(array, _begin), objects,  sizeof(id) * count);
    d_d(array, size) = count;
    return array;
}

CHArray *CHArray::arrayWithObjects(id object, ...)
{
    CHArray *array = new CHArray(16);
    uint32_t index = -1;
    va_list ap;
    va_start(ap, object);
    d_d(array, insertObjectAtIndex(object, ++index));
    id obj = va_arg(ap, id);
    while (obj) {
        d_d(array, insertObjectAtIndex(obj, ++index));
        obj = va_arg(ap, id);
    }
    va_end(ap);
    return array;
}

CHArray *CHArray::arrayWithArray(const CHArray *other)
{
    uint32_t size = d_d(other, size);
    CHArray *array = new CHArray(size);
    memcpy(d_d(array, _begin), d_d(other, _begin), sizeof(id) * size);
    d_d(array, size) = size;
    return array;
}

CHArray *CHArray::arrayByAddiObject(id object) const
{
    CHArray *array = this->duplicate();
    array->indexOfObject(object);
    return array;
}

CHArray *CHArray::arrayByAddingObjectsFromArray(const CHArray *otherArray) const
{
    CHArray *array = this->duplicate();
    uint32_t oldSize = count();
    uint32_t newSize = count() + otherArray->count();
    d_d(array, resize)(newSize);
    memcpy(d_d(array, _begin) + oldSize, d_d(otherArray, _begin), sizeof(id) * (newSize - oldSize));
    d_d(array, size) = newSize;
    return array;
}

CHString *CHArray::componentsJoinedByString(const CHString *separator) const
{
    // TODO: need to be done. need to change CHString'API.
    return nullptr;
}

bool CHArray::containsObject(id anObject) const
{
    return d_d(this, containsObject(anObject));
}

uint32_t CHArray::indexOfObject(id anObject) const
{
    return d_d(this, indexOfObject(anObject));
}

id CHArray::firstObject() const
{
    return d_d(this, firstObject());
}

id CHArray::lastObject() const
{
    return d_d(this, lastObject());
}

void CHArray::enumerateObjectsUsingBlock(CHArrayObjectCallback block) const
{
    d_d(this, enumerateObjectsUsingBlock(block));
}

void CHArray::sortedArrayUsingComparator(CHArraySortedComparator cmptr)
{
    d_d(this, sortedArrayUsingComparator(cmptr));
}
