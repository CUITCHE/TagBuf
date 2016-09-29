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
#include "CHString.hpp"

struct runtimeclass(CHArray)
{
    static struct method_list_t *methods()
    {
        static method_list_t method[24] = {
            {.method = {0, overloadFunc(Class(*)(std::nullptr_t),CHArray::getClass), selector(getClass), __Static} },
            {.method = {0, overloadFunc(Class(CHArray::*)()const, &CHArray::getClass), selector(getClass), __Member} },
            {.method = {0, funcAddr(&CHArray::allocateInstance), selector(allocateInstance), __Static} },

            {.method = {0, funcAddr(&CHArray::count), selector(count), __Member} },
            {.method = {0, funcAddr(&CHArray::objectAtIndex), selector(objectAtIndex), __Member} },

            {.method = {0, funcAddr(&CHArray::arrayWithObject), selector(arrayWithObject), __Static} },
            {.method = {0, overloadFunc(CHArray *(*)(const id[], uint32_t), &CHArray::arrayWithObjects), selector(arrayWithObjects), __Static|__Overload} },
            {.method = {0, overloadFunc(CHArray *(*)(id, ...), &CHArray::arrayWithObjects), selector(arrayWithObjects), __Static|__Overload} },
            {.method = {0, funcAddr(&CHArray::arrayWithArray), selector(arrayWithArray), __Static} },

            {.method = {0, funcAddr(&CHArray::arrayByAddiObject), selector(arrayByAddiObject), __Member} },
            {.method = {0, funcAddr(&CHArray::arrayByAddingObjectsFromArray), selector(arrayByAddingObjectsFromArray), __Member} },
            {.method = {0, funcAddr(&CHArray::componentsJoinedByString), selector(componentsJoinedByString), __Member} },

            {.method = {0, funcAddr(&CHArray::containsObject), selector(containsObject), __Member} },
            {.method = {0, funcAddr(&CHArray::indexOfObject), selector(indexOfObject), __Member} },
            {.method = {0, funcAddr(&CHArray::indexOfObjectInRange), selector(indexOfObjectInRange), __Member} },
            {.method = {0, funcAddr(&CHArray::isEqualToArray), selector(isEqualToArray), __Member} },

            {.method = {0, funcAddr(&CHArray::firstObject), selector(firstObject), __Member} },
            {.method = {0, funcAddr(&CHArray::lastObject), selector(lastObject), __Member} },

            {.method = {0, funcAddr(&CHArray::enumerateObjectsUsingBlock), selector(enumerateObjectsUsingBlock), __Member} },
            {.method = {0, funcAddr(&CHArray::sortedArrayUsingComparator), selector(sortedArrayUsingComparator), __Member} },

            {.method = {0, funcAddr(&CHArray::description), selector(description), __Member} },
            {.method = {0, funcAddr(&CHArray::equalTo), selector(equalTo), __Member} },
            {.method = {0, funcAddr(&CHArray::copyWithZone), selector(copyWithZone), __Member} },
            {.method = {0, funcAddr(&CHArray::mutableCopyWithZone), selector(mutableCopyWithZone), __Member} },
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
    24
};

Implement(CHArray);

#define STABLE_LENGTH_SET 8

struct CHArrayPrivate
{
    id *_begin = 0;
    uint32_t size = 0;
    uint32_t capacity = 0;

    CHArrayPrivate(uint32_t capacity)
    {
        resize(capacity);
    }

    ~CHArrayPrivate() { desctructor(); }

    CHArrayPrivate *duplicate() const
    {
        CHArrayPrivate *d = new CHArrayPrivate(size);
        memcpy(d->_begin, _begin, size);
        return d;
    }

    void insertObjectAtIndex(id obj, uint32_t index)
    {
        checkMemoryWithInsertCount(count() + 1);
        _begin[size++] = obj;
    }

    CHArrayPrivate *arrayByAddingObjectsFromArray(CHArrayPrivate *other)
    {
        CHArrayPrivate *d = duplicate();
        uint32_t oldSize = d->count();
        uint32_t newSize = oldSize + other->count();
        d->resize(newSize);
        memcpy(d->_begin + oldSize, other->_begin, sizeof(id) * (newSize - oldSize));
        return d;
    }

    id *end() const { return _begin + size; }
    uint32_t count() const { return size; }

    uint32_t indexOfObject(id anObject, CHRange inRange) const
    {
        do {
            if (CHMaxRange(inRange) > count()) {
                break;
            }
            id *_end = _begin + CHMaxRange(inRange);
            id *begin = _begin - 1 + inRange.location;
            while (++begin < _end) {
                if (anObject->equalTo(*begin)) {
                    return (uint32_t)(begin - _begin);
                }
            }
        } while (0);
        return CHNotFound;
    }

    bool isEqualToArray(CHArrayPrivate *otherArray) const
    {
        if (!otherArray) {
            return false;
        }
        if (count() != otherArray->count()) {
            return false;
        }
        id *end = _begin + count();
        id *begin = _begin;
        id *otherBegin = otherArray->_begin;
        while (begin < end) {
            if (!(*begin++)->equalTo(*otherBegin++)) {
                return false;
            }
        }
        return true;
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

    void removeObjectInRange(CHRange range)
    {
        uint32_t offset = CHMaxRange(range);
        if (offset > count()) {
            throwException(CHRangeException, "Argument range(%p@) is out of bounds(%u)", showRange(range), count());
        }
        for (uint32_t i=range.location; i<offset; ++i) {
            data()[i]->release();
        }
        if (offset != count()) {
            memcpy(data() + range.location, data() + offset, count() - offset);
        }
        size = count() - range.length;
    }

    void replaceObjectAtIndexWithObject(uint32_t index, id anObject)
    {
        if (index >= count()) {
            throwException(CHRangeException, "Argumnet index(%u) is out of bounds(%u-1)", index, count());
        }
        data()[index]->release();
        data()[index] = anObject;
    }

    void addObjectsFromArray(const CHArrayPrivate *other)
    {
        checkMemoryWithInsertCount(other->count());
        memcpy(data() + count(), other->data(), other->count());
        size += other->count();
    }

    void removeAllObjects()
    {
        int64_t i = count();
        id *obj = data();
        while (i --> 0) {
            (*obj++)->release();
        }
        size = 0;
    }

    void removeObjectInRange(id anObject, CHRange range)
    {
        uint32_t offset = CHMaxRange(range);
        if (offset > count()) {
            throwException(CHRangeException, "Argument range(%p@) is out of bounds(%u)", showRange(range), count());
        }
        CHRange removeRange{0, 1};
    continued:
        for (uint32_t i=range.location; i<offset; ++i) {
            if (data()[i]->equalTo(anObject)) {
                removeRange.location = i;
                removeObjectInRange(removeRange);
                offset -= i - range.location + 1;
                range.location = i;
                goto continued;
            }
        }
    }

    void removeObjectIdenticalToInRange(id anObject, CHRange range)
    {
        uint32_t offset = CHMaxRange(range);
        if (offset > count()) {
            throwException(CHRangeException, "Argument range(%p@) is out of bounds(%u)", showRange(range), count());
        }
        CHRange removeRange{0, 1};
    continued:
        for (uint32_t i=range.location; i<offset; ++i) {
            if (data()[i] == anObject) {
                removeRange.location = i;
                removeObjectInRange(removeRange);
                offset -= i - range.location + 1;
                range.location = i;
                goto continued;
            }
        }
    }

    void setArray(CHArrayPrivate *other)
    {
        removeAllObjects();
        size = 0;
        if (other == nullptr) {
            if (capacity > STABLE_LENGTH_SET) {
                _begin = (id *)realloc(_begin, sizeof(id *) * STABLE_LENGTH_SET);
                capacity = STABLE_LENGTH_SET;
            }
        } else {
            if (capacity < other->size) {
                resize(other->size);
            }
            memcpy(_begin, other->_begin, other->size);
        }
    }
private:
    void checkMemoryWithInsertCount(uint32_t insertCount)
    {
        if (size + insertCount > capacity) {
            resize(size + insertCount * 2);
        }
    }

    void desctructor()
    {
        removeAllObjects();
        free(_begin);
        _begin = NULL;
    }

    inline id *data() { return _begin; }
    inline const id *data() const { return _begin; }

    void resize(uint32_t newSize)
    {
        if (newSize < size) {
            id *_end = end();
            id *begin = _begin + newSize - 1;
            while (++begin < _end) {
                (*begin)->release();
            }
            size = newSize;
        } else if (newSize > size) {
            if (!capacity || newSize > capacity) {
                id *_new = (id *)malloc(sizeof(id) * newSize);
                memcpy(_new, _begin, size);
                free(_begin);
                _begin = _new;
                capacity = newSize;
            }
        }
    }
};

#define d_d(obj,field) ((CHArrayPrivate *)obj->reserved())->field

CHArray::CHArray() :CHObject() {}

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

 id CHArray::objectAtIndex(uint32_t index) const
{
    return d_d(this, _begin)[index];
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
    CHArrayPrivate *d = d_d(this, duplicate());
    CHArray *array = new CHArray();
    d->insertObjectAtIndex(object, d->count());
    array->setReserved(d);
    return array;
}

CHArray *CHArray::arrayByAddingObjectsFromArray(const CHArray *otherArray) const
{
    CHArrayPrivate *d = d_d(this, arrayByAddingObjectsFromArray((CHArrayPrivate *)otherArray->reserved()));

    CHArray *array = new CHArray();
    array->setReserved(d);
    return array;
}

CHString *CHArray::componentsJoinedByString(const CHString *separator) const
{
    if (count() == 0) {
        return tstr("");
    }

    id obj = firstObject();
    CHMutableString *string = CHMutableString::stringWithCapacity(128);
    CHString *str = obj->description();
    string->appendString(str);
    str->release();

    id *begin = d_d(this, _begin);
    id *end = d_d(this, end());
    while (++begin < end) {
        str = (*begin)->description();
        string->appendFormat("%p@%p@", separator, str);
        str->release();
    }
    return string;
}

bool CHArray::containsObject(id anObject) const
{
    return indexOfObjectInRange(anObject, CHMakeRange(0, this->count())) != CHNotFound;
}

uint32_t CHArray::indexOfObject(id anObject) const
{
    return indexOfObjectInRange(anObject, CHMakeRange(0, this->count()));
}

uint32_t CHArray::indexOfObjectInRange(id anObject, CHRange range) const
{
    return d_d(this, indexOfObject(anObject, range));
}

bool CHArray::isEqualToArray(CHArray *otherArray) const
{
    return d_d(this, isEqualToArray((CHArrayPrivate *)otherArray->reserved()));
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

CHArray *CHArray::sortedArrayUsingComparator(CHArraySortedComparator cmptr) const
{
    CHArray *array = new CHArray();
    CHArrayPrivate *d = d_d(this, duplicate());
    d->sortedArrayUsingComparator(cmptr);
    array->setReserved(d);
    return array;
}

CHString *CHArray::description() const
{
    if (count() == 0) {
        return tstr("");
    }

    CHMutableString *string = CHMutableString::stringWithCapacity(128);
    string->appendString(tstr("("));

    id obj = firstObject();
    CHString *str = obj->description();
    string->appendString(str);
    str->release();

    id *begin = d_d(this, _begin);
    id *end = d_d(this, end());
    while (++begin < end) {
        str = (*begin)->description();
        string->appendFormat(",%p@", str);
        str->release();
    }
    string->appendString(tstr(")"));
    return string;
}

bool CHArray::equalTo(id anObject) const
{
    if (this == anObject) {
        return true;
    }
    if (!anObject->isKindOfClass(CHArray::getClass(nullptr))) {
        return false;
    }
    return this->isEqualToArray((CHArray *)anObject);
}

id CHArray::copyWithZone(std::nullptr_t) const
{
    CHArrayPrivate *d = d_d(this, duplicate());
    CHArray *array = new CHArray();
    array->setReserved(d);
    return array;
}

id CHArray::mutableCopyWithZone(std::nullptr_t) const
{
    return copyWithZone(nullptr);
}

// CHMutableArray
struct runtimeclass(CHMutableArray)
{
    static struct method_list_t *methods()
    {
        static method_list_t method[27] = {
            {.method = {0, overloadFunc(Class(*)(std::nullptr_t),CHMutableArray::getClass), selector(getClass), __Static} },
            {.method = {0, overloadFunc(Class(CHMutableArray::*)()const, &CHMutableArray::getClass), selector(getClass), __Member} },
            {.method = {0, funcAddr(&CHMutableArray::allocateInstance), selector(allocateInstance), __Static} },

            {.method = {0, funcAddr(&CHMutableArray::addObject), selector(addObject), __Member} },
            {.method = {0, funcAddr(&CHMutableArray::insertObjectAtIndex), selector(insertObjectAtIndex), __Member} },
            {.method = {0, funcAddr(&CHMutableArray::removeLastObject), selector(removeLastObjectremoveLastObject), __Member} },
            {.method = {0, funcAddr(&CHMutableArray::removeObjectAtIndex), selector(removeObjectAtIndex), __Member} },
            {.method = {0, funcAddr(&CHMutableArray::replaceObjectAtIndexWithObject), selector(replaceObjectAtIndexWithObject), __Member} },
            {.method = {0, funcAddr(&CHMutableArray::addObjectsFromArray), selector(addObjectsFromArray), __Member} },
            {.method = {0, funcAddr(&CHMutableArray::exchangeObjectAtIndexWithObjectAtIndex), selector(exchangeObjectAtIndexWithObjectAtIndex), __Member} },
            {.method = {0, funcAddr(&CHMutableArray::removeAllObjects), selector(removeAllObjects), __Member} },
            {.method = {0, funcAddr(&CHMutableArray::removeObjectInRange), selector(removeObjectInRange), __Member} },
            {.method = {0, funcAddr(&CHMutableArray::removeObject), selector(removeObject), __Member} },
            {.method = {0, funcAddr(&CHMutableArray::removeObjectIdenticalToInRange), selector(removeObjectIdenticalToInRange), __Member} },
            {.method = {0, funcAddr(&CHMutableArray::removeObjectIdenticalTo), selector(removeObjectIdenticalTo), __Member} },
            {.method = {0, funcAddr(&CHMutableArray::removeObjectsInArray), selector(removeObjectsInArray), __Static} },
            {.method = {0, funcAddr(&CHMutableArray::removeObjectsInRange), selector(removeObjectsInRange), __Static} },
            {.method = {0, funcAddr(&CHMutableArray::setArray), selector(setArray), __Static} },
            {.method = {0, funcAddr(&CHMutableArray::sortUsingComparator), selector(sortUsingComparator), __Static} },

            {.method = {0, funcAddr(&CHMutableArray::arrayWithObject), selector(arrayWithObject), __Static} },
            {.method = {0, overloadFunc(CHMutableArray *(*)(const id[], uint32_t), &CHMutableArray::arrayWithObjects), selector(arrayWithObjects), __Static|__Overload} },
            {.method = {0, overloadFunc(CHMutableArray *(*)(id, ...), &CHMutableArray::arrayWithObjects), selector(arrayWithObjects), __Static|__Overload} },
            {.method = {0, funcAddr(&CHMutableArray::arrayWithArray), selector(arrayWithArray), __Static} },

            {.method = {0, funcAddr(&CHMutableArray::arrayByAddiObject), selector(arrayByAddiObject), __Member} },
            {.method = {0, funcAddr(&CHMutableArray::arrayByAddingObjectsFromArray), selector(arrayByAddingObjectsFromArray), __Member} },

            {.method = {0, funcAddr(&CHMutableArray::copyWithZone), selector(copyWithZone), __Member} },
            {.method = {0, funcAddr(&CHMutableArray::mutableCopyWithZone), selector(mutableCopyWithZone), __Member} },
        };
        return method;
    }
};

static class_t ClassNamed(CHMutableArray) = {
    CHArray::getClass(nullptr),
    selector(CHMutableArray),
    runtimeclass(CHMutableArray)::methods(),
    nullptr,
    allocateCache(),
    selector(^#CHMutableArray),
    static_cast<uint32_t>((class_registerClass(&ClassNamed(CHMutableArray)), sizeof(CHMutableArray))),
    0,
    27
};

Implement(CHMutableArray);

CHMutableArray::CHMutableArray() : CHArray() {}

CHMutableArray::~CHMutableArray() {}

void CHMutableArray::addObject(id anObject)
{
    d_d(this, insertObjectAtIndex(anObject, count()));
}

void CHMutableArray::insertObjectAtIndex(id anObject, uint32_t index)
{
    if (anObject == nullptr) {
        throwException(CHInvalidArgumentException, "Unexcepted argument: object is nil.");
    }
    if (index > count()) {
        throwException(CHRangeException, "Argument index(%u) is out of bounds(%u)", index, count());
    }
    d_d(this, insertObjectAtIndex(anObject, index));
}

void CHMutableArray::removeLastObject()
{
    d_d(this, removeObjectInRange(CHMakeRange(count() - 1, 1)));
}

void CHMutableArray::removeObjectAtIndex(uint32_t index)
{
    d_d(this, removeObjectInRange(CHMakeRange(index, 1)));
}

void CHMutableArray::replaceObjectAtIndexWithObject(uint32_t index, id anObject)
{
    d_d(this, replaceObjectAtIndexWithObject(index, anObject));
}

void CHMutableArray::addObjectsFromArray(CHArray *otherArray)
{
    d_d(this, addObjectsFromArray((CHArrayPrivate *)(((CHMutableArray *)otherArray)->reserved())));
}

void CHMutableArray::exchangeObjectAtIndexWithObjectAtIndex(uint32_t idx1, uint32_t idx2)
{
    if (idx1 == idx2) {
        return;
    }
    if (idx1 >= count()) {
        throwException(CHRangeException, "Argument idx1(%u) is out of bounds(%u)", idx1, count());
    }
    if (idx2 >= count()) {
        throwException(CHRangeException, "Argument idx2(%u) is out of bounds(%u)", idx2, count());
    }
    id *buffer = d_d(this, _begin);
    std::swap(buffer[idx1], buffer[idx2]);
}

void CHMutableArray::removeAllObjects()
{
    d_d(this, removeAllObjects());
}

void CHMutableArray::removeObjectInRange(id anObject, CHRange range)
{
    d_d(this, removeObjectInRange(anObject, range));
}

void CHMutableArray::removeObject(id anObject)
{
    d_d(this, removeObjectInRange(anObject, CHMakeRange(0, count())));
}

void CHMutableArray::removeObjectIdenticalTo(id anObject)
{
    d_d(this, removeObjectIdenticalToInRange(anObject, CHMakeRange(0, count())));
}

void CHMutableArray::removeObjectIdenticalToInRange(id anObject, CHRange range)
{
    d_d(this, removeObjectIdenticalToInRange(anObject, range));
}

void CHMutableArray::removeObjectsInArray(CHArray *otherArray)
{
    id *begin = d_d(((CHMutableArray *)otherArray), _begin);
    id *end = d_d(((CHMutableArray *)otherArray), end());
    while (begin < end) {
        removeObject(*begin++);
    }
}

void CHMutableArray::removeObjectsInRange(CHRange range)
{
    d_d(this, removeObjectInRange(range));
}

void CHMutableArray::setArray(CHArray *otherArray)
{
    d_d(this, setArray((CHArrayPrivate *)(((CHMutableArray *)otherArray)->reserved())));
}

void CHMutableArray::sortUsingComparator(CHArraySortedComparator cmptr)
{
    d_d(this, sortedArrayUsingComparator(cmptr));
}

id& CHMutableArray::operator[](uint32_t index) throw()
{
    if (index >= count()) {
        throwException(CHRangeException, "Argument idnex(%u) is out of bounds(%u)", index, count());
    }
    id &r = (d_d(this, _begin))[index];
    return r;
}

// creation
CHMutableArray *CHMutableArray::arrayWithCapacity(uint32_t capacity)
{
    CHMutableArray *array = new CHMutableArray();
    CHArrayPrivate *d = new CHArrayPrivate(capacity);
    array->setReserved(d);
    return array;
}

CHMutableArray *CHMutableArray::arrayWithObject(id obj)
{
    return (CHMutableArray *)CHArray::arrayWithObject(obj);
}

CHMutableArray *CHMutableArray::arrayWithObjects(const id objects[], uint32_t count)
{
    return (CHMutableArray *)CHArray::arrayWithObjects(objects, count);
}

CHMutableArray *CHMutableArray::arrayWithObjects(id object, ...)
{
    CHMutableArray *array = new CHMutableArray();
    array->setReserved(new CHArrayPrivate(16));
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

CHMutableArray *CHMutableArray::arrayWithArray(const CHArray *array)
{
    return (CHMutableArray *)CHArray::arrayWithArray(array);
}

// protocol

id CHMutableArray::copyWithZone(std::nullptr_t) const
{
    return CHArray::copyWithZone(nullptr);
}

id CHMutableArray::mutableCopyWithZone(std::nullptr_t) const
{
    return copyWithZone(nullptr);
}
