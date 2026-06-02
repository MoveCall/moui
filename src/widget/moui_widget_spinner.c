#include "moui_widget_spinner.h"
#include "../core/moui_theme.h"
#include "../core/moui_event_bus.h"
#include "../core/moui_theme.h"
#include <stdio.h>
#include <string.h>

/*
 * Format string whitelist for the spinner value. Only %d (or a bare literal
 * like "%%") is accepted; anything else (e.g. %s, %n) is rejected so a
 * caller-supplied format can never be turned into a read/write primitive.
 */
static bool spinner_format_safe(const char *fmt)
{
    for (const char *p = fmt; *p; p++) {
        if (*p != '%') continue;
        if (p[1] == '%') { p++; continue; }      /* literal % */
        if (p[1] == 'd') { p++; continue; }      /* signed decimal */
        return false;                             /* reject everything else */
    }
    return true;
}

static void spinner_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_spinner_t *sp = (moui_widget_spinner_t *)w;
    if (!sp->font) return;
    int x = w->bounds.x, y = w->bounds.y;
    int width = w->bounds.w, height = w->bounds.h;

    if (moui_widget_has_state(w, MOUI_STATE_FOCUSED))
        moui_draw_rounded_rect(ctx, &w->bounds, 4, moui_theme_fg());

    char buf[16];
    const char *fmt = sp->format ? sp->format : "%d";
    if (!spinner_format_safe(fmt)) fmt = "%d";
    snprintf(buf, sizeof(buf), fmt, (int)sp->value);

    int tw = moui_font_measure_str(sp->font, buf);
    int tx = x + (width - tw) / 2;
    int ty = y + (height - sp->font->line_height) / 2;
    moui_font_draw_str(ctx, sp->font, tx, ty, buf, moui_theme_fg());

    int arr_x_l = x + 4;
    int arr_x_r = x + width - 8;
    int arr_y = y + height / 2;

    moui_draw_line(ctx, arr_x_l + 4, arr_y - 3, arr_x_l, arr_y, moui_theme_fg());
    moui_draw_line(ctx, arr_x_l, arr_y, arr_x_l + 4, arr_y + 3, moui_theme_fg());

    moui_draw_line(ctx, arr_x_r, arr_y - 3, arr_x_r + 4, arr_y, moui_theme_fg());
    moui_draw_line(ctx, arr_x_r + 4, arr_y, arr_x_r, arr_y + 3, moui_theme_fg());
}

static bool spinner_on_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_widget_spinner_t *sp = (moui_widget_spinner_t *)w;

    if (!sp->capturing) {
        if (ev->type == MOUI_EV_ENCODER_PRESS) {
            sp->capturing = true;
            return true;
        }
        return false;
    }

    switch (ev->type) {
    case MOUI_EV_ENCODER_CW:
        if (sp->step > 0) {
            int64_t nv = (int64_t)sp->value + sp->step;
            if (nv <= sp->max_val) {
                sp->value = (int32_t)nv;
            } else if (sp->wrap) {
                sp->value = sp->min_val;
            } else {
                return true;
            }
        } else {
            return true;
        }
        if (sp->on_change) sp->on_change(sp, sp->value);
        moui_event_bus_publish(&moui_bus, &(moui_event_t){
            .type = MOUI_EVT_VALUE_CHANGED, .id = w->event_id,
            .value = sp->value, .sender = w,
        });
        return true;
    case MOUI_EV_ENCODER_CCW:
        if (sp->step > 0) {
            int64_t nv = (int64_t)sp->value - sp->step;
            if (nv >= sp->min_val) {
                sp->value = (int32_t)nv;
            } else if (sp->wrap) {
                sp->value = sp->max_val;
            } else {
                return true;
            }
        } else {
            return true;
        }
        if (sp->on_change) sp->on_change(sp, sp->value);
        moui_event_bus_publish(&moui_bus, &(moui_event_t){
            .type = MOUI_EVT_VALUE_CHANGED, .id = w->event_id,
            .value = sp->value, .sender = w,
        });
        return true;
    case MOUI_EV_ENCODER_PRESS:
    case MOUI_EV_ENCODER_BACK:
        sp->capturing = false;
        return true;
    default:
        return false;
    }
}

static void spinner_on_focus(moui_widget_t *w, bool gained)
{
    moui_widget_spinner_t *sp = (moui_widget_spinner_t *)w;
    if (!gained) sp->capturing = false;
}

static const moui_widget_vtable_t spinner_vtable = {
    .draw     = spinner_draw,
    .on_event = spinner_on_event,
    .on_focus = spinner_on_focus,
};

void moui_spinner_init(moui_widget_spinner_t *sp, const moui_font_t *font,
                      int32_t min_val, int32_t max_val, int32_t step)
{
    moui_widget_init(&sp->base, &spinner_vtable);
    sp->font = font;
    sp->min_val = min_val;
    sp->max_val = max_val;
    sp->step = step;
    sp->value = min_val;
    sp->format = NULL;
    sp->capturing = false;
    sp->wrap = true;
}
