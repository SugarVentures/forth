//
//  OpenGLFrame.m
//  OPPVSRendering
//
//  Created by Cao Minh Trang on 5/13/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "OpenGLFrame.h"

@interface OpenGLFrame()
{
    
}

- (void) drawForScreenInput;
- (void) drawForWebCamInput;

@end

@implementation OpenGLFrame

@synthesize pixelBuffer;
@synthesize frameWidth;
@synthesize frameHeight;

GLuint texName;

- (id) init
{
    self = [super init];
    if (self)
    {
        self.asynchronous = FALSE;  //Only redraw when there is new frame
        self.needsDisplayOnBoundsChange = TRUE; //Resized when the view is resized
        
        texName = 0;
        pixelBuffer = nil;
        frameWidth = 0;
        frameHeight = 0;
        [self setInitialized:TRUE];
        [self setBorderWidth:1];
        
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
    
    NSRect bounds = NSRectFromCGRect([self bounds]);
    GLfloat 	minX, minY, maxX, maxY;
    
    minX = NSMinX(bounds);
    minY = NSMinY(bounds);
    maxX = NSMaxX(bounds);
    maxY = NSMaxY(bounds);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( minX, maxX, minY, maxY, -1.0, 1.0);
    
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    
    if (pixelBuffer == NULL)
    {
        return;
    }
    
    if ([self isInitialized] == true)
    {
        if (texName)
            glDeleteTextures(1, &texName);
        texName = 0;
        [self setup];
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
                    pixelBuffer);
    //NSLog(@"%s \n", GetGLErrorString(glGetError()));
    
    glBindTexture(GL_TEXTURE_2D, texName);
    
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    
    glPushMatrix();
    
    if ([self isReverse])
    {
        [self drawForWebCamInput];
    }
    else
    {
        [self drawForScreenInput];
    }
    
    glPopMatrix();
    
    // Call super to finalize the drawing. By default all it does is call glFlush().
    [super drawInCGLContext:glContext pixelFormat:pixelFormat forLayerTime:timeInterval displayTime:timeStamp];
}

-(void)setup
{
    [self setFrame:CGRectMake(0, 0, frameWidth, frameHeight)];
    glClear(GL_COLOR_BUFFER_BIT);
    
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 frameWidth,
                 frameHeight,
                 0,
                 GL_BGRA,
                 GL_UNSIGNED_BYTE,
                 pixelBuffer);
    
}

- (void) windowResizeHandler: (int) windowWidth andHeight: (int) windowHeight
{
    const float aspectRatio = ((float)windowWidth) / windowHeight;
    float xSpan = 1;
    float ySpan = 1;
    
    if (aspectRatio > 1){
        // Width > Height, so scale xSpan accordinly.
        xSpan *= aspectRatio;
    }
    else{
        // Height >= Width, so scale ySpan accordingly.
        ySpan = xSpan / aspectRatio;
    }
    
    glOrtho(-1*xSpan, xSpan, -1*ySpan, ySpan, -1, 1);
    
    // Use the entire window for rendering.
    glViewport(0, 0, windowWidth, windowHeight);
    
}

- (void) drawForScreenInput
{
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-1.0, 1.0, 0.0);
    
    glTexCoord2f(1.0, 0.0);
    glVertex3f(1.0, 1.0, 0.0);
    
    glTexCoord2f(1.0, 1.0);
    glVertex3f(1.0, -1.0, 0.0);
    
    glTexCoord2f(0.0, 1.0);
    glVertex3f(-1.0, -1.0, 0.0);
    glEnd();
}

- (void) drawForWebCamInput
{
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(1.0, 1.0, 0.0);
    
    glTexCoord2f(0.0, 1.0);
    glVertex3f(1.0, -1.0, 0.0);
    
    glTexCoord2f(1.0, 1.0);
    glVertex3f(-1.0, -1.0, 0.0);
    
    glTexCoord2f(1.0, 0.0);
    glVertex3f(-1.0, 1.0, 0.0);
    glEnd();
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