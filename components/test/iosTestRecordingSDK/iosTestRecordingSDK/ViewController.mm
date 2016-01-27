//
//  ViewController.m
//  iosTestRecordingSDK
//
//  Created by Cao Minh Trang on 1/25/16.
//  Copyright © 2016 Cao Minh Trang. All rights reserved.
//

#import "ViewController.h"
#include "ios_video_engine.hpp"
#import <AVFoundation/AVFoundation.h>
#import "GLFrameView.h"

NSMutableData*   FrameStorage;

@interface ViewController ()
{
    oppvs::IosVideoEngine* mVideoEngine;
}

@property (nonatomic, strong) GLFrameView *demoView;

- (void)initEngine;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    
    CGRect demoFrame = CGRectMake(10, 10, _container.frame.size.width - 20, _container.frame.size.height - 20);
    
    self.demoView = [[GLFrameView alloc] initWithFrame:demoFrame];
    self.demoView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self.container addSubview:self.demoView];
    
    [self.demoView setupGL];
    [self initEngine];
    
    FrameStorage = [[NSMutableData alloc] init];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)viewDidDisappear:(BOOL)animated
{
    if (mVideoEngine)
    {
        oppvs::VideoActiveSource* activeSource = mVideoEngine->getSource();
        if (activeSource)
            mVideoEngine->stopCaptureSession(*activeSource);
    }
    
    [super viewDidDisappear:animated];
}

#pragma mark Callback functions

void frameCallback(oppvs::PixelBuffer& pf)
{
    if (pf.nbytes == 0)
        return;
    
    if ((UInt32)[FrameStorage length] < pf.nbytes)
    {
        [FrameStorage increaseLengthBy:pf.nbytes - [FrameStorage length]];
    }
    [FrameStorage replaceBytesInRange:NSMakeRange(0, pf.nbytes) withBytes:pf.plane[0] length:pf.nbytes];
    
    oppvs::ControllerLinker *controller = (oppvs::ControllerLinker*)pf.user;
    if (controller)
    {
        GLFrameView* view = (__bridge GLFrameView*)controller->render;
        dispatch_async(dispatch_get_main_queue(), ^{
            [view render:(GLubyte*)[FrameStorage bytes] withWidth:pf.width[0] andHeight:pf.height[0]];
        });
    }
}


- (void)initEngine
{
    void* user;
    mVideoEngine = new oppvs::IosVideoEngine(frameCallback, user);
    
    NSString* source = @"1"; //Back Camera
    CGRect rect;
    [self addVideoSource:source hasType:oppvs::VST_WEBCAM sourceRect:rect renderRect:rect withViewID:nil atIndex:0];
    
}

- (void) addVideoSource:(NSString *)sourceid hasType:(oppvs::VideoSourceType)type sourceRect:(CGRect)srect renderRect:(CGRect)rrect withViewID:(id)viewid atIndex:(NSInteger) index
{
    std::string source = [sourceid UTF8String];
    oppvs::window_rect_t sourceRect = createFromCGRect(srect);
    oppvs::window_rect_t renderRect = createFromCGRect(rrect);
    
    oppvs::ControllerLinker *controller = new oppvs::ControllerLinker();
    controller->render = (__bridge void*)self.demoView;
    
    oppvs::VideoActiveSource* activeSource = mVideoEngine->addSource(type, source, 24, sourceRect, renderRect, (void*)controller, (int)index);
    if (activeSource)
    {
        mVideoEngine->setupCaptureSession(activeSource);
        int error = mVideoEngine->startCaptureSession(*activeSource);
        switch (error) {
            case 0:
            {
                break;
            }
            case -1:
            {
                dispatch_async( dispatch_get_main_queue(), ^{
                    NSString *message = NSLocalizedString( @"ForthTV doesn't have permission to use the camera, please change privacy settings", @"Alert message when the user has denied access to the camera" );
                    UIAlertController *alertController = [UIAlertController alertControllerWithTitle:@"ForthTV" message:message preferredStyle:UIAlertControllerStyleAlert];
                    UIAlertAction *cancelAction = [UIAlertAction actionWithTitle:NSLocalizedString( @"OK", @"Alert OK button" ) style:UIAlertActionStyleCancel handler:nil];
                    [alertController addAction:cancelAction];
                    // Provide quick access to Settings.
                    UIAlertAction *settingsAction = [UIAlertAction actionWithTitle:NSLocalizedString( @"Settings", @"Alert button to open Settings" ) style:UIAlertActionStyleDefault handler:^( UIAlertAction *action ) {
                        [[UIApplication sharedApplication] openURL:[NSURL URLWithString:UIApplicationOpenSettingsURLString]];
                    }];
                    [alertController addAction:settingsAction];
                    [self presentViewController:alertController animated:YES completion:nil];
                } );
                break;
            }
            case -2:
            {
                dispatch_async( dispatch_get_main_queue(), ^{
                    NSString *message = NSLocalizedString( @"Unable to capture media", @"Alert message when something goes wrong during capture session configuration" );
                    UIAlertController *alertController = [UIAlertController alertControllerWithTitle:@"ForthTV" message:message preferredStyle:UIAlertControllerStyleAlert];
                    UIAlertAction *cancelAction = [UIAlertAction actionWithTitle:NSLocalizedString( @"OK", @"Alert OK button" ) style:UIAlertActionStyleCancel handler:nil];
                    [alertController addAction:cancelAction];
                    [self presentViewController:alertController animated:YES completion:nil];
                } );
                break;
            }
            default:
                break;
        }
    }
    else
        NSLog(@"Failed to add capture source");
    
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

#pragma mark Actions

- (IBAction)switchCamera:(id)sender {

    oppvs::VideoActiveSource* activeSource;
    if (mVideoEngine)
    {
        activeSource = mVideoEngine->getSource();
        if (activeSource)
        {
            if (activeSource->video_source_id.compare("0") == 0)
                activeSource->video_source_id = "1";
            else
                activeSource->video_source_id = "0";
        
            mVideoEngine->updateConfiguration(*activeSource);
        }
    }
    
}
@end
