//
//  ForthIOS.m
//  ForthIOS
//
//  Created by Cao Minh Trang on 10/27/15.
//  Copyright © 2015 Cao Minh Trang. All rights reserved.
//

#import "ForthIOS.h"

#include "streaming_engine.hpp"

using namespace oppvs;

@interface ForthStreaming()
{
@private
    StreamingEngine mStreamingEngine;
    AudioRingBuffer mAudioRingBuffer;
    VideoFrameBuffer mVideoFrameBuffer;
}

- (void) streamingCallback;

@end

@implementation ForthStreaming

void frameCallback(oppvs::PixelBuffer& pf)
{
    //printf("Receive streaming frame\n");
    delete [] pf.plane[0];
    ForthStreaming *forth = (__bridge ForthStreaming*)pf.user;
    [[forth delegate] frameCallback:pf.plane[0] withWidth:pf.width[0] andHeight:pf.height[0] andStride:pf.stride[0]];
}

void streamingCallback(void* user)
{
    ForthStreaming *forth = (__bridge ForthStreaming*)user;
    [forth streamingCallback];
}

- (void) startStreaming
{
    NSLog(@"Init forth streaming engine");

    mStreamingEngine.registerCallback(frameCallback, (__bridge void*)self);
    mStreamingEngine.registerCallback(streamingCallback, (__bridge void*)self);
    mStreamingEngine.attachBuffer(&mAudioRingBuffer);
    mStreamingEngine.attachBuffer(&mVideoFrameBuffer);
    if (mStreamingEngine.init(ROLE_VIEWER, STUN_SERVER_ADDRESS, TURN_SERVER_ADDRESS, TURN_SERVER_USER, TURN_SERVER_PASS, SIGN_SERVER_ADDRESS, SIGN_SERVER_PORT) < 0)
    {
        NSLog(@"Failed to init streaming engine");
        return;
    }
    
    std::string strStreamKey("7116f0d7-5c27-44e6-8aa4-bc4ddeea9935");
    if (mStreamingEngine.start(strStreamKey) < 0)
    {
        NSLog(@"Failed to start streaming engine");
        return;
    }
}

- (void) streamingCallback
{
    NSLog(@"Streaming callback");
}

@end
