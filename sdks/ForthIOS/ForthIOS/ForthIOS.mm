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

@interface ForthIOS()
{
@private
    StreamingEngine mStreamingEngine;
    AudioRingBuffer mAudioRingBuffer;
    VideoFrameBuffer mVideoFrameBuffer;
}

@end

@implementation ForthIOS

void frameCallback(oppvs::PixelBuffer& pf)
{
    printf("Receive frame\n");
    delete [] pf.plane[0];
}

void streamingCallback(void* user)
{
}

- (void) initStreamingEngine: (NSArray*) streamInfo
{
    NSLog(@"Init forth streaming engine");

    
    std::vector<VideoActiveSource> vSources;
    VideoActiveSource source;
    source.video_source_type = VST_WEBCAM;
    source.rect.bottom = 0;
    source.rect.top = 780;
    source.rect.left = 0;
    source.rect.right = 1280;
    source.stride = 1280 * 4;
    source.id = 1;
    vSources.push_back(source);
    std::vector<AudioActiveSource> aSources;
    
    //mStreamingEngine.setStreamInfo(vSources, aSources);
    mStreamingEngine.registerCallback(frameCallback);
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

@end
