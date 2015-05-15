//
//  ViewController.m
//  OPPVSGUI
//
//  Created by Cao Minh Trang on 3/13/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "ViewController.h"

@interface ViewController ()
{
    
}

@property NSUInteger videoDeviceIndex;
@property NSString* windowInputKey;


@end

@implementation ViewController
{
    NSMutableArray *shadeWindows;
    
    id frameViewID; //Used in capturing custom region to point to the rendering view
}

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
    
    
    /*[hostPreviewLayer setWantsLayer:YES];
    previewView = [CapturePreview layer];
    [previewView setAsynchronous:NO];
    [previewView setNeedsDisplay];
    [hostPreviewLayer setLayer:previewView];*/
    
    [tableView setDataSource:self];
    [tableView setDelegate: self];
    
    //Set target for drop down menu
    [addSourceButton setTarget:self];
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
        [addSourceButton setDataSource:[self listSources]];
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
    //CapturePreview *view = (CapturePreview*)previewView;
    //FrameView *view = (FrameView*)previewView;
    if (range.location != NSNotFound)
    {
        [self setWindowInputs: document.windowCaptureInputs];
        //[view setReverse:false];
    }
    else
    {
        [self setWindowInputs:nil];
        //[view setReverse:true];
    }
    
 
}

- (NSString *) selectedWindowInput {
    return windowInputKey;
}

- (void) setSelectedWindowInput:(NSString *)input
{
    windowInputKey = input;
}


#pragma mark Controlling Buttons

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
    //CapturePreview *view = (CapturePreview*)previewView;
    //[view setReset:true];
}


- (IBAction)AddClick:(id)sender {
    
    FrameView *superview = (FrameView*)hostPreviewLayer;
    int i = [self.listCaptureSources count];
    NSRect frame;
    NSValue *frameid;
    if (i == 0)
    {
        //frame = NSMakeRect(0, 0, 1280, 780);
        frame = [superview bounds];
        frameid = [NSValue valueWithRect: NSMakeRect(0, 0, 1280, 780)];
    }
    else
    {
        frame = NSMakeRect(0, 0, 480, 300);
        frameid = [NSValue valueWithRect: NSMakeRect(0, 0, 480, 300)];
    }
    id user = [superview addWindow:frame];
    
    NSLog(@"%f %f %f %f\n", frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
    
    
    @autoreleasepool {
        NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];
        [dict setObject:self.selectedVideoDevice forKey:kCSName];
        [dict setObject:user forKey:@"User"];
        [dict setObject:frameid forKey:@"Rect"];
        [self.listCaptureSources addObject:dict];
    }
    [tableView reloadData];
    
}

- (void) renderFrame: (oppvs::PixelBuffer*) pf
{
    if (pf != NULL)
    {        
        //CapturePreview *view = (CapturePreview*)previewView;
        //OpenGLFrame *view = (OpenGLFrame*)previewView;
        //[view setPixelBuffer:pf];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [previewView setNeedsDisplay];
        });
    }
   
}

- (void) reset
{
    //CapturePreview *view = (CapturePreview*)previewView;
    //[view setReset: true];
}

- (void) setStreamInfo:(NSString *)info
{
    [serverPort setStringValue:info ];
}

- (IBAction)stopStreaming:(id)sender {
    [self setStreaming:false];
    [document stopStreaming];
    
}

- (IBAction)startStreaming:(id)sender {
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

#pragma mark Setup Custom Region

#define kShadyWindowLevel   (NSDockWindowLevel + 1000)

/* Draws a crop rect on the display. */
- (void)drawMouseBoxView:(DrawMouseBoxView*)view didSelectRect:(NSRect)rect
{
    /* Map point into global coordinates. */
    NSRect globalRect = rect;
    NSRect windowRect = [[view window] frame];
    globalRect = NSOffsetRect(globalRect, windowRect.origin.x, windowRect.origin.y);
    globalRect.origin.y = CGDisplayPixelsHigh(CGMainDisplayID()) - globalRect.origin.y;
    CGDirectDisplayID displayID = CGMainDisplayID();
    uint32_t matchingDisplayCount = 0;
    /* Get a list of online displays with bounds that include the specified point. */
    CGError e = CGGetDisplaysWithPoint(NSPointToCGPoint(globalRect.origin), 1, &displayID, &matchingDisplayCount);
    if ((e == kCGErrorSuccess) && (1 == matchingDisplayCount))
    {
        /* Add the display as a capture input. */
        //[self addDisplayInputToCaptureSession:displayID cropRect:NSRectToCGRect(rect)];
        
    }
    
    for (NSWindow* w in [NSApp windows])
    {
        if ([w level] == kShadyWindowLevel)
        {
            [w close];
        }
    }
    [[NSCursor currentCursor] pop];
    [shadeWindows removeAllObjects];
    
    [document addSource:[NSString stringWithFormat:@"%u", displayID] hasType:oppvs::VST_WINDOW inRect:rect withViewID:frameViewID];
}


- (void)setRegion{
    if(!shadeWindows) {
        shadeWindows = [NSMutableArray array];
    }
    
    for (NSScreen* screen in [NSScreen screens])
    {
        NSRect frame = [screen frame];
        NSWindow * window = [[NSWindow alloc] initWithContentRect:frame styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];
        [window setBackgroundColor:[NSColor blackColor]];
        [window setAlphaValue:.5];
        [window setLevel:kShadyWindowLevel];
        [window setReleasedWhenClosed:NO];
        
        DrawMouseBoxView* drawMouseBoxView = [[DrawMouseBoxView alloc] initWithFrame:frame];
        drawMouseBoxView.delegate = self;
        [window setContentView:drawMouseBoxView];
        [window makeKeyAndOrderFront:self];
        [shadeWindows addObject:window];
    }
    
    [[NSCursor crosshairCursor] push];
}

#pragma mark Manage Capturing Sources

- (IBAction)addSource:(id)sender
{
    NSMenuItem* selectedItem = (NSMenuItem*)sender;
    NSInteger index = selectedItem.tag;
    NSDictionary *source = [[self listSources] objectAtIndex:index];
    if ([[source objectForKey:@"type"] isEqualToString:@"Monitor"])
    {
        //CGRect frame = CGDisplayBounds(displayID);
        int x = arc4random_uniform(50);
        int y = arc4random_uniform(50);
        CGRect frame = CGRectMake(x, y, 400, 400);
        id user = [self addSubView:frame];
        [document addSource:[[source objectForKey:@"id"] stringValue] hasType:oppvs::VST_WINDOW inRect:frame withViewID:user];
    }
    else if ([[source objectForKey:@"type"] isEqualToString:@"Device"])
    {
        int x = arc4random_uniform(50);
        int y = arc4random_uniform(50);
        CGRect frame = CGRectMake(x, y, 400, 400);
        id user = [self addSubView:frame];
        [document addSource: [source objectForKey:@"id"] hasType:oppvs::VST_WEBCAM inRect:frame withViewID:user];
    }
    else if ([[source objectForKey:@"type"] isEqualToString:@"Custom"])
    {
        int x = arc4random_uniform(50);
        int y = arc4random_uniform(50);
        CGRect frame = CGRectMake(x, y, 400, 400);
        id user = [self addSubView:frame];
        frameViewID = user;
        [self setRegion];
    }
}

- (id)addSubView: (NSRect)frame
{
    FrameView *superview = (FrameView*)hostPreviewLayer;
    id user = [superview addWindow:frame];
    return user;
}

@end
