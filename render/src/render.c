#include <assert.h>
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

static EGLDisplay display;
static EGLContext context;
static EGLConfig config;
static EGLSurface default_surface;
static EGLSurface surface;

static int gl_ready = 0;


static int surface_width;
static int surface_height;

static int surface_ready = 0;


static GLuint vshader;
static GLuint fshader;
static GLuint program;

static GLuint attr_vertex;
static GLuint attr_tcoord;
static GLuint uniform_previous;

static GLuint vertex_buffer;
static GLuint texture_buffer;

static int shader_ready = 0;


int init() {
	if(gl_ready)
		return 1;

	printlog("Intializing OpenGL...");

	if(!init_gl(&display, &context, &default_surface, &config))
		return 0;

	vshader = init_shader(vshader_source, GL_VERTEX_SHADER, NULL);

	if(!png_init())
		return 0;

	GLfloat vertex_data[] = {
		-1.0, -1.0,
		 1.0, -1.0,
		 1.0,  1.0,
		-1.0,  1.0
	};

	vertex_buffer = init_buffer(vertex_data, 8);
	texture_buffer = init_buffer(NULL, 8);

	printlog("Initialized OpenGL");

	gl_ready = 1;

	return 1;
}

int deinit() {
	if(!gl_ready)
		return 1;

	deinit_buffer(vertex_buffer);
	deinit_buffer(texture_buffer);
	
	if(!png_deinit())
		return 0;

	if(surface_ready) {
		if(!deinit_surface(display, surface))
			return 0;
	}

	if(!deinit_surface(display, default_surface))
		return 0;

	if(!deinit_gl(display, context))
		return 0;

	gl_ready = 0;

	return 1;
}

int set_size(int width, int height) {
	if(!gl_ready)
		return 0;

	printlog("Initializing surface...");

	if(surface_ready) {
		deinit_surface(display, surface);
	}

	surface_width = width;
	surface_height = height;

	if(!init_surface(width, height, display, context, config, &surface))
		return 0;

	glViewport(0, 0, width, height);
	check();

	surface_ready = 1;

	if(!png_set_size(width, height))
		return 0;

	printlog("Initialized surface");

	return 1;
}

int set_shader(const char* shader) {
	if(!gl_ready)
		return 0;

	printlog("Initializing shader...");

	if(shader_ready) {
		deinit_shader(program, fshader);
		deinit_program(program);
	}

	fshader = init_shader(shader, GL_FRAGMENT_SHADER, NULL);
	program = init_program(vshader, fshader, NULL);

	attr_vertex = glGetAttribLocation(program, "vertex");
	check();

	attr_tcoord = glGetAttribLocation(program, "texture_coordinate");
	check();

	bind_buffer(attr_vertex, vertex_buffer, 8);
	bind_buffer(attr_tcoord, texture_buffer, 8);

	uniform_previous = glGetUniformLocation(program, "previous");
	check();

	printlog("Initialized shader");

	shader_ready = 1;

	return 1;
}

int render(float xMin, float yMin, float xMax, float yMax, char** buffer) {
	if(!(gl_ready && surface_ready && shader_ready))
		return -1;

	int size = -1;

	printlog("Drawing fractal...");

	GLfloat tcoord_data[] = {
		xMin, yMin,
		xMax, yMin,
		xMax, yMax,
		xMin, yMax
	};

	update_buffer(texture_buffer, tcoord_data, 8);

	check();
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	glUniform1i(uniform_previous, GL_TEXTURE1);
	check();

	glClear(GL_COLOR_BUFFER_BIT);
	check();

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	check();

	glFlush();
	glFinish();
	check();

	printlog("Fractal drawn");


	printlog("Getting rendered pixels...");

	char* image = (char*) malloc(sizeof(char) * surface_width * surface_height * 3);
	glReadPixels(0, 0, surface_width, surface_height, GL_RGB, GL_UNSIGNED_BYTE, image);
	check();

	printlog("Got rendered pixels");


	printlog("Writing file...");

	size = png_encode(image, buffer);

	char log[20];
	sprintf(log, "%d bytes written", size);
	printlog(log);

	return size;
}
