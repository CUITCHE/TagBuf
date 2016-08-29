//
//  CFStringRefTools.h
//  TagBuf
//
//  Created by hejunqiu on 16/8/29.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef CFStringRefTools_h
#define CFStringRefTools_h

#include <CoreFoundation/CFString.h>

void CHCFStringGetBuffer(CFStringRef strPtr, const char *&dest, CFIndex &length);

#endif /* CFStringRefTools_h */
