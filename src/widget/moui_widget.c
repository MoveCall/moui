#include "moui_widget.h"
#include <string.h>

void moui_widget_init(moui_widget_t *w, const moui_widget_vtable_t *vtable)
{
    memset(w, 0, sizeof(*w));
    w->vtable  = vtable;
    w->visible = 1;
    w->enabled = 1;
    w->dirty   = 1;
    w->w_hint  = -1;
    w->h_hint  = -1;
}

static inline bool rect_intersects(const moui_rect_t *a, const moui_rect_t *b)
{
    return !(a->x + a->w <= b->x || a->x >= b->x + b->w ||
             a->y + a->h <= b->y || a->y >= b->y + b->h);
}

void moui_widget_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    if (!w->visible) return;
    if (w->bounds.w > 0 && w->bounds.h > 0) {
        if (!rect_intersects(&w->bounds, &ctx->clip)) return;
    }
    if (w->vtable && w->vtable->draw)
        w->vtable->draw(w, ctx);
}

bool moui_widget_send_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    if (!w->enabled) return false;
    if (w->vtable && w->vtable->on_event)
        return w->vtable->on_event(w, ev);
    return false;
}
