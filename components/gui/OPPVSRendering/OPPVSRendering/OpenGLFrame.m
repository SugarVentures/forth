//
//  OpenGLFrame.m
//  OPPVSRendering
//
//  Created by Cao Minh Trang on 5/13/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "OpenGLFrame.h"

static inline const char * GetGLErrorString(GLenum error)
{
    const char *str;
    switch( error )
    {
        case GL_NO_ERROR:
            str = "GL_NO_ERROR";
            break;
        case GL_INVALID_ENUM:
            str = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            str = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            str = "GL_INVALID_OPERATION";
            break;
#if defined __gl_h_ || defined __gl3_h_
        case GL_OUT_OF_MEMORY:
            str = "GL_OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            str = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
#endif
#if defined __gl_h_
        case GL_STACK_OVERFLOW:
            str = "GL_STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            str = "GL_STACK_UNDERFLOW";
            break;
        case GL_TABLE_TOO_LARGE:
            str = "GL_TABLE_TOO_LARGE";
            break;
#endif
        default:
            str = "(ERROR: Unknown Error Enum)";
            break;
    }
    return str;
}

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
GLuint fBO;
GLuint dBO;
float rotation_degree;

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
        //[self setup];
        
        self.initialized = false;
    }
    [self generatePBO];
    // This call is crucial, to ensure we are working with the correct context
    CGLSetCurrentContext(glContext);
    //glBindTexture(GL_TEXTURE_2D, texName);
    /*int w, h;
    int miplevel = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);
    NSLog(@"%s %d %d %d %d\n", GetGLErrorString(glGetError()), texName, w, h, pixelBuffer[100]);
    
    glPixelStorei(GL_UNPACK_ROW_LENGTH, frameWidth);*/

    /*glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    frameWidth,
                    frameHeight,
                    GL_BGRA,
                    GL_UNSIGNED_BYTE,
                    pixelBuffer);
    
    
    glBindTexture(GL_TEXTURE_2D, texName);
    
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);*/
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texName);
    glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, frameWidth, frameHeight, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixelBuffer);
    
    glColor4f(1.0, 1.0, 1.0, 1.0);
    
    GLfloat textureCoords[] = {
        0, frameHeight,
        frameWidth, frameHeight,
        frameWidth, 0,
        0, 0};
    
    GLfloat vertices[] = {
        -1.0, -1.0,
        1.0, -1.0,
        1.0, 1.0,
        -1.0, 1.0
    };
    
    glShadeModel(GL_SMOOTH);
    
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, textureCoords);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    //NSLog(@"%s \n", GetGLErrorString(glGetError()));
    glDisable(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_VERTEX_ARRAY);
    
    
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
    glDisable(GL_TEXTURE_RECTANGLE_ARB);
    glShadeModel(GL_FLAT);
    
    /*glPushMatrix();
    
    if ([self isReverse])
    {
        [self drawForWebCamInput];
    }
    else
    {
        [self drawForScreenInput];
    }
    // NSLog(@"%s \n", GetGLErrorString(glGetError()));
    glPopMatrix();*/
    
    // Call super to finalize the drawing. By default all it does is call glFlush().
    [super drawInCGLContext:glContext pixelFormat:pixelFormat forLayerTime:timeInterval displayTime:timeStamp];
}

-(void)setup
{
    //Generate texture
    glBindTexture(GL_TEXTURE_2D, 0);
    glGenTextures(1, &texName);
    //glGenTextures(5, textures);
    //texName = textures[[self indexTexture]];
    glBindTexture(GL_TEXTURE_2D, texName);

    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
                 NULL);
    
    NSRect bounds = [self bounds];
    glViewport (0, 0, bounds.size.width, bounds.size.height);
    
    
    glMatrixMode (GL_PROJECTION);                               // Select The Projection Matrix
    
    glOrtho(0, 0, frameWidth, frameHeight, -1.0, 1.0);
    
    glLoadIdentity ();                                          // Reset The Projection Matrix
    glMatrixMode (GL_MODELVIEW);                                // Select The Modelview Matrix
    glLoadIdentity ();
    
}

- (void) generatePBO
{
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    if (texName == 0)
        glGenTextures(1, &texName);
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texName);
    
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,
                 0,
                 GL_RGBA,
                 frameWidth,
                 frameHeight,
                 0,
                 GL_BGRA,
                 GL_UNSIGNED_INT_8_8_8_8_REV,
                 NULL);

    
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

}

- (void) generateFBO
{
    //Create frame buffer
    glGenFramebuffersEXT(1, &fBO);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fBO);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texName, 0);
    //Create depth buffer
    glGenRenderbuffersEXT(1, &dBO);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, dBO);
    glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, frameWidth, frameHeight);
    //Attach depth buffer to frame buffer
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, dBO);
    
    GLenum status;
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        NSLog(@"failed to make complete framebuffer object %x", status);
    }
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

- (void) renderFrame
{
    glClearColor(0, 0, 0, 0);
    //glClear(GL_COLOR_BUFFER_BIT);

    glLoadIdentity(); // Load the Identity Matrix to reset our drawing locations
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texName);
    
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
    NSLog(@"%s \n", GetGLErrorString(glGetError()));
    glBindTexture(GL_TEXTURE_2D, 0);

}

- (void) updateFrame
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fBO); // Bind our frame buffer for rendering
    glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT); // Push our glEnable and glViewport states
    glViewport(0, 0, frameWidth, frameHeight); // Set the size of the frame buffer view port
    
    glClearColor (0.0f, 0.0f, 0.0f, 0.0f); // Set the clear colour
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the depth and colour buffers
    
    
    glLoadIdentity();  // Reset the modelview matrix
    
  
    
    //glDrawPixels(frameWidth, frameHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer);
  
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
    //NSLog(@"%s \n", GetGLErrorString(glGetError()));
    glPopAttrib(); // Restore our glEnable and glViewport states*/
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); // Unbind our texture

    

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
    NSLog(@"Delete texture");
    if (texName)
        glDeleteTextures(1, &texName);
    texName = 0;
    if (dBO)
        glDeleteRenderbuffersEXT(1, &dBO);
    //Bind 0, which means render to back buffer, as a result, fb is unbound
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    if (fBO)
        glDeleteFramebuffersEXT(1, &fBO);
}


@end