#include <stdio.h>
#include <string.h>
#include <math.h>

#include "graphics.h"

static void draw_bitmap(fb_t *fb, FT_Bitmap* bitmap, int x, int y, rgba color) {
    int x_max = x + bitmap->width;
    int y_max = y + bitmap->rows;

    for (int i = x; i < x_max; i++) {
        int p = i - x;
        for (int j = y; j < y_max; j++) {
            int q = j - y;
            float val = bitmap->buffer[q * bitmap->pitch + p] / 255.0;
            rgba bg = get_pixel(fb, i, j);
            rgba color_to_set = {0};
            color_to_set.r = bg.r + (color.r - bg.r)*val;
            color_to_set.g = bg.g + (color.g - bg.g)*val;
            color_to_set.b = bg.b + (color.b - bg.b)*val;
            color_to_set.a = 255;
            set_pixel(fb, i, j, color_to_set);
        }
    }
}


void display_text(fb_t *fb, font_ctx ctx, char *text, Vector2 pos, rgba color, int pixel_size) {
  FT_Set_Pixel_Sizes(ctx.face, 0, pixel_size);
  FT_Vector pen = {0};
  pen.x = pos.x;
  pen.y = pos.y;
  FT_GlyphSlot slot = ctx.face->glyph;
  for (size_t n = 0; text[n] != 0; n++) {
      if (FT_Load_Glyph(ctx.face, FT_Get_Char_Index(ctx.face, text[n]), FT_LOAD_DEFAULT))
          continue;

      FT_Render_Glyph(ctx.face->glyph, FT_RENDER_MODE_NORMAL);

      draw_bitmap(fb, &slot->bitmap, 
              pen.x + slot->bitmap_left, 
              pen.y - slot->bitmap_top,
              color);

      pen.x += slot->advance.x >> 6;
  }
}

extern FT_Library get_freetype();

int load_font(char *filename, font_ctx *ctx) {
    if (FT_New_Face(get_freetype(), filename, 0, &ctx->face)) {
        return 0;
    }
    return 1;
}

void font_free(font_ctx ctx) {
    FT_Done_Face(ctx.face);
}
