//
//  TagBufferBuilder.hpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/31.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef TagBufferBuilder_hpp
#define TagBufferBuilder_hpp

#include "types.h"

class CHTagBuf;
class CHData;

void objectToTagBuffer(CHTagBuf *object, CHData &outdata);

void *objectWithTagBuffer(const CHData &data, Class cls);


class TagBufferBuilder {
    struct TagBufferBuilderPrivate *d;
public:
    TagBufferBuilder();
    ~TagBufferBuilder();

    void startBuildingWithObject(CHTagBuf *object);
    void *readTagBuffer(const CHData *data, Class cls);

    CHData &buildedData() const;
};

#endif /* TagBufferBuilder_hpp */
