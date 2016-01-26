//
//  ViewController.m
//  iosTestRecordingSDK
//
//  Created by Cao Minh Trang on 1/25/16.
//  Copyright © 2016 Cao Minh Trang. All rights reserved.
//

#import "ViewController.h"
#include "ios_video_engine.hpp"
#import <AVFoundation/AVFoundation.h>

@interface ViewController ()
{
    oppvs::IosVideoEngine* mVideoEngine;
}

- (void)initEngine;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    
    [self initEngine];
    
    
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark Callback functions

void frameCallback(oppvs::PixelBuffer& pf)
{
    if (pf.nbytes == 0)
        return;
    printf("Frame callback\n");
}


- (void)initEngine
{
    void* user;
    mVideoEngine = new oppvs::IosVideoEngine(frameCallback, user);
    
    //Get information of capture devices
    std::vector<oppvs::VideoCaptureDevice> videoDevices;
    mVideoEngine->getListCaptureDevices(videoDevices);
    
    NSString* source;
    if (videoDevices.size() > 0)
    {
        for (std::vector<oppvs::VideoCaptureDevice>::const_iterator it = videoDevices.begin(); it != videoDevices.end(); ++it)
        {
            NSString *deviceName = [NSString stringWithCString:it->device_name.c_str()
                                                      encoding:[NSString defaultCStringEncoding]];
            NSString *deviceId = [NSString stringWithCString:it->device_id.c_str()
                                                    encoding:[NSString defaultCStringEncoding]];
            NSLog(@"%@ %@", deviceName, deviceId);
            source = deviceId;
        }
    }
    
    CGRect rect;
    [self addVideoSource:source hasType:oppvs::VST_WEBCAM sourceRect:rect renderRect:rect withViewID:nil atIndex:0];
    
}

- (void) addVideoSource:(NSString *)sourceid hasType:(oppvs::VideoSourceType)type sourceRect:(CGRect)srect renderRect:(CGRect)rrect withViewID:(id)viewid atIndex:(NSInteger) index
{
    std::string source = [sourceid UTF8String];
    oppvs::window_rect_t sourceRect = createFromCGRect(srect);
    oppvs::window_rect_t renderRect = createFromCGRect(rrect);
    
    oppvs::VideoActiveSource *activeSource;
    oppvs::ControllerLinker *controller = new oppvs::ControllerLinker();
    
    activeSource = mVideoEngine->addSource(type, source, 24, sourceRect, renderRect, (void*)controller, (int)index);
    if (activeSource)
    {
        mVideoEngine->setupCaptureSession(activeSource);
        mVideoEngine->startCaptureSession(*activeSource);
    }
    else
        NSLog(@"Failed to add capture source");
    
}

#pragma mark Utilities

static oppvs::window_rect_t createFromCGRect(CGRect rect)
{
    oppvs::window_rect_t out;
    out.left = rect.origin.x;
    out.bottom = rect.origin.y;
    out.right = rect.origin.x + rect.size.width;
    out.top = rect.origin.y + rect.size.height;
    return out;
}

@end
