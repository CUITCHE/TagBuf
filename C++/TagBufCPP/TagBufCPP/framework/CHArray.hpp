//
//  CHArray.hpp
//  TagBufCPP
//
//  Created by hejunqiu on 2016/9/22.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef CHArray_hpp
#define CHArray_hpp

#include <stdio.h>
#include <functional>
#include "id.hpp"
#include "CHRange.hpp"

using CHArrayObjectCallback = std::function<void(const id obj, uint32_t index, bool *stop)>;
using CHArraySortedComparator = std::function<bool(const id obj1, const id obj2)>;

class CHString;

class CHArray : public CHObject,
                protocolTo CHCopying,
                protocolTo CHMutableCopying
{
    __SUPPORTRUNTIME__(CHArray);
protected:
    explicit CHArray();
    explicit CHArray(uint32_t capacity);
public:
    ~CHArray() override;

    uint32_t count() const;

    id objectAtIndex(uint32_t index) const;

    // creation
    static CHArray *arrayWithObject(id obj);
    static CHArray *arrayWithObjects(const id objects[], uint32_t count);
    static CHArray *arrayWithObjects(id object, ...) __attribute__((sentinel(0,1)));
    static CHArray *arrayWithArray(const CHArray *array);

    // extended array
    CHArray *arrayByAddiObject(id object) const;
    CHArray *arrayByAddingObjectsFromArray(const CHArray *otherArray) const;
    CHString *componentsJoinedByString(const CHString *separator) const;

    bool containsObject(id anObject) const;
    uint32_t indexOfObject(id anObject) const;
    uint32_t indexOfObjectInRange(id anObject, CHRange range) const;
    bool isEqualToArray(ARRAY_CONTAINS(id) otherArray) const;

    id firstObject() const;
    id lastObject() const;

    void enumerateObjectsUsingBlock(CHArrayObjectCallback block) const;
    ARRAY_CONTAINS(id) sortedArrayUsingComparator(CHArraySortedComparator cmptr) const;

    // runtime
    Class getClass() const override;
    static Class getClass(std::nullptr_t);

    // protocol
    CHString *description() const;
    bool equalTo(id anObject) const override;
protected:
    id copyWithZone(std::nullptr_t) const;
    id mutableCopyWithZone(std::nullptr_t) const;
private:
    static id allocateInstance();
};

class CHMutableArray : public CHArray
{
    __SUPPORTRUNTIME__(CHMutableArray);
    explicit CHMutableArray();
public:
    ~CHMutableArray() override;

    void addObject(id anObject);
    void insertObjectAtIndex(id anObject, uint32_t index);
    void removeLastObject();
    void removeObjectAtIndex(uint32_t index);
    void replaceObjectAtIndexWithObject(uint32_t index, id anObject);

    void addObjectsFromArray(ARRAY_CONTAINS(id) otherArray);
    void exchangeObjectAtIndexWithObjectAtIndex(uint32_t idx1, uint32_t idx2);

    void removeAllObjects();
    void removeObjectInRange(id anObject, CHRange range);
    void removeObject(id anObject);

    void removeObjectIdenticalToInRange(id anObject, CHRange range);
    void removeObjectIdenticalTo(id anObject);

    void removeObjectsInArray(ARRAY_CONTAINS(id) otherArray);
    void removeObjectsInRange(CHRange range);

    void setArray(ARRAY_CONTAINS(id) otherArray);
    void sortUsingComparator(CHArraySortedComparator cmptr);

    id& operator[](uint32_t index) throw();

    // creation
    static CHMutableArray *arrayWithCapacity(uint32_t capacity);
    static CHMutableArray *arrayWithObject(id obj);
    static CHMutableArray *arrayWithObjects(const id objects[], uint32_t count);
    static CHMutableArray *arrayWithObjects(id object, ...) __attribute__((sentinel(0,1)));
    static CHMutableArray *arrayWithArray(const CHArray *array);

    // runtime
    Class getClass() const override;
    static Class getClass(std::nullptr_t);
protected:
    id copyWithZone(std::nullptr_t) const;
    id mutableCopyWithZone(std::nullptr_t) const;
private:
    static id allocateInstance();
};

#endif /* CHArray_hpp */
