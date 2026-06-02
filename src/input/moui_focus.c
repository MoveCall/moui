#include "moui_focus.h"
#include <string.h>

void moui_focus_group_init(moui_focus_group_t *fg)
{
    memset(fg, 0, sizeof(*fg));
    fg->wrap = true;
}

void moui_focus_group_add(moui_focus_group_t *fg, moui_widget_t *w)
{
    if (!w) return;

    /* If already in a focus chain, remove it first to allow safe re-addition */
    if (w->focus_next != NULL) {
        if (w->focus_next == w && fg->focused == w) return;
        moui_focus_group_remove(fg, w);
    }

    if (!fg->focused) {
        fg->focused = w;
        w->focus_next = w;
        w->focus_prev = w;
        return;
    }

    moui_widget_t *last = fg->focused->focus_prev;
    last->focus_next = w;
    w->focus_prev = last;
    w->focus_next = fg->focused;
    fg->focused->focus_prev = w;
}

void moui_focus_group_remove(moui_focus_group_t *fg, moui_widget_t *w)
{
    if (!w || w->focus_next == NULL) return;

    if (fg->focused == w) {
        if (w->focus_next == w) {
            fg->focused = NULL;
        } else {
            fg->focused = w->focus_next;
        }
    }

    w->focus_prev->focus_next = w->focus_next;
    w->focus_next->focus_prev = w->focus_prev;
    w->focus_next = NULL;
    w->focus_prev = NULL;
}

void moui_focus_group_reset(moui_focus_group_t *fg)
{
    moui_widget_t *w = fg->focused;
    if (!w) return;
    moui_widget_t *first = w;
    int guard = MOUI_SCREEN_MAX_WIDGETS * MOUI_SCREEN_STACK_SIZE + 8;
    do {
        moui_widget_t *next = w->focus_next;
        w->focus_next = NULL;
        w->focus_prev = NULL;
        w = next;
    } while (w != first && w != NULL && --guard > 0);
    fg->focused = NULL;
}

static bool widget_focusable(const moui_widget_t *w)
{
    return w->visible && w->enabled;
}

static void move_focus(moui_focus_group_t *fg, bool forward)
{
    if (!fg->focused) return;

    moui_widget_t *start = fg->focused;
    moui_widget_t *candidate = forward ? start->focus_next : start->focus_prev;

    int guard = MOUI_SCREEN_MAX_WIDGETS * MOUI_SCREEN_STACK_SIZE + 8;
    while (candidate != start && --guard > 0) {
        if (widget_focusable(candidate)) break;
        candidate = forward ? candidate->focus_next : candidate->focus_prev;
    }

    if (candidate == start) {
        if (!fg->wrap) return;
        if (!widget_focusable(candidate)) return;
    }

    if (candidate == fg->focused) return;

    moui_widget_t *old = fg->focused;
    moui_widget_clear_state(old, MOUI_STATE_FOCUSED);
    if (old->vtable && old->vtable->on_focus)
        old->vtable->on_focus(old, false);

    fg->focused = candidate;
    moui_widget_set_state(candidate, MOUI_STATE_FOCUSED);
    if (candidate->vtable && candidate->vtable->on_focus)
        candidate->vtable->on_focus(candidate, true);
}

void moui_focus_next(moui_focus_group_t *fg) { move_focus(fg, true); }
void moui_focus_prev(moui_focus_group_t *fg) { move_focus(fg, false); }

void moui_focus_set(moui_focus_group_t *fg, moui_widget_t *w)
{
    if (fg->focused == w) return;
    if (fg->focused) {
        moui_widget_clear_state(fg->focused, MOUI_STATE_FOCUSED);
        if (fg->focused->vtable && fg->focused->vtable->on_focus)
            fg->focused->vtable->on_focus(fg->focused, false);
    }
    fg->focused = w;
    if (w) {
        moui_widget_set_state(w, MOUI_STATE_FOCUSED);
        if (w->vtable && w->vtable->on_focus)
            w->vtable->on_focus(w, true);
    }
}

void moui_focus_activate_first(moui_focus_group_t *fg)
{
    if (!fg->focused) return;
    moui_widget_set_state(fg->focused, MOUI_STATE_FOCUSED);
    if (fg->focused->vtable && fg->focused->vtable->on_focus)
        fg->focused->vtable->on_focus(fg->focused, true);
}

bool moui_focus_dispatch(moui_focus_group_t *fg, const moui_input_event_t *ev)
{
    if (!fg->focused) return false;

    if (moui_widget_send_event(fg->focused, ev))
        return true;

    /* Bubble to parent chain */
    moui_widget_t *p = fg->focused->parent;
    while (p) {
        if (moui_widget_send_event(p, ev))
            return true;
        p = p->parent;
    }

    switch (ev->type) {
    case MOUI_EV_ENCODER_CW:   moui_focus_next(fg); return true;
    case MOUI_EV_ENCODER_CCW:  moui_focus_prev(fg); return true;
    default: return false;
    }
}
