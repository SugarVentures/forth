/*
	Get information of video capture sources
*/

#include "mac_video_capture_interface.h"
#include "../interface/video_capture.hpp"

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

@interface MacVideoSourceInfo : NSObject 
- (id) init;
- (void) dealloc;
- (int) getListCaptureDevices : (std::vector<oppvs::VideoCaptureDevice>&) result;	//Return the number of capturing devices
- (int) getListWindows : (std::vector<oppvs::VideoScreenSource>&) result;	//Return the number of windows on screens and the id of each window
- (int) getWindowTitleByID : (std::string&) title : (int) windowid;
- (int) getAppNameByID : (std::string&) appname : (int) appid;
- (int) getCapabilities : (std::vector<oppvs::VideoSourceCapability>&) capabilities forDevice: (AVCaptureDevice*) device;
- (int) getDeviceIDByTitle : (std::string&)  title;
@end

@implementation MacVideoSourceInfo {
	
}

- (id) init {
 	self = [super init];
 	return self;
 }

 - (void) dealloc {
 	[super dealloc];
 }

 - (int) getListCaptureDevices : (std::vector<oppvs::VideoCaptureDevice>&) result {
 	int index = 0;
 	NSArray *devices = [AVCaptureDevice devicesWithMediaType: AVMediaTypeVideo];
 	for (AVCaptureDevice *device in devices) {
 		oppvs::VideoCaptureDevice vcd;
 		NSString* device_id = [device uniqueID];
 		NSString* device_name = [device localizedName];
 		std::string temp_id([device_id UTF8String]);
 		std::string temp_name([device_name UTF8String]);
 		vcd.device_id = temp_id;
 		vcd.device_name = temp_name;
        [self getCapabilities: vcd.capabilities forDevice: device];
        result.push_back(vcd);
 		index++;
 	}
 	return index;
 }

 - (int) getCapabilities : (std::vector<oppvs::VideoSourceCapability>&) capabilities forDevice: (AVCaptureDevice*) device
 {
    if (!device)
        return -1;

    int index = 0;
    for (AVCaptureDeviceFormat *format in [device formats])
    {
        CMMediaType mediaType = CMFormatDescriptionGetMediaType([format formatDescription]);
        if (mediaType != kCMMediaType_Video)
            continue;
        CMVideoDimensions dimensions = CMVideoFormatDescriptionGetDimensions([format formatDescription]);
        oppvs::VideoSourceCapability capability;
        capability.id = index;
        capability.width = dimensions.width;
        capability.height = dimensions.height;

        uint8_t maxFrameRate = 0;
        for (AVFrameRateRange *range in format.videoSupportedFrameRateRanges)
        {
            if (range.maxFrameRate > maxFrameRate)
                maxFrameRate = range.maxFrameRate;
        }
        capability.fps = maxFrameRate;
        capabilities.push_back(capability);
    }
    return 0;
 }

 - (int) getListWindows : (std::vector<oppvs::VideoScreenSource>&) result {
 	int index = 0;
 	CFArrayRef windowList = (CFArrayRef)(CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID));
 	if (!windowList)
 		return index;

 	long count = CFArrayGetCount(windowList);
    for (long i = 0; i < count; i++)
    {
    	CFDictionaryRef window = reinterpret_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(windowList, i));
    	CFStringRef window_name = reinterpret_cast<CFStringRef>(CFDictionaryGetValue(window, kCGWindowName));
    	CFNumberRef window_id_ref = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(window, kCGWindowNumber));
    	CFNumberRef window_owner_pid_ref = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(window, kCGWindowOwnerPID));
    	CFStringRef window_process_name = reinterpret_cast<CFStringRef>(CFDictionaryGetValue(window, kCGWindowOwnerName));

    	if (!window_id_ref || !window_owner_pid_ref || !window_name)
    		continue;

    	int window_owner_pid, window_id;
    	CFNumberGetValue(window_owner_pid_ref, kCFNumberIntType, &window_owner_pid);
    	CFNumberGetValue(window_id_ref, kCFNumberIntType, &window_id);

        oppvs::VideoScreenSource video_source;
        video_source.id = window_id;
        char title[255], apptitle[255];
        CFStringGetCString(window_name, title, sizeof(title), kCFStringEncodingMacRoman);
        video_source.title = title;
        video_source.app_id = window_owner_pid;
        CFStringGetCString(window_process_name, apptitle, sizeof(apptitle), kCFStringEncodingMacRoman);
        video_source.app_name = apptitle;
        result.push_back(video_source);
    	index++;
    	
    }
    CFRelease(windowList);
    return index;
 }

 - (int) getWindowTitleByID : (std::string&) title : (int) id {
    return 1;
 }

 - (int) getAppNameByID : (std::string&) appname : (int) appid {
    return 1;
 }

 - (int) getDeviceIDByTitle : (std::string&) title {
    int index = 0;
    NSString *device_name = [NSString stringWithCString:title.c_str()
                                                           encoding:[NSString defaultCStringEncoding]];
    NSArray *devices = [AVCaptureDevice devicesWithMediaType: AVMediaTypeVideo];
    for (AVCaptureDevice *device in devices) {
        NSRange range = [[device localizedName] rangeOfString:device_name options:NSCaseInsensitiveSearch];
        if (range.location != NSNotFound)
            return index;
        index++;
    }
    
    return -1;
 }

void* oppvs_vc_info_alloc() {
	return (void*)[[MacVideoSourceInfo alloc] init];
}

int oppvs_get_list_video_sources(void* cap, std::vector<oppvs::VideoCaptureDevice>& result) {
	return [(id)cap getListCaptureDevices: result];
}

int oppvs_get_list_windows(void* cap, std::vector<oppvs::VideoScreenSource>& result) {
	return [(id)cap getListWindows: result];
}

int oppvs_get_device_id(void* cap, std::string& title) {
    return [(id)cap getDeviceIDByTitle: title];
    
}

@end