
#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#define RED (rgba){255, 150, 150, 255}
#define WHITE (rgba){255, 255, 255, 255}
#define BLACK (rgba){0, 0, 0, 255}
#define GREEN (rgba){150, 255, 150, 255}
#define BLUE (rgba){150, 150, 255, 255}
#define MAGENTA (rgba){255, 150, 255, 255}
#define CYAN (rgba){150, 255, 255, 255}
#define YELLOW (rgba){255, 255, 150, 255}

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
    int width;
    int height;
} fb_t;

typedef struct {
    unsigned char *data;
    int width;
    int height;
    int channels;
    Vector2 data_offset;
    Vector2 data_size;
} texture;

typedef struct {
    texture *items;
    size_t count;
} texture_list;

enum {
    LOG_INFO,
    LOG_DEBUG,
    LOG_MAX
};

#define OBJ_IMAGE 0
#define OBJ_ATLAS 1

typedef struct {
    int kind;
    char *name;
    union {
        struct {
            texture *handle;
            Vector2 scale;
            Vector2 pos;
        } image;
        struct {
            texture_list *handle;
            Vector2 scale;
            Vector2 pos;
            int fps;
            float counter;
        } atlas; 
    };
} obj_t;

typedef struct {
    obj_t *items;
    size_t count;
    size_t capacity;   
} object_bucket;

typedef struct {
    object_bucket *buckets;
    size_t count; 
    size_t capacity;
} objects_map;

void graphics_logf(unsigned int level, const char *fmt, ...);

typedef void (*resize_callback_t)(int, int);
typedef void (*finish_resize_callback_t)(int, int);
typedef void (*keydown_callback_t)(void);
typedef void (*keyup_callback_t)(void);
typedef void (*rmouse_down_callback_t)(void);
typedef void (*lmouse_down_callback_t)(void);
typedef void (*rmouse_up_callback_t)(void);
typedef void (*lmouse_up_callback_t)(void);
typedef void (*mouse_moved_callback_t)(Vector2);
typedef void (*keypress_callback_t)(int);
typedef void (*keyrelease_callback_t)(int);

/* platform */
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
int init_from_atlas(char *filename, int entry_width, int entry_height, texture_list *out);

/* Objects */

obj_t *register_obj(char *name, int kind, void *handle);
obj_t *get_obj(char *name);
void update_atlas(obj_t *atlas, float dt);
texture *get_atlas_frame(obj_t *atlas);
int draw_image(fb_t *fb, obj_t *image);
int draw_atlas(fb_t *fb, obj_t *atlas, float dt);

/* Fonts */
int load_font(char *filename, font_t *ctx);
void display_text(fb_t *fb, font_t *ctx, char *text, Vector2 pos, rgba color, int pixel_size);
void font_free(font_t *ctx);

/* Vector math */
void mtx_multiply(Vector3 *a, Vector3 *b, Vector3 *res);
Vector2 rotate2(Vector2 base, Vector2 off, float theta);
Vector2 get_relative(fb_t *fb, Vector2 v);

/* User Input */
int is_key_up(int keycode);
int is_key_down(int keycode);
int is_key_pressed(int keycode);
int is_key_released(int keycode);
int get_key(int keycode);
int is_mouse_down(int mouseb);
int is_mouse_up(int mouseb);
void set_keydown_callback(keydown_callback_t keydown_callback);
void set_keyup_callback(keyup_callback_t keyup_callback);
void set_keypress_callback(keypress_callback_t keypress_callback);
void set_keyrelease_callback(keyrelease_callback_t keyrelease_callback);
void set_rmouse_down_callback(rmouse_down_callback_t rmouse_down_callback);
void set_lmouse_down_callback(lmouse_down_callback_t lmouse_down_callback);
void set_rmouse_up_callback(rmouse_up_callback_t rmouse_up_callback);
void set_lmouse_up_callback(lmouse_up_callback_t lmouse_up_callback);
void set_mouse_moved_callback(mouse_moved_callback_t mouse_moved_callback);
Vector2 get_mouse_pos();

#define GRAPHICS_KEYUP (1 << 0)
#define GRAPHICS_KEYDOWN (1 << 1)
#define GRAPHICS_KEYPRESS (1 << 2)
#define GRAPHICS_KEYRELEASE (1 << 3)

#define GRAPHICS_RMB 0
#define GRAPHICS_LMB 1

#define KEYCODEX \
    X(GRAPHICS_KEY_A, PLATFORM_KEY_A) \
    X(GRAPHICS_KEY_B, PLATFORM_KEY_B) \
    X(GRAPHICS_KEY_C, PLATFORM_KEY_C) \
    X(GRAPHICS_KEY_D, PLATFORM_KEY_D) \
    X(GRAPHICS_KEY_E, PLATFORM_KEY_E) \
    X(GRAPHICS_KEY_F, PLATFORM_KEY_F) \
    X(GRAPHICS_KEY_G, PLATFORM_KEY_G) \
    X(GRAPHICS_KEY_H, PLATFORM_KEY_H) \
    X(GRAPHICS_KEY_I, PLATFORM_KEY_I) \
    X(GRAPHICS_KEY_J, PLATFORM_KEY_J) \
    X(GRAPHICS_KEY_K, PLATFORM_KEY_K) \
    X(GRAPHICS_KEY_L, PLATFORM_KEY_L) \
    X(GRAPHICS_KEY_M, PLATFORM_KEY_M) \
    X(GRAPHICS_KEY_N, PLATFORM_KEY_N) \
    X(GRAPHICS_KEY_O, PLATFORM_KEY_O) \
    X(GRAPHICS_KEY_P, PLATFORM_KEY_P) \
    X(GRAPHICS_KEY_Q, PLATFORM_KEY_Q) \
    X(GRAPHICS_KEY_R, PLATFORM_KEY_R) \
    X(GRAPHICS_KEY_S, PLATFORM_KEY_S) \
    X(GRAPHICS_KEY_T, PLATFORM_KEY_T) \
    X(GRAPHICS_KEY_U, PLATFORM_KEY_U) \
    X(GRAPHICS_KEY_V, PLATFORM_KEY_V) \
    X(GRAPHICS_KEY_W, PLATFORM_KEY_W) \
    X(GRAPHICS_KEY_X, PLATFORM_KEY_X) \
    X(GRAPHICS_KEY_Y, PLATFORM_KEY_Y) \
    X(GRAPHICS_KEY_Z, PLATFORM_KEY_Z)

#define X(a, b) a,
enum {
   KEYCODEX 
   GRAPHICS_KEY_LAST
};
#undef X

#endif
