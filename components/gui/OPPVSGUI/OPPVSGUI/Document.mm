//
//  Document.m
//  OPPVSGUI
//
//  Created by Cao Minh Trang on 3/13/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "Document.h"
#import <IOKit/graphics/IOGraphicsLib.h>

bool isStreaming;

@interface Document ()
{
@private
    NSViewController* viewController;
    oppvs::StreamingEngine streamingEngine;
}
@end

@implementation Document


- (instancetype)init {
    self = [super init];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(cleanup)
                                                 name:NSApplicationWillTerminateNotification
                                               object:nil];
    return self;
}

- (void) startRecording
{
    
    ViewController* view = (ViewController*)viewController;
    [view reset];
    
}

- (void) stopRecording
{
    mVideoEngine->stopRecording();
    mVideoEngine->removeAllSource();


}

- (void) startStreaming: (NSString*) streamKey
{
    streamingEngine.setStreamInfo(mVideoEngine->getVideoActiveSources());
    
    dispatch_queue_t queue = dispatch_queue_create("oppvs.streaming.queue", DISPATCH_QUEUE_SERIAL);
    dispatch_async(queue, ^{
        if (streamingEngine.init(oppvs::ROLE_BROADCASTER, "52.76.92.162", "52.76.92.162", "turn", "password",
                                 "52.76.92.162", 33333) < 0)
        {
            NSLog(@"Failed to init streaming engine");
            return;
        }
        
        std::string strStreamKey([streamKey UTF8String]);
        if (streamingEngine.start(strStreamKey) < 0)
        {
            NSLog(@"Failed to start streaming engine");
            return;
        }
    });

}

- (void) stopStreaming
{
    
}

void frameCallback(oppvs::PixelBuffer& pf)
{
    if (pf.nbytes == 0)
        return;
    oppvs::ControllerLinker* controller = (oppvs::ControllerLinker*)pf.user;
    
    OpenGLFrame* renderingView = (__bridge OpenGLFrame*)controller->render;
    [renderingView setPixelBuffer:pf.plane[0]];
    [renderingView setFrameWidth:pf.width[0]];
    [renderingView setFrameHeight:pf.height[0]];
    [renderingView setStride:pf.stride[0]];
    [renderingView setIndexTexture:pf.source];

    pf.order = (uint8_t)renderingView.order;
    dispatch_async(dispatch_get_main_queue(), ^{
        [renderingView setNeedsDisplay];
    });
    
    oppvs::StreamingEngine* streamer = (oppvs::StreamingEngine*)controller->streamer;
    if (streamer)
    {
        if (streamer->isRunning())
        {
            streamer->pushData(pf);
        }
    }
}

#pragma mark Utilities

static oppvs::window_rect_t createFromCGRect(CGRect rect)
{
    oppvs::window_rect_t out;
    out.left = rect.origin.x;
    out.bottom = rect.origin.y;
    out.right = rect.origin.x + rect.size.width;
    out.top = rect.origin.y + rect.size.height;
    return out;
}

// Returns the io_service_t corresponding to a CG display ID, or 0 on failure.
// The io_service_t should be released with IOObjectRelease when not needed.
//
static io_service_t IOServicePortFromCGDisplayID(CGDirectDisplayID displayID)
{
    io_iterator_t iter;
    io_service_t serv, servicePort = 0;
    
    CFMutableDictionaryRef matching = IOServiceMatching("IODisplayConnect");
    
    // releases matching for us
    kern_return_t err = IOServiceGetMatchingServices(kIOMasterPortDefault,
                                                     matching,
                                                     &iter);
    if (err)
        return 0;
    
    while ((serv = IOIteratorNext(iter)) != 0)
    {
        CFDictionaryRef info;
        CFIndex vendorID, productID, serialNumber = 0;
        CFNumberRef vendorIDRef, productIDRef, serialNumberRef;
        Boolean success;
        
        info = IODisplayCreateInfoDictionary(serv,
                                             kIODisplayOnlyPreferredName);
        
        vendorIDRef = (CFNumberRef)CFDictionaryGetValue(info, CFSTR(kDisplayVendorID));
        productIDRef = (CFNumberRef)CFDictionaryGetValue(info, CFSTR(kDisplayProductID));
        serialNumberRef = (CFNumberRef)CFDictionaryGetValue(info, CFSTR(kDisplaySerialNumber));
        
        
        success = CFNumberGetValue(vendorIDRef, kCFNumberCFIndexType, &vendorID);
        success &= CFNumberGetValue(productIDRef, kCFNumberCFIndexType, &productID);
        
        if (serialNumberRef)
            success &= CFNumberGetValue(serialNumberRef, kCFNumberCFIndexType, &serialNumber);
        
        if (!success)
        {
            CFRelease(info);
            continue;
        }
        
        // If the vendor and product id along with the serial don't match
        // then we are not looking at the correct monitor.
        // NOTE: The serial number is important in cases where two monitors
        //       are the exact same.
        if (serialNumberRef)
        {
            if (CGDisplayVendorNumber(displayID) != vendorID  ||
                CGDisplayModelNumber(displayID) != productID  ||
                CGDisplaySerialNumber(displayID) != serialNumber)
            {
                CFRelease(info);
                continue;
            }
        }
        else
        {
            if (CGDisplayVendorNumber(displayID) != vendorID ||
                CGDisplayModelNumber(displayID) != productID)
            {
                CFRelease(info);
                continue;
            }
        }
        // The VendorID, Product ID, and the Serial Number all Match Up!
        // Therefore we have found the appropriate display io_service
        servicePort = serv;
        CFRelease(info);
        break;
    }
    
    IOObjectRelease(iter);
    return servicePort;
}

NSString* screenNameForDisplay(CGDirectDisplayID displayID)
{
    NSString *screenName = nil;
    CFStringRef value;
    io_service_t serv = IOServicePortFromCGDisplayID(displayID);
    if (serv == 0)
    {
        return @"Unknown";
    }
    
    CFDictionaryRef deviceInfo = IODisplayCreateInfoDictionary(serv, kIODisplayOnlyPreferredName);
    IOObjectRelease(serv);
    
    CFTypeRef name = CFDictionaryGetValue(deviceInfo, CFSTR(kDisplayProductName));
    if (!name || !CFDictionaryGetValueIfPresent((CFDictionaryRef)name, CFSTR("en_US"), (const void**) &value))
    {
        screenName = @"Built-In Display";
    }
    else
    {
        screenName = (__bridge NSString*)value;
    }
    CFRelease(deviceInfo);
    return screenName;
}

oppvs::MacVideoEngine* initVideoEngine(id document, id view)
{
    oppvs::MacVideoEngine *ve;
    std::vector<oppvs::VideoCaptureDevice> devices;
    std::vector<oppvs::VideoScreenSource> windows;
    std::vector<oppvs::Monitor> monitors;
    
    void* user = (__bridge void*)view;
    
    ve = new oppvs::MacVideoEngine(frameCallback, user);
    if (!ve)
        return NULL;
    
    NSMutableArray *listSources = [[NSMutableArray alloc] init];
    NSMutableDictionary *dashItem = [[NSMutableDictionary alloc] init];
    [dashItem setObject:@" " forKey:@"title"];
    [dashItem setObject:@"Empty" forKey:@"id"];
    [dashItem setObject:@"Dash" forKey:@"type"];
    
    ve->getListMonitors(monitors);
    if (monitors.size() == 0)
        return ve;
    
    for (std::vector<oppvs::Monitor>::const_iterator it = monitors.begin(); it != monitors.end(); ++it)
    {
        NSString *monitorName = screenNameForDisplay(it->id);
        NSMutableDictionary *monitorItem = [[NSMutableDictionary alloc] init];
        [monitorItem setObject:monitorName forKey:@"title"];
        NSNumber *monitorID = [NSNumber numberWithUnsignedLong:it->id];
        [monitorItem setObject:monitorID forKey:@"id"];
        [monitorItem setObject:@"Monitor" forKey:@"type"];
        [listSources addObject: monitorItem];
    }
    
    //Add dash
    [listSources addObject:dashItem];
    
    NSMutableDictionary *customItem = [[NSMutableDictionary alloc] init];
    [customItem setObject:@"Custom region" forKey:@"title"];
    [customItem setObject:@"Empty" forKey:@"id"];
    [customItem setObject:@"Custom" forKey:@"type"];
    [listSources addObject:customItem];
    
    //Add dash
    [listSources addObject:dashItem];

    ve->getListCaptureDevices(devices);
    if (devices.size() > 0)
    {
        for (std::vector<oppvs::VideoCaptureDevice>::const_iterator it = devices.begin(); it != devices.end(); ++it)
        {
            NSString *deviceName = [NSString stringWithCString:it->device_name.c_str()
                                                       encoding:[NSString defaultCStringEncoding]];
            NSString *deviceId = [NSString stringWithCString:it->device_id.c_str()
                                                    encoding:[NSString defaultCStringEncoding]];
            NSMutableDictionary *deviceItem = [[NSMutableDictionary alloc] init];
            [deviceItem setObject:deviceName forKey:@"title"];
            [deviceItem setObject:deviceId forKey:@"id"];
            [deviceItem setObject:@"Device" forKey:@"type"];
            [listSources addObject:deviceItem];
        }
    }
    
    [view setListSources:listSources];
    
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
            
        }
    }
    
    monitors.clear();
    devices.clear();
    windows.clear();
    
    return ve;
}

- (void) addSource:(NSString *)sourceid hasType:(oppvs::VideoSourceType)type sourceRect:(CGRect)srect renderRect:(CGRect)rrect withViewID:(id)viewid atIndex:(NSInteger) index
{
    std::string source = [sourceid UTF8String];
    oppvs::window_rect_t sourceRect = createFromCGRect(srect);
    oppvs::window_rect_t renderRect = createFromCGRect(rrect);
    
    oppvs::VideoActiveSource *activeSource;
    oppvs::ControllerLinker *controller = new oppvs::ControllerLinker();
    controller->streamer = &streamingEngine;
    controller->render = (__bridge void*)viewid;
    activeSource = mVideoEngine->addSource(type, source, 24, sourceRect, renderRect, (void*)controller, (int)index);
    if (activeSource)
    {
        mVideoEngine->setupCaptureSession(activeSource);
        mVideoEngine->startCaptureSession(*activeSource);
    }
    else
        NSLog(@"Failed to add capture source");

}

- (void) initEngines: (id) userid
{
    std::vector<oppvs::AudioDevice> audioDevices;
    std::vector<oppvs::VideoCaptureDevice> videoDevices;
    std::vector<oppvs::Monitor> monitors;
    
    oppvs::MacAudioEngine* audioEngine = new oppvs::MacAudioEngine;
    oppvs::MacVideoEngine* videoEngine = new oppvs::MacVideoEngine(frameCallback, (__bridge void*)userid);
    
    NSMutableArray *listSources = [[NSMutableArray alloc] init];
    
    NSMutableDictionary *dashItem = [[NSMutableDictionary alloc] init];
    [dashItem setObject: kMenuItemValueBlank forKey: kMenuItemKeyTitle];
    [dashItem setObject: kMenuItemValueEmpty forKey: kMenuItemKeyId];
    [dashItem setObject: kMenuItemValueDash forKey: kMenuItemKeyType];
    
    //Get information of monitors
    videoEngine->getListMonitors(monitors);
    if (monitors.size() > 0)
    {
        for (std::vector<oppvs::Monitor>::const_iterator it = monitors.begin(); it != monitors.end(); ++it)
        {
            NSString *monitorName = screenNameForDisplay(it->id);
            NSMutableDictionary *monitorItem = [[NSMutableDictionary alloc] init];
            [monitorItem setObject: monitorName forKey: kMenuItemKeyTitle];
            [monitorItem setObject: [NSNumber numberWithUnsignedLong:it->id] forKey: kMenuItemKeyId];
            [monitorItem setObject: kMenuItemValueMonitor forKey: kMenuItemKeyType];
            [listSources addObject: monitorItem];
        }
        //Add dash
        [listSources addObject:dashItem];
    }
    
    //Custom area
    NSMutableDictionary *customItem = [[NSMutableDictionary alloc] init];
    [customItem setObject: kMenuItemValueCustomRegion forKey: kMenuItemKeyTitle];
    [customItem setObject: kMenuItemValueEmpty forKey: kMenuItemKeyId];
    [customItem setObject: kMenuItemValueCustom forKey: kMenuItemKeyType];
    [listSources addObject:customItem];
    
    //Add dash
    [listSources addObject:dashItem];

    //Get information of capture devices
    videoEngine->getListCaptureDevices(videoDevices);
    if (videoDevices.size() > 0)
    {
        for (std::vector<oppvs::VideoCaptureDevice>::const_iterator it = videoDevices.begin(); it != videoDevices.end(); ++it)
        {
            NSString *deviceName = [NSString stringWithCString:it->device_name.c_str()
                                                      encoding:[NSString defaultCStringEncoding]];
            NSString *deviceId = [NSString stringWithCString:it->device_id.c_str()
                                                    encoding:[NSString defaultCStringEncoding]];
            NSMutableDictionary *deviceItem = [[NSMutableDictionary alloc] init];
            [deviceItem setObject: deviceName forKey: kMenuItemKeyTitle];
            [deviceItem setObject: deviceId forKey: kMenuItemKeyId];
            [deviceItem setObject: kMenuItemValueDevice forKey: kMenuItemKeyType];
            [listSources addObject: deviceItem];
        }
        //Add dash
        [listSources addObject:dashItem];
    }
    
    audioEngine->getListAudioDevices(audioDevices, true);
    if (audioDevices.size() > 0)
    {
        for (std::vector<oppvs::AudioDevice>::const_iterator it = audioDevices.begin(); it != audioDevices.end(); ++it)
        {
            NSString* audioDeviceName = [NSString stringWithCString:it->getDeviceName().c_str() encoding:[NSString defaultCStringEncoding]];
            NSNumber *audioDeviceId = [NSNumber numberWithUnsignedLong:it->getDeviceID()];
            NSMutableDictionary *audioItem = [[NSMutableDictionary alloc] init];
            [audioItem setObject: audioDeviceName forKey: kMenuItemKeyTitle];
            [audioItem setObject: audioDeviceId forKey: kMenuItemKeyId];
            [audioItem setObject: kMenuItemValueAudio forKey: kMenuItemKeyType];
            [listSources addObject: audioItem];
        }
    }
    
    [userid setListSources:listSources];
    mVideoEngine = videoEngine;
    mAudioEngine = audioEngine;
}

#pragma mark Default functions

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
    //mVideoEngine = initVideoEngine(self, viewController);
    [self initEngines: viewController];
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

- (void)cleanup
{
    NSLog(@"Cleanup");
    
    delete mVideoEngine;
    
}



@end
