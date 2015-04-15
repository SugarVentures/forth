//
//  CapturePreview.h
//  OPPVSGUI
//
//  Created by Cao Minh Trang on 3/16/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "mac_video_capture.hpp"

@interface CapturePreview : CAOpenGLLayer
{
@private
    oppvs::PixelBuffer* pixelBuffer;
}

@property oppvs::PixelBuffer* pixelBuffer;
@property (assign,getter=isReset) BOOL reset;
@property (assign,getter=isReverse) BOOL reverse;

- (void)redraw;

@end
