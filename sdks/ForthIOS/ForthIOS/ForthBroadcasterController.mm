//
//  ForthBroadcasterController.m
//  ForthIOS
//
//  Created by Cao Minh Trang on 1/27/16.
//  Copyright © 2016 Cao Minh Trang. All rights reserved.
//

#import "ForthBroadcasterController.h"

#include "ios_video_engine.hpp"
#include "streaming_engine.hpp"
#import <AVFoundation/AVFoundation.h>

@interface ForthBroadcasterController ()
{
    oppvs::IosVideoEngine* mVideoEngine;
    oppvs::ControllerLinker* controller;
    oppvs::StreamingEngine mStreamingEngine;
}

@end

NSMutableData*   FrameStorage;

@implementation ForthBroadcasterController

- (void)viewDidLoad {
    [super viewDidLoad];
    FrameStorage = [[NSMutableData alloc] init];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)viewDidDisappear:(BOOL)animated
{
    if (mVideoEngine)
    {
        oppvs::VideoActiveSource* activeSource = mVideoEngine->getSource();
        if (activeSource)
            mVideoEngine->stopCaptureSession(*activeSource);
    }
    
    [super viewDidDisappear:animated];
}

- (int) startCaptureSession
{
    void* user;
    mVideoEngine = new oppvs::IosVideoEngine(frameCallback, user);
    
    std::string source = "0"; //Back Camera
    
    oppvs::window_rect_t sourceRect;
    sourceRect.left = 0;
    sourceRect.right = 640;
    sourceRect.bottom = 0;
    sourceRect.top = 480;
    
    controller = new oppvs::ControllerLinker();
    controller->render = (__bridge void*)self.frameView;
    controller->streamer = &mStreamingEngine;
    
    oppvs::VideoActiveSource* activeSource = mVideoEngine->addSource(oppvs::VST_WEBCAM, source, 24, sourceRect, sourceRect, (void*)controller, 0);
    
    if (activeSource)
    {
        mVideoEngine->setupCaptureSession(activeSource);
        return mVideoEngine->startCaptureSession(*activeSource);
    }
    else
        return 0;
}

- (void)startStreaming:(NSString *)streamKey atServer:(NSString *)serverAddress
{
    if (mVideoEngine)
    {
        mStreamingEngine.setStreamInfo(mVideoEngine->getVideoActiveSources());
        
        dispatch_queue_t queue = dispatch_queue_create("oppvs.streaming.queue", DISPATCH_QUEUE_SERIAL);
        dispatch_async(queue, ^{
            std::string strServerAddress([serverAddress UTF8String]);
            if (mStreamingEngine.init(oppvs::ROLE_BROADCASTER, strServerAddress, strServerAddress, oppvs::TURN_SERVER_USER, oppvs::TURN_SERVER_PASS, strServerAddress, oppvs::SIGN_SERVER_PORT) < 0)
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
        });
    }
}

#pragma mark Callback functions

void frameCallback(oppvs::PixelBuffer& pf)
{
    if (pf.nbytes == 0)
        return;
    
    if ((UInt32)[FrameStorage length] < pf.nbytes)
    {
        [FrameStorage increaseLengthBy:pf.nbytes - [FrameStorage length]];
    }
    [FrameStorage replaceBytesInRange:NSMakeRange(0, pf.nbytes) withBytes:pf.plane[0] length:pf.nbytes];
    
    oppvs::ControllerLinker *controller = (oppvs::ControllerLinker*)pf.user;
    if (controller)
    {
        GLFrameView* view = (__bridge GLFrameView*)controller->render;
        dispatch_async(dispatch_get_main_queue(), ^{
            [view render:(GLubyte*)[FrameStorage bytes] withWidth:pf.width[0] andHeight:pf.height[0]];
        });
        
        oppvs::StreamingEngine* streamer = (oppvs::StreamingEngine*)controller->streamer;
        if (streamer)
        {
            if (streamer->isRunning())
            {
                streamer->pushData(pf);
            }
        }

    }
}

@end
