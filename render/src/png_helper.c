#include "png.h"
#include <stdlib.h>
#include <stdint.h>

struct png_buffer {
	char* buffer;
	size_t size;
};

static void png_write_data(png_structp png_ptr, png_bytep data, png_size_t length) {
	struct png_buffer* buffer = (struct png_buffer*) png_get_io_ptr(png_ptr);
	size_t size = buffer->size + length;

	if(buffer->buffer)
		buffer->buffer = realloc(buffer->buffer, size);
	else
		buffer->buffer = malloc(size);
	
	if(!buffer->buffer)
		png_error(png_ptr, "Write Error");
	
	memcpy(buffer->buffer + buffer->size, data, length);
	buffer->size += length;
}

static void png_flush(png_structp png_ptr) { }

static png_bytepp png_make_rows(const uint8_t* image, int width, int height, int components) {
	int row_size = width * components;
	png_bytepp rows = (png_bytepp) malloc(sizeof(png_bytep) * height);

	int i, j;
	for(i = 0; i < height; i++) {
		png_bytep row = (png_bytep) malloc(sizeof(png_byte) * row_size);

		for(j = 0; j < row_size; j++) {
			row[j] = image[j + i * row_size];
		}
		rows[height - i - 1] = row;
	}

	return rows;
}

static void png_free_rows(png_bytepp rows, int height) {
	int i;
	for(i = 0; i < height; i++) {
		free(rows[i]);
	}
	free(rows);
}

int png_write(const uint8_t* image, int width, int height, char** buffer) {
	png_bytepp rows = png_make_rows(image, width, height, 4);

	struct png_buffer state;
	state.buffer = NULL;
	state.size = 0;

	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info_ptr = png_create_info_struct(png_ptr);

	if(setjmp(png_jmpbuf(png_ptr))) {
		state.size = -1;
		goto finalize;
	}

	// Uncomment for a substantial (2-3x) speedup at the cost of a much (100x) larger image
	// png_set_compression_level(png_ptr, 0);
	// png_set_filter(png_ptr, 0, PNG_FILTER_NONE);

	png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_set_write_fn(png_ptr, &state, png_write_data, png_flush);
	png_set_rows(png_ptr, info_ptr, rows);

	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	*buffer = state.buffer;

	finalize:
		if(info_ptr) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		if(png_ptr) png_destroy_write_struct(&png_ptr, &info_ptr);
		png_free_rows(rows, height);
	
	return state.size;
}
