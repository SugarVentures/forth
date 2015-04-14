//
//  VideoPreview.h
//  OPPVSGUI
//
//  Created by Cao Minh Trang on 4/14/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "CapturePreview.h"

@class CapturePreview;

@interface VideoPreview : NSView
{
    NSPoint location;
    NSColor *backgroundColor;
    BOOL dragging;
    NSPoint *lastLocation;
    
    CAOpenGLLayer *renderView;
}

- (id)initWithFrame:(NSRect)frameRect;
- (void)drawRect:(NSRect)dirtyRect;

- (void)setLocation: (NSPoint)point;
- (NSPoint)location;

- (id)addWindow:(NSRect)frameRect;

@end
