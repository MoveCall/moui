#include "moui_widget_calendar.h"
#include "../core/moui_theme.h"
#include "../core/moui_event_bus.h"
#include "../core/moui_theme.h"
#include <stdio.h>

static int days_in_month(uint16_t y, uint8_t m)
{
    static const uint8_t dim[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (m < 1 || m > 12) return 30;
    int d = dim[m - 1];
    if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)) d = 29;
    return d;
}

static int day_of_week(uint16_t y, uint8_t m, uint8_t d)
{
    if (m < 3) { m += 12; y--; }
    int h = (d + 13 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7;
    return ((h + 6) % 7);
}

static void calendar_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_calendar_t *cal = (moui_widget_calendar_t *)w;
    if (!cal->font) return;

    int x0 = w->bounds.x, y0 = w->bounds.y;
    int cw = w->bounds.w / 7;
    int rh = cal->font->line_height + 4;

    char buf[16];
    snprintf(buf, sizeof(buf), "%04d-%02d", cal->year, cal->month);
    int tw = moui_font_measure_str(cal->font, buf);
    moui_draw_fill_rect(ctx, &(moui_rect_t){x0, y0, w->bounds.w, rh}, moui_theme_fg());
    moui_font_draw_str(ctx, cal->font, x0 + (w->bounds.w - tw) / 2, y0 + 2, buf, moui_theme_bg());

    int hy = y0 + rh;
    const char *hdr[] = {"Su","Mo","Tu","We","Th","Fr","Sa"};
    for (int i = 0; i < 7; i++) {
        moui_font_draw_str(ctx, cal->font, x0 + i * cw + 2, hy + 2, hdr[i], moui_theme_fg());
    }
    moui_draw_hline(ctx, x0, hy + rh, w->bounds.w, moui_theme_fg());

    int start_dow = day_of_week(cal->year, cal->month, 1);
    int dim = days_in_month(cal->year, cal->month);
    int dy = hy + rh + 2;

    for (int d = 1; d <= dim; d++) {
        int dow = (start_dow + d - 1) % 7;
        int week = (start_dow + d - 1) / 7;
        int cx = x0 + dow * cw;
        int cy = dy + week * rh;

        snprintf(buf, sizeof(buf), "%d", d);
        tw = moui_font_measure_str(cal->font, buf);

        if (d == cal->day) {
            moui_draw_fill_circle(ctx, cx + cw / 2, cy + rh / 2, rh / 2 - 1, moui_theme_fg());
            moui_font_draw_str(ctx, cal->font, cx + (cw - tw) / 2, cy + 2, buf, moui_theme_bg());
        } else if (cal->capturing && d == cal->cursor_day) {
            moui_draw_circle(ctx, cx + cw / 2, cy + rh / 2, rh / 2 - 1, moui_theme_fg());
            moui_font_draw_str(ctx, cal->font, cx + (cw - tw) / 2, cy + 2, buf, moui_theme_fg());
        } else {
            moui_font_draw_str(ctx, cal->font, cx + (cw - tw) / 2, cy + 2, buf, moui_theme_fg());
        }
    }

    if (moui_widget_has_state(w, MOUI_STATE_FOCUSED))
        moui_draw_rect(ctx, &w->bounds, moui_theme_fg());
}

static bool calendar_on_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_widget_calendar_t *cal = (moui_widget_calendar_t *)w;

    if (!cal->capturing) {
        if (ev->type == MOUI_EV_ENCODER_PRESS) {
            cal->capturing = true;
            cal->cursor_day = cal->day;
            return true;
        }
        return false;
    }

    int dim = days_in_month(cal->year, cal->month);

    switch (ev->type) {
    case MOUI_EV_ENCODER_CW:
        if (cal->cursor_day < dim) cal->cursor_day++;
        else cal->cursor_day = 1;
        return true;
    case MOUI_EV_ENCODER_CCW:
        if (cal->cursor_day > 1) cal->cursor_day--;
        else cal->cursor_day = dim;
        return true;
    case MOUI_EV_ENCODER_PRESS:
        cal->day = cal->cursor_day;
        cal->capturing = false;
        if (cal->on_select) cal->on_select(cal, cal->year, cal->month, cal->day);
        moui_event_bus_publish(&moui_bus, &(moui_event_t){
            .type = MOUI_EVT_VALUE_CHANGED, .id = w->event_id,
            .value = cal->day, .sender = w,
        });
        return true;
    case MOUI_EV_ENCODER_BACK:
        cal->capturing = false;
        return true;
    default: return false;
    }
}

static void calendar_on_focus(moui_widget_t *w, bool gained)
{
    moui_widget_calendar_t *cal = (moui_widget_calendar_t *)w;
    if (!gained) cal->capturing = false;
}

static const moui_widget_vtable_t calendar_vtable = {
    .draw = calendar_draw, .on_event = calendar_on_event, .on_focus = calendar_on_focus,
};

void moui_calendar_init(moui_widget_calendar_t *cal, const moui_font_t *font,
                       uint16_t year, uint8_t month, uint8_t day)
{
    moui_widget_init(&cal->base, &calendar_vtable);
    cal->font = font;
    cal->year = year;
    cal->month = month;
    cal->day = day;
    cal->cursor_day = day;
    cal->capturing = false;
}
