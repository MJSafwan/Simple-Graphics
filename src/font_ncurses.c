#include <ncurses.h>
#include "internals.h"


void init_font_library() {
    return;
}
void uninit_font_library() {
    return;
}
int load_font(char *filename, font_t *ctx) {
    return 1;
}
void display_text(fb_t *fb, font_t *ctx, char *text, Vector2 pos, rgba color, int pixel_size) {
    int i = pos.x + pos.y * fb->width;
    char *possible_text = ((char *)fb->userdata);
    for (int j = 0; text[j] != 0; ++j) {
        possible_text[i+j] = text[j];
    }
}
void font_free(font_t *ctx) {
    return;
}
