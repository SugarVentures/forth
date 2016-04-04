#ifndef FORTH_RENDERER_H
#define FORTH_RENDERER_H


#include <jni.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

namespace oppvs {
	class ForthRenderer
	{
	public:
		ForthRenderer();
		~ForthRenderer();
		int init();
		int setup(int w, int h);
		void renderFrame(GLubyte* data, GLuint w, GLuint h, bool flag);
		void render();

		void setContext(EGLContext context, EGLDisplay display, EGLConfig config);
	private:
		GLuint m_gProgram;

		GLuint m_positionSlot;
	    GLuint m_texCoordSlot;
	    GLuint m_textureUniform;
	    GLuint m_texName;

	    GLuint m_vertexBuffer;
	    GLuint m_indexBuffer;

	    GLuint m_viewWidth;
	    GLuint m_viewHeight;
	    GLuint m_frameWidth;
	    GLuint m_frameHeight;

	    EGLContext m_context;
	    EGLDisplay m_display;
	    EGLConfig m_eglConfig;
	    EGLContext m_textureContext;

	    bool m_isNeededBuildTexture;

	    void buildTexture();
	    void loadVBO();

	    GLuint loadShader(GLenum shaderType, const char* pSource);
	    GLuint createProgram(const char* pVertexSource, const char* pFragmentSource);
	};
	
} // oppvs

#endif // FORTH_RENDERER_H
