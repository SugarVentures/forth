//
//  RenderingLayer.h
//  OPPVSClient
//
//  Created by Cao Minh Trang on 3/29/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "datatypes.hpp"

@interface RenderingLayer : CAOpenGLLayer
{
@private
    oppvs::PixelBuffer* pixelBuffer;
}

@property oppvs::PixelBuffer* pixelBuffer;
@property (assign,getter=isReverse) BOOL reverse;

@end
