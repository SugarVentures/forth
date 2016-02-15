//
//  ViewController.m
//  ForthBroadcasterIosDemo
//
//  Created by Cao Minh Trang on 1/27/16.
//  Copyright © 2016 Cao Minh Trang. All rights reserved.
//

#import "ViewController.h"
#import "GLFrameView.h"

@interface ViewController ()

@end

@implementation ViewController

BOOL mIsRunning;

- (void)viewDidLoad {
    [super viewDidLoad];
    
    CGRect demoFrame = CGRectMake(0, 0, self.previewView.frame.size.width, self.previewView.frame.size.height);
    self.frameView = [[GLFrameView alloc] initWithFrame: demoFrame];
    self.frameView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;

    [self.previewView addSubview:self.frameView];
    [self.frameView setupGL];
    
    [self startCaptureSession];
    
    mIsRunning = NO;
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)startStreaming:(id)sender {
    //NSString* serverAddress = @"52.76.92.162";
    NSString* serverAddress = @"192.168.1.9";
    [self startStreaming:@"7116f0d7-5c27-44e6-8aa4-bc4ddeea9935" atServer:serverAddress];
    self.streamButton.enabled = NO;
}
@end
