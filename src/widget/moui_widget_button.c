#include "moui_widget_button.h"
#include "../core/moui_theme.h"
#include "../core/moui_event_bus.h"

static void button_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_button_t *b = (moui_widget_button_t *)w;
    const moui_font_t *font = b->font;
    if (!font) return;

    uint8_t pad = moui_style_pad(w->style);
    uint8_t rad = moui_style_radius(w->style);
    bool inv = moui_widget_has_state(w, MOUI_STATE_FOCUSED) && moui_style_invert_focus(w->style);

    if (inv) {
        moui_draw_fill_rounded_rect(ctx, &w->bounds, rad, moui_theme_fg());
        moui_font_draw_str(ctx, font, w->bounds.x + pad + 2, w->bounds.y + pad, b->label, moui_theme_bg());
    } else {
        if (moui_style_show_border(w->style))
            moui_draw_rounded_rect(ctx, &w->bounds, rad, moui_theme_fg());
        moui_font_draw_str(ctx, font, w->bounds.x + pad + 2, w->bounds.y + pad, b->label, moui_theme_fg());
    }

    if (b->type == MOUI_BTN_TOGGLE || b->type == MOUI_BTN_CHECKBOX) {
        int cx = w->bounds.x + w->bounds.w - 14;
        int cy = w->bounds.y + w->bounds.h / 2;
        moui_color_t col = inv ? MOUI_WHITE : MOUI_BLACK;

        if (b->type == MOUI_BTN_CHECKBOX) {
            moui_draw_rect(ctx, &(moui_rect_t){cx - 4, cy - 4, 9, 9}, col);
            if (b->state) {
                moui_draw_line(ctx, cx - 3, cy, cx - 1, cy + 2, col);
                moui_draw_line(ctx, cx - 1, cy + 2, cx + 3, cy - 3, col);
            }
        } else {
            if (b->state)
                moui_draw_fill_circle(ctx, cx, cy, 5, col);
            else
                moui_draw_circle(ctx, cx, cy, 5, col);
        }
    }
}

static bool button_on_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_widget_button_t *b = (moui_widget_button_t *)w;

    if (ev->type == MOUI_EV_ENCODER_PRESS) {
        if (b->type == MOUI_BTN_PUSH) {
            if (b->on_click) b->on_click(b);
            moui_event_bus_publish(&moui_bus, &(moui_event_t){
                .type = MOUI_EVT_BUTTON_CLICK, .id = w->event_id,
                .value = 1, .sender = w,
            });
        } else {
            b->state = !b->state;
            if (b->on_toggle) b->on_toggle(b, b->state);
            moui_event_bus_publish(&moui_bus, &(moui_event_t){
                .type = MOUI_EVT_VALUE_CHANGED, .id = w->event_id,
                .value = b->state, .sender = w,
            });
        }
        return true;
    }
    return false;
}

static const moui_widget_vtable_t button_vtable = {
    .draw     = button_draw,
    .on_event = button_on_event,
};

void moui_button_init(moui_widget_button_t *b, const char *label,
                     const moui_font_t *font, moui_btn_type_t type)
{
    moui_widget_init(&b->base, &button_vtable);
    b->label = label;
    b->font  = font;
    b->type  = type;
    b->state = false;
}
