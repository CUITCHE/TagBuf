//
//  TagBufBitset.hpp
//  TagBuf
//
//  Created by hejunqiu on 16/8/16.
//  Copyright © 2016年 CHE. All rights reserved.
//

#pragma once

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

class TagBufBitset {
    struct TagBufBitsetPrivate *_d;
public:
    /**
     * @author hejunqiu, 16-08-17 15:08:51
     *
     * Create an object that is ready to write bit data. The capacity is stable.
     * But you can change bit size by method: convertArrayToBitset(...).
     *
     * @param capacity The size of bitset. This is stable.
     */
    TagBufBitset(size_t capacity = 8);

    /**
     * @author hejunqiu, 16-08-17 15:08:36
     *
     * Overload constructor method.
     *
     * @param bitdata     An array contains char value. Each bit in char represent
     * bool value.
     * @param count       The size of bitdata.
     * @param bitposition The size of bit in bitdata.
     */
    TagBufBitset(const char * _Nonnull bitdata, size_t count, size_t bitposition);
    ~TagBufBitset();

    void push_back(bool bit);
    bool operator[](size_t bitposition) const;
    void bitset(const char *&bitset, size_t * _Nullable dataCount, size_t * _Nullable bitcount = nullptr) const;
    void convertArrayToBitset(NSArray/*NSNumber<bool or BOOL>*/ *bitArray);
    void convertToBooleanArray(NSMutableArray * _Nonnull bitArray);
};

NS_ASSUME_NONNULL_END