//
//  Algorithm.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 2016/9/24.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "Algorithm.hpp"
#include <algorithm>
#include <stdlib.h>
using namespace std;

namespace CHStringSearch {
    static inline void _computeGoodSubstringShift(const char *needle, int needleLength, unsigned int shift[], unsigned int suff[])
    {
        int f, g, i, j;

        // Compute suffix lengths

        suff[needleLength - 1] = needleLength;
        f = g = needleLength - 1;
        for (i = needleLength - 2; i >= 0; --i) {
            if (i > g && suff[i + needleLength - 1 - f] < i - g)
                suff[i] = suff[i + needleLength - 1 - f];
            else {
                if (i < g)
                    g = i;
                f = i;
                while (g >= 0 && needle[g] == needle[g + needleLength - 1 - f])
                    --g;
                suff[i] = f - g;
            }
        }

        // Compute shift table

        for (i = 0; i < needleLength; ++i)
            shift[i] = needleLength;
        j = 0;
        for (i = needleLength - 1; i >= 0; --i)
            if (suff[i] == i + 1)
                for (; j < needleLength - 1 - i; ++j)
                    if (shift[j] == needleLength)
                        shift[j] = needleLength - 1 - i;
        // Set the amount of shift necessary to move each of the suffix matches found into a position where it overlaps with the suffix. If there are duplicate matches the latest one is the one that should take effect.
        for (i = 0; i <= needleLength - 2; ++i)
            shift[needleLength - 1 - suff[i]] = needleLength - 1 - i;
        // Since the Boyer-Moore algorithm moves the pointer back while scanning substrings, add the distance to the end of the potential substring.
        for (i = 0; i < needleLength - 1; ++i) {
            shift[i] += (needleLength - 1 - i);
        }
    }
#define REVERSE_BUFFER(type, buf, len) { \
type tmp; \
for(int i = 0; i < (len)/2; i++) { \
tmp = (buf)[i]; \
(buf)[i] = (buf)[(len) - i - 1]; \
(buf)[(len) - i - 1] = tmp; \
} \
}

    static inline const char * __CHDataSearchBoyerMoore(const char *data, const char *haystack, int haystackLength, const char *needle, int needleLength, bool backwards) {
        unsigned int badCharacterShift[UCHAR_MAX + 1] = {0};
        unsigned int *goodSubstringShift = (unsigned int *)malloc(needleLength * sizeof(unsigned int));
        unsigned int *suffixLengths = (unsigned int *)malloc(needleLength * sizeof(unsigned int));
        if (!goodSubstringShift || !suffixLengths) {
            free(goodSubstringShift);
            free(suffixLengths);
            return nullptr;
        }

        if(backwards) {
            for (int i = 0; i < sizeof(badCharacterShift) / sizeof(*badCharacterShift); i++)
                badCharacterShift[i] = needleLength;

            for (int i = needleLength - 1; i >= 0; i--)
                badCharacterShift[needle[i]] = i;

            // To get the correct shift table for backwards search reverse the needle, compute the forwards shift table, and then reverse the result.
            char *needleCopy = (char *)malloc(needleLength * sizeof(char));
            if (!needleCopy) {
                //                __CHDataHandleOutOfMemory(data, needleLength * sizeof(uint8_t));
                return 0;
            }
            memmove(needleCopy, needle, needleLength);
            REVERSE_BUFFER(char, needleCopy, needleLength);
            _computeGoodSubstringShift(needleCopy, needleLength, goodSubstringShift, suffixLengths);
            REVERSE_BUFFER(unsigned int, goodSubstringShift, needleLength);
            free(needleCopy);
        } else {
            for (int i = 0; i < sizeof(badCharacterShift) / sizeof(*badCharacterShift); i++)
                badCharacterShift[i] = needleLength;

            for (int i = 0; i < needleLength; i++)
                badCharacterShift[needle[i]] = needleLength - i- 1;

            _computeGoodSubstringShift(needle, needleLength, goodSubstringShift, suffixLengths);
        }

        const char *scan_needle;
        const char *scan_haystack;
        const char *result = NULL;
        if(backwards) {
            const char *const end_needle = needle + needleLength;
            scan_needle = needle;
            scan_haystack = haystack + haystackLength - needleLength;
            while (scan_haystack >= haystack && scan_needle < end_needle) {
                if (*scan_haystack == *scan_needle) {
                    scan_haystack++;
                    scan_needle++;
                } else {
                    scan_haystack -= std::max(badCharacterShift[*scan_haystack], goodSubstringShift[scan_needle - needle]);
                    scan_needle = needle;
                }
            }
            if (scan_needle == end_needle) {
                result = (scan_haystack - needleLength);
            }
        } else {
            const char *const end_haystack = haystack + haystackLength;
            scan_needle = needle + needleLength - 1;
            scan_haystack = haystack + needleLength - 1;
            while (scan_haystack < end_haystack && scan_needle >= needle) {
                if (*scan_haystack == *scan_needle) {
                    scan_haystack--;
                    scan_needle--;
                } else {
                    scan_haystack += std::max(badCharacterShift[*scan_haystack], goodSubstringShift[scan_needle - needle]);
                    scan_needle = needle + needleLength - 1;
                }
            }
            if (scan_needle < needle) {
                result = (scan_haystack + 1);
            }
        }

        free(goodSubstringShift);
        free(suffixLengths);

        return result;
    }

    static inline CHRange _CHDataFindBytes(const char *data,
                                           unsigned int fullHaystackLength,
                                           const char *dataToFind,
                                           unsigned int needleLength,
                                           CHRange searchRange)
    {
        const char *fullHaystack = data;
        const char *needle = dataToFind;

        if(searchRange.length > fullHaystackLength - searchRange.location) {
            searchRange.length = fullHaystackLength - searchRange.location;
        }

        if(searchRange.length < needleLength || fullHaystackLength == 0 || needleLength == 0) {
            return CHMakeRange(CHNotFound, 0);
        }

        const char *haystack = fullHaystack + searchRange.location;
        const char *searchResult = __CHDataSearchBoyerMoore(data, haystack, searchRange.length, needle, needleLength, false);
        uint32_t resultLocation = (searchResult == NULL) ? CHNotFound : searchRange.location + (uint32_t)(searchResult - haystack);

        return CHMakeRange(resultLocation, resultLocation == CHNotFound ? 0: needleLength);
    }

    static inline CHRange _CHDataFindByteUsually(const char *src,
                                                 uint32_t srcLength,
                                                 const char *pattern,
                                                 uint32_t patternLength)
    {
        if (srcLength < patternLength || !src) {
            return CHMakeRange(CHNotFound, 0);
        }
        if (!pattern || patternLength == 0) {
            return CHMakeRange(0, 0);
        }
        const char *buf = src;
        const char *srcp = nullptr;
        const char *srcEnd = src + srcLength;
        const char *patp = nullptr;
        const char *patEnd = pattern + patternLength;
        while (buf < srcEnd) {
            srcp = buf;
            patp = pattern;
            do {
                if (patp == patEnd) {
                    return CHMakeRange((uint32_t)(buf - src), patternLength);
                }
            } while (*srcp++ == *patp++);
            ++buf;
        }
        return CHMakeRange(CHNotFound, 0);
    }

    static inline void searchAllOfOccurrencesOfString(const char *haystack, int haystackLength, const char *needle, int needleLength, vector<CHRange> &result)
    {
        unsigned int badCharacterShift[UCHAR_MAX + 1] = {0};
        unsigned int *goodSubstringShift = (unsigned int *)malloc(needleLength * sizeof(unsigned int));
        unsigned int *suffixLengths = (unsigned int *)malloc(needleLength * sizeof(unsigned int));
        if (!goodSubstringShift || !suffixLengths) {
            free(goodSubstringShift);
            free(suffixLengths);
            return;
        }

        for (int i = 0; i < sizeof(badCharacterShift) / sizeof(*badCharacterShift); i++)
            badCharacterShift[i] = needleLength;

        for (int i = 0; i < needleLength; i++)
            badCharacterShift[needle[i]] = needleLength - i- 1;

        _computeGoodSubstringShift(needle, needleLength, goodSubstringShift, suffixLengths);

        const char *scan_needle;
        const char *scan_haystack;
        const char *rest = NULL;
        const char *const end_haystack = haystack + haystackLength;
        scan_needle = needle + needleLength - 1;
        scan_haystack = haystack + needleLength - 1;
        CHRange searchResultRange{.location = 0, .length = (uint32_t)needleLength};
        while (scan_haystack < end_haystack) {
            if (*scan_haystack == *scan_needle) {
                scan_haystack--;
                scan_needle--;
            } else {
                scan_haystack += std::max(badCharacterShift[*scan_haystack], goodSubstringShift[scan_needle - needle]);
                scan_needle = needle + needleLength - 1;
            }
            if (scan_needle < needle) {
                rest = (scan_haystack + 1);
                searchResultRange.location = (uint32_t)(rest - haystack);
                result.push_back(searchResultRange);
                scan_needle = needle + needleLength - 1;
                scan_haystack += needleLength + 1;
            }
        }

        free(goodSubstringShift);
        free(suffixLengths);
    }
}

CHRange BMContainsString(const char *src, uint32_t srcLen, const char *pattern, uint32_t patternLength)
{
    return CHStringSearch::_CHDataFindBytes(src, srcLen, pattern, patternLength, CHMakeRange(0, srcLen));
}

CHRange containsStringUsually(const char *src, uint32_t srcLen, const char *pattern, uint32_t patternLength)
{
    return CHStringSearch::_CHDataFindByteUsually(src, srcLen, pattern, patternLength);
}

void searchAllOfOccurrencesOfString(const char *src, uint32_t srcLen, const char *pattern, uint32_t patternLength, vector<CHRange> &result)
{
    if (!src || patternLength > srcLen || !pattern) {
        return;
    }
    CHStringSearch::searchAllOfOccurrencesOfString(src, srcLen, pattern, patternLength, result);
}
