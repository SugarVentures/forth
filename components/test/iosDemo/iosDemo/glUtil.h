//
//  glUtil.h
//  OPPVSRendering
//
//  Created by Cao Minh Trang on 5/28/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#ifndef glUtil_h
#define glUtil_h

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
    "#ifdef GL_ES \n"
    "precision highp float; \n"
    "#endif \n"
    "uniform mat4 model_matrix;\n"
    "uniform vec4 pos;\n"
    "#if __VERSION__ >= 140 \n"
    "in vec3 coord3d; \n"
    "in vec2 texcoord; \n"
    "out vec3 vsTexCoord; \n"
    "#else \n"
    "attribute vec3 coord3d; \n"
    "attribute vec2 texcoord; \n"
    "varying vec2 vsTexCoord; \n"
    "#endif \n"
    "void main()"
    "{"
    " gl_Position = vec4(coord3d, 1.0);"
    " vsTexCoord = texcoord;"
    "}"
};


const char* frameFragmentShader = {
    "#ifdef GL_ES \n"
    "precision highp float; \n"
    "#endif \n"
    "uniform sampler2D tex; \n"
    "#if __VERSION__ >= 140 \n"
    "in vec2 vsTexCoord; \n"
    "#else \n"
    "varying vec2 vsTexCoord; \n"
    "#endif \n"
    "void main()"
    "{"
    "gl_FragColor = texture2D(tex,vsTexCoord);"
    "}"
};

#endif
