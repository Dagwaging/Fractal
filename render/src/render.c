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
static EGLSurface surface;

static int gl_ready = 0;


static int tex_width;
static int tex_height;

static GLuint tex;
static GLuint tex_fb;

static GLuint tex2;
static GLuint tex_fb2;

static int tex_ready = 0;


static GLuint vshader;
static GLuint fshader;
static GLuint program;

static GLuint attr_vertex;
static GLuint attr_tcoord;
static GLuint uniform_previous;

static int shader_ready = 0;


int init() {
	int success = 0;

	if(!gl_ready) {
		printlog("Intializing OpenGL...");

		init_gl(&display, &context, &surface);

		printlog("Initialized OpenGL");

		vshader = init_shader(vshader_source, GL_VERTEX_SHADER, NULL);

		gl_ready = 1;

		success = 1;
	}
	else {
		success = 1;
	}

	return success;
}

int deinit() {
	int success = 0;

	if(gl_ready) {
		deinit_gl(display, context, surface);

		gl_ready = 0;

		success = 1;
	}
	else {
		success = 1;
	}

	return success;
}

int set_size(int width, int height) {
	int success = 0;

	if(gl_ready) {
		if(tex_ready) {
			deinit_texture(tex);
			deinit_framebuffer(tex_fb);

			deinit_texture(tex2);
			deinit_framebuffer(tex_fb2);
		}

		tex_width = width;
		tex_height = height;

		tex = init_texture(GL_TEXTURE0, width, height);
		tex_fb = init_framebuffer(tex);

		tex2 = init_texture(GL_TEXTURE1, width, height);
		tex_fb2 = init_framebuffer(tex2);

		glViewport(0, 0, width, height);
		check();

		tex_ready = 1;

		success = 1;
	}

	return success;
}

int set_shader(const char* shader) {
	int success = 0;

	if(gl_ready) {
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

		uniform_previous = glGetUniformLocation(program, "previous");
		check();

		shader_ready = 1;

		success = 1;
	}

	return success;
}

int render(float xMin, float yMin, float xMax, float yMax, char** buffer) {
	int size = -1;

	if(gl_ready && tex_ready && shader_ready) {
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

		uint8_t* image = (uint8_t*) malloc(sizeof(uint8_t) * tex_width * tex_height * 4);
		glReadPixels(0, 0, tex_width, tex_height, GL_RGBA, GL_UNSIGNED_BYTE, image);
		check();

		printlog("Got rendered pixels");


		printlog("Writing file...");

		size = png_write(image, tex_width, tex_height, buffer);

		char log[20];
		sprintf(log, "%d bytes written", size);
		printlog(log);

		deinit_buffer(buf);
		deinit_buffer(tbuf);
	}

	return size;
}
