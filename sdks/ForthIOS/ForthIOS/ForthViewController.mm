//
//  ForthViewController.m
//  ForthIOS
//
//  Created by Cao Minh Trang on 1/27/16.
//  Copyright © 2016 Cao Minh Trang. All rights reserved.
//

#import "ForthViewController.h"

#include "ios_video_engine.hpp"
#import <AVFoundation/AVFoundation.h>

@interface ForthViewController ()
{
    oppvs::IosVideoEngine* mVideoEngine;
    oppvs::ControllerLinker* controller;
}

@end

NSMutableData*   FrameStorage;

@implementation ForthViewController

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

- (void)startCaptureSession
{
    void* user;
    mVideoEngine = new oppvs::IosVideoEngine(frameCallback, user);
    
    std::string source = "1"; //Back Camera
    
    oppvs::window_rect_t sourceRect;
    oppvs::window_rect_t renderRect;
    
    
    controller = new oppvs::ControllerLinker();
    controller->render = (__bridge void*)self.frameView;
    
    oppvs::VideoActiveSource* activeSource = mVideoEngine->addSource(oppvs::VST_WEBCAM, source, 24, sourceRect, renderRect, (void*)controller, 0);
    
    if (activeSource)
    {
        mVideoEngine->setupCaptureSession(activeSource);
        int error = mVideoEngine->startCaptureSession(*activeSource);
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
        dispatch_sync(dispatch_get_main_queue(), ^{
            [view render:(GLubyte*)[FrameStorage bytes] withWidth:pf.width[0] andHeight:pf.height[0]];
        });
    }
}

@end
