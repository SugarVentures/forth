//
//  Document.m
//  OPPVSGUI
//
//  Created by Cao Minh Trang on 3/13/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "Document.h"
#import <IOKit/graphics/IOGraphicsLib.h>
#include "audio_engine.hpp"

#include <stdlib.h>
#include <cfloat>
#include <cmath>

#define DBOFFSET -74.0
// DBOFFSET is An offset that will be used to normalize
// the decibels to a maximum of zero.
// This is an estimate, you can do your own or construct
// an experiment to find the right value
#define LOWPASSFILTERTIMESLICE .001
// LOWPASSFILTERTIMESLICE is part of the low pass filter
// and should be a small positive value


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
    streamingEngine.setStreamInfo(mVideoEngine->getVideoActiveSources(), mAudioEngine->getAudioActiveSources());
    
    dispatch_queue_t queue = dispatch_queue_create("oppvs.streaming.queue", DISPATCH_QUEUE_SERIAL);
    dispatch_async(queue, ^{
        if (streamingEngine.init(oppvs::ROLE_BROADCASTER, oppvs::STUN_SERVER_ADDRESS, oppvs::TURN_SERVER_ADDRESS, oppvs::TURN_SERVER_USER, oppvs::TURN_SERVER_PASS,
                                 oppvs::SIGN_SERVER_ADDRESS, oppvs::SIGN_SERVER_PORT) < 0)
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

#pragma mark Callback functions

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

static float caculateDecibels(float* samples, int numberFrames)
{
    float decibels = DBOFFSET;
    float currentFilteredValueOfSampleAmplitude, previousFilteredValueOfSampleAmplitude = 0;
    float peakValue = DBOFFSET;
    for (int i=0; i < numberFrames; i++) {
        float absoluteValueOfSampleAmplitude = std::abs(samples[i]) * 100; //Step 2: for each sample,
        // get its amplitude's absolute value.
        
        // Step 3: for each sample's absolute value, run it through a simple low-pass filter
        // Begin low-pass filter
        currentFilteredValueOfSampleAmplitude = LOWPASSFILTERTIMESLICE * absoluteValueOfSampleAmplitude + (1.0 - LOWPASSFILTERTIMESLICE) * previousFilteredValueOfSampleAmplitude;
        previousFilteredValueOfSampleAmplitude = currentFilteredValueOfSampleAmplitude;
        float amplitudeToConvertToDB = currentFilteredValueOfSampleAmplitude;
        // End low-pass filter
        
        float sampleDB = 20.0*log10(amplitudeToConvertToDB) + DBOFFSET;
        // Step 4: for each sample's filtered absolute value, convert it into decibels
        // Step 5: for each sample's filtered absolute value in decibels,
        // add an offset value that normalizes the clipping point of the device to zero.
        
        if((sampleDB == sampleDB) && (sampleDB != -DBL_MAX)) { // if it's a rational number and
            // isn't infinite
            
            if(sampleDB > peakValue) peakValue = sampleDB; // Step 6: keep the highest value
            // you find.
            decibels = peakValue; // final value
        }
    }
    
    return decibels;
    
}


void audioCallback(oppvs::GenericAudioBufferList& ab)
{
    float decibels = caculateDecibels((float*)ab.buffers[0].data, ab.nFrames);
    oppvs::ControllerLinker* linker = (oppvs::ControllerLinker*)ab.user;
    ViewController* view = (__bridge ViewController*)linker->render;
    [view setAudioDecibels: std::abs(decibels)];
    oppvs::StreamingEngine* streamer = (oppvs::StreamingEngine*)linker->streamer;
    if (streamer)
    {
        if (streamer->isRunning())
        {
            streamer->pushData(ab);
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


- (void) addVideoSource:(NSString *)sourceid hasType:(oppvs::VideoSourceType)type sourceRect:(CGRect)srect renderRect:(CGRect)rrect withViewID:(id)viewid atIndex:(NSInteger) index
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

- (void) addAudioSource:(NSString *)sourceid withViewID: (id) viewid
{
    int deviceid = [sourceid intValue];
    oppvs::ControllerLinker *controller = new oppvs::ControllerLinker();
    controller->streamer = &streamingEngine;
    controller->render = (__bridge void*)viewid;

    int result = mAudioEngine->addNewCapture(deviceid, controller);
    if (result < 0)
    {
        printf("Can not add the audio device %d for recording\n", deviceid);
    }
}

- (void) initEngines: (id) userid
{
    std::vector<oppvs::AudioDevice> audioDevices;
    std::vector<oppvs::VideoCaptureDevice> videoDevices;
    std::vector<oppvs::Monitor> monitors;
    
    oppvs::MacAudioEngine* audioEngine = new oppvs::MacAudioEngine;
    audioEngine->callbackAudio = audioCallback;
    
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

#pragma mark - Default functions

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
    delete mAudioEngine;
}



@end
