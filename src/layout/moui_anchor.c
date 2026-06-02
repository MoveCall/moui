#include "moui_anchor.h"

void moui_anchor_apply(const moui_anchor_t *anchor, moui_widget_t *w, const moui_rect_t *parent_bounds)
{
    if (!anchor || !w || !parent_bounds) return;

    uint8_t f = anchor->flags;
    int16_t pw = parent_bounds->w;
    int16_t ph = parent_bounds->h;
    int16_t px = parent_bounds->x;
    int16_t py = parent_bounds->y;

    /* Horizontal Anchors */
    if ((f & MOUI_ANCHOR_LEFT) && (f & MOUI_ANCHOR_RIGHT)) {
        w->bounds.x = px + anchor->margin_left;
        w->bounds.w = pw - anchor->margin_left - anchor->margin_right;
    } else if (f & MOUI_ANCHOR_RIGHT) {
        w->bounds.x = px + pw - w->bounds.w - anchor->margin_right;
    } else if (f & MOUI_ANCHOR_LEFT) {
        w->bounds.x = px + anchor->margin_left;
    } else if (f & MOUI_ANCHOR_CENTER_H) {
        w->bounds.x = px + (pw - w->bounds.w) / 2;
    }

    /* Vertical Anchors */
    if ((f & MOUI_ANCHOR_TOP) && (f & MOUI_ANCHOR_BOTTOM)) {
        w->bounds.y = py + anchor->margin_top;
        w->bounds.h = ph - anchor->margin_top - anchor->margin_bottom;
    } else if (f & MOUI_ANCHOR_BOTTOM) {
        w->bounds.y = py + ph - w->bounds.h - anchor->margin_bottom;
    } else if (f & MOUI_ANCHOR_TOP) {
        w->bounds.y = py + anchor->margin_top;
    } else if (f & MOUI_ANCHOR_CENTER_V) {
        w->bounds.y = py + (ph - w->bounds.h) / 2;
    }
}
