//
//  glUtil.h
//  OPPVSRendering
//
//  Created by Cao Minh Trang on 5/28/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#ifndef OPPVSRendering_glUtil_h
#define OPPVSRendering_glUtil_h

#import "vectorUtil.h"
#import "matrixUtil.h"

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

const char* frameVertexShader = {
    "uniform mat4 model_matrix;"
    "uniform vec4 pos;"
    "attribute vec3 coord3d;"
    "attribute vec2 texcoord;"
    "varying vec2 vsTexCoord;"
    "void main()"
    "{"
    " gl_Position = vec4(coord3d, 1.0);"
    " vsTexCoord = texcoord;"
    "}"
};

const char* frameFragmentShader = {
    "uniform sampler2D tex;"
    "varying vec2 vsTexCoord;"
    "void main()"
    "{"
    "gl_FragColor = texture2D(tex,vsTexCoord);"
    "}"
};

#endif
