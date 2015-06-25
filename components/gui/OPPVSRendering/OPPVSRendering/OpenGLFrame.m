//
//  OpenGLFrame.m
//  OPPVSRendering
//
//  Created by Cao Minh Trang on 5/13/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "OpenGLFrame.h"
#import "glUtil.h"

static GLuint texID[5];

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
@synthesize stride;

GLuint texName;
GLuint progName;

GLuint vao;
GLuint ebo;

GLuint fBO;
GLuint dBO;

GLuint pbo[2];  //0: read 1: write
GLuint pboIndex;
static GLint default_frame_buffer = 0;

- (id) init
{
    self = [super init];
    if (self)
    {
        self.asynchronous = FALSE;  //Only redraw when there is new frame
        self.needsDisplayOnBoundsChange = TRUE; //Resized when the view is resized
        
        texName = 0;
        progName = 0;
        pixelBuffer = nil;
        frameWidth = 0;
        frameHeight = 0;
        stride = 0;
        [self setInitialized:TRUE];
        [self setBorderWidth:1];
        
        pboIndex = 0;
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
    //glGetIntegerv(GL_FRAMEBUFFER_BINDING, &default_frame_buffer);

    if (pixelBuffer == NULL)
    {
        return;
    }
    
    if ([self isInitialized] == true)
    {
        //[self setup];
        glGenBuffers(2, pbo);
        self.initialized = false;
        [self generatePBO];
    };

    /*glUseProgram(progName);
    // This call is crucial, to ensure we are working with the correct context
    CGLSetCurrentContext(glContext);
    
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texName);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, frameWidth);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    frameWidth,
                    frameHeight,
                    GL_BGRA,
                    GL_UNSIGNED_BYTE,
                    pixelBuffer);
    
    
    GLuint uniform_texture = glGetUniformLocation(progName, "tex");
    glUniform1i(uniform_texture, 0);
    
    glBindVertexArrayAPPLE(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
    //NSLog(@"%s", GetGLErrorString(glGetError()));
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArrayAPPLE(0);
    
    glBindTexture(GL_TEXTURE_2D, 0);*/
    pboIndex = (pboIndex + 1) % 2;
    GLuint pboNextIndex = (pboIndex + 1) % 2;

    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texID[[self indexTexture]]);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo[pboIndex]);
    
    glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB,
                    0,
                    0,
                    0,
                    frameWidth,
                    frameHeight,
                    GL_BGRA,
                    GL_UNSIGNED_BYTE,
                    0);

    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo[pboNextIndex]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, stride*frameHeight, NULL, GL_STREAM_DRAW_ARB);
    glBufferSubDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0, stride*frameHeight, pixelBuffer);
        
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    
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
    
    glDisable(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_VERTEX_ARRAY);
    
    
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
    glDisable(GL_TEXTURE_RECTANGLE_ARB);
    glShadeModel(GL_FLAT);
    
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    // Call super to finalize the drawing. By default all it does is call glFlush().
    [super drawInCGLContext:glContext pixelFormat:pixelFormat forLayerTime:timeInterval displayTime:timeStamp];
}

#pragma mark OpenGL loading

- (void)setup
{
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    [self loadTexture];
    //[self generateFBO:frameWidth andHeight:frameHeight andTexture:texName];
    
    progName = [self buildProgram:frameVertexShader andFragment:frameFragmentShader];
    glUseProgram(progName);
    [self loadVBO:progName];
    glUseProgram(0);
}

-(void)loadTexture
{
    //Generate texture
    if (texName == 0)
        glGenTextures(1, &texName);
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
    glBindTexture(GL_TEXTURE_2D, 0);
}

-(void) loadVBO: (GLuint)proid
{
    static const GLfloat vertices[] =
    {
         1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f
    };
    
    /*static const GLfloat texcoords[] = {
        1.0, 1.0,
        0.0, 1.0,
        0.0, 0.0,
        1.0, 0.0
    };*/
    
    GLfloat texcoords[] = {
         1.0, 0.0,
         0.0, 0.0,
         0.0, 1.0,
         1.0, 1.0
    };
    
    static const GLbyte indices[] = {
        0, 1, 2,
        0, 2, 3
    };
    
    //Setup the element array buffer
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    GLuint buf;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(texcoords), NULL, GL_STATIC_DRAW);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(texcoords), texcoords);
    
    glGenVertexArraysAPPLE(1, &vao);
    glBindVertexArrayAPPLE(vao);
    
    GLuint coord3d_pos = glGetAttribLocation(proid, "coord3d");
    glEnableVertexAttribArray(coord3d_pos);
    glVertexAttribPointer(coord3d_pos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
    
    GLuint texcoord_pos = glGetAttribLocation(proid, "texcoord");
    glEnableVertexAttribArray(texcoord_pos);
    glVertexAttribPointer(texcoord_pos, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), (GLvoid*)(sizeof(vertices)));
    
    glBindVertexArrayAPPLE(0);

}


- (void) generatePBO
{
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    if (texID[0] == 0)
    {
        glGenTextures(5, texID);
    }
    
    texName = texID[[self indexTexture]];
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texName);

    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,
                 0,
                 GL_RGBA,
                 frameWidth,
                 frameHeight,
                 0,
                 GL_BGRA,
                 GL_UNSIGNED_INT_8_8_8_8_REV,
                 NULL);
    

}

- (void) generateFBO: (GLuint)width andHeight: (GLuint)height andTexture: (GLuint)tex
{

    //Create frame buffer
    glGenFramebuffersEXT(1, &fBO);
    glGenRenderbuffersEXT(1, &dBO);

    glBindTexture(GL_TEXTURE_2D, tex);
    
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, dBO);
    glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fBO);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex, 0);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, dBO);
    
    GLenum status;
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        NSLog(@"failed to make complete framebuffer object %x", status);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, default_frame_buffer);
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

- (GLuint) buildProgram: (const char*)vShader andFragment: (const char*)fShader
{
    GLuint progName;
    GLint logLength, status;
    
    // String to pass to glShaderSource
    GLchar* sourceString = NULL;
    
    float glLanguageVersion;
    sscanf((char *)glGetString(GL_SHADING_LANGUAGE_VERSION), "%f", &glLanguageVersion);
    GLuint version = 100 * glLanguageVersion;
    const GLsizei versionStringSize = sizeof("#version 123\n");
    
    // Create a program object
    progName = glCreateProgram();
    
    sourceString = malloc(strlen(vShader) + versionStringSize);
    sprintf(sourceString, "#version %d\n%s", version, vShader);
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const GLchar**)&(sourceString), NULL);
    glCompileShader(vertexShader);
    
    glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);
    
    if (logLength > 0)
    {
        GLchar *log = (GLchar*) malloc(logLength);
        glGetShaderInfoLog(vertexShader, logLength, &logLength, log);
        NSLog(@"Vtx Shader compile log:%s\n", log);
        free(log);
    }
    
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        NSLog(@"Failed to compile vtx shader:\n%s\n", sourceString);
        free(sourceString);
        return 0;
    }
    
    free(sourceString);
    sourceString = NULL;
    
    glAttachShader(progName, vertexShader);
    glDeleteShader(vertexShader);
    
    sourceString = malloc(strlen(fShader) + versionStringSize);
    sprintf(sourceString, "#version %d\n%s", version, fShader);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, (const GLchar**)&(sourceString), NULL);
    glCompileShader(fragShader);
    
    glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
    
    if (logLength > 0)
    {
        GLchar *log = (GLchar*) malloc(logLength);
        glGetShaderInfoLog(fragShader, logLength, &logLength, log);
        NSLog(@"Fragment Shader compile log: %s\n", log);
        free(log);
    }
    
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        NSLog(@"Failed to compile fragment shader: \n%s\n", sourceString);
        free(sourceString);
        return 0;
    }
    
    free(sourceString);
    sourceString = NULL;
    
    glAttachShader(progName, fragShader);
    glDeleteShader(fragShader);
    
    glLinkProgram(progName);
    glGetProgramiv(progName, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar*)malloc(logLength);
        glGetProgramInfoLog(progName, logLength, &logLength, log);
        NSLog(@"Program link log:\n%s\n", log);
        free(log);
    }
    
    glGetProgramiv(progName, GL_LINK_STATUS, &status);
    if (status == 0)
    {
        NSLog(@"Failed to link program");
        return 0;
    }
    
    glValidateProgram(progName);
    glGetProgramiv(progName, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar*)malloc(logLength);
        glGetProgramInfoLog(progName, logLength, &logLength, log);
        NSLog(@"Program validate log:\n%s\n", log);
        free(log);
    }
    
    glGetProgramiv(progName, GL_VALIDATE_STATUS, &status);
    if (status == 0)
    {
        NSLog(@"Failed to validate program");
        return 0;
    }
    
    return progName;

}

@end