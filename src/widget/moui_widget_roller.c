#include "moui_widget_roller.h"
#include "../core/moui_theme.h"
#include <string.h>

static void roller_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_roller_t *r = (moui_widget_roller_t *)w;
    moui_rect_t b = w->bounds;
    const moui_font_t *f = r->font ? r->font : &moui_font_ascii_6x8;

    moui_draw_rounded_rect(ctx, &b, 3, moui_theme_fg());

    if (r->item_count == 0) return;

    int item_h = f->line_height + 4;
    int mid_y = b.y + (b.h - item_h) / 2;

    /* Center Highlight Box */
    moui_rect_t mid_r = { (int16_t)(b.x + 2), (int16_t)mid_y, (int16_t)(b.w - 4), (int16_t)item_h };
    moui_draw_fill_rect(ctx, &mid_r, moui_theme_fg());

    /* Selected Item (Inverted Text) */
    if (r->selected < r->item_count && r->items[r->selected]) {
        int tw = moui_font_measure_str(f, r->items[r->selected]);
        int tx = b.x + (b.w - tw) / 2;
        int ty = mid_y + (item_h - f->line_height) / 2;
        moui_font_draw_str(ctx, f, tx, ty, r->items[r->selected], moui_theme_bg());
    }

    /* Top Item (Previous) */
    int prev_idx = (int)r->selected - 1;
    if (prev_idx < 0 && r->loop && r->item_count > 0) prev_idx = r->item_count - 1;
    if (prev_idx >= 0 && prev_idx < r->item_count && r->items[prev_idx]) {
        int tw = moui_font_measure_str(f, r->items[prev_idx]);
        int tx = b.x + (b.w - tw) / 2;
        int ty = mid_y - item_h + (item_h - f->line_height) / 2;
        if (ty >= b.y)
            moui_font_draw_str(ctx, f, tx, ty, r->items[prev_idx], moui_theme_fg());
    }

    /* Bottom Item (Next) */
    int next_idx = (int)r->selected + 1;
    if (next_idx >= r->item_count && r->loop) next_idx = 0;
    if (next_idx < r->item_count && r->items[next_idx]) {
        int tw = moui_font_measure_str(f, r->items[next_idx]);
        int tx = b.x + (b.w - tw) / 2;
        int ty = mid_y + item_h + (item_h - f->line_height) / 2;
        if (ty + f->line_height <= b.y + b.h)
            moui_font_draw_str(ctx, f, tx, ty, r->items[next_idx], moui_theme_fg());
    }
}

static bool roller_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_widget_roller_t *r = (moui_widget_roller_t *)w;
    if (!w->enabled || r->item_count == 0) return false;

    if (ev->type == MOUI_EV_ENCODER_CCW || ev->type == MOUI_EV_GESTURE_SWIPE_DOWN) {
        if (r->selected > 0) {
            r->selected--;
        } else if (r->loop) {
            r->selected = r->item_count - 1;
        }
        if (r->on_change) r->on_change(r, r->selected);
        return true;
    }

    if (ev->type == MOUI_EV_ENCODER_CW || ev->type == MOUI_EV_GESTURE_SWIPE_UP) {
        if (r->selected + 1 < r->item_count) {
            r->selected++;
        } else if (r->loop) {
            r->selected = 0;
        }
        if (r->on_change) r->on_change(r, r->selected);
        return true;
    }

    if (ev->type == MOUI_EV_POINTER_DOWN || ev->type == MOUI_EV_GESTURE_TAP) {
        int rel_y = ev->y - w->bounds.y;
        if (rel_y < w->bounds.h / 3) {
            if (r->selected > 0) r->selected--;
            else if (r->loop) r->selected = r->item_count - 1;
        } else if (rel_y > (w->bounds.h * 2) / 3) {
            if (r->selected + 1 < r->item_count) r->selected++;
            else if (r->loop) r->selected = 0;
        }
        if (r->on_change) r->on_change(r, r->selected);
        return true;
    }

    return false;
}

static const moui_widget_vtable_t roller_vtable = {
    .draw = roller_draw,
    .on_event = roller_event,
};

void moui_roller_init(moui_widget_roller_t *r, const moui_font_t *font)
{
    moui_widget_init(&r->base, &roller_vtable);
    r->font = font;
    r->item_count = 0;
    r->selected = 0;
    r->loop = true;
    r->on_change = NULL;
    memset(r->items, 0, sizeof(r->items));
}

void moui_roller_set_items(moui_widget_roller_t *r, const char **items, uint16_t count)
{
    if (count > MOUI_ROLLER_MAX_ITEMS) count = MOUI_ROLLER_MAX_ITEMS;
    r->item_count = count;
    for (uint16_t i = 0; i < count; i++) {
        r->items[i] = items[i];
    }
    if (r->selected >= count && count > 0) r->selected = count - 1;
}

void moui_roller_set_selected(moui_widget_roller_t *r, uint16_t index)
{
    if (index < r->item_count) r->selected = index;
}
