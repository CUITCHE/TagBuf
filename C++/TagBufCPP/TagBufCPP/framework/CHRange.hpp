//
//  CHRange.hpp
//  TagBufCPP
//
//  Created by hejunqiu on 2016/9/23.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef CHRange_hpp
#define CHRange_hpp

#include "types.h"

class CHString;

typedef struct _CHRange {
    uint32_t location;
    uint32_t length;
} CHRange;

static inline CHRange CHMakeRange(uint32_t loc, uint32_t len) {
    CHRange r;
    r.location = loc;
    r.length = len;
    return r;
}

static inline uint32_t CHMaxRange(CHRange range) {
    return (range.location + range.length);
}

static inline bool CHLocationInRange(uint32_t loc, CHRange range) {
    return (!(loc < range.location) && (loc - range.location) < range.length) ? true : false;
}

static inline bool CHEqualRanges(CHRange range1, CHRange range2) {
    return (range1.location == range2.location && range1.length == range2.length);
}

extern CHString *showRange(CHRange range);

#endif /* CHRange_hpp */
