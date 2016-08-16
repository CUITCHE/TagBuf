//
//  CHTagBufferBuilder.h
//  TagBuf
//
//  Created by hejunqiu on 16/7/22.
//  Copyright © 2016年 CHE. All rights reserved.
//

#pragma once

#import <Foundation/Foundation.h>

class CHTagBufferBuilder
{
    struct CHTagBufferBuilderPrivate *_d;
public:
    CHTagBufferBuilder();
    ~CHTagBufferBuilder();

    void startBuildingWithObject(id instance);
    id readTagBuffer(NSData *data, id instance);

    NSData *buildedData() const;
};