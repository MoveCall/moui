#include "moui_widget_switch.h"
#include "../core/moui_theme.h"
#include "../core/moui_event_bus.h"
#include "../core/moui_theme.h"

static void switch_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_switch_t *sw = (moui_widget_switch_t *)w;
    int x = w->bounds.x, y = w->bounds.y;
    int width = w->bounds.w, height = w->bounds.h;
    if (width < 4 || height < 4) return;
    int track_w = width, track_h = height;
    int rad = track_h / 2;

    moui_draw_rounded_rect(ctx, &(moui_rect_t){x, y, track_w, track_h}, rad, moui_theme_fg());

    if (sw->state) {
        moui_draw_fill_rounded_rect(ctx, &(moui_rect_t){x + 1, y + 1, track_w - 2, track_h - 2}, rad - 1, moui_theme_fg());
        moui_draw_fill_circle(ctx, x + track_w - rad, y + rad, rad - 3, moui_theme_bg());
    } else {
        moui_draw_fill_circle(ctx, x + rad, y + rad, rad - 3, moui_theme_fg());
    }

    if (moui_widget_has_state(w, MOUI_STATE_FOCUSED))
        moui_draw_rounded_rect(ctx, &(moui_rect_t){x - 1, y - 1, width + 2, height + 2}, rad + 1, moui_theme_fg());
}

static bool switch_on_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_widget_switch_t *sw = (moui_widget_switch_t *)w;
    if (ev->type == MOUI_EV_ENCODER_PRESS) {
        sw->state = !sw->state;
        if (sw->on_change) sw->on_change(sw, sw->state);
        moui_event_bus_publish(&moui_bus, &(moui_event_t){
            .type = MOUI_EVT_VALUE_CHANGED, .id = w->event_id,
            .value = sw->state, .sender = w,
        });
        return true;
    }
    return false;
}

static const moui_widget_vtable_t switch_vtable = {
    .draw = switch_draw, .on_event = switch_on_event,
};

void moui_switch_init(moui_widget_switch_t *sw)
{
    moui_widget_init(&sw->base, &switch_vtable);
    sw->state = false;
}
