#include "ios_video_capture_interface.h"

#import <AVFoundation/AVFoundation.h>

@interface IosVideoAVFoundationCapture : NSObject<AVCaptureVideoDataOutputSampleBufferDelegate>
{
@private
	AVCaptureSession *session;
	AVCaptureDeviceInput *videoDeviceInput;
	AVCaptureVideoDataOutput *videoDataOutput;

	oppvs::frame_callback callback_frame;
    oppvs::PixelBuffer pixel_buffer;
    void* callback_user;

    int is_pixel_buffer_set;
    int pixel_format;	
}

- (id)init;


@end

@implementation IosVideoAVFoundationCapture {

}

- (id)init
{
    self = [super init];
    if (self)
    {
        NSLog(@"Init av foundation capture engine\n");
        is_pixel_buffer_set = 0;
    }
    videoDeviceInput = nil;
    videoDataOutput = nil;
    return self;
}

void* oppvs_vc_av_alloc() {
    return (void*)[[IosVideoAVFoundationCapture alloc] init];
}

@end

