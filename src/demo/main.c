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
texture image = {0};
texture fanart_image = {0};

Vector2 get_relative(fb_t *fb, Vector2 v) {
    return (Vector2){v.x*fb->width, v.y*fb->height};
}

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

Vector2 zv = {0, 0};
Vector2 ra = {0, 0};
Vector2 rb = {1, 0};
Vector2 rc = {0, 1};

int does_rotate = 1;
float counter = 0;
float map_counter = 0;
float sparkle_counter = 0;
float phase_counter = 0;
texture_list atlas = {0};
texture_list waving_atlas = {0};
texture_list pat_atlas = {0};
texture_list sparkle_atlas = {0};
rgba color_map[] = {RED, CYAN, BLUE, MAGENTA, GREEN, YELLOW};
size_t color_map_len = sizeof(color_map)/sizeof(rgba);

Vector2 get_center(texture t, Vector2 abs_center, Vector2 scale) {
    return (Vector2){abs_center.x - (t.width*scale.x)/2, abs_center.y - (t.height*scale.y)/2};
}

void draw_sceene(fb_t *fb) {
    float dt = get_deltatime();
    make_bgfb(fb->width, fb->height, phase_counter);
    fbcpy(fb, &bgfb, (Vector2){0, 0});
    counter += dt*6;
    map_counter += dt*3;
    phase_counter += dt;
    sparkle_counter += dt*6;
    if (counter >= atlas.count) {
        counter = 0;
    }
    if (sparkle_counter >= sparkle_atlas.count) {
        sparkle_counter = 0;
    }

    if (map_counter >= color_map_len) {
        map_counter = 0;
    }

    Vector2 center = get_relative(&bgfb, (Vector2){0.5, 0.5});
    Vector2 scale = (Vector2){0.5, 0.5};

    /* todo: function to get width of text */
    /* todo: add shaders function: rendering pipeline */

    display_text(fb, font, "Good Morning!", (Vector2){center.x-150, 100}, color_map[(int)map_counter], 32);
    texture ral_frame = atlas.items[(int)counter];
    texture waving_frame = waving_atlas.items[(int)counter];
    texture pat_frame = pat_atlas.items[(int)counter];
    Vector2 waving_scale = {2.5, 2.5};
    Vector2 pat_scale = {0.35, 0.35};
    draw_texture(fb, ral_frame, get_center(ral_frame, center, scale), scale);
    draw_texture(fb, waving_frame, get_center(waving_frame, (Vector2){100, 500}, waving_scale), waving_scale);
    draw_texture(fb, pat_frame, get_center(pat_frame, (Vector2){650, 200}, pat_scale), pat_scale);

    Vector2 image_scale = {0.35, 0.35};
    draw_texture(fb, image, get_center(image, (Vector2){650, 500}, image_scale), image_scale);
    Vector2 fanart_scale = {0.25, 0.25};
    draw_texture(fb, fanart_image, (Vector2){25, 50}, fanart_scale);

    texture sparkle_frame = sparkle_atlas.items[(int)sparkle_counter];
    Vector2 sparkle_scale = {2.5, 2.5};
    draw_texture(fb, sparkle_frame, get_center(sparkle_frame, center, sparkle_scale), sparkle_scale);
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

    if (!load_texture("ral.png", &image)) {
        printf("Couldn't load image!\n");
        exit(1);
    }

    if (!load_texture("Ralsei.jpg", &fanart_image)) {
        printf("Couldn't load image!\n");
        exit(1);
    }

    if (!init_from_atlas("ralsei_sheet.png", 580, 800, &atlas)) {
        graphics_logf(LOG_INFO, "Couldn't find asset 'ralsei_sheet.png'!\n");
        exit(1);
    }
    atlas.count--;

    if (!init_from_atlas("ral_waving.png", 111, 162, &waving_atlas)) {
        graphics_logf(LOG_INFO, "Couldn't find asset 'ral_waving.png'!\n");
        exit(1);
    }
    waving_atlas.count = atlas.count;

    if (!init_from_atlas("ral_pat.png", 498, 498, &pat_atlas)) {
        graphics_logf(LOG_INFO, "Couldn't find asset 'ral_pat.png'!\n");
        exit(1);
    }
    if (!init_from_atlas("sparkle.png", 350, 350, &sparkle_atlas)) {
        graphics_logf(LOG_INFO, "Couldn't find asset 'ral_pat.png'!\n");
        exit(1);
    }
    pat_atlas.count = atlas.count;

    set_resize_callback(on_resize);

    main_loop();

    close_window();
    framebuffer_free(&bgfb);
    return 0;
}

