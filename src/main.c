#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include "graphics.h"

fb_t bgfb = {0};

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

font_ctx font = {0};
void on_resize(int w, int h) {
    fb_t *fb = get_framebuffer();
    fill_background(fb, WHITE);
    display_text(fb, font, "Resizing...", (Vector2){w/2 - 100, h/2}, BLACK, 32);
    framebuffer_free(&bgfb);
    make_bgfb(w, h);
}

extern int is_being_resized();

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

    texture image = {0};
    if (!load_texture("test.jpg", &image)) {
        printf("Couldn't load image!\n");
        return 1;
    }

    Vector2 a = {0, 0};
    Vector2 b = {1, 0};
    Vector2 c = {0, 1};
    Vector2 axis = {0, 0};
    double dtheta = 2*M_PI;

    a = rotate2(a, axis, 0.5);
    b = rotate2(b, axis, 0.5);
    c = rotate2(c, axis, 0.5);

    fb_t *fb = get_framebuffer();
    make_bgfb(fb->width, fb->height);

    set_resize_callback(on_resize);

    rgba rcolor = BLACK;
    while (!window_should_close()) {
        sync_frames_start();
        float dt = get_deltatime();
        fb = get_framebuffer();

        if (is_some_key_down()) {
            if (is_key_down(KEY_A)) {
                rcolor = BLUE;
            }
        }

        if (is_some_key_up()) {
            if (is_key_up(KEY_A)) {
                rcolor = BLACK;
            }
        }

        fbcpy(fb, &bgfb, (Vector2){0, 0});
        set_pixel(fb, 300, 300, BLUE);
        static char sn_tmp[1028] = {0};
        snprintf(sn_tmp, 1028, "fps %d", (int)(1/dt));
        display_text(fb, font, sn_tmp, (Vector2){350, 200}, GREEN, 32);

        draw_rect(fb, (Vector2){400, 400}, (Vector2){100, 150}, rcolor);
        draw_circle(fb, (Vector2){100, 400}, 50, RED);
        draw_line(fb, (Vector2){100, 500}, (Vector2){200, 550}, 3, GREEN); 

        Vector3 rot_mtx[3] = {(Vector3){b.x-a.x, b.y-a.y, 0}, (Vector3){c.x-a.x, c.y-a.y, 0}, (Vector3){0}};
        Vector3 trans_mtx[3] = {(Vector3){1, 0, -100}, (Vector3){0, 1, -100}, (Vector3){0, 0, 1}};
        Vector3 mtx[3] = {0};


        mtx_multiply(rot_mtx, trans_mtx, mtx);
        draw_texture_mtx(fb, image, (Vector2){200, 200}, (Vector2){0.2, 0.2}, mtx);
        a = rotate2(a, axis, dtheta*dt);
        b = rotate2(b, axis, dtheta*dt);
        c = rotate2(c, axis, dtheta*dt);

        draw_triangle(fb, (Vector2){400, 300}, (Vector2){550,325}, (Vector2){550, 350}, BLUE);

        update_framebuffer();
        poll_events();
        sync_frames_end();
    }
    close_window();
    framebuffer_free(&bgfb);
    return 0;
}

