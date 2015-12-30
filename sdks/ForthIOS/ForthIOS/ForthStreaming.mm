//
//  ForthStreaming.m
//  ForthIOS
//
//  Created by Cao Minh Trang on 11/4/15.
//  Copyright © 2015 Cao Minh Trang. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "ForthStreaming.h"

#include "streaming_engine.hpp"
#include "mac_audio_play.hpp"

using namespace oppvs;

@interface ForthStreaming()
{
@private
    StreamingEngine mStreamingEngine;
    AudioRingBuffer mAudioRingBuffer;
    VideoFrameBuffer mVideoFrameBuffer;
    MacAudioPlay* mPlayer;
    
    std::string mServerAddress;
}

- (void) streamingCallback;

@end

@implementation ForthStreaming

void frameCallback(oppvs::PixelBuffer& pf)
{
    //printf("Receive streaming frame\n");
    ForthStreaming *forth = (__bridge ForthStreaming*)pf.user;
    [[forth delegate] frameCallback:pf.plane[0] withWidth:pf.width[0] andHeight:pf.height[0] andStride:pf.stride[0]];
}

void streamingCallback(void* user)
{
    ForthStreaming *forth = (__bridge ForthStreaming*)user;
    [forth streamingCallback];
}

- (void) startStreaming: (NSString*)streamKey
{
    NSLog(@"Init forth streaming engine");
    
    mStreamingEngine.registerCallback(frameCallback, (__bridge void*)self);
    mStreamingEngine.registerCallback(streamingCallback, (__bridge void*)self);
    mStreamingEngine.attachBuffer(&mAudioRingBuffer);
    mStreamingEngine.attachBuffer(&mVideoFrameBuffer);
    if (mStreamingEngine.init(ROLE_VIEWER, mServerAddress, mServerAddress, TURN_SERVER_USER, TURN_SERVER_PASS, mServerAddress, SIGN_SERVER_PORT) < 0)
    {
        NSLog(@"Failed to init streaming engine");
        return;
    }
    
    std::string strStreamKey([streamKey UTF8String]);
    if (mStreamingEngine.start(strStreamKey) < 0)
    {
        NSLog(@"Failed to start streaming engine");
        return;
    }
}

- (void) streamingCallback
{
    mPlayer->start();
    mPlayer->setFirstInputTime(0.0);
}

- (void) setupAudioPlayer
{
    AudioDevice output(0);
    mPlayer = new MacAudioPlay(output, 48000, 2);
    
    if (mPlayer->init() < 0)
    {
        NSLog(@"Cannot init audio player");
        return;
    }
    
    mPlayer->attachBuffer(&mAudioRingBuffer);

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
