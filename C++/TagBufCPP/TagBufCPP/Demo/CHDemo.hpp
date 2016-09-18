//
//  CHDemo.hpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/8/30.
//  Copyright © 2016年 CHE. All rights reserved.
//

#ifndef CHDemo_hpp
#define CHDemo_hpp

#include "tagBuf.hpp"

Interface(CHDemo2)
    property_pod(_1, double);
};

Interface(CHDemo)
    property_pod(_1, int);
    property_class(_2, CHDemo2);
    property_pod(_3, int);
    property_pod(_4, int);
    property_pod(tableId, int);
};

#endif /* CHDemo_hpp */
