/*
	Use AVFoundation to capture
*/

#include "../include/datatypes.hpp"
#include "../error/error.hpp"
#include "mac_video_capture_interface.h"

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreFoundation/CoreFoundation.h>
#import <ApplicationServices/ApplicationServices.h> //For saving screenshots

@interface MacVideoAVFoundationCapture : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate, 
    AVCaptureFileOutputDelegate,AVCaptureFileOutputRecordingDelegate> {
@private
    NSMutableArray              *sessionList;
    AVCaptureSession            *session;
    AVCaptureDeviceInput        *videoDeviceInput;
    AVCaptureScreenInput        *screenDeviceInput;
    AVCaptureDevice             *videoDevice;
    AVCaptureVideoDataOutput    *videoDataOuput;
    AVCaptureMovieFileOutput    *captureMovieFileOutput;

    oppvs::frame_callback callback_frame;
    oppvs::PixelBuffer pixel_buffer;
    void* callback_user;

    int is_pixel_buffer_set;
    int pixel_format;

    int windowid;
}


 - (id) init;
 - (void) dealloc;
 - (oppvs::error_video_capture_t) openCaptureDevice;
 - (oppvs::error_video_capture_t) openScreenDevice;
 - (void) closeDevice;
 - (void) startRecording;
 - (void) stopRecording;
 - (void) showPreview;
 - (void) captureOutput: (AVCaptureOutput*) captureOutput didOutputSampleBuffer: (CMSampleBufferRef) sampleBuffer fromConnection: (AVCaptureConnection*) connection; /* Capture callback implementation */
 - (void) setCallback: (oppvs::frame_callback) fc fromuser: (void*) u;
 - (void) setWindowId: (int) wid;
 - (void) setFrameRate: (int) fps;
 - (int) setPixelFormat: (int) pf;
 - (int) listDisplay;
 - (void) saveScreenShot: (CGImageRef) image_ref;
@end

@implementation MacVideoAVFoundationCapture {

}
 - (id) init {
 	self = [super init];
    if (self) {
        NSLog(@"Init av foundation capture engine\n");
        is_pixel_buffer_set = 0;
    }
    videoDeviceInput = nil;
    screenDeviceInput = nil;
    videoDataOuput = nil;
 	return self;
 }
 - (void) dealloc {
    [self closeDevice];
 	[super dealloc];

 }

//For testing
 - (void) saveScreenShot: (CGImageRef) image_ref
 {
    NSURL * url = [NSURL fileURLWithPath:@"/Users/caominhtrang/Desktop/blabla.jpg" isDirectory:NO];
    CGImageDestinationRef fileDest = CGImageDestinationCreateWithURL((__bridge CFURLRef)url,kUTTypeJPEG,1,NULL);
    CGImageDestinationAddImage(fileDest, image_ref, NULL);
    CGImageDestinationFinalize(fileDest);
    CFRelease(fileDest);
 }

 - (int) listDisplay {
    CGDisplayErr err;
    CGDisplayCount displayCount, i;
    CGDirectDisplayID mainDisplay;
    CGDisplayCount maxDisplays = 5;
    CGDirectDisplayID onlineDisplay[maxDisplays];

    mainDisplay = CGMainDisplayID();
    err = CGGetOnlineDisplayList(maxDisplays, onlineDisplay, &displayCount);

    for (i = 0; i < displayCount; i++)
    {
        CGDirectDisplayID did = onlineDisplay[i];
        printf("%-16p %lux%lu %32s", did, CGDisplayPixelsWide(did), CGDisplayPixelsHigh(did),
            (did == mainDisplay) ? "[main display]\n" : "\n");
    }
    return 0;
 }

 - (oppvs::error_video_capture_t) openCaptureDevice {
    // Create session
    session = [[AVCaptureSession alloc] init];
    if(session == nil) {
        printf("Error: cannot create the capture session.\n");
        return (oppvs::ERR_VIDEO_CAPTURE_SESSION_INIT_FAILED);
    }

    //Attach devices
    AVCaptureDevice *device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    NSError* nserror;
    if (device)
    {
        [session beginConfiguration];


        if ([device lockForConfiguration: NULL] == YES)
        {
            device.activeVideoMinFrameDuration = CMTimeMake(1, 15);
            [device unlockForConfiguration];
        }
        videoDevice = device;

        //Setup session
        session.sessionPreset = AVCaptureSessionPreset1280x720;

        //Webcam
        
        AVCaptureDeviceInput *input = [AVCaptureDeviceInput deviceInputWithDevice: videoDevice error:&nserror];
        if (input == nil)
        {
            printf("Error when create device input\n");
            return oppvs::ERR_VIDEO_CAPTURE_INPUT_DEVICE_FAILED;
        }
        if ([session canAddInput: input] == NO)
        {
            printf("Can't add the device input \n");
            return oppvs::ERR_VIDEO_CAPTURE_INPUT_DEVICE_FAILED;
        }
        [session addInput: input];
        self->videoDeviceInput = input;

        
        /*CGDirectDisplayID screen_id = CGMainDisplayID();
        CGRect mainMonitor = CGDisplayBounds(screen_id);
        CGFloat displayHeight = CGRectGetHeight(mainMonitor)/2;
        CGFloat displayWidth = CGRectGetWidth(mainMonitor)/2;
        CGRect cropRect = CGRectMake(0, 0, displayWidth, displayHeight);

        CGImageRef screenShot = CGWindowListCreateImage(cropRect, kCGWindowListOptionIncludingWindow, 69, kCGWindowImageDefault);
        [self saveScreenShot: screenShot];        
        CFRelease(screenShot);

        AVCaptureScreenInput *screen_input = [[[AVCaptureScreenInput alloc] initWithDisplayID: screen_id] autorelease];
        if ([session canAddInput: screen_input] == NO)
        {
            printf("Can't add the screen input \n");
            return oppvs::ERR_VIDEO_CAPTURE_INPUT_DEVICE_FAILED;
        }
        
        [session addInput: screen_input];
        
        [screen_input setCropRect: cropRect];

        int is_error = [self setPixelFormat: oppvs::PF_UYVY422];

        */
        [session commitConfiguration];
        [self showPreview];
    
        AVCaptureVideoDataOutput *output = [[AVCaptureVideoDataOutput alloc] init];
        if (output == nil)
        {
            printf("Error when allocating the video data output \n");
            return oppvs::ERR_VIDEO_CAPTURE_OUTPUT_DEVICE_FAILED;
        }
        if ([session canAddOutput: output] == NO)
        {
            printf("Can't add the output for capturing \n");
            return oppvs::ERR_VIDEO_CAPTURE_OUTPUT_DEVICE_FAILED;
        }

        [output setAlwaysDiscardsLateVideoFrames: YES];
        [session addOutput: output];
        self->videoDataOuput = output;

        dispatch_queue_t queue = dispatch_queue_create("oppvs.videocapture.queue", DISPATCH_QUEUE_SERIAL);
        [videoDataOuput setSampleBufferDelegate:self queue:queue];
        dispatch_release(queue);

        output.videoSettings = [NSDictionary dictionaryWithObject:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA] forKey:(id)kCVPixelBufferPixelFormatTypeKey];
        
        //Add file output (testing only)
        /*captureMovieFileOutput = [[AVCaptureMovieFileOutput alloc] init];
        [captureMovieFileOutput setDelegate: self];
        if ([session canAddOutput: captureMovieFileOutput] == NO)
        {
            printf("Can't add the file ouput for capturing \n");
            return oppvs::ERR_VIDEO_CAPTURE_OUTPUT_DEVICE_FAILED;
        }
        [session addOutput: captureMovieFileOutput];*/
        
    }

    return oppvs::ERR_VIDEO_CAPTURE_NONE;
 }

 - (oppvs::error_video_capture_t) openScreenDevice {
    // Create session
    session = [[AVCaptureSession alloc] init];
    if(session == nil) {
        printf("Error: cannot create the capture session.\n");
        return (oppvs::ERR_VIDEO_CAPTURE_SESSION_INIT_FAILED);
    }
    
    [session beginConfiguration];


    CGDirectDisplayID screen_id = CGMainDisplayID();
    AVCaptureScreenInput *screen_input = [[[AVCaptureScreenInput alloc] initWithDisplayID: screen_id] autorelease];
    if ([session canAddInput: screen_input] == NO)
    {
        printf("Can't add the screen input \n");
        return oppvs::ERR_VIDEO_CAPTURE_INPUT_DEVICE_FAILED;
    }
    screen_input.removesDuplicateFrames = 1;
    
    [session addInput: screen_input];
    screenDeviceInput = screen_input;

    //Setup session
    if ([session canSetSessionPreset:AVCaptureSessionPreset1280x720]) {
        [session setSessionPreset: AVCaptureSessionPreset1280x720];
    }
    else {
        NSLog(@"Cannot set prest\n");
    }
    
    //[self showPreview];

    AVCaptureVideoDataOutput *output = [[AVCaptureVideoDataOutput alloc] init];
    if (output == nil)
    {
        printf("Error when allocating the video data output \n");
        return oppvs::ERR_VIDEO_CAPTURE_OUTPUT_DEVICE_FAILED;
    }
    if ([session canAddOutput: output] == NO)
    {
        printf("Can't add the output for capturing \n");
        return oppvs::ERR_VIDEO_CAPTURE_OUTPUT_DEVICE_FAILED;
    }

    [output setAlwaysDiscardsLateVideoFrames: YES];

    NSDictionary *pixelBufferOptions = [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithDouble:1280], (id)kCVPixelBufferWidthKey,
                              [NSNumber numberWithDouble:720], (id)kCVPixelBufferHeightKey,
                              [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA], (id)kCVPixelBufferPixelFormatTypeKey,
                              nil];
    [output setVideoSettings:pixelBufferOptions];

    [session addOutput: output];
    self->videoDataOuput = output;

    dispatch_queue_t queue = dispatch_queue_create("oppvs.videocapture.queue", DISPATCH_QUEUE_SERIAL);
    [videoDataOuput setSampleBufferDelegate:self queue:queue];
    dispatch_release(queue);

    //output.videoSettings = [NSDictionary dictionaryWithObject:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA] forKey:(id)kCVPixelBufferPixelFormatTypeKey];
    [session commitConfiguration];
    return oppvs::ERR_VIDEO_CAPTURE_NONE;
 }

 - (void) closeDevice {
    if (session != nil)
    {
        if (self->videoDeviceInput != nil)
        {
            [session removeInput: videoDeviceInput];
            //[videoDeviceInput release];
        }

        if (self->screenDeviceInput != nil)
        {
            [session removeInput: screenDeviceInput];
        }

        if (self->videoDataOuput != nil)
        {
            [session removeOutput: videoDataOuput];
            //[videoDataOuput release];
        }
        [session release];
        pixel_buffer.reset();
    }

    is_pixel_buffer_set = 0;
    videoDeviceInput = nil;
    screenDeviceInput = nil;
    videoDataOuput = nil;
    session = nil;

 }
 - (void) showPreview {
    printf("show preview \n");
 }

 - (void) setFrameRate: (int) fps {
    AVCaptureDeviceFormat *device_format;
    AVFrameRateRange *frame_rate;
    if (!videoDevice)
        return;
    for (AVCaptureDeviceFormat *format in [videoDevice formats])
    {
        NSLog(@"Format desc: %@ index: %d \n", CMFormatDescriptionGetExtension([format formatDescription], 
            kCMFormatDescriptionExtension_FormatName), 
        [[videoDevice formats] indexOfObject: format]
        );
        for (AVFrameRateRange *range in format.videoSupportedFrameRateRanges)
        {
            NSLog(@"Max frame rate: %f Min frame rate: %f\n",        
                range.maxFrameRate, range.minFrameRate);
        }
    }
 }

 - (int) setPixelFormat: (int) pf_index {
    AVCaptureDeviceFormat *device_format = [[videoDevice formats] objectAtIndex: pf_index];
    if (device_format)
    {
        printf("Not found the pixel format you want to set \n");
        return 0;
    }
    return 1;
 }

 - (void) startRecording {
    printf("Start recording \n");
    [session startRunning];
    //Create a recording file

    /*char* outfile = strdup([[[NSString stringWithFormat: @"~/Desktop/testoppvs_%i", windowid] stringByStandardizingPath] fileSystemRepresentation]);

    if (outfile)
    {
        int file_descriptor = mkstemp(outfile);
        if (file_descriptor != -1)
        {
            NSString *filenameStr = [[NSFileManager defaultManager] stringWithFileSystemRepresentation:outfile length:strlen(outfile)];
            filenameStr = [filenameStr stringByAppendingPathExtension:@"mov"];            
            NSLog(@"%@ \n", filenameStr);
            if ([[NSFileManager defaultManager] fileExistsAtPath:filenameStr])
            {
                NSError *err;
                if (![[NSFileManager defaultManager] removeItemAtPath:filenameStr error:&err])
                {
                    NSLog(@"Error deleting existing movie %@",[err localizedDescription]);
                }
            }

            //[captureMovieFileOutput startRecordingToOutputFileURL: [NSURL fileURLWithPath: filenameStr] 
            //    recordingDelegate:self];
        }
        remove(outfile);
        free(outfile);
    }*/
    
 }

 - (void) stopRecording {
    printf("Stop recording \n");
    //[captureMovieFileOutput stopRecording];

    if ([session isRunning] == NO)
        return;
    [self closeDevice];
    [session stopRunning];
 }

 - (void) captureOutput: (AVCaptureOutput*) captureOutput didOutputSampleBuffer: (CMSampleBufferRef) sampleBuffer fromConnection: (AVCaptureConnection*) connection {
    CVPixelBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    if (imageBuffer == NULL)
    {
        printf("Return null image buffer \n");
        return;
    }

    /* Fill the pixel_buffer member with some info that won't change per frame. */
    if (0 == is_pixel_buffer_set) {
        printf("CVPixelBufferIsPlanar :%d \n", CVPixelBufferIsPlanar(imageBuffer));
        if (true == CVPixelBufferIsPlanar(imageBuffer)) {
            size_t plane_count = CVPixelBufferGetPlaneCount(imageBuffer);
            if (plane_count > 3) {
                printf("Error: we got a plane count bigger then 3, not supported yet. Stopping.\n");
                exit(EXIT_FAILURE);
            }
            for (size_t i = 0; i < plane_count; ++i) {
                pixel_buffer.width[i] = CVPixelBufferGetWidthOfPlane(imageBuffer, i);
                pixel_buffer.height[i] = CVPixelBufferGetHeightOfPlane(imageBuffer, i);
                pixel_buffer.stride[i] = CVPixelBufferGetBytesPerRowOfPlane(imageBuffer, i);
                pixel_buffer.nbytes += pixel_buffer.stride[i] * pixel_buffer.height[i];


                printf("width: %lu, height: %lu, stride: %lu, nbytes: %lu, plane_count: %lu\n", 
                     pixel_buffer.width[i],
                     pixel_buffer.height[i],
                     pixel_buffer.stride[i],
                     pixel_buffer.nbytes,
                     plane_count);
                }
            }
        else {
            pixel_buffer.width[0] = CVPixelBufferGetWidth(imageBuffer);
            pixel_buffer.height[0] = CVPixelBufferGetHeight(imageBuffer);
            pixel_buffer.stride[0] = CVPixelBufferGetBytesPerRow(imageBuffer);
            pixel_buffer.nbytes = pixel_buffer.stride[0] * pixel_buffer.height[0];
        }
        is_pixel_buffer_set = 1;
    }

    //Lock before processing pixel data with CPU
    CVPixelBufferLockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);

    CGSize imageSize = CVImageBufferGetEncodedSize(imageBuffer);
    pixel_format = oppvs::PF_BGRA32;
    if (oppvs::PF_YUYV422 == pixel_format     /* kCVPixelFormatType_422YpCbCr8_yuvs */
        || oppvs::PF_UYVY422 == pixel_format  /* kCVPixelFormatType_422YpCbCr8 */
        || oppvs::PF_BGRA32 == pixel_format   /* kCVPixelFormatType_32BGRA */
    )
    {
        pixel_buffer.plane[0] = (uint8_t*)CVPixelBufferGetBaseAddress(imageBuffer);
    }
    else if (oppvs::PF_YUVJ420BP == pixel_format    /* kCVPixelFormatType_420YpCbCr8BiPlanarFullRange */
             || oppvs::PF_YUV420BP == pixel_format) /* kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange */
    {
        pixel_buffer.plane[0] = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(imageBuffer, 0);
        pixel_buffer.plane[1] = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(imageBuffer, 1);
    }
    else {
        printf("Error: unhandled or unknown pixel format: %d.\n", pixel_format);
    }

    pixel_buffer.format = oppvs::PF_BGRA32;
    callback_frame(pixel_buffer);
    
    CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);

    
 }

/* Informs the delegate when all pending data has been written to the output file. */
- (void)captureOutput:(AVCaptureFileOutput *)captureOutput didFinishRecordingToOutputFileAtURL:(NSURL *)outputFileURL fromConnections:(NSArray *)connections error:(NSError *)error
{
    if (error) 
    {
        NSLog(@"Error: %@", error);
        return;
    }
    
    
}
 
- (BOOL)captureOutputShouldProvideSampleAccurateRecordingStart:(AVCaptureOutput *)captureOutput
{
    // We don't require frame accurate start when we start a recording. If we answer YES, the capture output
    // applies outputSettings immediately when the session starts previewing, resulting in higher CPU usage
    // and shorter battery life.
    return NO;
}

- (void) setWindowId : (int) wid {
    windowid = wid;
}

 - (void) setCallback : (oppvs::frame_callback) fc fromuser: (void*) u {
    callback_frame = fc;
    callback_user = u;
    pixel_buffer.user = u;
 }
 

 void* oppvs_vc_av_alloc() {
    return (void*)[[MacVideoAVFoundationCapture alloc] init];
 }

 oppvs::error_video_capture_t oppvs_setup_capture_sessions(void* cap, std::vector<oppvs::VideoActiveSource>& sources) {
    for (std::vector<oppvs::VideoActiveSource>::const_iterator i = sources.begin(); i != sources.end(); ++i)
    {
        //NSLog(@"Source: %d type: %d\n", i->video_source_id, i->video_source_type);
        if (i->video_source_type == oppvs::VST_WEBCAM)
            return [(id)cap openCaptureDevice];
        if (i->video_source_type == oppvs::VST_WINDOW)
            return [(id)cap openScreenDevice];

    }
    return oppvs::ERR_VIDEO_CAPTURE_SESSION_INIT_FAILED;
 }

 void oppvs_start_video_recording(void* cap) {
    return [(id)cap startRecording];
 }

 void oppvs_stop_video_recording(void* cap) {
    return [(id)cap stopRecording];
 }

void oppvs_av_set_callback(void* cap, oppvs::frame_callback fc, void* user) {
    return [(id)cap setCallback : fc fromuser: user];
}

void oppvs_set_window_id(void* cap, int wid) {
    return [(id)cap setWindowId: wid];
}

@end



