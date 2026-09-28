#include "graphics.h"

#ifndef INTERNALS_H_
#define INTERNALS_H_

typedef struct window_handle window_handle_t;
typedef struct screen screen_t;
typedef struct timer timer_t;
typedef void *font_library_t;

typedef struct {
    fb_t framebuffer;
    window_handle_t *handle;
    int width;
    int height;
    Vector2 screen_pos;
    int window_should_close;
    char *title;
} window_t;

typedef struct {
    window_t window;
    screen_t *current_screen;
    timer_t *timer;

    font_library_t font_library;
    float dt;

    struct {
        char *rendering_backend;
        char *font_backend;
    } platform_info;

    struct {
        resize_callback_t resize_callback;
        finish_resize_callback_t finish_resize_callback;
        keydown_callback_t keydown_callback;
        keyup_callback_t keyup_callback;
        rmouse_down_callback_t rmouse_down_callback;
        lmouse_down_callback_t lmouse_down_callback;
        rmouse_up_callback_t rmouse_up_callback;
        lmouse_up_callback_t lmouse_up_callback;   
        mouse_moved_callback_t mouse_moved_callback;
        keypress_callback_t keypress_callback;
        keyrelease_callback_t keyrelease_callback;
    } callbacks;

    struct {
        char keycode[255];
        char keycode_translation[255];
        char mouse_buttons[16];
        Vector2 mouse_pos;
        Vector2 window_mouse_pos;
    } events;
} core_t;

#endif
