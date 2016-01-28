//
//  ForthViewerController.m
//  ForthIOS
//
//  Created by Cao Minh Trang on 1/28/16.
//  Copyright © 2016 Cao Minh Trang. All rights reserved.
//

#import "ForthViewerController.h"
#include "streaming_engine.hpp"

using namespace oppvs;

@interface ForthViewerController ()
{
    StreamingEngine mStreamingEngine;
    AudioRingBuffer mAudioRingBuffer;
}
@end

@implementation ForthViewerController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)startStreaming:(NSString *)streamKey atServer:(NSString *)serverAddress
{
    NSLog(@"Init forth streaming engine");
    
    mStreamingEngine.registerCallback(frameCallback, (__bridge void*)self.frameView);
    mStreamingEngine.registerCallback(streamingCallback, (__bridge void*)self);
    mStreamingEngine.attachBuffer(&mAudioRingBuffer);
    
    std::string strServerAddress([serverAddress UTF8String]);
    if (mStreamingEngine.init(ROLE_VIEWER, strServerAddress, strServerAddress, TURN_SERVER_USER, TURN_SERVER_PASS, strServerAddress, SIGN_SERVER_PORT) < 0)
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

#pragma mark Callback functions
void frameCallback(oppvs::PixelBuffer& pf)
{
    uint8_t* data = pf.plane[0];
    uint16_t width = pf.width[0];
    uint16_t height = pf.height[0];
    GLFrameView* view = (__bridge GLFrameView*)pf.user;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [view render:data withWidth:width andHeight:height freeWhenDone:YES];
    });
}

void streamingCallback(void* user)
{
}

@end
