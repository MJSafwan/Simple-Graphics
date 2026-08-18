#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "internals.h"

#define PLATFORM_NCURSES

extern core_t core;

struct timer {
    struct timespec frame_start, frame_end;
};

void init_platform() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    resizeterm(core.window.width, core.window.height);
    core.timer = malloc(sizeof(timer_t));
    core.window.framebuffer = make_framebuffer(core.window.width, core.window.height);
    core.window.framebuffer.userdata = malloc(core.window.width * core.window.height);
    memset(core.window.framebuffer.userdata, 0, core.window.width * core.window.height);
}

void uninit_platform() {
    endwin(); 
}

char ctable[] = " .:'\"-=*%$@";

char cchar(rgba c) {
    float norm = ((c.r+c.g+c.b)/(255.0f*3.0f));
    size_t len = sizeof(ctable)-1;
    if (norm == 1) {
        return ctable[len-1];
    }
    return ctable[(int)floorf(norm*(float)(len))];
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
    return;
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
