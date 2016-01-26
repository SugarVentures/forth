
#include "video_capture.hpp"
#include "ios_video_capture_interface.h"

#import <AVFoundation/AVFoundation.h>

@interface IosVideoSourceInfo : NSObject
- (id) init;
- (void) dealloc;
- (int) getListCaptureDevices : (std::vector<oppvs::VideoCaptureDevice>&) result;	//Return the number of capturing devices
@end

@implementation IosVideoSourceInfo {
	
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
        result.push_back(vcd);
 		index++;
 	}
 	return index;
 }

void* oppvs_vc_info_alloc() {
    return (void*)[[IosVideoSourceInfo alloc] init];
}

int oppvs_get_list_video_sources(void* cap, std::vector<oppvs::VideoCaptureDevice>& result) {
    return [(id)cap getListCaptureDevices: result];
}

 @end