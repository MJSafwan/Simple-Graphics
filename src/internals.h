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
        resize_callback_t resize_callback;
        finish_resize_callback_t finish_resize_callback;
        keydown_callback_t keydown_callback;
        keyup_callback_t keyup_callback;
        /* todo: mouse callbacks */
    } callbacks;

    struct {
        char type[255];
        char keycode[255];
        /* todo: mouse buttons */
    } events;
} core_t;


#endif
