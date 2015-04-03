//
//  ViewController.m
//  OPPVSClient
//
//  Created by Cao Minh Trang on 3/27/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "ViewController.h"
#import "RenderingLayer.h"


@implementation ViewController

@synthesize previewView;

- (id) init
{
    
    self = [super init];
    nbytes = 0;
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
        if (pf->nbytes != nbytes)
        {
            localbuf.width[0] = pf->width[0];
            localbuf.height[0] = pf->height[0];
            localbuf.nbytes = pf->nbytes;
            delete [] localbuf.plane[0];
            localbuf.plane[0] = new uint8_t[pf->nbytes];
            nbytes = localbuf.nbytes;
        }
        if (nbytes == 0)
            return;
        
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
