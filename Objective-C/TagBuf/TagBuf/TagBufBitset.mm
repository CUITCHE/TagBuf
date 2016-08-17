//
//  TagBufBitset.cpp
//  TagBuf
//
//  Created by hejunqiu on 16/8/16.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import "TagBufBitset.h"
#include <vector>
#include <bitset>
using namespace std;

struct TagBufBitsetPrivate {
    vector<char> *buf;
    size_t writePointer = -1;
    TagBufBitsetPrivate(size_t capacity)
    {
        buf = new vector<char>(capacity);
    }
    ~TagBufBitsetPrivate()
    {
        delete buf;
    }
};

TagBufBitset::TagBufBitset(size_t capacity/* = 1*/)
    :_d(new TagBufBitsetPrivate(capacity/8))
{
}

TagBufBitset::~TagBufBitset()
{
    delete _d;
}

TagBufBitset::TagBufBitset(const char *bitdata, size_t count, size_t bitposition)
    :_d(new  TagBufBitsetPrivate(count))
{
    NSCAssert(count * 8 >= bitposition + 1, @"Param value-bitposition is overflow. \
              Because bitdata contains up to 8 * (count) = %ld, bitposition(%ld) \
              greater than it.", 8 * count, bitposition);
    _d->buf->resize(count);
    memcpy(_d->buf->data(), bitdata, sizeof(char) * count);
    _d->writePointer = bitposition;
}

#define bit0 0x80
#define bit1 0x40
#define bit2 0x20
#define bit3 0x10
#define bit4 0x8
#define bit5 0x4
#define bit6 0x2
#define bit7 0x1

void TagBufBitset::push_back(bool bit)
{
    const size_t writePosition = _d->writePointer + 1; // position which is ready to write.
    char &bitChar = _d->buf->at(writePosition / 8);
    switch (writePosition % 8) {
        case 0:
            bitChar &= bit ? bit0 : ~bit0; break;
        case 1:
            bitChar &= bit ? bit1 : ~bit1; break;
        case 2:
            bitChar &= bit ? bit2 : ~bit2; break;
        case 3:
            bitChar &= bit ? bit3 : ~bit3; break;
        case 4:
            bitChar &= bit ? bit4 : ~bit4; break;
        case 5:
            bitChar &= bit ? bit5 : ~bit5; break;
        case 6:
            bitChar &= bit ? bit6 : ~bit6; break;
        case 7:
            bitChar &= bit ? bit7 : ~bit7; break;
        default:
            NSCAssert(NO, @"Logic error.");
            break;
    }
    _d->writePointer = writePosition;
}

bool TagBufBitset::operator[](size_t bitposition) const
{
/*
 * A char:
 *  ---------------
 * |0|1|2|3|4|5|6|7|
 *  ---------------
 */
    assert(bitposition <= _d->writePointer);
    char bit = _d->buf->at(bitposition / 8);
    switch (bitposition % 8) {
        case 0: return bit & bit0;
        case 1: return bit & bit1;
        case 2: return bit & bit2;
        case 3: return bit & bit3;
        case 4: return bit & bit4;
        case 5: return bit & bit5;
        case 6: return bit & bit6;
        case 7: return bit & bit7;
        default:
            NSCAssert(NO, @"Logic error.");
            return false;
    }
}

void TagBufBitset::bitset(const char *&bitset, size_t *dataCount, size_t *bitcount /*= nullptr*/) const
{
    bitset = _d->buf->data();
    if (bitcount) {
        *bitcount = _d->writePointer + 1;
    }
    if (dataCount) {
        *dataCount = _d->buf->size();
    }
}

void TagBufBitset::convertArrayToBitset(NSArray *bitArray)
{
    if (!bitArray) {
        NSCAssert(NO, @"Param is nil.");
        return;
    }
    NSCAssert([bitArray.firstObject isKindOfClass:[NSNumber class]], @"NSArray must contain NSNumber type.");
    auto &bitBuf = *_d->buf;
    bitBuf.clear();
    uint8_t bitSet = 0;
    uint8_t i = 0;
    for (NSNumber *number in bitArray) {
        switch (i) {
            case 0:
                bitSet &= number.boolValue ? bit0 : ~bit0; break;
            case 1:
                bitSet &= number.boolValue ? bit1 : ~bit1; break;
            case 2:
                bitSet &= number.boolValue ? bit2 : ~bit2; break;
            case 3:
                bitSet &= number.boolValue ? bit3 : ~bit3; break;
            case 4:
                bitSet &= number.boolValue ? bit4 : ~bit4; break;
            case 5:
                bitSet &= number.boolValue ? bit5 : ~bit5; break;
            case 6:
                bitSet &= number.boolValue ? bit6 : ~bit6; break;
            case 7:
                bitSet &= number.boolValue ? bit7 : ~bit7;
                i = -1;
                bitBuf.push_back(bitSet);
                bitSet = 0;
                break;
            default:
                NSCAssert(NO, @"Logic error.");
                break;
        }
        ++i;
    }
    _d->writePointer = bitBuf.size() * 8 + i;
    if (bitSet) {
        bitBuf.push_back(bitSet);
    }
}

void TagBufBitset::convertToBooleanArray(NSMutableArray *bitArray)
{
    NSCAssert(bitArray, @"Param bitArray must not be nil.");
    vector<char> &bitVector = *_d->buf;
    char storage = bitVector.back();
    bitVector.pop_back();
    @try {
        size_t i = -1;
        for (char bitChar : bitVector) {
            [bitArray addObject:@(!!(bitChar & bit0))];
            [bitArray addObject:@(!!(bitChar & bit1))];
            [bitArray addObject:@(!!(bitChar & bit2))];
            [bitArray addObject:@(!!(bitChar & bit3))];
            [bitArray addObject:@(!!(bitChar & bit4))];
            [bitArray addObject:@(!!(bitChar & bit5))];
            [bitArray addObject:@(!!(bitChar & bit6))];
            [bitArray addObject:@(!!(bitChar & bit7))];
            i += 8;
        }
        size_t rest = _d->writePointer - i;
        for (i=0; i<rest; ++i) {
            switch (i) {
                case 0:
                    [bitArray addObject:@(!!(storage & bit0))];
                    break;
                case 1:
                    [bitArray addObject:@(!!(storage & bit1))];
                    break;
                case 2:
                    [bitArray addObject:@(!!(storage & bit2))];
                    break;
                case 3:
                    [bitArray addObject:@(!!(storage & bit3))];
                    break;
                case 4:
                    [bitArray addObject:@(!!(storage & bit4))];
                    break;
                case 5:
                    [bitArray addObject:@(!!(storage & bit5))];
                    break;
                case 6:
                    [bitArray addObject:@(!!(storage & bit6))];
                    break;
                case 7:
                    [bitArray addObject:@(!!(storage & bit7))];
                    break;
                default:
                    NSCAssert(NO, @"Logic error");
                    break;
            }
        }
    } @catch (NSException *exception) {
        NSLog(@"%@", exception);
    } @finally {
        bitVector.push_back(storage);
    }

}
