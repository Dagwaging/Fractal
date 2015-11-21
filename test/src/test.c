#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "render.h"

static const char* mandelbrot_fshader_source =
"varying vec2 tcoord;"
"uniform sampler2D previous;"
"void main(void) {"
"  vec4 last = texture2D(previous, tcoord);"
"  float real = tcoord.x;"
"  float imaginary = tcoord.y;"
"  float const_real = real;"
"  float const_imaginary = imaginary;"
"  float z2 = 0.0;"
"  int iter_count = 0;"
"  for(int iter = 0; iter < 32; ++iter)"
"  {"
"    float temp_real = real;"
"    real = (temp_real * temp_real) - (imaginary * imaginary) + const_real;"
"    imaginary = 2.0 * temp_real * imaginary + const_imaginary;"
"    z2 = real * real + imaginary * imaginary;"
"    iter_count = iter;"
"    if (z2 > 4.0)"
"      break;"
"  }"
"  if(z2 < 4.0)"
"    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);"
"  else"
"    gl_FragColor = vec4(0.0, float(iter_count) * 0.03125, 0.0, 1.0);"
"}";

int main() {
	struct timespec time;
	clock_gettime(CLOCK_REALTIME, &time);
	fprintf(stderr, "%ld.%09ld\t\t%s\n", (long) time.tv_sec, time.tv_nsec, "Initializing...");

	init();
	set_size(256, 256);
	set_shader(mandelbrot_fshader_source);

	char* buffer = NULL;
	int size = render(0.295, 0.015, 0.305, 0.025, &buffer);

	if(size > 0) {
		fwrite(buffer, sizeof(char), size, stdout);
	}
	fclose(stdout);

	deinit();

	clock_gettime(CLOCK_REALTIME, &time);
	fprintf(stderr, "%ld.%09ld\t\t%s\n", (long) time.tv_sec, time.tv_nsec, "Complete");

	return 0;
}
