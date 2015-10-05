//
//  Document.h
//  OPPVSGUI
//
//  Created by Cao Minh Trang on 3/13/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#pragma once

#import <Cocoa/Cocoa.h>
#include "mac_video_capture.hpp"
#include "mac_audio_engine.hpp"
#include "streaming_engine.hpp"

#include "ViewController.h"
#include "Util.h"

@class ViewController;

@interface Document : NSDocument
{
@private
    oppvs::MacVideoEngine* mVideoEngine;
    oppvs::MacAudioEngine* mAudioEngine;
}

- (void) addSource: (NSString*) sourceid hasType: (oppvs::VideoSourceType) type sourceRect: (CGRect)srect renderRect: (CGRect)rrect withViewID: (id) viewid atIndex: (NSInteger) index;
- (void) startRecording;
- (void) stopRecording;
- (void) startStreaming: (NSString*)streamKey;
- (void) stopStreaming;
- (void) initEngines: (id) userid;

@end

