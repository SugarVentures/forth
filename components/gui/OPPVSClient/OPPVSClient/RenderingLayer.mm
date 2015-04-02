//
//  RenderingLayer.m
//  OPPVSClient
//
//  Created by Cao Minh Trang on 3/29/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "RenderingLayer.h"
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>


@interface RenderingLayer ()
{
@private
    
    GLsizei frameWidth;
    GLsizei frameHeight;
}

@end

@implementation RenderingLayer

@synthesize pixelBuffer;

GLuint texName;


static int count = 0;

- (id) init
{
    self = [super init];
    if (self)
    {
        self.asynchronous = FALSE;  //Only redraw when there is new frame
        self.needsDisplayOnBoundsChange = TRUE; //Resized when the view is resized
        
        texName = 0;
        frameWidth = 0;
        frameHeight = 0;
        
    }
    return self;
}


-(CGLPixelFormatObj)copyCGLPixelFormatForDisplayMask:(uint32_t)mask
{
    return [super copyCGLPixelFormatForDisplayMask:mask];
}

-(CGLContextObj)copyCGLContextForPixelFormat:(CGLPixelFormatObj)pixelFormat
{
    
    return [super copyCGLContextForPixelFormat:pixelFormat];
}

-(BOOL)canDrawInCGLContext:(CGLContextObj)glContext pixelFormat:(CGLPixelFormatObj)pixelFormat forLayerTime:(CFTimeInterval)timeInterval displayTime:(const CVTimeStamp *)timeStamp
{
    return YES;
}


-(void)drawInCGLContext:(CGLContextObj)glContext pixelFormat:(CGLPixelFormatObj)pixelFormat forLayerTime:(CFTimeInterval)timeInterval displayTime:(const CVTimeStamp *)timeStamp
{
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    /*NSRect bounds = [self bounds];
     size_t height = bounds.size.height;
     size_t width = bounds.size.width;*/
    
    if (pixelBuffer == NULL)
    {
        return;
    }
    
    //For testing only
    //pixels = (GLubyte*)malloc(pixelBuffer->nbytes);
    //memcpy(pixels, pixelBuffer->plane[0], pixelBuffer->nbytes);
    
    if (count == 0)
    {
        [self setup];
        count++;
    }
    
    // This call is crucial, to ensure we are working with the correct context
    CGLSetCurrentContext(glContext);
    
    glBindTexture(GL_TEXTURE_2D, texName);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    frameWidth,
                    frameHeight,
                    GL_BGRA,
                    GL_UNSIGNED_BYTE,
                    pixelBuffer->plane[0]);
    //NSLog(@"%s \n", GetGLErrorString(glGetError()));
    
    glBindTexture(GL_TEXTURE_2D, texName);
    
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    
    glPushMatrix();
    
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0.0, 0.0);
        glVertex3f(1.0, 1.0, 0.0);
        
        glTexCoord2f(0.0, 1.0);
        glVertex3f(1.0, -1.0, 0.0);
        
        glTexCoord2f(1.0, 1.0);
        glVertex3f(-1.0, -1.0, 0.0);
        
        glTexCoord2f(1.0, 0.0);
        glVertex3f(-1.0, 1.0, 0.0);
    }
    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    
    // Call super to finalize the drawing. By default all it does is call glFlush().
    [super drawInCGLContext:glContext pixelFormat:pixelFormat forLayerTime:timeInterval displayTime:timeStamp];
}

- (void) setup
{
    frameWidth = (GLsizei)pixelBuffer->width[0];
    frameHeight = (GLsizei)pixelBuffer->height[0];
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 frameWidth,
                 frameHeight,
                 0,
                 GL_BGRA,
                 GL_UNSIGNED_BYTE,
                 pixelBuffer->plane[0]);
    //NSLog(@"%s \n", GetGLErrorString(glGetError()));
    
    
}

- (void) resizeWindow: (int) w : (int) h
{
    float viewWidth = 1.1;
    float viewHeight = 1.1;
    glViewport(0, 0, w, h);
    h = (h==0) ? 1 : h;
    w = (w==0) ? 1 : w;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if ( h < w ) {
        viewWidth *= (float)w/(float)h;
    }
    else {
        viewHeight *= (float)h/(float)w;
    }
    glOrtho( -viewWidth, viewWidth, -viewHeight, viewHeight, -1.0, 1.0 );
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



-(void)releaseCGLContext:(CGLContextObj)glContext
{
    [super releaseCGLContext:glContext];
}

-(void)releaseCGLPixelFormat:(CGLPixelFormatObj)pixelFormat
{
    [super releaseCGLPixelFormat:pixelFormat];
}

- (void)dealloc
{
    if (texName)
        glDeleteTextures(1, &texName);
    texName = 0;
        
}



@end

