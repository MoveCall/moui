#ifndef MOUI_ANCHOR_H
#define MOUI_ANCHOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../widget/moui_widget.h"

typedef enum {
    MOUI_ANCHOR_NONE        = 0,
    MOUI_ANCHOR_LEFT        = (1 << 0),
    MOUI_ANCHOR_RIGHT       = (1 << 1),
    MOUI_ANCHOR_TOP         = (1 << 2),
    MOUI_ANCHOR_BOTTOM      = (1 << 3),
    MOUI_ANCHOR_CENTER_H    = (1 << 4),
    MOUI_ANCHOR_CENTER_V    = (1 << 5),
    MOUI_ANCHOR_FILL        = (MOUI_ANCHOR_LEFT | MOUI_ANCHOR_RIGHT | MOUI_ANCHOR_TOP | MOUI_ANCHOR_BOTTOM),
} moui_anchor_flags_t;

typedef struct {
    uint8_t flags;
    int16_t margin_left;
    int16_t margin_right;
    int16_t margin_top;
    int16_t margin_bottom;
} moui_anchor_t;

void moui_anchor_apply(const moui_anchor_t *anchor, moui_widget_t *w, const moui_rect_t *parent_bounds);

#ifdef __cplusplus
}
#endif
#endif
