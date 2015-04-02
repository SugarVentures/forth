//
//  ViewController.m
//  OPPVSClient
//
//  Created by Cao Minh Trang on 3/27/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "ViewController.h"
#import "RenderingLayer.h"

oppvs::PixelBuffer localbuf;

@implementation ViewController

@synthesize previewView;

- (id) init
{
    
    self = [super init];
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [hostPreviewLayer setWantsLayer:YES];
    previewView = [RenderingLayer layer];
    [previewView setAsynchronous:NO];
    [previewView setNeedsDisplay];
    [hostPreviewLayer setLayer:previewView];
    
    [serverIP setStringValue:@"127.0.0.1"];
    [serverPort setStringValue:@"33432"];
    
    localbuf.width[0] = 1280;
    localbuf.height[0] = 720;
    localbuf.nbytes = 3686400;
    localbuf.plane[0] = new uint8_t[localbuf.nbytes];
    
}

- (void)viewWillAppear
{
    
    document = [self.view.window.windowController document];
}


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

- (void) renderFrame: (oppvs::PixelBuffer*) pf
{
    if (pf != NULL)
    {
        memcpy(localbuf.plane[0], pf->plane[0], pf->nbytes);
        RenderingLayer *view = (RenderingLayer*)previewView;
        [view setPixelBuffer:&localbuf];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [previewView setNeedsDisplay];
        });
    }
    
}


- (IBAction)Play:(id)sender {
    NSString* ip = [serverIP stringValue];
    NSInteger port = [serverPort integerValue];
    [document initReceiver:ip withPort:port];
}

- (IBAction)rotate:(id)sender {
    RenderingLayer *view = (RenderingLayer*)previewView;
    [view setReverse:!view.reverse];
    
}
@end
