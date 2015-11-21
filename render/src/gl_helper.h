#ifndef gl_helper_h__
#define gl_helper_h__

#include "GLES2/gl2.h"
#include "EGL/egl.h"

void init_gl(EGLDisplay* outDisplay, EGLContext* outContext, EGLSurface* outSurface);
void deinit_gl(EGLDisplay display, EGLContext context, EGLSurface surface);

GLuint init_shader(const GLchar* source, GLenum type, char** log);
void deinit_shader(GLuint program, GLuint shader);

GLuint init_program(GLuint vshader, GLuint fshader, char** log);
void deinit_program(GLuint program);

GLuint init_texture(GLenum texture, int width, int height);
void deinit_texture(GLuint texture);

GLuint init_framebuffer(GLuint texture);
void deinit_framebuffer(GLuint framebuffer);

GLuint init_buffer(GLuint attribute, GLfloat data[], int length);
void deinit_buffer(GLuint buffer);

#endif  // gl_helper_h__
