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
}

- (BOOL)buildProgramWithShaders: (const char*)vShader andFragment: (const char*)fShader;

@end

@implementation GLFrameView

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
            return nil;
        }
    }
    return self;
}

#pragma mark -  OpenGL ES

- (void)setupGL
{
    [EAGLContext setCurrentContext:_context];
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
    return YES;
}

@end
