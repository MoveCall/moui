#include "moui_widget_extra.h"
#include "../core/moui_theme.h"
#include "../core/moui_event_bus.h"
#include <stdio.h>
#include <string.h>

/* ══════════════════════════════════════════════════════
 *  Page Dots
 * ══════════════════════════════════════════════════════ */

static void dots_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_dots_t *d = (moui_widget_dots_t *)w;
    moui_color_t fg = moui_theme_fg();
    int r = d->dot_r > 0 ? d->dot_r : 3;
    int gap = d->gap > 0 ? d->gap : 8;
    int total_w = d->total * (r * 2) + (d->total - 1) * gap;
    int sx = w->bounds.x + (w->bounds.w - total_w) / 2;
    int cy = w->bounds.y + w->bounds.h / 2;

    for (int i = 0; i < d->total; i++) {
        int cx = sx + i * (r * 2 + gap) + r;
        if (i == d->active)
            moui_draw_fill_circle(ctx, cx, cy, r, fg);
        else
            moui_draw_circle(ctx, cx, cy, r, fg);
    }
}

static const moui_widget_vtable_t dots_vtable = { .draw = dots_draw };

void moui_dots_init(moui_widget_dots_t *d, uint8_t total)
{
    moui_widget_init(&d->base, &dots_vtable);
    d->total = total;
    d->active = 0;
    d->dot_r = 3;
    d->gap = 8;
    d->base.enabled = 0;
}

/* ══════════════════════════════════════════════════════
 *  Number Display
 * ══════════════════════════════════════════════════════ */

static void number_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_number_t *n = (moui_widget_number_t *)w;
    if (!n->font) return;
    moui_color_t fg = moui_theme_fg();

    char buf[16];
    if (n->leading_zero && n->digits > 0)
        snprintf(buf, sizeof(buf), "%0*d", n->digits, (int)n->value);
    else
        snprintf(buf, sizeof(buf), "%d", (int)n->value);

    int tw = moui_font_measure_str(n->font, buf);
    int uw = n->unit ? moui_font_measure_str(n->font, n->unit) : 0;
    int total = tw + uw + (uw > 0 ? 2 : 0);

    int x = w->bounds.x + (w->bounds.w - total) / 2;
    int y = w->bounds.y + (w->bounds.h - n->font->line_height) / 2;

    moui_font_draw_str(ctx, n->font, x, y, buf, fg);
    if (n->unit)
        moui_font_draw_str(ctx, n->font, x + tw + 2, y, n->unit, fg);
}

static const moui_widget_vtable_t number_vtable = { .draw = number_draw };

void moui_number_init(moui_widget_number_t *n, const moui_font_t *font, uint8_t digits)
{
    moui_widget_init(&n->base, &number_vtable);
    n->font = font;
    n->value = 0;
    n->digits = digits;
    n->leading_zero = true;
    n->unit = NULL;
    n->base.enabled = 0;
}

/* ══════════════════════════════════════════════════════
 *  Stepper
 * ══════════════════════════════════════════════════════ */

static void stepper_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_stepper_t *s = (moui_widget_stepper_t *)w;
    if (!s->font) return;
    moui_color_t fg = moui_theme_fg();
    moui_color_t bg = moui_theme_bg();
    int x = w->bounds.x, y = w->bounds.y;
    int width = w->bounds.w, height = w->bounds.h;
    int btn_w = height;

    /* [-] button */
    moui_draw_rect(ctx, &(moui_rect_t){x, y, btn_w, height}, fg);
    moui_draw_hline(ctx, x + 4, y + height / 2, btn_w - 8, fg);

    /* [+] button */
    moui_draw_rect(ctx, &(moui_rect_t){x + width - btn_w, y, btn_w, height}, fg);
    moui_draw_hline(ctx, x + width - btn_w + 4, y + height / 2, btn_w - 8, fg);
    moui_draw_vline(ctx, x + width - btn_w / 2 - 1, y + 4, height - 8, fg);

    /* Value in center */
    char buf[12];
    snprintf(buf, sizeof(buf), "%d", (int)s->value);
    int tw = moui_font_measure_str(s->font, buf);
    moui_font_draw_str(ctx, s->font, x + (width - tw) / 2,
                      y + (height - s->font->line_height) / 2, buf, fg);

    if (moui_widget_has_state(w, MOUI_STATE_FOCUSED))
        moui_draw_rounded_rect(ctx, &w->bounds, 2, fg);
    (void)bg;
}

static bool stepper_on_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_widget_stepper_t *s = (moui_widget_stepper_t *)w;
    if (!s->capturing) {
        if (ev->type == MOUI_EV_ENCODER_PRESS) { s->capturing = true; return true; }
        return false;
    }
    switch (ev->type) {
    case MOUI_EV_ENCODER_CW:
        if (s->value + s->step <= s->max_val) s->value += s->step;
        if (s->on_change) s->on_change(s, s->value);
        return true;
    case MOUI_EV_ENCODER_CCW:
        if (s->value - s->step >= s->min_val) s->value -= s->step;
        if (s->on_change) s->on_change(s, s->value);
        return true;
    case MOUI_EV_ENCODER_PRESS:
    case MOUI_EV_ENCODER_BACK:
        s->capturing = false;
        return true;
    default: return false;
    }
}

static const moui_widget_vtable_t stepper_vtable = {
    .draw = stepper_draw, .on_event = stepper_on_event,
};

void moui_stepper_init(moui_widget_stepper_t *s, const moui_font_t *font,
                      int32_t min_val, int32_t max_val, int32_t step)
{
    moui_widget_init(&s->base, &stepper_vtable);
    s->font = font;
    s->value = min_val;
    s->min_val = min_val;
    s->max_val = max_val;
    s->step = step;
    s->capturing = false;
}

/* ══════════════════════════════════════════════════════
 *  Sparkline
 * ══════════════════════════════════════════════════════ */

static void sparkline_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_sparkline_t *s = (moui_widget_sparkline_t *)w;
    if (s->count == 0) return;
    moui_color_t fg = moui_theme_fg();
    int x0 = w->bounds.x, y0 = w->bounds.y;
    int width = w->bounds.w, height = w->bounds.h;
    int range = s->y_max - s->y_min;
    if (range <= 0) range = 1;

    int n = s->count < MOUI_SPARKLINE_MAX ? s->count : MOUI_SPARKLINE_MAX;
    int prev_x = -1, prev_y = -1;

    for (int i = 0; i < n; i++) {
        int idx = (s->write_pos - n + i + MOUI_SPARKLINE_MAX) % MOUI_SPARKLINE_MAX;
        int val = s->data[idx];
        int px = x0 + (i * width) / (n - 1 > 0 ? n - 1 : 1);
        int py = y0 + height - 1 - (int)((float)(val - s->y_min) / range * (height - 1));
        if (py < y0) py = y0;
        if (py >= y0 + height) py = y0 + height - 1;

        if (prev_x >= 0)
            moui_draw_line(ctx, prev_x, prev_y, px, py, fg);
        prev_x = px;
        prev_y = py;
    }
}

static const moui_widget_vtable_t sparkline_vtable = { .draw = sparkline_draw };

void moui_sparkline_init(moui_widget_sparkline_t *s, int16_t y_min, int16_t y_max)
{
    moui_widget_init(&s->base, &sparkline_vtable);
    s->y_min = y_min;
    s->y_max = y_max;
    s->count = 0;
    s->write_pos = 0;
    s->base.enabled = 0;
}

void moui_sparkline_push(moui_widget_sparkline_t *s, int16_t val)
{
    s->data[s->write_pos] = val;
    s->write_pos = (s->write_pos + 1) % MOUI_SPARKLINE_MAX;
    if (s->count < MOUI_SPARKLINE_MAX) s->count++;
}

/* ══════════════════════════════════════════════════════
 *  Checkbox List
 * ══════════════════════════════════════════════════════ */

static void checklist_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_checklist_t *cl = (moui_widget_checklist_t *)w;
    if (!cl->font || cl->count == 0) return;
    moui_color_t fg = moui_theme_fg();
    int x = w->bounds.x, y = w->bounds.y;

    for (int i = 0; i < cl->count; i++) {
        int iy = y + i * cl->item_h;
        int cx = x + 6, cy = iy + cl->item_h / 2;

        /* Checkbox */
        moui_draw_rect(ctx, &(moui_rect_t){cx - 4, cy - 4, 9, 9}, fg);
        if (cl->checked & (1 << i)) {
            moui_draw_line(ctx, cx - 3, cy, cx - 1, cy + 2, fg);
            moui_draw_line(ctx, cx - 1, cy + 2, cx + 3, cy - 3, fg);
        }

        /* Label */
        moui_font_draw_str(ctx, cl->font, x + 16,
                          iy + (cl->item_h - cl->font->line_height) / 2,
                          cl->items[i], fg);

        /* Cursor indicator */
        if (cl->capturing && i == cl->cursor) {
            moui_draw_fill_rect(ctx, &(moui_rect_t){x, iy, 3, cl->item_h}, fg);
        }
    }

    if (moui_widget_has_state(w, MOUI_STATE_FOCUSED) && !cl->capturing)
        moui_draw_rect(ctx, &w->bounds, fg);
}

static bool checklist_on_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_widget_checklist_t *cl = (moui_widget_checklist_t *)w;
    if (cl->count == 0) return false;
    if (!cl->capturing) {
        if (ev->type == MOUI_EV_ENCODER_PRESS) { cl->capturing = true; return true; }
        return false;
    }
    switch (ev->type) {
    case MOUI_EV_ENCODER_CW:
        if (cl->cursor < cl->count - 1) cl->cursor++;
        return true;
    case MOUI_EV_ENCODER_CCW:
        if (cl->cursor > 0) cl->cursor--;
        return true;
    case MOUI_EV_ENCODER_PRESS:
        cl->checked ^= (1 << cl->cursor);
        if (cl->on_change)
            cl->on_change(cl, cl->cursor, (cl->checked >> cl->cursor) & 1);
        return true;
    case MOUI_EV_ENCODER_BACK:
        cl->capturing = false;
        return true;
    default: return false;
    }
}

static void checklist_on_focus(moui_widget_t *w, bool gained)
{
    moui_widget_checklist_t *cl = (moui_widget_checklist_t *)w;
    if (!gained) cl->capturing = false;
}

static const moui_widget_vtable_t checklist_vtable = {
    .draw = checklist_draw, .on_event = checklist_on_event, .on_focus = checklist_on_focus,
};

void moui_checklist_init(moui_widget_checklist_t *cl, const moui_font_t *font)
{
    moui_widget_init(&cl->base, &checklist_vtable);
    cl->font = font;
    cl->count = 0;
    cl->checked = 0;
    cl->cursor = 0;
    cl->item_h = font ? font->line_height + 6 : 16;
    cl->capturing = false;
}

void moui_checklist_set_items(moui_widget_checklist_t *cl, const char **items, uint8_t count)
{
    if (count > MOUI_CHECKLIST_MAX) count = MOUI_CHECKLIST_MAX;
    for (int i = 0; i < count; i++) cl->items[i] = items[i];
    cl->count = count;
}

bool moui_checklist_is_checked(const moui_widget_checklist_t *cl, uint8_t idx)
{
    if (idx >= cl->count || idx >= MOUI_CHECKLIST_MAX) return false;
    return (cl->checked >> idx) & 1;
}
