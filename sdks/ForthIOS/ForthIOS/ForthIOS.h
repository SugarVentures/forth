//
//  ForthIOS.h
//  ForthIOS
//
//  Created by Cao Minh Trang on 10/27/15.
//  Copyright © 2015 Cao Minh Trang. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Constants.h"

@protocol ForthStreamingDelegate <NSObject>

- (void)frameCallback: (void*)data withWidth: (int)width andHeight: (int)height andStride: (int)stride;

@end

@interface ForthStreaming : NSObject
{
    id <ForthStreamingDelegate> _delegate;
}

@property (nonatomic,strong) id delegate;
- (void) startStreaming;


@end
