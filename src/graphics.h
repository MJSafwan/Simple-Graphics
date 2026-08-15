#include "ft2build.h"
#include FT_FREETYPE_H

#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#define RED (rgba){255, 100, 100, 255}
#define WHITE (rgba){255, 255, 255, 255}
#define BLACK (rgba){0, 0, 0, 255}
#define GREEN (rgba){100, 255, 100, 255}
#define BLUE (rgba){100, 100, 255, 255}

typedef struct {
    unsigned char r, g, b, a;
} rgba;

typedef struct {
    double x, y;
} Vector2;

typedef struct {
    double x, y, z;
} Vector3;

typedef struct {
    FT_Face face; 
} font_ctx;

typedef struct {
    char *items; 
    Vector2 dim;
} regions_t;

typedef struct {
    rgba *data;
    char *dirty;
    size_t width;
    size_t height;
    regions_t *regions;
} fb_t;

typedef struct {
    unsigned char *data;
    int width;
    int height;
    int channels;
} texture;

typedef struct {
    Vector2 pos;
    rgba color;
} point;

/* Core */
void init_window(int width, int height, char *title);
fb_t *get_framebuffer();
void update_framebuffer();
int window_should_close();
void poll_events();
int get_window_height();
int get_window_width();
void close_window();
int rgba_equal(rgba a, rgba b);
fb_t make_framebuffer(int width, int height);
void sync_frames_start();
void sync_frames_end();
float get_deltatime();
void framebuffer_free(fb_t *fb);

/* Primitive Rendering */
void set_pixel(fb_t *fb, size_t x, size_t y, rgba color);
rgba get_pixel(fb_t *fb, int x, int y);
void fill_background(fb_t *fb, rgba color);
void draw_line(fb_t *fb, Vector2 initial, Vector2 final, int thick, rgba color);
void draw_triangle(fb_t *fb, Vector2 a, Vector2 b, Vector2 c, rgba color);
void draw_rect(fb_t *fb, Vector2 pos, Vector2 dim, rgba color);
void draw_circle(fb_t *fb, Vector2 pos, int rad, rgba color);
int load_texture(char *filename, texture *out);
void draw_texture(fb_t *fb, texture t, Vector2 pos, Vector2 scale);
void draw_rectv(fb_t *fb, Vector2 a, Vector2 b, Vector2 c, Vector2 d, rgba color);
void fbcpy(fb_t *dst, fb_t *src, Vector2 off);
void draw_texture_mtx(fb_t *fb, texture t, Vector2 pos, Vector2 scale, Vector3 mtx[3]);

/* Fonts */
int load_font(char *filename, font_ctx *ctx);
void display_text(fb_t *fb, font_ctx ctx, char *text, Vector2 pos, rgba color, int pixel_size);
void font_free(font_ctx ctx);

/* Vector math */
void mtx_multiply(Vector3 *a, Vector3 *b, Vector3 *res);
Vector2 rotate2(Vector2 base, Vector2 off, float theta);

#endif
