//
//  Document.m
//  OPPVSClient
//
//  Created by Cao Minh Trang on 3/27/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "Document.h"
#include <errno.h>


ViewController* render;

@interface Document ()
{
@private
    NSViewController* viewController;
    oppvs::StreamingEngine *streamEngine;
}
@end


@implementation Document


- (instancetype)init {
    self = [super init];
    if (self) {
        oppvs::AudioDevice output(0);
        mPlayer = new oppvs::MacAudioPlay(output, 48000, 2);
        mPlayer->attachBuffer(&mAudioRingBuffer);
        if (mPlayer->init() < 0)
        {
            printf("Can not init audio player\n");
        }
        
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(cleanup)
                                                     name:NSApplicationWillTerminateNotification
                                                   object:nil];        
    }
    return self;
}

void frameCallback(oppvs::PixelBuffer& pf)
{
    //ViewController *view = (__bridge ViewController*)pf.user;
    //[view renderFrame:&pf];
    [render renderFrame:&pf];
}


- (void)initReceiver: (NSString*)streamKey
{
    streamEngine = new oppvs::StreamingEngine();
    streamEngine->registerCallback(frameCallback);
    streamEngine->attachBuffer(&mAudioRingBuffer);
    
    dispatch_queue_t queue = dispatch_queue_create("oppvs.streaming.queue", DISPATCH_QUEUE_SERIAL);
    dispatch_async(queue, ^{
        if (streamEngine->init(oppvs::ROLE_VIEWER, oppvs::STUN_SERVER_ADDRESS, oppvs::TURN_SERVER_ADDRESS, oppvs::TURN_SERVER_USER, oppvs::TURN_SERVER_PASS,
                               oppvs::SIGN_SERVER_ADDRESS, oppvs::SIGN_SERVER_PORT) < 0)
        {
            NSLog(@"Failed to init streaming engine");
            return;
        }
        
        std::string strStreamKey([streamKey UTF8String]);
        if (streamEngine->start(strStreamKey) < 0)
        {
            NSLog(@"Failed to start streaming engine");
            return;
        }
    });
    render = (ViewController*)viewController;
}

- (void)cleanup
{
    delete mPlayer;
}

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

@end
