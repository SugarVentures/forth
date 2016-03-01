#ifndef FORTH_RENDERER_H
#define FORTH_RENDERER_H


#include <jni.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

namespace oppvs {
	class ForthRenderer
	{
	public:
		~ForthRenderer();
		int init();
		int setup(int w, int h);
		void renderFrame(GLubyte* data, GLuint w, GLuint h, bool flag);
		void render();
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

	    uint8_t* m_data;

	    void buildTexture();
	    void loadVBO();

	};
	
} // oppvs

#endif // FORTH_RENDERER_H
