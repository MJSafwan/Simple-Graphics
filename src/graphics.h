
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

enum {
    LOG_INFO,
    LOG_DEBUG,
    LOG_MAX
};

void graphics_logf(unsigned int level, const char *fmt, ...);

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
int is_key_up(int keycode);
int is_key_down(int keycode);
void set_keydown_callback(keydown_callback_t keydown_callback);
void set_keyup_callback(keyup_callback_t keyup_callback);
Vector2 get_mouse_pos();

#define PREFIX_KEYUP 0
#define PREFIX_KEYDOWN 1

#define KEYCODEX \
    X(PREFIX_KEY_A, KEY_A) \
    X(PREFIX_KEY_B, KEY_B) \
    X(PREFIX_KEY_C, KEY_C) \
    X(PREFIX_KEY_D, KEY_D) \
    X(PREFIX_KEY_E, KEY_E) \
    X(PREFIX_KEY_F, KEY_F) \
    X(PREFIX_KEY_G, KEY_G) \
    X(PREFIX_KEY_H, KEY_H) \
    X(PREFIX_KEY_I, KEY_I) \
    X(PREFIX_KEY_J, KEY_J) \
    X(PREFIX_KEY_K, KEY_K) \
    X(PREFIX_KEY_L, KEY_L) \
    X(PREFIX_KEY_M, KEY_M) \
    X(PREFIX_KEY_N, KEY_N) \
    X(PREFIX_KEY_O, KEY_O) \
    X(PREFIX_KEY_P, KEY_P) \
    X(PREFIX_KEY_Q, KEY_Q) \
    X(PREFIX_KEY_R, KEY_R) \
    X(PREFIX_KEY_S, KEY_S) \
    X(PREFIX_KEY_T, KEY_T) \
    X(PREFIX_KEY_U, KEY_U) \
    X(PREFIX_KEY_V, KEY_V) \
    X(PREFIX_KEY_W, KEY_W) \
    X(PREFIX_KEY_X, KEY_X) \
    X(PREFIX_KEY_Y, KEY_Y) \
    X(PREFIX_KEY_Z, KEY_Z)

#define X(a, b) a,
enum {
   KEYCODEX 
};
#undef X

#endif
