//
//  ViewController.m
//  iosTestRecordingSDK
//
//  Created by Cao Minh Trang on 1/25/16.
//  Copyright © 2016 Cao Minh Trang. All rights reserved.
//

#import "ViewController.h"
#include "ios_video_engine.hpp"

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
    
}


- (void)initEngine
{
    void* user;
    mVideoEngine = new oppvs::IosVideoEngine(frameCallback, user);
    
    //Get information of capture devices
    std::vector<oppvs::VideoCaptureDevice> videoDevices;
    mVideoEngine->getListCaptureDevices(videoDevices);
    if (videoDevices.size() > 0)
    {
        for (std::vector<oppvs::VideoCaptureDevice>::const_iterator it = videoDevices.begin(); it != videoDevices.end(); ++it)
        {
            NSString *deviceName = [NSString stringWithCString:it->device_name.c_str()
                                                      encoding:[NSString defaultCStringEncoding]];
            NSString *deviceId = [NSString stringWithCString:it->device_id.c_str()
                                                    encoding:[NSString defaultCStringEncoding]];
            NSLog(@"%@ %@", deviceName, deviceId);
            
        }
    }
}


@end
