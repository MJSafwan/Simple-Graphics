#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>

#include "internals.h"

#define PLATFORM_NCURSES

extern core_t core;

struct timer {
    struct timespec frame_start, frame_end;
};


void uninit_platform() {
    endwin(); 
}

static char ctable[] = " .:'\"-=*%$@";

static char cchar(rgba c) {
    float norm = ((c.r+c.g+c.b)/(255.0f*3.0f));
    size_t len = sizeof(ctable)-1;
    if (norm == 1) {
        return ctable[len-1];
    }
    return ctable[(int)floorf(norm*(float)(len))];
}

char *platform_name = "ncurses";

#define PLATFORM_KEY_A 'a' 
#define PLATFORM_KEY_S 's'
#define PLATFORM_KEY_D 'd'
#define PLATFORM_KEY_F 'f'
#define PLATFORM_KEY_H 'h'
#define PLATFORM_KEY_G 'g'
#define PLATFORM_KEY_Z 'z'
#define PLATFORM_KEY_X 'x'
#define PLATFORM_KEY_C 'c'
#define PLATFORM_KEY_V 'v'
#define PLATFORM_KEY_B 'b'
#define PLATFORM_KEY_Q 'q'
#define PLATFORM_KEY_W 'w'
#define PLATFORM_KEY_E 'e'
#define PLATFORM_KEY_R 'r'
#define PLATFORM_KEY_Y 'y'
#define PLATFORM_KEY_T 't'
#define PLATFORM_KEY_O 'o'
#define PLATFORM_KEY_U 'u'
#define PLATFORM_KEY_I 'i'
#define PLATFORM_KEY_P 'p'
#define PLATFORM_KEY_L 'l'
#define PLATFORM_KEY_J 'j'
#define PLATFORM_KEY_K 'k'
#define PLATFORM_KEY_N 'n'
#define PLATFORM_KEY_M 'm'

static void register_keys() {
#define X(a, b) core.events.keycode_translation[b] = a;
    KEYCODEX
#undef X
}

void update_framebuffer() {
    fb_t *fb = get_framebuffer();
    for (int y = 0; y < fb->height; ++y) {
        for (int x = 0; x < fb->width; ++x) {
            int i = x + y*fb->width;
            /*
            if (!fb->dirty[i])
                continue;
            fb->dirty[i] = 0;
            */
            char *possible_text = ((char *)fb->userdata);
            if (possible_text[i]) {
                mvaddch(y, x, possible_text[i]);
                possible_text[i] = 0; 
                continue;
            }
            rgba color = fb->data[i];
            mvaddch(y, x, cchar(color));
        }
    }
    fb->dirty_box.minx = 0;
    fb->dirty_box.miny = 0;
    fb->dirty_box.maxx = 0;
    fb->dirty_box.maxy = 0;
    refresh();
}



void poll_events() {
    int ch = 0;
    while ((ch = getch()) != ERR) {
        int keycode_generic = core.events.keycode_translation[ch];
        core.events.keycode[keycode_generic] = ~core.events.keycode[keycode_generic] & 0b0011;
    }
}


void sync_frames_start() {
    clock_gettime(CLOCK_MONOTONIC, &core.timer->frame_start);
}

void sync_frames_end() {
    clock_gettime(CLOCK_MONOTONIC, &core.timer->frame_end);
    int64_t etn = (int64_t)core.timer->frame_end.tv_sec*1000000000 + (int64_t)core.timer->frame_end.tv_nsec;
    int64_t stn = (int64_t)core.timer->frame_start.tv_sec*1000000000 + (int64_t)core.timer->frame_start.tv_nsec;
    core.dt = (etn-stn)/(1000000000.0);
}

void init_platform() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
    resizeterm(core.window.width, core.window.height);
    core.timer = malloc(sizeof(timer_t));
    core.window.framebuffer = make_framebuffer(core.window.width, core.window.height);
    core.window.framebuffer.userdata = malloc(core.window.width * core.window.height);
    memset(core.window.framebuffer.userdata, 0, core.window.width * core.window.height);

    register_keys();
}
