#ifndef png_helper_h__
#define png_helper_h__

// Writes an image in unsigned RGBA 8888 format to a buffer in PNG format
// Puts a pointer to the allocated buffer in the buffer parameter only if successful
// Returns the number of bytes written, or -1 on failure
int png_write(const uint8_t* image, int width, int height, char** buffer);

#endif  // png_helper_h__
