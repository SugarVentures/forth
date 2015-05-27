//
//  Document.m
//  OPPVSClient
//
//  Created by Cao Minh Trang on 3/27/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "Document.h"
#include <errno.h>


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
        
    }
    return self;
}

void frameCallback(oppvs::PixelBuffer& pf)
{
    ViewController *view = (__bridge ViewController*)pf.user;
    [view renderFrame:&pf];
}


- (void)initReceiver: (NSString*)server withPort: (NSInteger)port
{
    oppvs::PixelBuffer *pixelBuffer = new oppvs::PixelBuffer();
    streamEngine = new oppvs::StreamingEngine(pixelBuffer);
    pixelBuffer->user = (__bridge void*)viewController;
    streamEngine->registerCallback(frameCallback);
    oppvs::ServiceInfo service;
    service.type = oppvs::ST_VIDEO_STREAMING;
    service.key = 123;
    std::string serverAddr([server UTF8String]);
    streamEngine->initSubscribeChannel(serverAddr, port, service);
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
