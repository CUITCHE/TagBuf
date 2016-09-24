//
//  Algorithm.hpp
//  TagBufCPP
//
//  Created by hejunqiu on 2016/9/24.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef Algorithm_hpp
#define Algorithm_hpp

#include "CHRange.hpp"
#include <vector>
using std::vector;

extern CHRange BMContainsString(const char *src, uint32_t srcLen, const char *pattern, uint32_t patternLength);

extern CHRange containsStringUsually(const char *src, uint32_t srcLen, const char *pattern, uint32_t patternLength);

extern void searchAllOfOccurrencesOfString(const char *src, uint32_t srcLen, const char *pattern, uint32_t patternLength, vector<CHRange> &result);

#endif /* Algorithm_hpp */
