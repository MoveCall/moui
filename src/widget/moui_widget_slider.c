#include "moui_widget_slider.h"
#include "../core/moui_theme.h"
#include "../core/moui_event_bus.h"
#include <stdio.h>

static void slider_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_slider_t *s = (moui_widget_slider_t *)w;
    int x = w->bounds.x, y = w->bounds.y;
    int width = w->bounds.w, height = w->bounds.h;

    uint8_t pad = moui_style_pad(w->style);
    uint8_t rad = moui_style_radius(w->style);

    if (moui_widget_has_state(w, MOUI_STATE_FOCUSED)) {
        if (s->capturing)
            moui_draw_fill_rounded_rect(ctx, &w->bounds, rad, moui_theme_fg());
        else
            moui_draw_rounded_rect(ctx, &w->bounds, rad, moui_theme_fg());
    }

    bool inv = moui_widget_has_state(w, MOUI_STATE_FOCUSED) && s->capturing;
    moui_color_t fg = inv ? MOUI_WHITE : MOUI_BLACK;

    int bar_x = x + pad;
    int bar_y = y + height / 2 - 2;
    int bar_w = width - pad * 2;
    int bar_h = 5;

    if (s->show_value && s->font) {
        char buf[12];
        snprintf(buf, sizeof(buf), "%d", (int)s->value);
        int tw = moui_font_measure_str(s->font, buf);
        bar_w -= tw + 4;
        if (bar_w < 6) bar_w = 6;
        moui_font_draw_str(ctx, s->font, x + width - tw - pad,
                          y + (height - s->font->line_height) / 2, buf, fg);
    }

    moui_draw_rounded_rect(ctx, &(moui_rect_t){bar_x, bar_y, bar_w, bar_h}, 2, fg);

    if (s->max_val > s->min_val && bar_w > 6) {
        float frac = (float)(s->value - s->min_val) / (float)(s->max_val - s->min_val);
        int fill_w = (int)(frac * (bar_w - 2));
        if (fill_w > 0)
            moui_draw_fill_rect(ctx, &(moui_rect_t){bar_x + 1, bar_y + 1, fill_w, bar_h - 2}, fg);

        int knob_x = bar_x + (int)(frac * (bar_w - 6));
        if (knob_x < bar_x) knob_x = bar_x;
        if (knob_x > bar_x + bar_w - 6) knob_x = bar_x + bar_w - 6;
        moui_draw_fill_rounded_rect(ctx, &(moui_rect_t){knob_x, bar_y - 2, 6, bar_h + 4}, 2, fg);
    }
}

static bool slider_on_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_widget_slider_t *s = (moui_widget_slider_t *)w;

    if (!s->capturing) {
        if (ev->type == MOUI_EV_ENCODER_PRESS) {
            s->capturing = true;
            return true;
        }
        return false;
    }

    switch (ev->type) {
    case MOUI_EV_ENCODER_CW:
        if (s->step > 0 && s->value <= s->max_val - s->step) {
            s->value += s->step;
            if (s->on_change) s->on_change(s, s->value);
            moui_event_bus_publish(&moui_bus, &(moui_event_t){
                .type = MOUI_EVT_VALUE_CHANGED, .id = w->event_id,
                .value = s->value, .sender = w,
            });
        }
        return true;
    case MOUI_EV_ENCODER_CCW:
        if (s->step > 0 && s->value >= s->min_val + s->step) {
            s->value -= s->step;
            if (s->on_change) s->on_change(s, s->value);
            moui_event_bus_publish(&moui_bus, &(moui_event_t){
                .type = MOUI_EVT_VALUE_CHANGED, .id = w->event_id,
                .value = s->value, .sender = w,
            });
        }
        return true;
    case MOUI_EV_ENCODER_PRESS:
    case MOUI_EV_ENCODER_BACK:
        s->capturing = false;
        return true;
    default:
        return false;
    }
}

static void slider_on_focus(moui_widget_t *w, bool gained)
{
    moui_widget_slider_t *s = (moui_widget_slider_t *)w;
    if (!gained) s->capturing = false;
}

static const moui_widget_vtable_t slider_vtable = {
    .draw     = slider_draw,
    .on_event = slider_on_event,
    .on_focus = slider_on_focus,
};

void moui_slider_init(moui_widget_slider_t *s, const moui_font_t *font,
                     int32_t min_val, int32_t max_val, int32_t step)
{
    moui_widget_init(&s->base, &slider_vtable);
    s->font       = font;
    s->min_val    = min_val;
    s->max_val    = max_val;
    s->step       = step;
    s->value      = min_val;
    s->show_value = true;
    s->capturing  = false;
}
