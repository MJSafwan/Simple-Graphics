
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
    float x, y;
} Vector2;

typedef struct {
    float x, y, z;
} Vector3;

typedef void *font_t;

typedef struct {
    rgba *data;
    char *dirty;
    struct {
        int minx, maxx, miny, maxy;
    } dirty_box;
    void *userdata;
    int zindex; /* todo: implement z-index */
    int width;
    int height;
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
/* todo: mouse callbacks */

/* platform */
void init_platform();
void uninit_platform();
void update_framebuffer();
void poll_events();
void sync_frames_start();
void sync_frames_end();

/* core */
void init_window(int width, int height, char *title);
fb_t *get_framebuffer();
int window_should_close();
void close_window();
int rgba_equal(rgba a, rgba b);
fb_t make_framebuffer(int width, int height);
float get_deltatime();
void framebuffer_free(fb_t *fb);
void set_resize_callback(void (*resize_callback)(int, int));
void set_finish_resize_callback(void (*finish_resize_callback)(int, int));

/* Primitive Rendering */
void set_pixel(fb_t *fb, int x, int y, rgba color);
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
void init_font_library();
void uninit_font_library();
int load_font(char *filename, font_t *ctx);
void display_text(fb_t *fb, font_t *ctx, char *text, Vector2 pos, rgba color, int pixel_size);
void font_free(font_t *ctx);

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
/* todo: this is platform specific, so keep a translation table. */
enum {
    PREFIX_KEY_A = 0,
    PREFIX_KEY_S = 1,
    PREFIX_KEY_D = 2,
    PREFIX_KEY_F = 3,
    PREFIX_KEY_H = 4,
    PREFIX_KEY_G = 5,
    PREFIX_KEY_Z = 6,
    PREFIX_KEY_X = 7,
    PREFIX_KEY_C = 8,
    PREFIX_KEY_V = 9,
    PREFIX_KEY_B = 11,
    PREFIX_KEY_Q = 12,
    PREFIX_KEY_W = 13,
    PREFIX_KEY_E = 14,
    PREFIX_KEY_R = 15,
    PREFIX_KEY_Y = 16,
    PREFIX_KEY_T = 17,
    PREFIX_KEY_1 = 18,
    PREFIX_KEY_2 = 19,
    PREFIX_KEY_3 = 20,
    PREFIX_KEY_4 = 21,
    PREFIX_KEY_6 = 22,
    PREFIX_KEY_5 = 23,
    PREFIX_KEY_EQUALS = 24,
    PREFIX_KEY_9 = 25,
    PREFIX_KEY_7 = 26,
    PREFIX_KEY_MINUS = 27,
    PREFIX_KEY_8 = 28,
    PREFIX_KEY_0 = 29,
    PREFIX_KEY_RIGHTBRACKET = 30,
    PREFIX_KEY_O = 31,
    PREFIX_KEY_U = 32,
    PREFIX_KEY_LEFTBRACKET = 33,
    PREFIX_KEY_I = 34,
    PREFIX_KEY_P = 35,
    PREFIX_KEY_RETURN = 36,
    PREFIX_KEY_L = 37,
    PREFIX_KEY_J = 38,
    PREFIX_KEY_APOSTROPHE = 39,
    PREFIX_KEY_K = 40,
    PREFIX_KEY_SEMICOLON = 41,
    PREFIX_KEY_FRONTSLASH = 42,
    PREFIX_KEY_COMMA = 43,
    PREFIX_KEY_BACKSLASH = 44,
    PREFIX_KEY_N = 45,
    PREFIX_KEY_M = 46,
    PREFIX_KEY_PERIOD = 47,
    PREFIX_KEY_TAB = 48,
    PREFIX_KEY_BACKAPOSTROPHE = 50,
    PREFIX_KEY_DELETE = 51,
    PREFIX_KEY_ESCAPE = 53,
    PREFIX_KEY_COMMAND = 55,
    PREFIX_KEY_SHIFT = 56,
    PREFIX_KEY_CAPSLOCK = 57,
    PREFIX_KEY_OPTION = 58,
    PREFIX_KEY_CONTROL = 59,
    PREFIX_KEY_UP = 126,
    PREFIX_KEY_DOWN = 125,
    PREFIX_KEY_LEFT = 123,
    PREFIX_KEY_RIGHT = 124,
};

#endif
