#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "internals.h"

#define EMPTY (rgba){0, 0, 0, 0}

#define DA_APPEND(da, val) \
    if ((da)->count >= (da)->capacity) { \
        if ((da)->capacity == 0) { \
            (da)->capacity = 16; \
        } else { \
            (da)->capacity *= 2; \
        } \
        (da)->items = realloc((da)->items, (da)->capacity * sizeof(val)); \
        assert((da)->items); \
    } \
    (da)->items[(da)->count++] = val


core_t core = {0};
static char *log_prefix[] = {
    [LOG_INFO] = "[INFO]",
    [LOG_DEBUG] = "[DEBUG]",
};

void graphics_logf(unsigned int level, const char *fmt, ...) {
    assert(level < LOG_MAX);
    printf("%s ", log_prefix[level]); 
    va_list l;
    va_start(l, fmt);
    vprintf(fmt, l);
    va_end(l);
}

Vector2 get_mouse_pos() {
    return core.events.window_mouse_pos;
}

int is_mouse_down(int button) {
    return core.events.mouse_buttons[button] == 1;
}

int is_mouse_up(int button) {
    return core.events.mouse_buttons[button] == 0;
}

extern void init_font_library();
extern void uninit_font_library();
extern void init_platform(int width, int height, char *title);
extern void uninit_platform();

static void init_objects() {
    core.objs.capacity = 16; 
    core.objs.buckets = malloc(sizeof(object_bucket) * core.objs.capacity); 
}

void init_window(int width, int height, char *title) {
    graphics_logf(LOG_INFO, "init_window(width=%d, height=%d, title='%s')\n", width, height, title);
    graphics_logf(LOG_INFO, "Initializing platform [%s]\n", core.platform_info.rendering_backend);
    memset(core.events.keycode, GRAPHICS_KEYUP, sizeof(core.events.keycode));
    init_platform(width, height, title);
    graphics_logf(LOG_INFO, "Initializing font library [%s]\n", core.platform_info.font_backend);
    init_font_library();
    init_objects();
    graphics_logf(LOG_INFO, "Done creating window.\n");
}

void close_window() {
    framebuffer_free(&core.window.framebuffer);
    uninit_platform();
    uninit_font_library();
}

void set_resize_callback(resize_callback_t resize_callback) {
    core.callbacks.resize_callback = resize_callback;
}

void set_finish_resize_callback(finish_resize_callback_t finish_resize_callback) {
    core.callbacks.finish_resize_callback = finish_resize_callback;
}

void set_keydown_callback(keydown_callback_t keydown_callback) {
    core.callbacks.keydown_callback = keydown_callback;
}

void set_keyup_callback(keyup_callback_t keyup_callback) {
    core.callbacks.keyup_callback = keyup_callback;
}
void set_rmouse_down_callback(rmouse_down_callback_t rmouse_down_callback) {
    core.callbacks.rmouse_down_callback = rmouse_down_callback;
}
void set_lmouse_down_callback(lmouse_down_callback_t lmouse_down_callback) {
    core.callbacks.lmouse_down_callback = lmouse_down_callback;
}
void set_rmouse_up_callback(rmouse_up_callback_t rmouse_up_callback) {
    core.callbacks.rmouse_up_callback = rmouse_up_callback;

}
void set_lmouse_up_callback(lmouse_up_callback_t lmouse_up_callback) {
    core.callbacks.lmouse_up_callback = lmouse_up_callback;
}
void set_mouse_moved_callback(mouse_moved_callback_t mouse_moved_callback) {
    core.callbacks.mouse_moved_callback = mouse_moved_callback;
}
void set_keypress_callback(keypress_callback_t keypress_callback) {
    core.callbacks.keypress_callback = keypress_callback;
}

void set_keyrelease_callback(keyrelease_callback_t keyrelease_callback) {
    core.callbacks.keyrelease_callback = keyrelease_callback;
}

float get_deltatime() {
    return core.dt;
}


void framebuffer_free(fb_t *fb) {
    free(fb->data);
    free(fb->dirty);
}

fb_t *get_framebuffer() {
    return &core.window.framebuffer;
}

int window_should_close() {
    return core.window.window_should_close;
}

int is_key_up(int keycode) {
    return (core.events.keycode[keycode] & GRAPHICS_KEYUP) != 0;
}
int is_key_down(int keycode) {
    return (core.events.keycode[keycode] & GRAPHICS_KEYDOWN) != 0;
}

int is_key_pressed(int keycode) {
    return (core.events.keycode[keycode] & GRAPHICS_KEYPRESS) != 0;
}

int is_key_released(int keycode) {
    return (core.events.keycode[keycode] & GRAPHICS_KEYRELEASE) != 0;
}

int get_key(int keycode) {
    return core.events.keycode[keycode];
}

rgba get_pixel(fb_t *fb, int x, int y) {
    if (x < 0 || y < 0)
        return EMPTY;
    if (x >= fb->width || y >= fb->height)
        return EMPTY;
    int i = x + fb->width * y;
    return fb->data[i];
}

fb_t make_framebuffer(int width, int height) {
    fb_t bgfb = {0};
    bgfb.data = malloc(width * height * sizeof(rgba));
    memset(bgfb.data, 0, sizeof(rgba)*width * height);
    bgfb.dirty = malloc(width * height);
    memset(bgfb.dirty, 0, width * height);
    bgfb.width = width;
    bgfb.height = height;
    return bgfb;
}

int rgba_equal(rgba a, rgba b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

void set_pixel(fb_t *fb, int x, int y, rgba color) {
    if (x < 0 || y < 0)
        return;
    if (x >= fb->width || y >= fb->height)
        return;

    int i = x + fb->width * y;
    if (!rgba_equal(fb->data[i], color)) {
        fb->dirty[i] = 1;
        if (x < fb->dirty_box.minx) {
            fb->dirty_box.minx = x;
        } else if (x > fb->dirty_box.maxx) {
            fb->dirty_box.maxx = x;
        }

        if (y < fb->dirty_box.miny) {
            fb->dirty_box.miny = y;
        } else if (y > fb->dirty_box.maxy) {
            fb->dirty_box.maxy = y;
        }
        fb->data[i] = color;
    }
}

void fbcpy(fb_t *dst, fb_t *src, Vector2 off) {
    for (int x = 0; x < src->width; x++) {
        for (int y = 0; y < src->height; y++) {
            set_pixel(dst, x+off.x, y+off.y, get_pixel(src, x, y));
        }
    }
}

void fill_background(fb_t *fb, rgba color) {
    for (int y = 0; y < fb->height; y++) {
        for (int x = 0; x < fb->width; ++x) {
            set_pixel(fb, x, y, color);
        }
    }
}

void draw_line(fb_t *fb, Vector2 initial, Vector2 final, int thick, rgba color) {
    float dx = final.x - initial.x;
    if (dx == 0) {
        for (int y = initial.y; y < final.y; ++y) {
            set_pixel(fb, initial.x, y, color);
        }            
        return;
    }

    float dy = final.y - initial.y;
    float m = dy/dx;
    float res = 0.01f;
    for (float x = initial.x; x < final.x; x += res) {
        float y = m * (x - initial.x) + initial.y;
        draw_circle(fb, (Vector2){x, y}, thick, color);
    }
}

static float sign(Vector2 p1, Vector2 p2, Vector2 p3) {
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

static int PointInTriangle(Vector2 pt, Vector2 v1, Vector2 v2, Vector2 v3) {
    float d1 = sign(pt, v1, v2);
    float d2 = sign(pt, v2, v3);
    float d3 = sign(pt, v3, v1);

    int has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    int has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

void draw_circle(fb_t *fb, Vector2 pos, int rad, rgba color) {
    int sqlen = 2*rad;
    for (int y = -rad; y < rad; ++y) {
        for (int x = -rad; x < rad; ++x) {
            if (x*x+y*y<=rad*rad) {
                set_pixel(fb, x + pos.x, y + pos.y, color);  
            }
        }
    }
}

void draw_rect(fb_t *fb, Vector2 pos, Vector2 dim, rgba color) {
    for (int y = pos.y; y < pos.y + dim.y; ++y) {
        for (int x = pos.x; x < pos.x + dim.x; ++x) {
            set_pixel(fb, x, y, color);  
        }
    }
}

static Vector2 vector2_max3x(Vector2 a, Vector2 b, Vector2 c) {
    Vector2 max = a; 
    if (b.x > max.x) max = b;
    if (c.x > max.x) max = c;
    return max;
}

static Vector2 vector2_max3y(Vector2 a, Vector2 b, Vector2 c) {
    Vector2 max = a; 
    if (b.y > max.y) max = b;
    if (c.y > max.y) max = c;
    return max;
}

static Vector2 vector2_min3x(Vector2 a, Vector2 b, Vector2 c) {
    Vector2 min = a; 
    if (b.x < min.x) min = b;
    if (c.x < min.x) min = c;
    return min;
}

Vector2 vector2_min3y(Vector2 a, Vector2 b, Vector2 c) {
    Vector2 min = a; 
    if (b.y < min.y) min = b;
    if (c.y < min.y) min = c;
    return min;
}

void draw_triangle(fb_t *fb, Vector2 a, Vector2 b, Vector2 c, rgba color) {
    Vector2 minx = vector2_min3x(a, b, c);
    Vector2 maxx = vector2_max3x(a, b, c);
    Vector2 miny = vector2_min3y(a, b, c);
    Vector2 maxy = vector2_max3y(a, b, c);
    Vector2 pos = {minx.x, miny.y};
    Vector2 dim = {maxx.x - minx.x, maxy.y - miny.y};
    for (int y = pos.y; y < pos.y + dim.y; ++y) {
        for (int x = pos.x; x < pos.x + dim.x; ++x) {
            Vector2 pt = {x, y};
            if (PointInTriangle(pt, a, b, c)) {
                set_pixel(fb, x, y, color);  
            }
        }
    }
}

void draw_rectv(fb_t *fb, Vector2 a, Vector2 b, Vector2 c, Vector2 d, rgba color) {
    draw_triangle(fb, a, b, c, color);
    draw_triangle(fb, b, c, d, color);
}


int load_texture(char *filename, texture *t) {
    t->data = stbi_load(filename, &t->width, &t->height, &t->channels, 0);
    if (!t->data) {
        return 0;
    }
    t->data_size = (Vector2){t->width, t->height};
    return 1;
}

typedef struct {
    unsigned char r, g, b;
} rgb;

void draw_texture_opaque(fb_t *fb, texture t, Vector2 pos, Vector2 scale) {
    rgb *data = (rgb *)t.data;
    assert(t.channels == 3);
    int i = 0;
    assert(scale.x > 0 && scale.y > 0);
    for (int y = t.data_offset.y*scale.y; y < (t.data_offset.y + t.height) * scale.y; ++y) {
        for (int x = t.data_offset.x*scale.x; x < (t.data_offset.x + t.width) * scale.x; ++x) {
            int tx = (x / scale.x);
            int ty = (y /scale.y);
            int i = (tx + ty * t.data_size.x);
            rgba c = {0};
            c.r = data[i].r;
            c.g = data[i].g;
            c.b = data[i].b;
            c.a = 255;
            set_pixel(fb, x+pos.x-t.data_offset.x*scale.x, y+pos.y-t.data_offset.y*scale.y, c);
        }
    }
}

void draw_texture_transparent(fb_t *fb, texture t, Vector2 pos, Vector2 scale) {
    rgba *data = (rgba *)t.data;
    int i = 0;
    assert(scale.x > 0 && scale.y > 0);
    for (int y = t.data_offset.y*scale.y; y < (t.data_offset.y + t.height) * scale.y; ++y) {
        for (int x = t.data_offset.x*scale.x; x < (t.data_offset.x + t.width) * scale.x; ++x) {
            int tx = (x / scale.x);
            int ty = (y /scale.y);
            int i = (tx + ty * t.data_size.x);
            int screenx = x+pos.x-t.data_offset.x*scale.x;
            int screeny = y+pos.y-t.data_offset.y*scale.y;
            rgba c = {0};
            rgba bg = get_pixel(fb, screenx, screeny);
            float val = data[i].a / 255.0f;
            c.r = bg.r + (data[i].r - bg.r)*val;
            c.g = bg.g + (data[i].g - bg.g)*val;
            c.b = bg.b + (data[i].b - bg.b)*val;
            c.a = 255;
            set_pixel(fb, screenx, screeny, c);
        }
    }
}

void draw_texture(fb_t *fb, texture t, Vector2 pos, Vector2 scale) {
    if (t.channels == 3) {
        draw_texture_opaque(fb, t, pos, scale);
    } else {
        draw_texture_transparent(fb, t, pos, scale);
    }
}

void draw_texture_mtx(fb_t *fb, texture t, Vector2 pos, Vector2 scale, Vector3 mtx[3]) {
    rgb *data = (rgb *)t.data;
    assert(t.channels == 3);
    int i = 0;
    assert(scale.x > 0 && scale.y > 0);
    for (float y = t.data_offset.y*scale.y; y < (t.data_offset.y + t.height) * scale.y; y+=0.5) {
        for (float x = t.data_offset.x*scale.x; x < (t.data_offset.x + t.width) * scale.x; x+=0.5) {
            int tx = (x / scale.x);
            int ty = (y /scale.y);

            int screenx = mtx[0].x * x + mtx[0].y * y + mtx[0].z;
            int screeny = mtx[1].x * x + mtx[1].y * y + mtx[1].z;

            int i = tx + ty * t.data_size.x;
            rgba c = {0};
            c.r = data[i].r;
            c.g = data[i].g;
            c.b = data[i].b;
            c.a = 255;
            set_pixel(fb, screenx+pos.x-t.data_offset.x*scale.x, screeny+pos.y-t.data_offset.y*scale.y, c);
        }
    }
}

int init_from_atlas(char *filename, int entry_width, int entry_height, texture_list *l) {
    texture t = {0};
    if (!load_texture(filename, &t)) {
        return 0;
    }

    int hori_count = t.width / entry_width;
    int vert_count = t.height / entry_height;

    l->count = hori_count * vert_count;
    l->items = malloc(l->count * sizeof(texture));
    for (int y = 0; y < vert_count; ++y) {
        for (int x = 0; x < hori_count; ++x) {
            int i = x + hori_count * y;
            l->items[i].data = t.data;
            l->items[i].width = entry_width;
            l->items[i].height = entry_height;
            l->items[i].channels = t.channels;
            l->items[i].data_offset = (Vector2){x*entry_width, y*entry_height};
            l->items[i].data_size = t.data_size;
        }
    }
    return 1;
}

void mtx_multiply(Vector3 *a, Vector3 *b, Vector3 *res) {
    res[0].x = a[0].x * b[0].x + a[0].y * b[1].x + a[0].z * b[2].x;
    res[0].y = a[0].x * b[0].y + a[0].y * b[1].y + a[0].z * b[2].y;
    res[0].z = a[0].x * b[0].z + a[0].y * b[1].z + a[0].z * b[2].z;
                                                            
    res[1].x = a[1].x * b[0].x + a[1].y * b[1].x + a[1].z * b[2].x;
    res[1].y = a[1].x * b[0].y + a[1].y * b[1].y + a[1].z * b[2].y;
    res[1].z = a[1].x * b[0].z + a[1].y * b[1].z + a[1].z * b[2].z;
                                                            
    res[2].x = a[2].x * b[0].x + a[2].y * b[1].x + a[2].z * b[2].x;
    res[2].y = a[2].x * b[0].y + a[2].y * b[1].y + a[2].z * b[2].y;
    res[2].z = a[2].x * b[0].z + a[2].y * b[1].z + a[2].z * b[2].z;
}

Vector2 rotate2(Vector2 base, Vector2 off, float theta) {
    return (Vector2){((base.x-off.x)*cos(theta)-(base.y-off.y)*sin(theta))+off.x, ((base.x-off.x)*sin(theta)+(base.y-off.y)*cos(theta))+off.y};
}

Vector2 get_relative(fb_t *fb, Vector2 v) {
    return (Vector2){v.x*fb->width, v.y*fb->height};
}

static uint64_t obj_hash(char *name) {
    uint64_t sum = 0;
    for (size_t i = 0; name[i] != 0; ++i) {
        sum += (i+1)*name[i];
    }
    return sum % (core.objs.capacity);
}

static int exists_in_obj_map(uint64_t hash, char *name, size_t *idx_if_exists) {
    object_bucket bucket = core.objs.buckets[hash];
    for (size_t i = 0; i < bucket.count; ++i) {
        obj_t entry = bucket.items[i];
        if (strcmp(name, entry.name) == 0) {
            *idx_if_exists = i;
            return 1;
        }
    }
    return 0;
}

static int fill_in_obj(obj_t *obj, char *name, int kind, void *handle) {
    obj->kind = kind;
    obj->name = name;
    if (kind == OBJ_IMAGE) {
        obj->image.handle = (texture *) handle;
        obj->image.pos = (Vector2) {0, 0};
        obj->image.scale = (Vector2) {1, 1};
        return 1;
    }

    if (kind == OBJ_ATLAS) {
        obj->atlas.handle = (texture_list *) handle;
        obj->atlas.pos = (Vector2) {0, 0};
        obj->atlas.scale = (Vector2) {1, 1};
        return 1;
    }

    /* todo: implement counter */
    return 0;
}

static int replace_obj(uint64_t hash, obj_t obj, size_t idx) {
    core.objs.buckets[hash].items[idx] = obj;
    return 1;
}

static int put_in_obj_map(char *name, int kind, void *handle) {
    uint64_t hash = obj_hash(name);
    obj_t obj = {0};
    if (!fill_in_obj(&obj, name, kind, handle)) {
        return 0;
    }

    size_t idx = 0;
    if (exists_in_obj_map(hash, name, &idx)) {
        return replace_obj(hash, obj, idx);
    } else {
        DA_APPEND(&core.objs.buckets[hash], obj);
        return 1;
    }
}

obj_t *get_obj(char *name) {
    size_t idx = 0;
    uint64_t hash = obj_hash(name);
    int exists = exists_in_obj_map(hash, name, &idx);
    if (!exists) {
        return NULL;
    } else {
        return &core.objs.buckets[hash].items[idx];
    }
}

obj_t *register_obj(char *name, int kind, void *handle) {
   put_in_obj_map(name, kind, handle);
   return get_obj(name);
}

void update_atlas(obj_t *atlas, float dt) {
    assert(atlas->kind = OBJ_ATLAS);
    texture_list *tlist = atlas->atlas.handle;
    atlas->atlas.counter += (float) atlas->atlas.fps * dt;
    if (atlas->atlas.counter >= tlist->count) {
        atlas->atlas.counter = 0;
    }
}

texture *get_atlas_frame(obj_t *atlas) {
    assert(atlas->kind = OBJ_ATLAS);
    texture_list *tlist = atlas->atlas.handle;
    return &tlist->items[(int) atlas->atlas.counter];
}

int draw_image(fb_t *fb, obj_t *image) {
    if (image->kind != OBJ_IMAGE)
        return 0;
    draw_texture(fb, *image->image.handle, image->image.pos, image->image.scale);
    return 1;
}

int draw_atlas(fb_t *fb, obj_t *atlas, float dt) {
    if (atlas->kind != OBJ_ATLAS)
        return 0;
    draw_texture(fb, *get_atlas_frame(atlas), atlas->atlas.pos, atlas->atlas.scale);
    update_atlas(atlas, dt);
    return 1;
}

