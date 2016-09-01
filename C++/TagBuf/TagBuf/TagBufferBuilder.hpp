//
//  TagBufferBuilder.hpp
//  TagBuf
//
//  Created by hejunqiu on 16/8/31.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef TagBufferBuilder_hpp
#define TagBufferBuilder_hpp

#include "types.h"
#include <vector>

class CHTagBuf;

void objectToTagBuffer(CHTagBuf *object, std::vector<char> &outdata);

void *objectWithTagBuffer(const std::vector<char> &data, id cls);


class TagBufferBuilder {
    struct TagBufferBuilderPrivate *d;
public:
    TagBufferBuilder();
    ~TagBufferBuilder();

    void startBuildingWithObject(CHTagBuf *object);
    void *readTagBuffer(std::vector<char> &data, id cls);

    std::vector<char> &buildedData() const;
};

#endif /* TagBufferBuilder_hpp */
