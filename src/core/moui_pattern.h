#ifndef MOUI_PATTERN_H
#define MOUI_PATTERN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_draw.h"

typedef enum {
    MOUI_PAT_DOTS,
    MOUI_PAT_HLINES,
    MOUI_PAT_VLINES,
    MOUI_PAT_GRID,
    MOUI_PAT_DIAG_LEFT,
    MOUI_PAT_DIAG_RIGHT,
    MOUI_PAT_CROSS_HATCH,
    MOUI_PAT_BRICK,
} moui_pattern_t;

void moui_draw_fill_rect_pattern(moui_draw_ctx_t *ctx, const moui_rect_t *r,
                                moui_pattern_t pat, uint8_t spacing);

void moui_draw_fill_circle_pattern(moui_draw_ctx_t *ctx, int cx, int cy, int radius,
                                  moui_pattern_t pat, uint8_t spacing);


#ifdef __cplusplus
}
#endif
#endif
