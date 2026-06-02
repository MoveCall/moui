#include "moui_widget_radio.h"
#include "../core/moui_theme.h"
#include "../core/moui_event_bus.h"
#include "../core/moui_theme.h"

static void radio_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_radio_t *r = (moui_widget_radio_t *)w;
    if (!r->font || r->option_count == 0) return;

    int x = w->bounds.x;
    int y = w->bounds.y;

    if (moui_widget_has_state(w, MOUI_STATE_FOCUSED))
        moui_draw_rect(ctx, &w->bounds, moui_theme_fg());

    for (int i = 0; i < r->option_count; i++) {
        int iy = y + i * r->item_h;
        int cx = x + 8;
        int cy = iy + r->item_h / 2;

        moui_draw_circle(ctx, cx, cy, 4, moui_theme_fg());
        if (i == r->selected)
            moui_draw_fill_circle(ctx, cx, cy, 2, moui_theme_fg());

        moui_font_draw_str(ctx, r->font, x + 16, iy + (r->item_h - r->font->line_height) / 2,
                          r->options[i], moui_theme_fg());
    }

    if (r->capturing) {
        int iy = y + r->selected * r->item_h;
        moui_draw_fill_rect(ctx, &(moui_rect_t){x + 1, iy, 3, r->item_h}, moui_theme_fg());
    }
}

static bool radio_on_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_widget_radio_t *r = (moui_widget_radio_t *)w;
    if (r->option_count == 0) return false;

    if (!r->capturing) {
        if (ev->type == MOUI_EV_ENCODER_PRESS) {
            r->capturing = true;
            return true;
        }
        return false;
    }

    switch (ev->type) {
    case MOUI_EV_ENCODER_CW:
        if (r->selected < r->option_count - 1) r->selected++;
        if (r->on_change) r->on_change(r, r->selected);
        moui_event_bus_publish(&moui_bus, &(moui_event_t){
            .type = MOUI_EVT_VALUE_CHANGED, .id = w->event_id,
            .value = r->selected, .sender = w,
        });
        return true;
    case MOUI_EV_ENCODER_CCW:
        if (r->selected > 0) r->selected--;
        if (r->on_change) r->on_change(r, r->selected);
        moui_event_bus_publish(&moui_bus, &(moui_event_t){
            .type = MOUI_EVT_VALUE_CHANGED, .id = w->event_id,
            .value = r->selected, .sender = w,
        });
        return true;
    case MOUI_EV_ENCODER_PRESS:
    case MOUI_EV_ENCODER_BACK:
        r->capturing = false;
        return true;
    default:
        return false;
    }
}

static void radio_on_focus(moui_widget_t *w, bool gained)
{
    moui_widget_radio_t *r = (moui_widget_radio_t *)w;
    if (!gained) r->capturing = false;
}

static const moui_widget_vtable_t radio_vtable = {
    .draw     = radio_draw,
    .on_event = radio_on_event,
    .on_focus = radio_on_focus,
};

void moui_radio_init(moui_widget_radio_t *r, const moui_font_t *font)
{
    moui_widget_init(&r->base, &radio_vtable);
    r->font = font;
    r->item_h = font ? font->line_height + 6 : 16;
    r->option_count = 0;
    r->selected = 0;
    r->capturing = false;
}

void moui_radio_set_options(moui_widget_radio_t *r, const char **opts, uint8_t count)
{
    if (count > MOUI_RADIO_MAX_OPTIONS) count = MOUI_RADIO_MAX_OPTIONS;
    for (int i = 0; i < count; i++) r->options[i] = opts[i];
    r->option_count = count;
    r->selected = 0;
}
