#include "forth_renderer.h"
#include "logs.h"

namespace oppvs {

	static void printGLString(const char *name, GLenum s) {
	    const char *v = (const char *) glGetString(s);
	    LOGD("GL %s = %s\n", name, v);
	}

	static void checkGlError(const char* op) {
	    for (GLint error = glGetError(); error; error
	            = glGetError()) {
	        LOGD("after %s() glError (0x%x)\n", op, error);
	    }
	}

	const char* frameVertexShader = {
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
	    "precision mediump float; \n"
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

	const GLfloat vertices[] =
	{
	    1.0f,  1.0f, 0.0f,
	    -1.0f,  1.0f, 0.0f,
	    -1.0f, -1.0f, 0.0f,
	    1.0f, -1.0f, 0.0f
	};

	const GLfloat texcoords[] = {
	    1.0, 0.0,
	    0.0, 0.0,
	    0.0, 1.0,
	    1.0, 1.0
	};

	const GLbyte indices[] = {
	    0, 1, 2,
	    0, 2, 3
	};

	GLuint ForthRenderer::loadShader(GLenum shaderType, const char* pSource) {
	    GLuint shader = glCreateShader(shaderType);
	    if (shader) {
	        glShaderSource(shader, 1, &pSource, NULL);
	        glCompileShader(shader);
	        GLint compiled = 0;
	        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	        if (!compiled) {
	            GLint infoLen = 0;
	            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
	            if (infoLen) {
	                char* buf = (char*) malloc(infoLen);
	                if (buf) {
	                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
	                    LOGE("Could not compile shader %d:\n%s\n",
	                            shaderType, buf);
	                    free(buf);
	                }
	                glDeleteShader(shader);
	                shader = 0;
	            }
	        }
	    }
	    return shader;
	}

	GLuint ForthRenderer::createProgram(const char* pVertexSource, const char* pFragmentSource) {
	    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
	    if (!vertexShader) {
	        return 0;
	    }

	    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
	    if (!pixelShader) {
	        return 0;
	    }

	    GLuint program = glCreateProgram();
	    if (program) {
	        glAttachShader(program, vertexShader);
	        checkGlError("glAttachShader Vertex");
	        glAttachShader(program, pixelShader);
	        checkGlError("glAttachShader Fragment");
	        glLinkProgram(program);
	        GLint linkStatus = GL_FALSE;
	        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	        if (linkStatus != GL_TRUE) {
	            GLint bufLength = 0;
	            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
	            if (bufLength) {
	                char* buf = (char*) malloc(bufLength);
	                if (buf) {
	                    glGetProgramInfoLog(program, bufLength, NULL, buf);
	                    LOGE("Could not link program:\n%s\n", buf);
	                    free(buf);
	                }
	            }
	            glDeleteProgram(program);
	            program = 0;
	        }
	    }
	    return program;
	}

	ForthRenderer::~ForthRenderer()
	{
		if (m_data)
			delete [] m_data;
	}

	void ForthRenderer::buildTexture()
	{
	    if (m_texName == 0)
	    {
	        glGenTextures(1, &m_texName);
	    }
	    ;
	    glBindTexture(GL_TEXTURE_2D, m_texName);
	    checkGlError("BindTexture");
	    
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	void ForthRenderer::loadVBO()
	{
		glGenBuffers(1, &m_indexBuffer);
	    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
	    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	    
	    glGenBuffers(1, &m_vertexBuffer);
	    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(texcoords), NULL, GL_STATIC_DRAW);
	    
	    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(texcoords), texcoords);
	}

	int ForthRenderer::init()
	{
		printGLString("Version", GL_VERSION);
		printGLString("Vendor", GL_VENDOR);
	    printGLString("Renderer", GL_RENDERER);
	    printGLString("Extensions", GL_EXTENSIONS);

	    buildTexture();

	    m_gProgram = createProgram(frameVertexShader, frameFragmentShader);
	    if (!m_gProgram) {
	    	LOGE("Could not create program.");
	    	return -1;
	    }
	    
	    m_positionSlot = glGetAttribLocation(m_gProgram, "coord3d");
	    glEnableVertexAttribArray(m_positionSlot);
	    
	    m_texCoordSlot = glGetAttribLocation(m_gProgram, "texcoord");
	    glEnableVertexAttribArray(m_texCoordSlot);
	    m_textureUniform = glGetUniformLocation(m_gProgram, "tex");

	    loadVBO();

	    m_data = NULL;
		return 0;
	}

	int ForthRenderer::setup(int w, int h)
	{
	    glViewport(0, 0, w, h);
	    checkGlError("glViewport");

	    LOGD("w %d h %d", w, h);
	    m_viewWidth = w;
	    m_viewHeight = h;
		return 0;
	}

	void ForthRenderer::renderFrame(GLubyte* data, GLuint w, GLuint h, bool flag)
	{
		if (!data)
			return;

		if (!m_data)
			m_data = new uint8_t[w*h*4];
		
		memcpy(m_data, data, w*h*4);
		m_frameWidth = w;
		m_frameHeight = h;

		if (flag) {
		    delete [] data;
		}

	}

	void ForthRenderer::render()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(m_gProgram);
		checkGlError("glUseProgram");
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texName);
		glTexImage2D(GL_TEXTURE_2D,
		             0,
		             GL_RGBA,
		             m_frameWidth,
		             m_frameHeight,
		             0,
		             GL_RGBA,
		             GL_UNSIGNED_BYTE,
		             m_data);

		checkGlError("glTexImage2D");	

		glVertexAttribPointer(m_positionSlot, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
		checkGlError("glVertexAttribPointer");
		glVertexAttribPointer(m_texCoordSlot, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (GLvoid*) sizeof(vertices));
		checkGlError("glVertexAttribPointer");

		
		glUniform1i(m_textureUniform, 0);
		checkGlError("glUniform1i");

		glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_BYTE, 0);
		checkGlError("glDrawElements");
		
	}
} // oppvs