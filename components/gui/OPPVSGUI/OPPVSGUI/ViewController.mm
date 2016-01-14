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
@property (assign) NSTimer *audioLevelTimer;

@end

@implementation ViewController
{
    NSMutableArray *shadeWindows;
    CGFloat scaleFactor;
    NSStatusItem *statusItem;
    NSStatusBarButton *statusButton;
    
    bool isBackground;
}

@synthesize videoDevices;
@synthesize windowInputs;

@synthesize videoDeviceIndex;
@synthesize windowInputKey;

@synthesize audioLevelMeter;
@synthesize audioLevelTimer;
@synthesize audioDecibels;

NSString* kCSName = @"CSName";

- (id) init
{
    
    self = [super init];
    isBackground = false;
    audioDecibels = 0.0;
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    NSRect bounds = [hostPreviewLayer bounds];
    scaleFactor = oppvs::DEFAULT_VIDEO_FRAME_HEIGHT / bounds.size.height;
    
    [tableView setDataSource:self];
    [tableView setDelegate: self];
    
    NSMutableArray *array = [[NSMutableArray alloc] init];
    [self setListCaptureSources:array];
    
    //Set target for drop down menu
    [addSourceButton setTarget:self];
    
    // Start updating the audio level meter
    [self setAudioLevelTimer:[NSTimer scheduledTimerWithTimeInterval:0.1f target:self selector:@selector(updateAudioLevels:) userInfo:nil repeats:YES]];
    
    [streamKey setStringValue:@"7116f0d7-5c27-44e6-8aa4-bc4ddeea9935"];
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
        if (videoDevices.count > 0)
        {
            [self setSelectedVideoDevice:videoDevices[0]];
        }
        [addSourceButton setDataSource:[self listSources]];
        
        FrameView *superview = (FrameView*)hostPreviewLayer;
        [superview setBackingScaleFactor:document.windowForSheet.backingScaleFactor];
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
    if (range.location != NSNotFound)
    {
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
    unsigned long i = [self.listCaptureSources count];
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
}

- (void) reset
{
    
}


- (IBAction)stopStreaming:(id)sender {
    [self setStreaming:false];
    [document stopStreaming];
    
    [self.view.window makeKeyAndOrderFront:nil];
    statusItem = nil;
}

- (IBAction)startStreaming:(id)sender {
    if (streamKey.stringValue && streamKey.stringValue.length > 0)
    {
        [self setStreaming:true];
        [document startStreaming: [streamKey stringValue]];
        if (self.hidden)
        {
            [self hideWindow: sender];
            [self activateStatusMenu];
        }
    }
    else
        NSLog(@"Please enter stream key");
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
    bool hasRegion = false;
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
        if (rect.size.width > 0 && rect.size.height > 0)
            hasRegion = true;
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
    
    if (hasRegion)
    {
        CGRect renderFrame;
        unsigned int width = rect.size.width;

        if (isBackground)
            renderFrame = CGRectMake(0, 0, [hostPreviewLayer bounds].size.width, [hostPreviewLayer bounds].size.height);
        else
            renderFrame = CGRectMake(0, 0, 400, 300);

        if (width + rect.origin.x > CGDisplayPixelsWide(displayID))
            width = (unsigned int)CGDisplayPixelsWide(displayID) - rect.origin.x;
        
        if (width % 20)
        {
            width = (width/20) * 20;
        }
        
        CGRect sourceFrame = CGRectMake(rect.origin.x, rect.origin.y, width, ceil(rect.size.height));
        //CGRect outrect = CGRectMake(rect.origin.x, rect.origin.y, 600, 588);
        OpenGLFrame *user = [self addSubView:renderFrame];
        [document addVideoSource:[NSString stringWithFormat:@"%u", displayID] hasType:oppvs::VST_CUSTOM sourceRect:sourceFrame renderRect:renderFrame withViewID:user atIndex: user.order];
        
        @autoreleasepool {
            NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];
            [dict setObject:@"Test" forKey:@"id"];
            [[self listCaptureSources] addObject:dict];
        }
    }
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
    int x, y;
    OpenGLFrame* user;
    CGRect sourceFrame;
    CGRect renderFrame;
    if ((unsigned long)[[self listCaptureSources] count] == 0)
    {
        x = 0;
        y = 0;
        isBackground = true;
        sourceFrame = CGRectMake(0, 0, oppvs::DEFAULT_VIDEO_FRAME_WIDTH, oppvs::DEFAULT_VIDEO_FRAME_HEIGHT);
        renderFrame = CGRectMake(x, y, [hostPreviewLayer bounds].size.width, [hostPreviewLayer bounds].size.height);
    }
    else
    {
        isBackground = false;
        x = arc4random_uniform(50);
        y = arc4random_uniform(50);
        renderFrame = CGRectMake(x, y, 400/scaleFactor + x, 300/scaleFactor + y);
        sourceFrame = CGRectMake(0, 0, 400, 300);
    }
    
    if ([[source objectForKey: kMenuItemKeyType] isEqualToString: kMenuItemValueMonitor])
    {
        user = [self addSubView:renderFrame];
        [document addVideoSource:[[source objectForKey: kMenuItemKeyId] stringValue] hasType:oppvs::VST_WINDOW sourceRect:sourceFrame renderRect:renderFrame withViewID:user atIndex:user.order];
        [[self listCaptureSources] addObject: source];
    }
    else if ([[source objectForKey: kMenuItemKeyType] isEqualToString: kMenuItemValueDevice])
    {
        user = [self addSubView:renderFrame];
        
        [document addVideoSource: [source objectForKey: kMenuItemKeyId] hasType:oppvs::VST_WEBCAM sourceRect:sourceFrame renderRect:renderFrame withViewID:user atIndex:user.order];
        [[self listCaptureSources] addObject: source];
    }
    else if ([[source objectForKey: kMenuItemKeyType] isEqualToString: kMenuItemValueCustom])
    {
        [self setRegion];
    }
    else if ([[source objectForKey: kMenuItemKeyType] isEqualToString: kMenuItemValueAudio])
    {
        [document addAudioSource: [[source objectForKey: kMenuItemKeyId] stringValue] withViewID: self];
    }
    
}

- (id)addSubView: (NSRect)frame
{
    FrameView *superview = (FrameView*)hostPreviewLayer;
    id user = [superview addWindow:frame];
    return user;
}

#pragma mark Status Bar

- (void)activateStatusMenu
{
    NSStatusBar *bar = [NSStatusBar systemStatusBar];
    
    statusItem = [bar statusItemWithLength:NSVariableStatusItemLength];
    statusButton = statusItem.button;

    [statusButton setTitle:@"OPPVS"];
    [statusButton setAction:@selector(popStatusMenu:)];
    [statusButton setTarget:self];
    
    [statusButton setMenu:[self statusMenu]];
    
}

- (void)popStatusMenu: (id)sender
{
    NSPoint loc = NSMakePoint([statusButton frame].origin.x, [statusButton frame].origin.y +
                              [statusButton frame].size.height + 5);

    [[self statusMenu] popUpMenuPositioningItem:nil atLocation:loc inView:statusButton];
}

- (void)hideWindow: (id)sender
{
    [self.view.window orderOut:nil];
}

- (IBAction)showPreview:(id)sender
{
    [self.view.window makeKeyAndOrderFront:nil];
    statusItem = nil;
}

- (void) cleanup
{
    
}

#pragma mark - Audio Preview
- (void)updateAudioLevels: (NSTimer*)timer
{
    float decibels = [self audioDecibels] / 10;
    [[self audioLevelMeter] setFloatValue: decibels];
}


@end
