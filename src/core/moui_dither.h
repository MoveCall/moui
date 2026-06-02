#ifndef MOUI_DITHER_H
#define MOUI_DITHER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_draw.h"

typedef enum {
    MOUI_DITHER_25 = 0,
    MOUI_DITHER_50,
    MOUI_DITHER_75,
    MOUI_DITHER_CHECKER,
    MOUI_DITHER_HLINE,
    MOUI_DITHER_VLINE,
    MOUI_DITHER_DIAG,
} moui_dither_pattern_t;

void moui_draw_fill_rect_dither(moui_draw_ctx_t *ctx, const moui_rect_t *r,
                               moui_dither_pattern_t pattern);

void moui_draw_fill_circle_dither(moui_draw_ctx_t *ctx, int cx, int cy, int radius,
                                 moui_dither_pattern_t pattern);

static inline bool moui_dither_pixel(int x, int y, moui_dither_pattern_t pattern)
{
    switch (pattern) {
    case MOUI_DITHER_25:
        return ((x & 1) == 0 && (y & 1) == 0);
    case MOUI_DITHER_50:
        return ((x + y) & 1) == 0;
    case MOUI_DITHER_75:
        return !((x & 1) == 0 && (y & 1) == 0);
    case MOUI_DITHER_CHECKER:
        return ((x >> 1) + (y >> 1)) & 1;
    case MOUI_DITHER_HLINE:
        return (y & 1) == 0;
    case MOUI_DITHER_VLINE:
        return (x & 1) == 0;
    case MOUI_DITHER_DIAG:
        return ((x + y) & 3) == 0;
    }
    return false;
}


#ifdef __cplusplus
}
#endif
#endif
