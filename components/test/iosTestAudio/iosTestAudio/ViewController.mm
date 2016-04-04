//
//  ViewController.m
//  iosTestAudio
//
//  Created by Cao Minh Trang on 2/4/16.
//  Copyright © 2016 Cao Minh Trang. All rights reserved.
//

#import "ViewController.h"
#include "mac_audio_engine.hpp"

@interface ViewController ()
{
    oppvs::MacAudioEngine mAudioEngine;
    AudioFileID fOutputAudioFile;
}

@end

void audioCallback(oppvs::GenericAudioBufferList& ab)
{
    printf("Receive audio data %d %d %d %d\n", ab.nFrames, ab.nBuffers, ab.buffers[0].numberChannels, ab.buffers[0].dataLength);
}

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];    
    std::vector<oppvs::AudioDevice> devices;
    mAudioEngine.callbackAudio = audioCallback;
    mAudioEngine.init();
    mAudioEngine.getListAudioDevices(devices, true);
    mAudioEngine.printAudioDeviceList();
    mAudioEngine.addNewCapture(0, nullptr);
    
    [self configureOutputFile];
}

- (OSStatus) configureOutputFile
{
    OSStatus err = noErr;
    CAStreamBasicDescription format;
    format.mSampleRate = 48000;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mBitsPerChannel = 32;
    format.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
    format.mFramesPerPacket = 1;
    format.mChannelsPerFrame = 2;
    format.mBytesPerFrame = sizeof(Float32) * format.mChannelsPerFrame;
    format.mBytesPerPacket = format.mFramesPerPacket * format.mBytesPerFrame;
    
    NSArray*paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSString *recordFile1 = [documentsDirectory stringByAppendingPathComponent: @"testaudio.wav"];
    CFURLRef url1 = CFURLCreateWithFileSystemPath (	NULL,	(CFStringRef)recordFile1, kCFURLPOSIXPathStyle,	false	);
    
    
    err = AudioFileCreateWithURL(url1, kAudioFileCAFType, &format, kAudioFileFlags_EraseFile, &fOutputAudioFile);
    NSLog(@"%d", err);
    AudioFileClose(fOutputAudioFile);
    return err;
    
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
