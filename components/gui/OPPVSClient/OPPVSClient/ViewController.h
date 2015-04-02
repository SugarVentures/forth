//
//  ViewController.h
//  OPPVSClient
//
//  Created by Cao Minh Trang on 3/27/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Document.h"

@class Document;

@interface ViewController : NSViewController
{
    Document* document;

    IBOutlet NSTextField *serverPort;
    IBOutlet NSTextField *serverIP;
    IBOutlet NSView *hostPreviewLayer;
    CAOpenGLLayer *previewView;
}
- (IBAction)Play:(id)sender;
- (IBAction)rotate:(id)sender;

@property (retain) CAOpenGLLayer *previewView;

- (void) renderFrame: (oppvs::PixelBuffer*) pixelBuffer;

@end

