#include "moui_pattern.h"

static bool pattern_test(int x, int y, moui_pattern_t pat, uint8_t sp)
{
    if (sp < 2) sp = 2;
    switch (pat) {
    case MOUI_PAT_DOTS:         return (x % sp == 0) && (y % sp == 0);
    case MOUI_PAT_HLINES:       return (y % sp == 0);
    case MOUI_PAT_VLINES:       return (x % sp == 0);
    case MOUI_PAT_GRID:         return (x % sp == 0) || (y % sp == 0);
    case MOUI_PAT_DIAG_LEFT:    return ((x + y) % sp == 0);
    case MOUI_PAT_DIAG_RIGHT:   return ((x - y + sp * 100) % sp == 0);
    case MOUI_PAT_CROSS_HATCH:  return ((x + y) % sp == 0) || ((x - y + sp * 100) % sp == 0);
    case MOUI_PAT_BRICK:
        if (y % sp == 0) return true;
        if ((y / sp) & 1) return ((x + sp / 2) % sp == 0);
        return (x % sp == 0);
    }
    return false;
}

static inline int max_i(int a, int b) { return a > b ? a : b; }
static inline int min_i(int a, int b) { return a < b ? a : b; }

void moui_draw_fill_rect_pattern(moui_draw_ctx_t *ctx, const moui_rect_t *r,
                                moui_pattern_t pat, uint8_t spacing)
{
    int y0 = max_i(r->y, ctx->clip.y);
    int y1 = min_i(r->y + r->h, ctx->clip.y + ctx->clip.h);
    int x0 = max_i(r->x, ctx->clip.x);
    int x1 = min_i(r->x + r->w, ctx->clip.x + ctx->clip.w);
    if (x0 >= x1 || y0 >= y1) return;

    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            if (pattern_test(x, y, pat, spacing))
                moui_draw_pixel(ctx, x, y, MOUI_BLACK);
        }
    }
}

void moui_draw_fill_circle_pattern(moui_draw_ctx_t *ctx, int cx, int cy, int radius,
                                  moui_pattern_t pat, uint8_t spacing)
{
    int r2 = radius * radius;
    for (int y = cy - radius; y <= cy + radius; y++) {
        for (int x = cx - radius; x <= cx + radius; x++) {
            int dx = x - cx, dy = y - cy;
            if (dx * dx + dy * dy <= r2) {
                if (pattern_test(x, y, pat, spacing))
                    moui_draw_pixel(ctx, x, y, MOUI_BLACK);
            }
        }
    }
}
