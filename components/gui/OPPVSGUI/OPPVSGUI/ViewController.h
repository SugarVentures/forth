//
//  ViewController.h
//  OPPVSGUI
//
//  Created by Cao Minh Trang on 3/13/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Document.h"
#import "DrawMouseBoxView.h"
#import "FrameView.h"
#import "DropDownMenu.h"

@class Document;


@interface ViewController : NSViewController<NSTableViewDataSource, NSTableViewDelegate, DrawMouseBoxViewDelegate>
{
@private
    NSArray *videoDevices;
    NSDictionary *windowInputs;
    
    NSString *selectedVideoDevice;
    NSString *selectedWindowInput;
    Document* document;
    
    IBOutlet NSTextField *streamKey;
    
    IBOutlet NSView *hostPreviewLayer;
    
    IBOutlet NSTableView *tableView;
    
    IBOutlet DropDownMenu *addSourceButton;
   
}
@property (strong) IBOutlet NSMenu *statusMenu;

@property (retain) NSMutableArray *listSources;

@property (retain) NSArray *videoDevices;
@property (retain) NSDictionary *windowInputs;

@property (assign) NSString *selectedVideoDevice;
@property (assign) NSString *selectedWindowInput;

@property (retain) NSMutableArray *listCaptureSources;

@property (assign,getter=isRecording) BOOL recording;
@property (assign,getter=isStreaming) BOOL streaming;

@property (assign) IBOutlet NSLevelIndicator *audioLevelMeter;
@property (assign) Float32 audioDecibels;

@property BOOL hidden;


- (IBAction)AddClick:(id)sender;
- (void) renderFrame: (oppvs::PixelBuffer*) pixelBuffer;
- (IBAction)startStreaming:(id)sender;
- (IBAction)stopStreaming:(id)sender;
- (void) reset;
- (void) setRegion;
- (IBAction) addSource:(id)sender;
- (IBAction)showPreview:(id)sender;

- (void)cleanup;

@end

