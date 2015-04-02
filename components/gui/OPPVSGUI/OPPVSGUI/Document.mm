//
//  Document.m
//  OPPVSGUI
//
//  Created by Cao Minh Trang on 3/13/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "Document.h"

oppvs::Stream* oppvsStream;

@interface Document ()
{
@private
    NSViewController* viewController;
}
@end

@implementation Document

@synthesize videoCaptureDevices;
@synthesize windowCaptureInputs;

- (instancetype)init {
    self = [super init];
    oppvsStream = NULL;
    return self;
}

- (void) startRecording
{
    oppvs::window_rect_t rect(0, 0, 500, 500); //Currently not use yet
    videoEngine->stopRecording();
    videoEngine->removeAllSource();
    ViewController* view = (ViewController*)viewController;
    [view reset];
    
    NSRange range = [[view selectedVideoDevice] rangeOfString:@"Screen Capturing" options:NSCaseInsensitiveSearch];
    if (range.location != NSNotFound)
        videoEngine->addSource(oppvs::VST_WINDOW, 0, 1, rect);
    else
    {
        std::string title([[view selectedVideoDevice] UTF8String]);
        int index = videoEngine->getDeviceID(title);
        if (index < 0)
        {
            NSLog(@"Device not found\n");
            return;
        }
        videoEngine->addSource(oppvs::VST_WEBCAM, index, 1, rect);
    }
    videoEngine->setupCaptureSessions();
    videoEngine->startRecording();
}

- (void) stopRecording
{
    videoEngine->stopRecording();
    
}

- (void) startStreaming
{
    int status;
    oppvs::StreamSetting ss;
    ss.port = (int)self.severPort;

    oppvsStream = initStream();
    oppvsStream->setting(ss);
    status = oppvsStream->initServer();
    if (status < 0)
        NSLog(@"Failed to create signaling server\n");
}

- (void) stopStreaming
{
    delete oppvsStream;
}

void frameCallback(oppvs::PixelBuffer& pf)
{
    if (pf.nbytes == 0)
        return;

    ViewController* view = (__bridge ViewController*)pf.user;
    
    if (view.isStreaming)
        oppvsStream->pushData(pf);
    
    if (view.isRecording)
        [view renderFrame: &pf];
}

oppvs::MacVideoEngine* initVideoEngine(id document, id view)
{
    oppvs::MacVideoEngine *ve;
    std::vector<oppvs::VideoCaptureDevice> devices;
    std::vector<oppvs::VideoScreenSource> windows;
    void* user = (__bridge void*)view;
    
    ve = new oppvs::MacVideoEngine(frameCallback, user);
    if (!ve)
        return NULL;
    ve->getListCaptureDevices(devices);
    if (devices.size() > 0)
    {
        @autoreleasepool {
            NSMutableArray *nsdevices = [[NSMutableArray alloc] init];
            [nsdevices addObject: @"Screen Capturing"];
            for (std::vector<oppvs::VideoCaptureDevice>::const_iterator it = devices.begin(); it != devices.end(); ++it)
            {
                NSString *device_name = [NSString stringWithCString:it->device_name.c_str()
                                                           encoding:[NSString defaultCStringEncoding]];
                [nsdevices addObject:device_name];
            }
            
            [document setVideoCaptureDevices: nsdevices];
        }
    }
    
    ve->getListVideoSource(windows);
    if (windows.size() > 0)
    {
        @autoreleasepool {
            NSMutableDictionary *nswindows = [[NSMutableDictionary alloc] init];
            for (std::vector<oppvs::VideoScreenSource>::const_iterator it = windows.begin(); it != windows.end(); ++it)
            {
                
                NSString *window_name = [NSString stringWithCString:it->title.c_str() encoding:[NSString defaultCStringEncoding]];
                NSString *app_name = [NSString stringWithCString:it->app_name.c_str() encoding:[NSString defaultCStringEncoding]];
                NSString *title;
                if ([window_name length] == 0 && [app_name length] == 0)
                    continue;
                if ([window_name length] == 0)
                {
                    title = app_name;
                }
                else
                {
                    title = [NSString stringWithFormat:@"%@ - %@", app_name, window_name];
                }
                [nswindows setObject: title forKey: [NSString stringWithFormat:@"%d", it->id]];
            }
            
            [document setWindowCaptureInputs:nswindows];
        }
        
    }
    devices.clear();
    windows.clear();
    return ve;
}


oppvs::Stream* initStream()
{
    oppvs::Stream *stream = new oppvs::Stream();
    return stream;
}

- (void)windowControllerDidLoadNib:(NSWindowController *)aController {
    [super windowControllerDidLoadNib:aController];
    // Add any code here that needs to be executed once the windowController has loaded the document's window.
    
}

+ (BOOL)autosavesInPlace {
    return YES;
}

- (void)makeWindowControllers {
    // Override to return the Storyboard file name of the document.
    [self addWindowController:[[NSStoryboard storyboardWithName:@"Main" bundle:nil] instantiateControllerWithIdentifier:@"Document Window Controller"]];
    viewController = self.windowForSheet.contentViewController;
    videoEngine = initVideoEngine(self, viewController);
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError {
    // Insert code here to write your document to data of the specified type. If outError != NULL, ensure that you create and set an appropriate error when returning nil.
    // You can also choose to override -fileWrapperOfType:error:, -writeToURL:ofType:error:, or -writeToURL:ofType:forSaveOperation:originalContentsURL:error: instead.
    [NSException raise:@"UnimplementedMethod" format:@"%@ is unimplemented", NSStringFromSelector(_cmd)];
    return nil;
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError {
    // Insert code here to read your document from the given data of the specified type. If outError != NULL, ensure that you create and set an appropriate error when returning NO.
    // You can also choose to override -readFromFileWrapper:ofType:error: or -readFromURL:ofType:error: instead.
    // If you override either of these, you should also override -isEntireFileLoaded to return NO if the contents are lazily loaded.
    [NSException raise:@"UnimplementedMethod" format:@"%@ is unimplemented", NSStringFromSelector(_cmd)];
    return YES;
}

- (void)dealloc
{
    
    
}

@end
