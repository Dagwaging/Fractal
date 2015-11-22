#include <assert.h>

#include "bcm_host.h"
#include "GLES2/gl2.h"
#include "EGL/egl.h"

#define check() //assert(glGetError() == 0)

int init_gl(EGLDisplay* outDisplay, EGLContext* outContext, EGLSurface* outSurface) {
	int success = 1;

	EGLConfig config;
	EGLint num_config;

	EGLDisplay display;
	EGLContext context;
	EGLSurface surface;

	static const EGLint attribute_list[] =
	{
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
	};

	static const EGLint context_attributes[] = 
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	bcm_host_init();

	success = success && (display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) != EGL_NO_DISPLAY;
	check();

	success = success && eglInitialize(display, NULL, NULL) != EGL_FALSE;
	check();

	success = success && eglChooseConfig(display, attribute_list, &config, 1, &num_config) != EGL_FALSE;
	check();

	success = success && eglBindAPI(EGL_OPENGL_ES_API) != EGL_FALSE;
	check();

	success = success && (context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attributes)) != EGL_NO_CONTEXT;
	check();

	success = success && (surface = eglCreateWindowSurface(display, config, NULL, NULL)) != EGL_NO_SURFACE;
	check();

	success = success && eglMakeCurrent(display, surface, surface, context) != EGL_FALSE;
	check();

	if(success) {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		check();

		*outDisplay = display;
		*outContext = context;
		*outSurface = surface;
	}

	return success;
}

int deinit_gl(EGLDisplay display, EGLContext context, EGLSurface surface) {
	int success = 1;

	success = success && eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_FALSE;
	check();

	success = success && eglDestroySurface(display, surface) != EGL_FALSE;
	check();
	
	success = success && eglDestroyContext(display, context) != EGL_FALSE;
	check();

	success = success && eglTerminate(display) != EGL_FALSE;
	check();

	bcm_host_deinit();

	return success;
}


GLuint init_shader(const GLchar* source, GLenum type, char** log) {
	GLuint shader = glCreateShader(type);
	check();

	if(shader)
		glShaderSource(shader, 1, &source, 0);
	check();

	glCompileShader(shader);
	check();

	if(log != NULL) {
		*log = (char*) malloc(sizeof(char) * 1024);
		glGetShaderInfoLog(shader, 1024, NULL, *log);
	}

	return shader;
}

void deinit_shader(GLuint program, GLuint shader) {
	glDetachShader(program, shader);
	check();

	glDeleteShader(shader);
	check();
}


GLuint init_program(GLuint vshader, GLuint fshader, char** log) {
	GLuint program = glCreateProgram();

	glAttachShader(program, vshader);
	check();

	glAttachShader(program, fshader);
	check();

	glLinkProgram(program);
	check();

	if(log != NULL) {
		*log = (char*) malloc(sizeof(char) * 1024);
		glGetProgramInfoLog(program, 1024, NULL, *log);
	}

	glUseProgram(program);
	check();

	return program;
}

void deinit_program(GLuint program) {
	glUseProgram(0);
	check();

	glDeleteProgram(program);
	check();
}


GLuint init_texture(GLenum texture, int width, int height) {
	GLuint tex;

	glGenTextures(1, &tex);
	check();

	glActiveTexture(texture);
	check();

	glBindTexture(GL_TEXTURE_2D, tex);
	check();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
	check();

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	check();

	return tex;
}

void deinit_texture(GLuint texture) {
	glDeleteTextures(1, &texture);
	check();
}


GLuint init_framebuffer(GLuint texture) {
	GLuint framebuffer;

	glGenFramebuffers(1, &framebuffer);
	check();

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	check();

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	check();

	return framebuffer;
}

void deinit_framebuffer(GLuint framebuffer) {
	glDeleteFramebuffers(1, &framebuffer);
	check();
}


GLuint init_buffer(GLuint attribute, GLfloat data[], int length) {
	GLuint buffer;

	glGenBuffers(1, &buffer);
	check();

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	check();

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * length, data, GL_DYNAMIC_DRAW);
	check();

	glVertexAttribPointer(attribute, 2, GL_FLOAT, 0, length, 0);
	check();

	glEnableVertexAttribArray(attribute);
	check();

	return buffer;
}

void deinit_buffer(GLuint buffer) {
	glDeleteBuffers(1, &buffer);
	check();
}
