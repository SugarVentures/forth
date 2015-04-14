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

NSString* kCSName = @"CSName";

- (id) init
{
    
    self = [super init];
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    [serverPort setStringValue:@"33432" ];
    
    /*[hostPreviewLayer setWantsLayer:YES];
    previewView = [CapturePreview layer];
    [previewView setAsynchronous:NO];
    [previewView setNeedsDisplay];
    [hostPreviewLayer setLayer:previewView];*/
    
    [tableView setDataSource:self];
    [tableView setDelegate: self];
    
    
    NSMutableArray *array = [[NSMutableArray alloc] init];
    [self setListCaptureSources: array];
    
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
    if ([self.listCaptureSources count] == 0)
    {
        NSLog(@"Please choose at least a catpure device\n");
        return;
    }
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
    VideoPreview *superview = (VideoPreview*)hostPreviewLayer;
    int i = [self.listCaptureSources count];
    NSRect frame;
    if (i == 0)
        frame = [superview bounds];
    else
        frame = NSMakeRect(200, 200, 300, 200);
    //NSButton *button = [[NSButton alloc] initWithFrame:frame];
    //VideoPreview* pv = [[VideoPreview alloc] initWithFrame:frame];
    //[button setTitle:@"Click me!"];
    //[superview addSubview:pv];
    //[superview addSubview:button];
    id user = [superview addWindow:frame];

    
    @autoreleasepool {
        NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];
        [dict setObject:self.selectedVideoDevice forKey:kCSName];
        [dict setObject:user forKey:@"User"];
        [self.listCaptureSources addObject:dict];
    }
    [tableView reloadData];
    
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

- (NSInteger)numberOfRowsInTableView:(NSTableView *)atableView {
    return [self.listCaptureSources count];
}

- (NSView*)tableView:(NSTableView *)atableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSString *identifier = [tableColumn identifier];
    NSMutableDictionary *dict = [self.listCaptureSources objectAtIndex:row];
    NSTableCellView *result = [atableView makeViewWithIdentifier:kCSName owner:self];
    if (result == nil)
    {
        result = [[NSTableCellView alloc] init];
        result.identifier = kCSName;
    }
    result.textField.stringValue = [dict valueForKey:identifier];

    return result;
}




@end
