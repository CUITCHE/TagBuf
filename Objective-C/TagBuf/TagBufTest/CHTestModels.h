//
//  CHTestModels.h
//  TagBuf
//
//  Created by hejunqiu on 16/8/18.
//  Copyright © 2016年 CHE. All rights reserved.
//

#import "CHTagBuffer.h"
#import "tagBuf.h"

#define __superClass CHTagBuffer

#define __implementation(cls) @implementation Test##cls \
                              @end

@interface CHTestModels : __superClass

@end

#define __testCaseClassBegin(cls) @interface Test##cls : __superClass
#define __testCaseClassEnd() @end

#define __property(p) @property (nonatomic, strong) p
#define __property_pod(p) @property (nonatomic) p

// test NSNumber
__testCaseClassBegin(NSNumber)
__property(NSNumber<NSNumberBoolean> *_1);
__property(NSNumber<NSNumberInt8> *_2);
__property(NSNumber<NSNumberInt16> *_3);
__property(NSNumber<NSNumberInt32> *_4);
__property(NSNumber<NSNumberInt64> *_5);
__property(NSNumber<NSNumberFloat> *_6);
__property(NSNumber<NSNumberDouble> *_7);
//__property(NSNumber<NSNumberInt8> *_8);
__testCaseClassEnd()


// test pod type
__testCaseClassBegin(PodType)
__property_pod(BOOL _1);
__property_pod(uint8 _2);
__property_pod(uint16 _3);
__property_pod(uint32 _4);
__property_pod(uint64 _5);
__property_pod(float _6);
__property_pod(double _7);
__property_pod(bool _8);
__testCaseClassEnd()

// test NSString
__testCaseClassBegin(NSString)
__property(NSString *_1);
__property(NSString<optional> *_2);
__property(NSString<ignore> *_3);
__testCaseClassEnd()

// test NSData
__testCaseClassBegin(NSData)
__property(NSData *_1);
__property(NSData<optional> *_2);
__property(NSData<ignore> *_3);
__testCaseClassEnd()

// test NSArray empty
__testCaseClassBegin(NSArrayEmpty)
__property(NSArray *_1);
__testCaseClassEnd()

// test NSArray
__testCaseClassBegin(NSArray)

__property(NSArray *_1);
__property(NSArray<optional> *_2);
__property(NSArray<ignore> *_3);

__property(NSArray<NSArrayNSArray> *_4);
__property(NSArray<NSArrayNSString> *_5);
__property(NSArray<NSArrayNSData> *_6);

__property(NSArray<NSNumberBoolean> *_7);
__property(NSArray<NSNumberInt8> *_8);
__property(NSArray<NSNumberInt16> *_9);
__property(NSArray<NSNumberInt32> *_10);
__property(NSArray<NSNumberInt64> *_11);
__property(NSArray<NSNumberFloat> *_12);
__property(NSArray<NSNumberDouble> *_13);

__testCaseClassEnd()

// test multi protocol
__testCaseClassBegin(NSArrayMultiProtocol)
@property(nonatomic, strong) NSArray<NSArrayNSArray, ignore> *_14;
@property(nonatomic, strong) NSArray<NSArrayNSString, ignore> *_15;
@property(nonatomic, strong) NSArray<NSArrayNSData, ignore> *_16;

@property(nonatomic, strong) NSArray<NSNumberBoolean, ignore> *_17;
@property(nonatomic, strong) NSArray<NSNumberInt8, ignore> *_18;
@property(nonatomic, strong) NSArray<NSNumberInt16, ignore> *_19;
@property(nonatomic, strong) NSArray<NSNumberInt32, ignore> *_20;
@property(nonatomic, strong) NSArray<NSNumberInt64, ignore> *_21;
@property(nonatomic, strong) NSArray<NSNumberFloat, ignore> *_22;
@property(nonatomic, strong) NSArray<NSNumberDouble, ignore> *_23;

@property(nonatomic, strong) NSArray<NSArrayNSArray, optional> *_24;
@property(nonatomic, strong) NSArray<NSArrayNSString, optional> *_25;
@property(nonatomic, strong) NSArray<NSArrayNSData, optional> *_26;

@property(nonatomic, strong) NSArray<NSNumberBoolean, optional> *_27;
@property(nonatomic, strong) NSArray<NSNumberInt8, optional> *_28;
@property(nonatomic, strong) NSArray<NSNumberInt16, optional> *_29;
@property(nonatomic, strong) NSArray<NSNumberInt32, optional> *_30;
@property(nonatomic, strong) NSArray<NSNumberInt64, optional> *_31;
@property(nonatomic, strong) NSArray<NSNumberFloat, optional> *_32;
@property(nonatomic, strong) NSArray<NSNumberDouble, optional> *_33;
__testCaseClassEnd()


// test custom object
@protocol TestPodType
@end

__testCaseClassBegin(NSArrayOtherObject)
__property(NSArray<TestPodType> *_1);
__testCaseClassEnd()