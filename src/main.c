#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include "graphics.h"

fb_t bgfb = {0};
font_ctx font = {0};
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
    Vector2 center = get_relative(&bgfb, (Vector2){0.5, 0.5});
    draw_texture(&bgfb, image, (Vector2){center.x-image.width*0.2*0.5, center.y-image.height*0.2*0.5}, (Vector2){0.2, 0.2});
}


void draw_sceene(fb_t *fb) {
    float dt = get_deltatime();
    static char sn_tmp[1028] = "FPS N/A";
    fbcpy(get_framebuffer(), &bgfb, (Vector2){0, 0});
    snprintf(sn_tmp, 1028, "fps %d", (int)(1/dt));
    display_text(fb, font, sn_tmp, (Vector2){0, 60}, BLACK, 32);
    // printf("FPS %f\n", 1/dt);

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
        fb = get_framebuffer();

        draw_sceene(fb);

        update_framebuffer();
        poll_events();
        sync_frames_end();
    }
}

int main(void) {
    init_window(600, 600, "hello, World!");

    if (chdir("assets") == -1) {
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

