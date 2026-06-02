#include "moui_widget_scroll.h"
#include "../core/moui_theme.h"
#include "../anim/moui_ease.h"
#include "../core/moui_theme.h"

static void scroll_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_scroll_t *s = (moui_widget_scroll_t *)w;

    moui_draw_push_clip(ctx, &w->bounds);

    int scroll_y = (int)w->anim_val[0];
    for (int i = 0; i < s->child_count; i++) {
        moui_widget_t *ch = s->children[i];
        moui_rect_t saved = ch->bounds;
        ch->bounds.y -= scroll_y;
        if (ch->bounds.y + ch->bounds.h >= w->bounds.y &&
            ch->bounds.y < w->bounds.y + w->bounds.h) {
            moui_widget_draw(ch, ctx);
        }
        ch->bounds = saved;
    }

    moui_draw_pop_clip(ctx);

    if (s->show_scrollbar && s->content_h > w->bounds.h) {
        int bar_h = w->bounds.h;
        int thumb_h = bar_h * w->bounds.h / s->content_h;
        if (thumb_h < 6) thumb_h = 6;
        int max_scroll = s->content_h - w->bounds.h;
        int thumb_y = w->bounds.y;
        if (max_scroll > 0)
            thumb_y += (int)((float)scroll_y / max_scroll * (bar_h - thumb_h));
        int sb_x = w->bounds.x + w->bounds.w - 3;
        moui_draw_fill_rounded_rect(ctx, &(moui_rect_t){sb_x, thumb_y, 2, thumb_h}, 1, moui_theme_fg());
    }

    if (moui_widget_has_state(w, MOUI_STATE_FOCUSED) && s->capturing) {
        moui_draw_rect(ctx, &w->bounds, moui_theme_fg());
    }
}

static bool scroll_on_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_widget_scroll_t *s = (moui_widget_scroll_t *)w;
    int max_scroll = s->content_h - w->bounds.h;
    if (max_scroll < 0) max_scroll = 0;

    /* Touch events bypass the capturing gate */
    if (ev->type == MOUI_EV_GESTURE_SWIPE_DOWN) {
        w->anim_tgt[0] += s->scroll_step * 3;
        if (w->anim_tgt[0] > (float)max_scroll) w->anim_tgt[0] = (float)max_scroll;
        return true;
    }
    if (ev->type == MOUI_EV_GESTURE_SWIPE_UP) {
        w->anim_tgt[0] -= s->scroll_step * 3;
        if (w->anim_tgt[0] < 0) w->anim_tgt[0] = 0;
        return true;
    }
    if (ev->type == MOUI_EV_POINTER_DOWN) {
        s->drag_start_y = ev->y;
        s->drag_base_scroll = (int16_t)w->anim_val[0];
        s->dragging = true;
        return true;
    }
    if (ev->type == MOUI_EV_POINTER_MOVE && s->dragging) {
        int dy = s->drag_start_y - ev->y;
        float target = (float)(s->drag_base_scroll + dy);
        if (target < 0) target = 0;
        if (target > (float)max_scroll) target = (float)max_scroll;
        w->anim_tgt[0] = target;
        w->anim_val[0] = target;
        return true;
    }
    if (ev->type == MOUI_EV_POINTER_UP) {
        s->dragging = false;
        return true;
    }

    /* Encoder events require capturing */
    if (!s->capturing) {
        if (ev->type == MOUI_EV_ENCODER_PRESS) {
            s->capturing = true;
            return true;
        }
        return false;
    }

    switch (ev->type) {
    case MOUI_EV_ENCODER_CW:
        w->anim_tgt[0] += s->scroll_step;
        if (w->anim_tgt[0] > max_scroll) w->anim_tgt[0] = (float)max_scroll;
        w->anim_val[0] = w->anim_tgt[0];
        return true;
    case MOUI_EV_ENCODER_CCW:
        w->anim_tgt[0] -= s->scroll_step;
        if (w->anim_tgt[0] < 0) w->anim_tgt[0] = 0;
        w->anim_val[0] = w->anim_tgt[0];
        return true;
    case MOUI_EV_ENCODER_PRESS:
    case MOUI_EV_ENCODER_BACK:
        s->capturing = false;
        return true;
    default:
        return false;
    }
}

static void scroll_on_focus(moui_widget_t *w, bool gained)
{
    moui_widget_scroll_t *s = (moui_widget_scroll_t *)w;
    if (gained) {
        if (w->focus_next == w)
            s->capturing = true;
    } else {
        s->capturing = false;
    }
}

static const moui_widget_vtable_t scroll_vtable = {
    .draw     = scroll_draw,
    .on_event = scroll_on_event,
    .on_focus = scroll_on_focus,
};

void moui_scroll_init(moui_widget_scroll_t *s)
{
    moui_widget_init(&s->base, &scroll_vtable);
    s->child_count    = 0;
    s->content_h      = 0;
    s->scroll_step    = 20;
    s->capturing      = false;
    s->show_scrollbar = true;
    s->dragging       = false;
}

void moui_scroll_add(moui_widget_scroll_t *s, moui_widget_t *child)
{
    if (s->child_count >= MOUI_SCROLL_MAX_CHILDREN) return;
    s->children[s->child_count++] = child;
    child->parent = &s->base;
}

void moui_scroll_set_content_height(moui_widget_scroll_t *s, int16_t h)
{
    s->content_h = h;
}
