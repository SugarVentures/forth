//
//  Document.h
//  OPPVSClient
//
//  Created by Cao Minh Trang on 3/27/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "streaming_engine.hpp"

#include "ViewController.h"

@class ViewController;


@interface Document : NSDocument

- (void) initReceiver: (NSString*)streamKey;

@end

