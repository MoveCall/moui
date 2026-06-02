#include "moui_dither.h"

static inline int max_i(int a, int b) { return a > b ? a : b; }
static inline int min_i(int a, int b) { return a < b ? a : b; }

void moui_draw_fill_rect_dither(moui_draw_ctx_t *ctx, const moui_rect_t *r,
                               moui_dither_pattern_t pattern)
{
    int y0 = max_i(r->y, ctx->clip.y);
    int y1 = min_i(r->y + r->h, ctx->clip.y + ctx->clip.h);
    int x0 = max_i(r->x, ctx->clip.x);
    int x1 = min_i(r->x + r->w, ctx->clip.x + ctx->clip.w);
    if (x0 >= x1 || y0 >= y1) return;

    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            if (moui_dither_pixel(x, y, pattern))
                moui_draw_pixel(ctx, x, y, MOUI_BLACK);
        }
    }
}

void moui_draw_fill_circle_dither(moui_draw_ctx_t *ctx, int cx, int cy, int radius,
                                 moui_dither_pattern_t pattern)
{
    int x = 0, y = radius, d = 3 - 2 * radius;
    while (x <= y) {
        for (int i = cx - x; i <= cx + x; i++) {
            if (moui_dither_pixel(i, cy + y, pattern)) moui_draw_pixel(ctx, i, cy + y, MOUI_BLACK);
            if (moui_dither_pixel(i, cy - y, pattern)) moui_draw_pixel(ctx, i, cy - y, MOUI_BLACK);
        }
        for (int i = cx - y; i <= cx + y; i++) {
            if (moui_dither_pixel(i, cy + x, pattern)) moui_draw_pixel(ctx, i, cy + x, MOUI_BLACK);
            if (moui_dither_pixel(i, cy - x, pattern)) moui_draw_pixel(ctx, i, cy - x, MOUI_BLACK);
        }
        if (d < 0) d += 4 * x + 6;
        else { d += 4 * (x - y) + 10; y--; }
        x++;
    }
}
