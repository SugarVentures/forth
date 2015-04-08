//
//  ViewController.m
//  OPPVSGUI
//
//  Created by Cao Minh Trang on 3/13/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "ViewController.h"
#import "CapturePreview.h"

@class CapturePreview;

@interface ViewController ()
{
    
}

@property NSUInteger videoDeviceIndex;
@property NSString* windowInputKey;


@end

@implementation ViewController

@synthesize videoDevices;
@synthesize windowInputs;

@synthesize videoDeviceIndex;
@synthesize windowInputKey;

@synthesize previewView;

- (id) init
{
    
    self = [super init];
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    [serverPort setStringValue:@"33432" ];
    
    [hostPreviewLayer setWantsLayer:YES];
    previewView = [CapturePreview layer];
    [previewView setAsynchronous:NO];
    [previewView setNeedsDisplay];
    [hostPreviewLayer setLayer:previewView];
    
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
    
    // Update the view, if already loaded.
    
}

- (void)viewWillAppear
{

    document = [self.view.window.windowController document];
    if (document)
    {
        [self setVideoDevices: document.videoCaptureDevices];
        if (videoDevices.count > 0)
        {
            [self setSelectedVideoDevice:videoDevices[0]];
        }
    }
}

- (NSString *) selectedVideoDevice {
    return [videoDevices objectAtIndex:videoDeviceIndex];
}

- (void) setSelectedVideoDevice:(NSString *)device
{
 
    if (device == nil)
    {
        [self setWindowInputs:nil];
        videoDeviceIndex = 0;
        return;
    }
    videoDeviceIndex = [videoDevices indexOfObject:device];
    NSRange range = [device rangeOfString:@"Screen Capturing" options:NSCaseInsensitiveSearch];
    CapturePreview *view = (CapturePreview*)previewView;
    if (range.location != NSNotFound)
    {
        [self setWindowInputs: document.windowCaptureInputs];
        [view setReverse:false];
    }
    else
    {
        [self setWindowInputs:nil];
        [view setReverse:true];
    }
    
 
}

- (NSString *) selectedWindowInput {
    return windowInputKey;
}

- (void) setSelectedWindowInput:(NSString *)input
{
    windowInputKey = input;
}


- (IBAction)startRecording:(id)sender {
    [self setRecording:true];    
    [document startRecording];
    
}

- (IBAction)stopRecording:(id)sender {
    [self setRecording:false];
    [document stopRecording];
    CapturePreview *view = (CapturePreview*)previewView;
    [view setReset:true];
}


- (IBAction)AddClick:(id)sender {
    NSLog(@"%tu %@", videoDeviceIndex, windowInputKey);
}

- (void) renderFrame: (oppvs::PixelBuffer*) pf
{
    if (pf != NULL)
    {        
        CapturePreview *view = (CapturePreview*)previewView;
        [view setPixelBuffer:pf];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [previewView setNeedsDisplay];
        });
    }
   
}

- (void) reset
{
    CapturePreview *view = (CapturePreview*)previewView;
    [view setReset: true];
}

- (IBAction)stopStreaming:(id)sender {
    [self setStreaming:false];
    [document stopStreaming];
    
}

- (IBAction)startStreaming:(id)sender {
    NSLog(@"Port: %@\n", [serverPort stringValue]);
    [document setSeverPort:[serverPort integerValue]];
    if (!self.recording)
    {
        [self setRecording:true];
        [document startRecording];        
    }
    [self setStreaming:true];
    [document startStreaming];
}

- (void)dealloc
{
}


@end
