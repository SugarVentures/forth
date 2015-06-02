//
//  FrameView.h
//  OPPVSRendering
//
//  Created by Cao Minh Trang on 5/13/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OpenGLFrame.h"

@interface FrameView : NSView
{
@private
    NSPoint location;
    NSColor *backgroundColor;
    BOOL dragging;
    NSPoint lastLocation;

    

    NSView* currentDraggingView;
}

@property CGFloat backingScaleFactor;

- (id)initWithFrame:(NSRect)frameRect;
- (void)drawRect:(NSRect)dirtyRect;

- (void)setLocation: (NSPoint)point;
- (NSPoint)location;

- (id)addWindow:(NSRect)frameRect;
- (id)addWindow:(NSRect)frameRect atIndex: (NSInteger)index;

-(void)mouseDown:(NSEvent *)event;
-(void)mouseDragged:(NSEvent *)event;
-(void)mouseUp:(NSEvent *)event;


@end
