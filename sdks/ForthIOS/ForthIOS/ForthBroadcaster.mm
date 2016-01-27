//
//  ForthBroadcaster.m
//  ForthIOS
//
//  Created by Cao Minh Trang on 1/27/16.
//  Copyright © 2016 Cao Minh Trang. All rights reserved.
//

#import "ForthBroadcaster.h"
#include "streaming_engine.hpp"

using namespace oppvs;

@implementation ForthBroadcaster
{
@private
    StreamingEngine mStreamingEngine;
    std::string mServerAddress;
}

- (id) init:(NSString *)serverAddress
{
    self = [super init];
    if (self)
    {
        mServerAddress = std::string([serverAddress UTF8String]);
        return self;
    }
    return self;
}

- (id) init
{
    self = [super init];
    if (self) {
        mServerAddress = STUN_SERVER_ADDRESS;
    }
    return self;
}

@end
