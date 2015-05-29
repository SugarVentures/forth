//
//  FrameView.m
//  OPPVSRendering
//
//  Created by Cao Minh Trang on 5/13/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "FrameView.h"

static int noView = 0;

@implementation FrameView

@synthesize backingScaleFactor;

- (id)init
{
    self = [super init];
    if (self)
    {
        [self resetToDefault: self];
    }
    return self;
}

- (void)dealloc
{
    
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
    viewList = [[NSMutableArray alloc] init];
    
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
    
    OpenGLFrame *renderView = [[OpenGLFrame alloc] init];
    [renderView setAsynchronous:NO];
    [renderView setNeedsDisplay];
    renderView.contentsScale = [self backingScaleFactor];
    [view setLayer:renderView];
    [self addSubview:view];
    
    [renderView setIndexTexture:noView];
    noView++;
    return (id)renderView;
}

- (void)mouseDown:(NSEvent *)event
{
    NSPoint clickLocation = [self convertPoint:[event locationInWindow] fromView:nil];
    NSView* view = nil;
    //[viewList exchangeObjectAtIndex:[viewList indexOfObject:view] withObjectAtIndex:[viewList count] - 1];
    //[self setSubviews:viewList];
    for (NSView *subView in [self subviews]) {
        if (![subView isHidden] && [subView hitTest:clickLocation])
            view = subView;
    }
    
    if (view != nil)
    {
        /*NSRect nonborderRect = NSInsetRect([view bounds], 4, 4);
         NSPoint localClickLocation = [self convertPoint:[event locationInWindow] fromView:view];
         if (NSPointInRect(localClickLocation, nonborderRect))
         {*/
        dragging = YES;
        currentDraggingView = view;
        lastLocation = clickLocation;
        /*}
         else
         {
         NSLog(@"Resizing\n");
         
         }*/
    }
}

- (void)mouseDragged:(NSEvent *)event
{
    if (dragging) {
        NSPoint newDragLocation=[self convertPoint:[event locationInWindow] fromView:nil];
        NSPoint newOrigin = [currentDraggingView frame].origin;
        newOrigin.x += newDragLocation.x - lastLocation.x;
        newOrigin.y += newDragLocation.y - lastLocation.y;
        [currentDraggingView setFrameOrigin:newOrigin];
        lastLocation = newDragLocation;
    }
    
}

- (void)mouseUp:(NSEvent *)event
{
    dragging = NO;
    
}

@end
