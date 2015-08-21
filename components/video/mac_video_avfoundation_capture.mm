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

    int windowId;
    uint8_t* pixels; //Used in window capture
    dispatch_source_t captureTimer; //Used in window capture
    CGRect windowBound;
    CGDisplayStreamRef captureStream;

    NSDictionary *pixelBufferOptions;
    oppvs::VideoActiveSource *sourceInfo;
}


 - (id) init;
 - (void) dealloc;
 - (oppvs::error_video_capture_t) openCaptureDevice: (CGRect) rect : (int) pixelformat : (int) fps;
 - (oppvs::error_video_capture_t) openScreenDevice: (CGRect) rect : (bool)isDrop : (int) pixelformat : (int) fps;
 - (oppvs::error_video_capture_t) captureWindow: (int) wid for: (CGRect) rect;
 - (void) closeDevice;
 - (void) startRecording;
 - (void) stopRecording;
 - (void) showPreview;
 - (void) captureOutput: (AVCaptureOutput*) captureOutput didOutputSampleBuffer: (CMSampleBufferRef) sampleBuffer fromConnection: (AVCaptureConnection*) connection; /* Capture callback implementation */
 - (void) setCallback: (oppvs::frame_callback) fc fromuser: (void*) u;
 - (void) setWindowId: (int) wid;
 - (void) setFrameRate: (int) fps;
 - (int) setPixelFormat: (int) pf;
 - (void) saveScreenShot: (CGImageRef) image_ref as: (NSString*) filename;
 - (void) updateConfiguration: (CGRect) rect : (int) pixelformat : (int) fps;
 - (void) setSource: (oppvs::VideoActiveSource*) source;
@end

@implementation MacVideoAVFoundationCapture {

 AVCaptureOutput *oldOutput;

}
 - (id) init
 {
 	self = [super init];
    if (self)
    {
        NSLog(@"Init av foundation capture engine\n");
        is_pixel_buffer_set = 0;
    }
    videoDeviceInput = nil;
    screenDeviceInput = nil;
    videoDataOuput = nil;

    pixels = NULL;
    captureTimer = nil;
    captureStream = nil;

 	return self;
 }

 - (void) dealloc {
    [self closeDevice];
    if (captureTimer || captureStream)
        [self closeStream];

    [pixelBufferOptions release];
 	[super dealloc];
}

//For testing
 - (void) saveScreenShot: (CGImageRef) image_ref as: (NSString*) filename
 {
    NSURL * url = [NSURL fileURLWithPath:filename isDirectory:NO];
    CGImageDestinationRef fileDest = CGImageDestinationCreateWithURL((__bridge CFURLRef)url,kUTTypeJPEG,1,NULL);
    CGImageDestinationAddImage(fileDest, image_ref, NULL);
    CGImageDestinationFinalize(fileDest);
 }

 
 - (oppvs::error_video_capture_t) openCaptureDevice: (CGRect) rect : (int) pixelformat : (int) fps
 {
    //Set flip
    pixel_buffer.flip = 1;
    pixel_buffer.originx = CGRectGetMinX(rect);
    pixel_buffer.originy = CGRectGetMinY(rect);
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

        videoDevice = device;
        [self setFrameRate: fps];

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
        videoDeviceInput = input;

           
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

        pixelBufferOptions = [NSDictionary dictionaryWithObjectsAndKeys:
                              @(CGRectGetWidth(rect)), (id)kCVPixelBufferWidthKey,
                              @(CGRectGetHeight(rect)), (id)kCVPixelBufferHeightKey,
                              [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA], (id)kCVPixelBufferPixelFormatTypeKey,
                              nil];

        [output setVideoSettings:pixelBufferOptions];

        [session addOutput: output];
        videoDataOuput = output;

        dispatch_queue_t queue = dispatch_queue_create("oppvs.videocapture.queue", DISPATCH_QUEUE_SERIAL);
        [videoDataOuput setSampleBufferDelegate:self queue:queue];
        dispatch_release(queue);

        [session commitConfiguration];

        //output.videoSettings = [NSDictionary dictionaryWithObject:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA] forKey:(id)kCVPixelBufferPixelFormatTypeKey];
        
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

 - (oppvs::error_video_capture_t) openScreenDevice: (CGRect) rect : (bool) isDrop : (int) pixelformat : (int) fps
 {
    //Set flip
    pixel_buffer.flip = 0;
    // Create session
    session = [[AVCaptureSession alloc] init];
    if(session == nil) {
        printf("Error: cannot create the capture session.\n");
        return (oppvs::ERR_VIDEO_CAPTURE_SESSION_INIT_FAILED);
    }
    
    [session beginConfiguration];
    CGDirectDisplayID screen_id = CGMainDisplayID();

    AVCaptureScreenInput *screen_input = [[AVCaptureScreenInput alloc] initWithDisplayID: screen_id];
    if ([session canAddInput: screen_input] == NO)
    {
        printf("Can't add the screen input \n");
        return oppvs::ERR_VIDEO_CAPTURE_INPUT_DEVICE_FAILED;
    }
    screen_input.removesDuplicateFrames = 1;
    screen_input.scaleFactor = 0.5;
    if (isDrop)
    {        
        [screen_input setCropRect: rect];
    }

    [screen_input setMinFrameDuration: CMTimeMake(1, fps)];
    [session addInput: screen_input];
    
    screenDeviceInput = screen_input;

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

    pixelBufferOptions = [NSDictionary dictionaryWithObjectsAndKeys:
                              //@(CGRectGetWidth(rect)), (id)kCVPixelBufferWidthKey,
                              //@(CGRectGetHeight(rect)), (id)kCVPixelBufferHeightKey,
                              [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA], (id)kCVPixelBufferPixelFormatTypeKey,
                              nil];
    [output setVideoSettings:pixelBufferOptions];

    [session addOutput: output];
    self->videoDataOuput = output;

    dispatch_queue_t queue = dispatch_queue_create("oppvs.videocapture.queue", DISPATCH_QUEUE_SERIAL);
    [videoDataOuput setSampleBufferDelegate:self queue:queue];
    dispatch_release(queue);

    [session commitConfiguration];
    return oppvs::ERR_VIDEO_CAPTURE_NONE;
 }

 - (void) closeDevice {
    if (session != nil)
    {
        if (videoDeviceInput != nil)
        {
            [session removeInput: videoDeviceInput];
            //[videoDeviceInput release];
        }

        if (screenDeviceInput != nil)
        {
            [session removeInput: screenDeviceInput];
            [screenDeviceInput release];
        }

        if (videoDataOuput != nil)
        {
            [session removeOutput: videoDataOuput];
            [videoDataOuput release];
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

- (void) closeStream
{
    is_pixel_buffer_set = 0;
    delete [] pixels;
    if (captureTimer)
    {
        dispatch_source_cancel(captureTimer);
        dispatch_release(captureTimer);
        captureTimer = nil;
    }
    if (captureStream)
    {
        CGDisplayStreamStop(captureStream);
        captureStream = nil;
    }
}

 - (void) showPreview {
    printf("show preview \n");
 }

 - (void) setFrameRate: (int) fps {
    bool isValid = false;
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
            if ((range.minFrameRate <= fps) && (range.maxFrameRate >= fps))
            {
                isValid = true;
                break;
            }
            if (range.maxFrameRate > frame_rate.maxFrameRate)
            {
                device_format = format;
                frame_rate = range;
            }
        }
        if (isValid)
            break;
    }
    
    if ([videoDevice lockForConfiguration: NULL] == YES)
    {
        if (isValid)
        {
            videoDevice.activeVideoMinFrameDuration = CMTimeMake(1, fps);
            videoDevice.activeVideoMaxFrameDuration = CMTimeMake(1, fps);
        }
        else if (frame_rate)
        {
            videoDevice.activeVideoMinFrameDuration = frame_rate.minFrameDuration;
            videoDevice.activeVideoMaxFrameDuration = frame_rate.maxFrameDuration;
        }
        [videoDevice unlockForConfiguration];
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

    if ([session isRunning] == YES)
    {
        [session stopRunning];
        [self closeDevice];        
        return;
    }

    if (is_pixel_buffer_set)
        [self closeStream];
 }

 -(CGImageRef)imageRefFromSampleBuffer:(CMSampleBufferRef)sampleBuffer {
 
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer); 
    CVPixelBufferLockBaseAddress(imageBuffer,0); 
    uint8_t *baseAddress = (uint8_t *)CVPixelBufferGetBaseAddress(imageBuffer); 
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer); 
    size_t width = CVPixelBufferGetWidth(imageBuffer); 
    size_t height = CVPixelBufferGetHeight(imageBuffer); 
     
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB(); 
    CGContextRef context = CGBitmapContextCreate(baseAddress, width, height, 8, bytesPerRow, colorSpace, kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst); 
    CGImageRef newImage = CGBitmapContextCreateImage(context); 
    CVPixelBufferUnlockBaseAddress(imageBuffer,0);
    CGContextRelease(context); 
    CGColorSpaceRelease(colorSpace);
    return newImage;
 
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


            sourceInfo->rect.right = sourceInfo->rect.left + pixel_buffer.width[0];
            sourceInfo->rect.top = sourceInfo->rect.bottom + pixel_buffer.height[0];
            //printf("Stride: %d\n", pixel_buffer.stride[0]);
            sourceInfo->stride = pixel_buffer.stride[0];
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
    else if (oppvs::PF_YUVJ420BP == pixel_format     /* kCVPixelFormatType_420YpCbCr8BiPlanarFullRange */
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

- (CGImageRef) resizeImage: (CGImageRef) image toWidth: (int) width andHeight: (int) height
{
    CGColorSpaceRef colorspace = CGImageGetColorSpace(image);
    CGContextRef context = CGBitmapContextCreate(NULL, width, height, 
        CGImageGetBitsPerComponent(image),
        CGImageGetBytesPerRow(image),
        colorspace,
        CGImageGetAlphaInfo(image));
    CGColorSpaceRelease(colorspace);

    if(context == NULL)
        return nil;

    // draw image to context (resizing it)
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
    // extract resulting image from context
    CGImageRef imgRef = CGBitmapContextCreateImage(context);
    CGContextRelease(context);

    return imgRef;
}

- (dispatch_source_t) createCaptureTimer: (uint64_t) interval : (uint64_t) leeway 
    : (dispatch_queue_t) queue : (dispatch_block_t) block 
{
    dispatch_source_t timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,
                                                     0, 0, queue);
    if (timer)
    {
        dispatch_source_set_timer(timer, dispatch_walltime(NULL, 0), interval, leeway);
        dispatch_source_set_event_handler(timer, block);
        dispatch_resume(timer);
    }
    return timer;
}


- (void) repeatCatpureWindow
{
    //NSLog(@"Capture window\n");
    CGImageRef screenShot = CGWindowListCreateImage(windowBound, kCGWindowListOptionIncludingWindow, windowId, kCGWindowImageDefault);
    
    //CGImageRef screenShot = [self resizeImage: img toWidth: 1280 andHeight: 720];
    //CGImageRelease(img);

    //[self saveScreenShot: screenShot];

    CGDataProviderRef provider = CGImageGetDataProvider(screenShot);
    CFDataRef imageData = CGDataProviderCopyData(provider);
    
    if (is_pixel_buffer_set == 0)
    {
        pixel_buffer.width[0] = CGImageGetWidth(screenShot);
        pixel_buffer.height[0] = CGImageGetHeight(screenShot);
        pixel_buffer.stride[0] = CGImageGetBytesPerRow(screenShot);
        long len = pixel_buffer.height[0] * pixel_buffer.stride[0];
        if (pixels != NULL)
            delete [] pixels;
        pixels = new uint8_t[len];
        is_pixel_buffer_set = 1;
        pixel_buffer.nbytes = len;
        pixel_buffer.format = oppvs::PF_BGRA32;
    }
    CFDataGetBytes(imageData, CFRangeMake(0, pixel_buffer.nbytes), pixels);
    
    pixel_buffer.plane[0] = pixels;
    
    callback_frame(pixel_buffer);

    //CGDataProviderRelease(provider);
    CFRelease(imageData);
    CGImageRelease(screenShot);

   
}

- (void) streamHandler: (CGDisplayStreamFrameStatus) status : (uint64_t) displayTime : (IOSurfaceRef) frameSurface
    : (CGDisplayStreamUpdateRef) updateRef
{
    IOSurfaceLock(frameSurface, kIOSurfaceLockReadOnly, NULL);

    if (is_pixel_buffer_set == 0)
    {
        pixel_buffer.width[0] = IOSurfaceGetWidthOfPlane(frameSurface, 0);
        pixel_buffer.height[0] = IOSurfaceGetHeightOfPlane(frameSurface, 0);
        pixel_buffer.stride[0] = IOSurfaceGetBytesPerRow(frameSurface);
        pixel_buffer.format = oppvs::PF_BGRA32;
        pixel_buffer.nbytes = pixel_buffer.stride[0]*pixel_buffer.height[0];
        pixels = new uint8_t[pixel_buffer.nbytes];

        is_pixel_buffer_set = 1;
    }
    
    //pixel_buffer.plane[0] = (uint8_t*)IOSurfaceGetBaseAddressOfPlane(frameSurface, 0);
    
    const CGRect * rects;
    size_t num_rects;    
    rects = CGDisplayStreamUpdateGetRects(updateRef, kCGDisplayStreamUpdateDirtyRects, &num_rects);    
    
    if (num_rects == 0)
        return;
    
    CGRect uRect;
    uRect = *rects;
    for (size_t i = 0; i < num_rects; i++)
    {
        /*printf("\t\t(%f,%f),(%f,%f)\n\n",
               (rects+i)->origin.x,
               (rects+i)->origin.y,
               (rects+i)->origin.x + (rects+i)->size.width,
               (rects+i)->origin.y + (rects+i)->size.height);*/
        
        uRect = CGRectUnion(uRect, *(rects+i));
    }
    
    /*printf("\t\tUnion: (%f,%f),(%f,%f)\n\n",
           uRect.origin.x,
           uRect.origin.y,
           uRect.origin.x + uRect.size.width,
           uRect.origin.y + uRect.size.height);*/
    
    void* baseAddress = IOSurfaceGetBaseAddressOfPlane(frameSurface, 0);
    for (int i = 0; i < (uint32_t)uRect.size.height; i++)
    {
        uint32_t offset = pixel_buffer.stride[0]*(uRect.origin.y + i) + uRect.origin.x*4;
        memcpy(pixels + offset, (uint8_t*)baseAddress + offset, uRect.size.width*4);
    }
    pixel_buffer.plane[0] = pixels;
    callback_frame(pixel_buffer);

    IOSurfaceUnlock(frameSurface, kIOSurfaceLockReadOnly, NULL);

}

- (oppvs::error_video_capture_t) captureDisplay
{
    CGDirectDisplayID screen_id = CGMainDisplayID();
    dispatch_queue_t queue = dispatch_queue_create("oppvs.videocapture.queue", DISPATCH_QUEUE_SERIAL);

    CFDictionaryRef opts;
    
    void * keys[2];
    void * values[2];
    
    float fps = 1000.0/30*1000; //30 fps
    keys[0] = (void *) kCGDisplayStreamShowCursor;
    values[0] = (void *) kCFBooleanFalse;
    keys[1] = (void *) kCGDisplayStreamMinimumFrameTime;
    values[1] = (void *) CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &fps);
    
    opts = CFDictionaryCreate(kCFAllocatorDefault, (const void **) keys, (const void **) values, 2, NULL, NULL);

    CGDisplayStreamRef stream = CGDisplayStreamCreateWithDispatchQueue(screen_id, 1280, 720, 
        kCVPixelFormatType_32BGRA, opts, queue,
        ^(CGDisplayStreamFrameStatus status, uint64_t displayTime, IOSurfaceRef frameSurface, CGDisplayStreamUpdateRef updateRef) {
            [self streamHandler: status : displayTime : frameSurface : updateRef];
        });
    
    CGError err;
    
    err = CGDisplayStreamStart(stream);
    if(err != kCGErrorSuccess)
    {
        printf("Failed to start displaystream!! err = %d\n", err);
        return oppvs::ERR_VIDEO_CAPTURE_SESSION_INIT_FAILED;
    }

    captureStream = stream;
    dispatch_release(queue);
    CFRelease(opts);
    return oppvs::ERR_VIDEO_CAPTURE_NONE;
}

- (oppvs::error_video_capture_t) captureWindow: (int) wid for: (CGRect) rect
{
    [self setWindowId: wid];
    [self setWindowBound: rect];

    dispatch_queue_t queue = dispatch_queue_create("oppvs.videocapture.queue", DISPATCH_QUEUE_SERIAL);

    captureTimer = [self createCaptureTimer: NSEC_PER_SEC/30 : 1ull * NSEC_PER_SEC :
         queue : ^{ [self repeatCatpureWindow];} ];

    dispatch_release(queue);
    return oppvs::ERR_VIDEO_CAPTURE_NONE;
}

- (void) setWindowId: (int) wid {
    windowId = wid;
}

- (void) setWindowBound: (CGRect) rect
{
    windowBound = rect;
}

- (void) setCallback: (oppvs::frame_callback) fc fromuser: (void*) u {
    callback_frame = fc;
    callback_user = u;
    pixel_buffer.user = u;
}
 
- (void) updateConfiguration: (CGRect) rect : (int) pf : (int) fps {
    if (session != nil)
    {
        [session beginConfiguration];
        pixelBufferOptions = [NSDictionary dictionaryWithObjectsAndKeys:
                              @(CGRectGetWidth(rect)), (id)kCVPixelBufferWidthKey,
                              @(CGRectGetHeight(rect)), (id)kCVPixelBufferHeightKey,
                              [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA], (id)kCVPixelBufferPixelFormatTypeKey,
                              nil];
        [videoDataOuput setVideoSettings:pixelBufferOptions];
        is_pixel_buffer_set = 0;
        [session commitConfiguration];
    }
}

- (void) setSource: (oppvs::VideoActiveSource*) source;
{
    sourceInfo = source;
    pixel_buffer.source = source->id;
}

 void* oppvs_vc_av_alloc() {
    return (void*)[[MacVideoAVFoundationCapture alloc] init];
 }

 int oppvs_setup_capture_session(void* cap, oppvs::VideoActiveSource* source) {
    CGRect rect = CGRectMake(source->rect.left, source->rect.bottom, 
        source->rect.right - source->rect.left, source->rect.top - source->rect.bottom);
    [(id)cap setSource: source];

    switch (source->video_source_type)
    {
        case oppvs::VST_WEBCAM:
            return [(id)cap openCaptureDevice: rect : source->pixel_format : source->fps];
        case oppvs::VST_WINDOW:
            return [(id)cap openScreenDevice: rect : false: source->pixel_format : source->fps];
        case oppvs::VST_CUSTOM:
            return [(id)cap openScreenDevice: rect : true: source->pixel_format : source->fps];
        default:
            return oppvs::ERR_VIDEO_CAPTURE_SESSION_INIT_FAILED;
    }
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

void oppvs_update_configuration(void* cap, oppvs::VideoActiveSource& source) {
    CGRect rect = CGRectMake(source.rect.left, source.rect.bottom, 
        source.rect.right - source.rect.left, source.rect.top - source.rect.bottom);
    return [(id)cap updateConfiguration: rect : source.pixel_format : source.fps];
}


@end



