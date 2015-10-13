//
//  Document.h
//  OPPVSClient
//
//  Created by Cao Minh Trang on 3/27/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "streaming_engine.hpp"

#include "mac_audio_play.hpp"
#include "ViewController.h"

@class ViewController;


@interface Document : NSDocument
{
    oppvs::MacAudioPlay* mPlayer;
    oppvs::AudioRingBuffer mAudioRingBuffer;
    oppvs::VideoFrameBuffer mVideoFrameBuffer;
}

- (void) initReceiver: (NSString*)streamKey;
- (void) startAudioPlayer;
- (void) cleanup;

@end

