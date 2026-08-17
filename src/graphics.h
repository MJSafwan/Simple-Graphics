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
    rgba *data;
    char *dirty;
    struct {
        int minx, maxx, miny, maxy;
    } dirty_box;
    int zindex;
    size_t width;
    size_t height;
} fb_t;

typedef struct {
    unsigned char *data;
    int width;
    int height;
    int channels;
} texture;

typedef void (*resize_callback_t)(int, int);
typedef void (*finish_resize_callback_t)(int, int);
typedef void (*keydown_callback_t)(void);
typedef void (*keyup_callback_t)(void);

/* Core */
void init_window(int width, int height, char *title);
fb_t *get_framebuffer();
void swap_buffers();
void update_framebuffer();
int window_should_close();
void poll_events();
void close_window();
int rgba_equal(rgba a, rgba b);
fb_t make_framebuffer(size_t width, size_t height);
void sync_frames_start();
void sync_frames_end();
float get_deltatime();
void framebuffer_free(fb_t *fb);
void set_resize_callback(void (*resize_callback)(int, int));
void set_finish_resize_callback(void (*finish_resize_callback)(int, int));

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

/* User Input */
int is_key_down(int keycode);
int is_key_up(int keycode);
int is_some_key_down();
int is_some_key_up();
void set_keydown_callback(keydown_callback_t keydown_callback);
void set_keyup_callback(keyup_callback_t keyup_callback);

/* From https://boredzo.org/blog/archives/2007-05-22/virtual-key-codes */
#define KEY_A 0
#define KEY_S 1
#define KEY_D 2
#define KEY_F 3
#define KEY_H 4
#define KEY_G 5
#define KEY_Z 6
#define KEY_X 7
#define KEY_C 8
#define KEY_V 9
#define KEY_B 11
#define KEY_Q 12
#define KEY_W 13
#define KEY_E 14
#define KEY_R 15
#define KEY_Y 16
#define KEY_T 17
#define KEY_1 18
#define KEY_2 19
#define KEY_3 20
#define KEY_4 21
#define KEY_6 22
#define KEY_5 23
#define KEY_EQUALS 24
#define KEY_9 25
#define KEY_7 26
#define KEY_MINUS 27
#define KEY_8 28
#define KEY_0 29
#define KEY_RIGHTBRACKET 30
#define KEY_O 31
#define KEY_U 32
#define KEY_LEFTBRACKET 33
#define KEY_I 34
#define KEY_P 35
#define KEY_RETURN 36
#define KEY_L 37
#define KEY_J 38
#define KEY_APOSTROPHE 39
#define KEY_K 40
#define KEY_SEMICOLON 41
#define KEY_FRONTSLASH 42
#define KEY_COMMA 43
#define KEY_BACKSLASH 44
#define KEY_N 45
#define KEY_M 46
#define KEY_PERIOD 47
#define KEY_TAB 48
#define KEY_BACKAPOSTROPHE 50
#define KEY_DELETE 51
#define KEY_ESCAPE 53
#define KEY_COMMAND 55
#define KEY_SHIFT 56
#define KEY_CAPSLOCK 57
#define KEY_OPTION 58
#define KEY_CONTROL 59
#define KEY_UP 126
#define KEY_DOWN 125
#define KEY_LEFT 123
#define KEY_RIGHT 124

#endif
