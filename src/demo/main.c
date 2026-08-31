#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/param.h>
#include <string.h>
#include <mach-o/dyld.h>
#include <stdint.h>
#include "graphics.h"

fb_t bgfb = {0};
font_t font;
texture image = {0};

Vector2 get_relative(fb_t *fb, Vector2 v) {
    return (Vector2){v.x*fb->width, v.y*fb->height};
}

void make_bgfb(int w, int h) {
    bgfb = make_framebuffer(w, h);
    for (int y = 0; y < bgfb.height; ++y) {
        for (int x = 0; x < bgfb.width; ++x) {
            double theta = 2.0 * M_PI * (x / (double)bgfb.width);
            double phi = 2.0 * M_PI * (y / (double)bgfb.height);

            double a = sin(phi) * cos(theta);
            double b = sin(phi) * sin(theta);
            double c = cos(phi);
            rgba color = {0};
            color.r = ((a+1)/2)*255;
            color.g = ((b+1)/2)*255;
            color.b = ((c+1)/2)*255;
            color.a = 255;
            set_pixel(&bgfb, x, y, color);
        }
    }
}

Vector2 zv = {0, 0};
Vector2 ra = {0, 0};
Vector2 rb = {1, 0};
Vector2 rc = {0, 1};

int does_rotate = 1;

void draw_sceene(fb_t *fb) {
    float dt = get_deltatime();
    static char sn_tmp[1028] = "FPS N/A";
    fbcpy(get_framebuffer(), &bgfb, (Vector2){0, 0});
    snprintf(sn_tmp, 1028, "fps %d", (int)(1/dt));
    display_text(fb, font, sn_tmp, (Vector2){0, 60}, BLACK, 32);

    Vector2 mouse_pos = get_mouse_pos();
    snprintf(sn_tmp, 1028, "mouse = (%f, %f)", mouse_pos.x, mouse_pos.y);
    display_text(fb, font, sn_tmp, (Vector2){500, 300}, BLACK, 32);

    if (does_rotate) {
        ra = rotate2(ra,zv, 2*M_PI*dt);
        rb = rotate2(rb,zv, 2*M_PI*dt);
        rc = rotate2(rc,zv, 2*M_PI*dt);
    }

    Vector2 center = get_relative(&bgfb, (Vector2){0.5, 0.5});
    Vector2 scale = (Vector2){0.2, 0.2};

    Vector3 rot[3] = {
        (Vector3){rb.x - ra.x, rb.y - ra.y, 0},
        (Vector3){rc.x - ra.x, rc.y - ra.y, 0},
        (Vector3){0, 0, 0}
    };

    Vector3 trans1[3] = {
        (Vector3){1, 0 ,image.width*scale.x*0.5},
        (Vector3){0, 1 ,image.height*scale.y*0.5},
        (Vector3){0, 0, 1}
    };

    Vector3 trans2[3] = {
        (Vector3){1, 0 ,-image.width*scale.x*0.5},
        (Vector3){0, 1, -image.height*scale.y*0.5},
        (Vector3){0, 0, 1}
    };

    Vector3 mtx1[3]; 
    Vector3 mtx[3]; 
    mtx_multiply(rot, trans2,  mtx);

    draw_texture_mtx(fb, image, (Vector2){center.x-image.width*scale.x*0.5, center.y-image.height*scale.y*0.5}, scale, mtx);

    // draw_triangle(fb, (Vector2){400, 300}, (Vector2){550,325}, (Vector2){550, 350}, BLUE);
}

void on_resize(int w, int h) {
    fb_t *fb = get_framebuffer();
    framebuffer_free(&bgfb);
    make_bgfb(w, h);
    draw_sceene(fb);
}

void on_finish_resize(int w, int h) {
}

void main_loop(void) {
    fb_t *fb = get_framebuffer();
    make_bgfb(fb->width, fb->height);
    rgba rcolor = BLACK;
    while (!window_should_close()) {
        sync_frames_start();
        poll_events();
        fb = get_framebuffer();

        if (is_key_pressed(GRAPHICS_KEY_Q)) {
            printf("Q Pressed\n");
            does_rotate = 0;
        }

        if (is_key_released(GRAPHICS_KEY_Q)) {
            printf("Q Released\n");
            does_rotate = 1;
        }

        draw_sceene(fb);

        update_framebuffer();
        sync_frames_end();
    }
}

int main(void) {
    init_window(600, 600, "hello, World!");

    static char path_buff[MAXPATHLEN+1] = {0};
    uint32_t path_size = MAXPATHLEN;
    _NSGetExecutablePath(path_buff, &path_size);
    path_size = strlen(path_buff);
    for (int i = path_size; i >= 0; --i) {
        if (path_buff[i] == '/') {
            path_buff[i+1] = 0;
            path_size = i+1;
            break;
        }
    }

    chdir(path_buff);

    if (chdir("./assets") == -1) {
        perror("chdir: assets");
        return 1;
    }

    if (!load_font("OpenDyslexic-Regular.otf", &font)) {
        printf("Couldn't load base font!\n");
        return 1;
    }

    if (!load_texture("test.jpg", &image)) {
        printf("Couldn't load image!\n");
    }

    set_resize_callback(on_resize);
    set_finish_resize_callback(on_finish_resize);

    main_loop();

    close_window();
    framebuffer_free(&bgfb);
    return 0;
}

