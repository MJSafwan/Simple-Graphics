#include <stdio.h>
#include <stdlib.h>
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

void make_bgfb(int w, int h, float phase) {
    for (int y = 0; y < bgfb.height; ++y) {
        for (int x = 0; x < bgfb.width; ++x) {
            double theta = 2.0 * M_PI * (x / (double)bgfb.width);
            double phi = 2.0 * M_PI * (y / (double)bgfb.height);

            double a = sin(phi+phase) * cos(theta+phase);
            double b = sin(phi+phase) * sin(theta+phase);
            double c = cos(phi+phase);
            rgba color = {0};
            color.r = ((a+1)/2)*255;
            color.g = ((b+1)/2)*255;
            color.b = ((c+1)/2)*255;
            color.a = 255;
            set_pixel(&bgfb, x, y, color);
        }
    }
}

void draw_sceene(fb_t *fb) {
    float dt = get_deltatime();
    make_bgfb(fb->width, fb->height, 0);
    fbcpy(fb, &bgfb, (Vector2){0, 0});

    Vector2 center = get_relative(fb, (Vector2){0.5, 0.5});

    obj_t *ral_sheet = get_obj("RalSheet");
    ral_sheet->atlas.scale = (Vector2) {0.5, 0.5};
    ral_sheet->atlas.pos = center;

    obj_t *ral = get_obj("Ral");
    ral->image.pos = (Vector2) {25, 50};
    ral->image.scale = (Vector2) {0.25, 0.25};

    draw_atlas(fb, ral_sheet, dt);
    draw_image(fb, ral);
}

void on_resize(int w, int h) {
    fb_t *fb = get_framebuffer();
    framebuffer_free(&bgfb);
    bgfb = make_framebuffer(w, h);
    make_bgfb(w, h, 0);
    draw_sceene(fb);
}

void main_loop(void) {
    fb_t *fb = get_framebuffer();
    bgfb = make_framebuffer(fb->width, fb->height);
    make_bgfb(fb->width, fb->height, 0);
    rgba rcolor = BLACK;
    while (!window_should_close()) {
        sync_frames_start();
        poll_events();
        fb = get_framebuffer();
        draw_sceene(fb);

        update_framebuffer();
        sync_frames_end();
    }
}

void try_to_load_texture(char *name, texture *handle) {
    if (!load_texture(name, handle)) {
        printf("Couldn't load image '%s'!\n", name);
        exit(1);
    }
}

void try_to_load_atlas(char *name, int x, int y, texture_list *handle) {
    if (!init_from_atlas(name, x, y, handle)) {
        graphics_logf(LOG_INFO, "Couldn't find asset '%s'!\n", name);
        exit(1);
    }
}

int main(void) {
    init_window(800, 600, "hello, World!");

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
        exit(1);
    }

    if (!load_font("OpenDyslexic-Regular.otf", &font)) {
        printf("Couldn't load base font!\n");
        exit(1);
    }

    texture_list atlas = {0};
    texture fanart_image = {0};

    try_to_load_texture("Ralsei.jpg", &fanart_image);
    try_to_load_atlas("ralsei_sheet.png", 580, 800, &atlas);
    atlas.count--;

    obj_t *ral = register_obj("Ral", OBJ_IMAGE, &fanart_image);

    obj_t *ral_sheet = register_obj("RalSheet", OBJ_ATLAS, &atlas);
    ral_sheet->atlas.fps = 6;

    set_resize_callback(on_resize);

    main_loop();

    close_window();
    framebuffer_free(&bgfb);
    return 0;
}

