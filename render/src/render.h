#ifndef render_h__
#define render_h__

extern int init();
extern int deinit();

extern int set_size(int width, int height);
extern int set_shader(const char* shader);

extern int render(float xMin, float yMin, float xMax, float yMax, char** buffer);

#endif  // render_h__
