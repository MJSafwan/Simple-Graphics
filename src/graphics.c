#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "internals.h"

#define EMPTY (rgba){0, 0, 0, 0}

core_t core = {0};

void init_window(int width, int height, char *title) {
    core.window.width = width;
    core.window.height = height;
    core.window.title = title;
    init_platform();
    init_font_library();
}

void close_window() {
    /* todo: uninitialize font library */
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

float sign(Vector2 p1, Vector2 p2, Vector2 p3) {
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

int PointInTriangle(Vector2 pt, Vector2 v1, Vector2 v2, Vector2 v3) {
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

Vector2 vector2_max3x(Vector2 a, Vector2 b, Vector2 c) {
    Vector2 max = a; 
    if (b.x > max.x) max = b;
    if (c.x > max.x) max = c;
    return max;
}

Vector2 vector2_max3y(Vector2 a, Vector2 b, Vector2 c) {
    Vector2 max = a; 
    if (b.y > max.y) max = b;
    if (c.y > max.y) max = c;
    return max;
}

Vector2 vector2_min3x(Vector2 a, Vector2 b, Vector2 c) {
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
    t->data = stbi_load(filename, &t->width, &t->height, &t->channels, 3);
    if (!t->data) {
        return 0;
    }
    return 1;
}

typedef struct {
    unsigned char r, g, b;
} rgb;

void draw_texture(fb_t *fb, texture t, Vector2 pos, Vector2 scale) {
    rgb *data = (rgb *)t.data;
    assert(t.channels == 3);
    int i = 0;
    assert(scale.x > 0 && scale.y > 0);
    for (int y = 0; y < t.height * scale.y; ++y) {
        for (int x = 0; x < t.width * scale.x; ++x) {
            int tx = (x / scale.x);
            int ty = (y /scale.y);
            int i = tx + ty * t.width;
            rgba c = {0};
            c.r = data[i].r;
            c.g = data[i].g;
            c.b = data[i].b;
            c.a = 255;
            set_pixel(fb, x+pos.x, y+pos.y, c);
        }
    }
}

void draw_texture_mtx(fb_t *fb, texture t, Vector2 pos, Vector2 scale, Vector3 mtx[3]) {
    rgb *data = (rgb *)t.data;
    assert(t.channels == 3);
    int i = 0;
    assert(scale.x > 0 && scale.y > 0);
    for (float y = 0; y < t.height * scale.y; y+=0.5) {
        for (float x = 0; x < t.width * scale.x; x+=0.5) {
            int tx = (x / scale.x);
            int ty = (y /scale.y);

            int screenx = mtx[0].x * x + mtx[0].y * y + mtx[0].z;
            int screeny = mtx[1].x * x + mtx[1].y * y + mtx[1].z;

            int i = tx + ty * t.width;
            rgba c = {0};
            c.r = data[i].r;
            c.g = data[i].g;
            c.b = data[i].b;
            c.a = 255;
            set_pixel(fb, screenx+pos.x, screeny+pos.y, c);
        }
    }
}

void mtx_multiply(Vector3 *a, Vector3 *b, Vector3 *res) {
    res[0].x = a[0].x*b[0].x + a[0].y*b[1].x+a[0].z*b[2].x;
    res[0].y = a[0].x*b[0].y + a[0].y*b[1].y+a[0].z*b[2].y;
    res[0].z = a[0].x*b[0].z + a[0].y*b[1].z+a[0].z*b[2].z;

    res[1].x = a[1].x*b[0].x + a[1].y*b[1].x+a[1].z*b[2].x;
    res[1].y = a[1].x*b[0].y + a[1].y*b[1].y+a[1].z*b[2].y;
    res[1].z = a[1].x*b[0].z + a[1].y*b[1].z+a[1].z*b[2].z;

    res[2].x = a[2].x*b[0].x + a[2].y*b[1].x+a[2].z*b[2].x;
    res[2].y = a[2].x*b[0].y + a[2].y*b[1].y+a[2].z*b[2].y;
    res[2].z = a[2].x*b[0].z + a[2].y*b[1].z+a[2].z*b[2].z;
}

Vector2 rotate2(Vector2 base, Vector2 off, float theta) {
    return (Vector2){((base.x-off.x)*cos(theta)-(base.y-off.y)*sin(theta))+off.x, ((base.x-off.x)*sin(theta)+(base.y-off.y)*cos(theta))+off.y};
}
