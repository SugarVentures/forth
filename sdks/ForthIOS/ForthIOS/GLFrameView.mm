//
//  GLFrameView_h.m
//  iosDemo
//
//  Created by Cao Minh Trang on 10/30/15.
//  Copyright © 2015 Cao Minh Trang. All rights reserved.
//

#import "GLFrameView.h"
#import "glUtil.h"

@interface GLFrameView ()
{
    //The pixel dimensions of the CAEAGLLayer
    GLint _backingWidth;
    GLint _backingHeight;
    
    EAGLContext* _context;
    GLuint _frameBufferHandle;
    GLuint _colorBufferHandle;
    
    GLuint _positionSlot;
    GLuint _texCoordSlot;
    GLuint _textureUniform;
    
    GLuint _vertexBuffer;
    GLuint _indexBuffer;
    
    GLuint _texName;
}

@property GLuint program;

- (void)setupBuffers;
- (BOOL)buildProgramWithShaders: (const char*)vShader andFragment: (const char*)fShader;

@end

@implementation GLFrameView

static const GLfloat vertices[] =
{
    1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f
};

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


+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

- (id) initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    
    if (self)
    {
        // Use 2x scale factor on Retina displays.
        self.contentScaleFactor = [[UIScreen mainScreen] scale];
        
        // Get and configure the layer.
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = @{ kEAGLDrawablePropertyRetainedBacking :[NSNumber numberWithBool:NO],
                                          kEAGLDrawablePropertyColorFormat : kEAGLColorFormatRGBA8};
        
        // Set the context into which the frames will be drawn.
        _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

        if (!_context || ![EAGLContext setCurrentContext:_context]) {
            NSLog(@"Failed to setup OpenGL context");
            return nil;
        }
        
        _texName = 0;
    }

    return self;
}

- (void)setupBuffers
{
    glDisable(GL_DEPTH_TEST);
    
    glGenFramebuffers(1, &_frameBufferHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferHandle);
    
    glGenRenderbuffers(1, &_colorBufferHandle);
    glBindRenderbuffer(GL_RENDERBUFFER, _colorBufferHandle);
    
    [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)self.layer];
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_backingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_backingHeight);
    
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorBufferHandle);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }

}

#pragma mark -  OpenGL ES

- (void)setupGL
{
    [EAGLContext setCurrentContext:_context];
    
    [self buildTexture];
    [self setupBuffers];
    if ([self buildProgramWithShaders:frameVertexShader andFragment:frameFragmentShader] == NO)
        return;
    
    glUseProgram(self.program);
    [self loadVBO:self.program];
}

- (BOOL)buildProgramWithShaders:(const char*)vShader andFragment: (const char*)fShader
{
    GLuint progName;
    GLint logLength, status;
    
    // String to pass to glShaderSource
    GLchar* sourceString = NULL;
    
    float glLanguageVersion;

    sscanf((char *)glGetString(GL_SHADING_LANGUAGE_VERSION), "OpenGL ES GLSL ES %f", &glLanguageVersion);
    
    GLuint version = 100 * glLanguageVersion;
    const GLsizei versionStringSize = sizeof("#version 123\n");
    
    // Create a program object
    progName = glCreateProgram();
    
    sourceString = (GLchar*)malloc(strlen(vShader) + versionStringSize);
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
    
    sourceString = (GLchar*)malloc(strlen(fShader) + versionStringSize);
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
    
    self.program = progName;
    
    _positionSlot = glGetAttribLocation(self.program, "coord3d");
    glEnableVertexAttribArray(_positionSlot);
    
    _texCoordSlot = glGetAttribLocation(self.program, "texcoord");
    glEnableVertexAttribArray(_texCoordSlot);
    _textureUniform = glGetUniformLocation(self.program, "tex");
    
    return YES;
}

- (void)render:(GLubyte *)data withWidth:(GLuint)width andHeight:(GLuint)height
{
    [self render:data withWidth:width andHeight:height freeWhenDone:false];

}

- (void)render:(GLubyte *)data withWidth:(GLuint)width andHeight:(GLuint)height freeWhenDone:(bool)flag
{
    if (data == NULL)
        return;
    
    [EAGLContext setCurrentContext: _context];
    glClearColor(0, 104.0/255.0, 55.0/255.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glViewport(0, 0, _backingWidth, _backingHeight);
    
    glVertexAttribPointer(_positionSlot, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    glVertexAttribPointer(_texCoordSlot, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (GLvoid*) sizeof(vertices));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texName);
    glUniform1i(_textureUniform, 0);
    
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 width,
                 height,
                 0,
                 GL_BGRA,
                 GL_UNSIGNED_BYTE,
                 data);
    
    if (flag) {
        delete [] data;
    }
    
    glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_BYTE, 0);
    
    [_context presentRenderbuffer:GL_RENDERBUFFER];
}

-(void) loadVBO: (GLuint)proid
{
    
    //Setup the element array buffer
    glGenBuffers(1, &_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(texcoords), NULL, GL_STATIC_DRAW);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(texcoords), texcoords);
    
    
}

- (void) buildTexture
{
    glEnable(GL_TEXTURE_2D);
    if (_texName == 0)
    {
        glGenTextures(1, &_texName);
    }
    
    glBindTexture(GL_TEXTURE_2D, _texName);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
}


@end
