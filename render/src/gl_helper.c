#include <assert.h>

#include "bcm_host.h"
#include "GLES2/gl2.h"
#include "EGL/egl.h"

#define check() //assert(glGetError() == 0)

void init_gl(EGLDisplay* outDisplay, EGLContext* outContext, EGLSurface* outSurface) {
	EGLConfig config;
	EGLBoolean result;
	EGLint num_config;

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

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(display != EGL_NO_DISPLAY);
	check();

	result = eglInitialize(display, NULL, NULL);
	assert(EGL_FALSE != result);
	check();

	result = eglChooseConfig(display, attribute_list, &config, 1, &num_config);
	assert(EGL_FALSE != result);
	check();

	result = eglBindAPI(EGL_OPENGL_ES_API);
	assert(EGL_FALSE != result);
	check();

	EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attributes);
	assert(context != EGL_NO_CONTEXT);
	check();

	EGLSurface surface = eglCreateWindowSurface(display, config, NULL, NULL);
	assert(surface != EGL_NO_SURFACE);
	check();

	result = eglMakeCurrent(display, surface, surface, context);
	assert(EGL_FALSE != result);
	check();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	check();

	*outDisplay = display;
	*outContext = context;
	*outSurface = surface;
}

void deinit_gl(EGLDisplay display, EGLContext context, EGLSurface surface) {
	EGLBoolean result;

	result = eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	assert(EGL_FALSE != result);
	check();

	result = eglDestroySurface(display, surface);
	assert(EGL_FALSE != result);
	check();
	
	result = eglDestroyContext(display, context);
	assert(EGL_FALSE != result);
	check();

	result = eglTerminate(display);
	assert(EGL_FALSE != result);
	check();

	bcm_host_deinit();
}


GLuint init_shader(const GLchar* source, GLenum type, char** log) {
	GLuint shader = glCreateShader(type);
	check();

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

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0);
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

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * length, data, GL_STATIC_DRAW);
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
