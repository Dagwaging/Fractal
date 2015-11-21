#include <stdio.h>
#include <stdint.h>

#include "png_helper.h"
#include "gl_helper.h"

#define check() //assert(glGetError() == 0)

static void printlog(const char* message) {
	//struct timespec time;
	//clock_gettime(CLOCK_REALTIME, &time);
	//fprintf(stderr, "%ld.%09ld\t\t%s\n", (long) time.tv_sec, time.tv_nsec, message);
}

static const GLchar *vshader_source =
"attribute highp vec4 vertex;"
"attribute highp vec2 texture_coordinate;"
"varying highp vec2 tcoord;"
"void main(void) {"
" gl_Position = vertex;"
" tcoord = texture_coordinate;"
"}";

int render(const char* shader, int width, int height, float xMin, float yMin, float xMax, float yMax, char** buffer)
{
	EGLDisplay display;
	EGLContext context;
	EGLSurface surface;
	
	printlog("Intializing OpenGL...");

	init_gl(&display, &context, &surface);

	printlog("Initialized OpenGL");


	printlog("Initializing shaders...");

	GLuint vshader = init_shader(vshader_source, GL_VERTEX_SHADER, NULL);
	GLuint fshader = init_shader(shader, GL_FRAGMENT_SHADER, NULL);
	GLuint program = init_program(vshader, fshader, NULL);

	GLuint attr_vertex = glGetAttribLocation(program, "vertex");
	check();

	GLuint attr_tcoord = glGetAttribLocation(program, "texture_coordinate");
	check();

	GLuint uniform_previous = glGetUniformLocation(program, "previous");
	check();

	printlog("Shaders initialized");


	GLuint tex = init_texture(GL_TEXTURE0, width, height);
	GLuint tex_fb = init_framebuffer(tex);

	GLuint tex2 = init_texture(GL_TEXTURE1, width, height);
	GLuint tex_fb2 = init_framebuffer(tex2);

	glViewport(0, 0, width, height);
	check();


	GLfloat vertex_data[] = {
		-1.0,-1.0,
		1.0,-1.0,
		1.0,1.0,
		-1.0,1.0
	};

	GLfloat tcoord_data[] = {
		 xMin, yMin,
		 xMax, yMin,
		 xMax, yMax,
		 xMin, yMax
	};

	GLuint buf = init_buffer(attr_vertex, vertex_data, 8);
	GLuint tbuf = init_buffer(attr_tcoord, tcoord_data, 8);


	printlog("Drawing fractal...");

	glBindFramebuffer(GL_FRAMEBUFFER, tex_fb);
	check();
	glBindBuffer(GL_ARRAY_BUFFER, buf);

	glUniform1i(uniform_previous, GL_TEXTURE1);
	check();

	glClear(GL_COLOR_BUFFER_BIT);
	check();

	check();
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	check();

	glFlush();
	glFinish();
	check();

	printlog("Fractal drawn");


	printlog("Getting rendered pixels...");

	uint8_t* image = (uint8_t*) malloc(sizeof(uint8_t) * width * height * 4);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image);
	check();

	printlog("Got rendered pixels");


	printlog("Writing file...");

	int size = png_write(image, width, height, buffer);

	char log[20];
	sprintf(log, "%d bytes written", size);
	printlog(log);


	deinit_gl(display, context, surface);

	return size;
}
