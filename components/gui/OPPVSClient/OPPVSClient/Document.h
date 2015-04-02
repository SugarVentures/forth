//
//  Document.h
//  OPPVSClient
//
//  Created by Cao Minh Trang on 3/27/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "network.hpp"
#import "stream.hpp"

#include "ViewController.h"

@class ViewController;


@interface Document : NSDocument

- (void) initReceiver: (NSString*)server withPort: (NSInteger)port;
- (void) runStreaming: (oppvs::SRTPSocket) cs;

@end

