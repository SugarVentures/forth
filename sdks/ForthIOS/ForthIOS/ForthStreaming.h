//
//  ForthStreaming.h
//  ForthIOS
//
//  Created by Cao Minh Trang on 11/4/15.
//  Copyright © 2015 Cao Minh Trang. All rights reserved.
//

#ifndef ForthStreaming_h
#define ForthStreaming_h

@protocol ForthStreamingDelegate <NSObject>

- (void)frameCallback: (void*)data withWidth: (int)width andHeight: (int)height andStride: (int)stride;

@end

@interface ForthStreaming : NSObject
{
    id <ForthStreamingDelegate> _delegate;
}

@property (nonatomic,strong) id delegate;
- (void) startStreaming: (NSString*)streamKey;
- (void) setupAudioPlayer;
- (id) init: (NSString*)serverAddress;
- (id) init;

@end


#endif /* ForthStreaming_h */
