//
//  VideoPreview.m
//  OPPVSGUI
//
//  Created by Cao Minh Trang on 4/14/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "VideoPreview.h"

@implementation VideoPreview

- (id)init
{
    self = [super init];
    if (self)
    {
        [self resetToDefault: self];
    }
    return self;
}

- (id)initWithFrame: (NSRect)frame {
    self = [super initWithFrame:frame];
    if (self)
    {
        [self resetToDefault:self];
    }
    return self;
}

- (void)resetToDefault: sender
{
    [self setLocation:NSMakePoint(0.0, 0.0)];
    
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    [[NSColor whiteColor] set];
    [NSBezierPath fillRect:dirtyRect];
}

- (void)setLocation:(NSPoint)point
{
    if (!NSEqualPoints(point, location))
    {
        location = point;
    }
}

- (NSPoint)location
{
    return location;
}

- (id)addWindow:(NSRect)frameRect
{
    NSView* view = [[NSView alloc] initWithFrame:frameRect];
    [view setWantsLayer:true];
    renderView = [CapturePreview layer];
    [renderView setAsynchronous:NO];
    [renderView setNeedsDisplay];
    [view setLayer:renderView];    
    [self addSubview:view];
    return (id)renderView;
}

@end
