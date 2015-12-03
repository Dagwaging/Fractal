#ifndef png_helper_h__
#define png_helper_h__

#include "EGL/egl.h"
#include "EGL/eglext.h"

// Writes an image in unsigned ABGR8888 format to a buffer in PNG format
// Puts a pointer to the allocated buffer in the buffer parameter only if successful
// Returns the number of bytes written, or -1 on failure
int png_encode(EGLImageKHR eglImage, char** output_image);
int png_init();
int png_set_size(int width, int height);
int png_deinit();

#endif  // png_helper_h__
