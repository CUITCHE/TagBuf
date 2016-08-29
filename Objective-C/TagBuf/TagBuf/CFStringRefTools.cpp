//
//  CFStringRefTools.cpp
//  TagBuf
//
//  Created by hejunqiu on 16/8/29.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "CFStringRefTools.h"
#include <CoreFoundation/CoreFoundation.h>

enum {
    __kCFFreeContentsWhenDoneMask = 0x020,
    __kCFFreeContentsWhenDone = 0x020,
    __kCFContentsMask = 0x060,
    __kCFHasInlineContents = 0x000,
    __kCFNotInlineContentsNoFree = 0x040,		// Don't free
    __kCFNotInlineContentsDefaultFree = 0x020,	// Use allocator's free function
    __kCFNotInlineContentsCustomFree = 0x060,		// Use a specially provided free function
    __kCFHasContentsAllocatorMask = 0x060,
    __kCFHasContentsAllocator = 0x060,		// (For mutable strings) use a specially provided allocator
    __kCFHasContentsDeallocatorMask = 0x060,
    __kCFHasContentsDeallocator = 0x060,
    __kCFIsMutableMask = 0x01,
    __kCFIsMutable = 0x01,
    __kCFIsUnicodeMask = 0x10,
    __kCFIsUnicode = 0x10,
    __kCFHasNullByteMask = 0x08,
    __kCFHasNullByte = 0x08,
    __kCFHasLengthByteMask = 0x04,
    __kCFHasLengthByte = 0x04,
    // !!! Bit 0x02 has been freed up
};

typedef struct __CHCFRuntimeBase {
    uintptr_t _cfisa;
    uint8_t _cfinfo[4];
#if __LP64__
    uint32_t _rc;
#endif
} CHCFRuntimeBase;

/* !!! Never do sizeof(CFString); the union is here just to make it easier to access some fields.
 */
struct __CHCFString {
    CHCFRuntimeBase base;
    union {	// In many cases the allocated structs are smaller than these
        struct __inline1 {
            CFIndex length;
        } inline1;                                      // Bytes follow the length
        struct __notInlineImmutable1 {
            void *buffer;                               // Note that the buffer is in the same place for all non-inline variants of CFString
            CFIndex length;
            CFAllocatorRef contentsDeallocator;		// Optional; just the dealloc func is used
        } notInlineImmutable1;                          // This is the usual not-inline immutable CFString
        struct __notInlineImmutable2 {
            void *buffer;
            CFAllocatorRef contentsDeallocator;		// Optional; just the dealloc func is used
        } notInlineImmutable2;                          // This is the not-inline immutable CFString when length is stored with the contents (first byte)
        struct __notInlineMutable{} notInlineMutable; // We don't use it.
    } variants;
};

const int CF_INFO_BITS = 0;

CF_INLINE SInt32 __CHCFStrSkipAnyLengthByte(CFStringRef str)
{
    // Number of bytes to skip over the length byte in the contents
    return ((((struct __CHCFString *)str)->base._cfinfo[CF_INFO_BITS] & __kCFHasLengthByteMask) == __kCFHasLengthByte) ? 1 : 0;
}

CF_INLINE Boolean __CHCFStrIsInline(CFStringRef str)
{
    return (((struct __CHCFString *)str)->base._cfinfo[CF_INFO_BITS] & __kCFContentsMask) == __kCFHasInlineContents;
}

CF_INLINE Boolean __CHCFStrHasExplicitLength(CFStringRef str)
{
    // Has explicit length if (1) mutable or (2) not mutable and no length byte
    return (((struct __CHCFString *)str)->base._cfinfo[CF_INFO_BITS] & (__kCFIsMutableMask | __kCFHasLengthByteMask)) != __kCFHasLengthByte;
}

CF_INLINE const void *__CHCFStrContents(CFStringRef str)
{
    if (__CHCFStrIsInline(str)) {
        return (const void *)(((uintptr_t)&(((struct __CHCFString *)str)->variants)) + (__CHCFStrHasExplicitLength(str) ? sizeof(CFIndex) : 0));
    } else {	// Not inline; pointer is always word 2
        return ((struct __CHCFString *)str)->variants.notInlineImmutable1.buffer;
    }
}

CF_INLINE CFIndex __CHCFStrLength2(CFStringRef str, const void *buffer)
{
    if (__CHCFStrHasExplicitLength(str)) {
        if (__CHCFStrIsInline(str)) {
            return ((struct __CHCFString *)str)->variants.inline1.length;
        } else {
            return ((struct __CHCFString *)str)->variants.notInlineImmutable1.length;
        }
    } else {
        return (CFIndex)(*((uint8_t *)buffer));
    }
}

void CHCFStringGetBuffer(CFStringRef str, const char *&dest, CFIndex &length)
{
    const uint8_t *contents = (const uint8_t *)__CHCFStrContents(str);
    length = __CHCFStrLength2(str, contents);
    dest = (const char *)contents + __CHCFStrSkipAnyLengthByte(str);
}