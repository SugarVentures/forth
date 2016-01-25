#include "ios_video_capture_interface.h"

#import <AVFoundation/AVFoundation.h>

@interface IosVideoAVFoundationCapture : NSObject<AVCaptureVideoDataOutputSampleBufferDelegate>
{
@private
	AVCaptureSession *session;
	AVCaptureDeviceInput *videoDeviceInput;
	AVCaptureVideoDataOutput *videoDeviceOutput;

	oppvs::frame_callback callback_frame;
    oppvs::PixelBuffer pixel_buffer;
    void* callback_user;

    int is_pixel_buffer_set;
    int pixel_format;	
}




@end

@implementation IosVideoAVFoundationCapture {

}
@end
