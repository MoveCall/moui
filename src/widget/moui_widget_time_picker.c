#include "moui_widget_time_picker.h"
#include "../core/moui_theme.h"
#include <stdio.h>

static void time_picker_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_time_picker_t *tp = (moui_widget_time_picker_t *)w;
    moui_rect_t b = w->bounds;
    const moui_font_t *f = tp->font ? tp->font : &moui_font_ascii_6x8;

    moui_draw_rounded_rect(ctx, &b, 3, moui_theme_fg());

    char buf[16];
    if (tp->show_sec)
        snprintf(buf, sizeof(buf), "%02d : %02d : %02d", tp->hour, tp->min, tp->sec);
    else
        snprintf(buf, sizeof(buf), "%02d : %02d", tp->hour, tp->min);

    int tw = moui_font_measure_str(f, buf);
    int tx = b.x + (b.w - tw) / 2;
    int ty = b.y + (b.h - f->line_height) / 2;

    moui_font_draw_str(ctx, f, tx, ty, buf, moui_theme_fg());

    /* Highlight active column box */
    int col_w = moui_font_measure_str(f, "00");
    int colon_w = moui_font_measure_str(f, " : ");
    int hx = tx;
    if (tp->active_col == 1) hx += col_w + colon_w;
    else if (tp->active_col == 2) hx += (col_w + colon_w) * 2;

    moui_rect_t hr = { (int16_t)(hx - 2), (int16_t)(ty - 2), (int16_t)(col_w + 4), (int16_t)(f->line_height + 4) };
    if (moui_widget_has_state(w, MOUI_STATE_FOCUSED)) {
        moui_draw_rect(ctx, &hr, moui_theme_fg());
    }
}

bool moui_time_picker_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_widget_time_picker_t *tp = (moui_widget_time_picker_t *)w;
    if (!w->enabled) return false;

    uint8_t max_cols = tp->show_sec ? 3 : 2;

    if (ev->type == MOUI_EV_POINTER_DOWN || ev->type == MOUI_EV_GESTURE_TAP) {
        int rel_x = ev->x - w->bounds.x;
        int col = (rel_x * max_cols) / w->bounds.w;
        if (col < 0) col = 0;
        if (col >= max_cols) col = max_cols - 1;
        tp->active_col = (uint8_t)col;
        return true;
    }

    if (ev->type == MOUI_EV_ENCODER_CCW || ev->type == MOUI_EV_GESTURE_SWIPE_LEFT) {
        if (tp->active_col > 0) tp->active_col--;
        else tp->active_col = max_cols - 1;
        return true;
    }

    if (ev->type == MOUI_EV_ENCODER_CW || ev->type == MOUI_EV_GESTURE_SWIPE_RIGHT) {
        if (tp->active_col + 1 < max_cols) tp->active_col++;
        else tp->active_col = 0;
        return true;
    }

    if (ev->type == MOUI_EV_GESTURE_SWIPE_UP) {
        if (tp->active_col == 0) tp->hour = (tp->hour + 1) % 24;
        else if (tp->active_col == 1) tp->min = (tp->min + 1) % 60;
        else if (tp->active_col == 2) tp->sec = (tp->sec + 1) % 60;
        if (tp->on_change) tp->on_change(tp, tp->hour, tp->min, tp->sec);
        return true;
    }

    if (ev->type == MOUI_EV_GESTURE_SWIPE_DOWN) {
        if (tp->active_col == 0) tp->hour = (tp->hour + 23) % 24;
        else if (tp->active_col == 1) tp->min = (tp->min + 59) % 60;
        else if (tp->active_col == 2) tp->sec = (tp->sec + 59) % 60;
        if (tp->on_change) tp->on_change(tp, tp->hour, tp->min, tp->sec);
        return true;
    }

    return false;
}

static const moui_widget_vtable_t tp_vtable = {
    .draw = time_picker_draw,
    .on_event = moui_time_picker_event,
};

void moui_time_picker_init(moui_widget_time_picker_t *tp, const moui_font_t *font)
{
    moui_widget_init(&tp->base, &tp_vtable);
    tp->font = font;
    tp->hour = 12;
    tp->min = 0;
    tp->sec = 0;
    tp->active_col = 0;
    tp->show_sec = true;
    tp->on_change = NULL;
}

void moui_time_picker_set_time(moui_widget_time_picker_t *tp, uint8_t h, uint8_t m, uint8_t s)
{
    tp->hour = h % 24;
    tp->min = m % 60;
    tp->sec = s % 60;
}
