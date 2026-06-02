#include "moui_widget_dropdown.h"
#include "../core/moui_theme.h"
#include "../core/moui_event_bus.h"

static void dropdown_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_dropdown_t *dd = (moui_widget_dropdown_t *)w;
    if (!dd->font || dd->option_count == 0) return;

    const moui_style_t *s = w->style ? w->style : &moui_style_default;
    uint8_t rad = s->corner_radius;
    uint8_t pad = s->padding;

    if (moui_widget_has_state(w, MOUI_STATE_FOCUSED) && !dd->open)
        moui_draw_fill_rounded_rect(ctx, &w->bounds, rad, moui_theme_fg());
    else
        moui_draw_rounded_rect(ctx, &w->bounds, rad, moui_theme_fg());

    bool inv = moui_widget_has_state(w, MOUI_STATE_FOCUSED) && !dd->open;
    moui_font_draw_str(ctx, dd->font, w->bounds.x + pad + 2,
                      w->bounds.y + (w->bounds.h - dd->font->line_height) / 2,
                      dd->options[dd->selected], inv ? MOUI_WHITE : moui_theme_fg());

    int ax = w->bounds.x + w->bounds.w - 10;
    int ay = w->bounds.y + w->bounds.h / 2;
    moui_draw_line(ctx, ax, ay - 2, ax + 3, ay + 1, inv ? MOUI_WHITE : moui_theme_fg());
    moui_draw_line(ctx, ax + 3, ay + 1, ax + 6, ay - 2, inv ? MOUI_WHITE : moui_theme_fg());

    if (dd->open) {
        int dy = w->bounds.y + w->bounds.h + 1;
        int dh = dd->visible_count * dd->item_h;
        moui_draw_fill_rect(ctx, &(moui_rect_t){w->bounds.x, dy, w->bounds.w, dh}, moui_theme_bg());
        moui_draw_rect(ctx, &(moui_rect_t){w->bounds.x, dy, w->bounds.w, dh}, moui_theme_fg());

        int scroll = 0;
        if (dd->selected >= dd->visible_count)
            scroll = dd->selected - dd->visible_count + 1;

        moui_draw_push_clip(ctx, &(moui_rect_t){w->bounds.x, dy, w->bounds.w, dh});
        for (int i = 0; i < dd->visible_count && (i + scroll) < dd->option_count; i++) {
            int item_idx = i + scroll;
            int iy = dy + i * dd->item_h;
            if (item_idx == dd->selected) {
                moui_draw_fill_rect(ctx, &(moui_rect_t){w->bounds.x + 1, iy, w->bounds.w - 2, dd->item_h}, moui_theme_fg());
                moui_font_draw_str(ctx, dd->font, w->bounds.x + pad + 2,
                                  iy + (dd->item_h - dd->font->line_height) / 2,
                                  dd->options[item_idx], moui_theme_bg());
            } else {
                moui_font_draw_str(ctx, dd->font, w->bounds.x + pad + 2,
                                  iy + (dd->item_h - dd->font->line_height) / 2,
                                  dd->options[item_idx], moui_theme_fg());
            }
        }
        moui_draw_pop_clip(ctx);
    }
}

static bool dropdown_on_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_widget_dropdown_t *dd = (moui_widget_dropdown_t *)w;
    if (dd->option_count == 0) return false;

    if (!dd->open) {
        if (ev->type == MOUI_EV_ENCODER_PRESS) {
            dd->open = true;
            return true;
        }
        return false;
    }

    switch (ev->type) {
    case MOUI_EV_ENCODER_CW:
        if (dd->selected < dd->option_count - 1) dd->selected++;
        return true;
    case MOUI_EV_ENCODER_CCW:
        if (dd->selected > 0) dd->selected--;
        return true;
    case MOUI_EV_ENCODER_PRESS:
        dd->open = false;
        if (dd->on_change) dd->on_change(dd, dd->selected);
        moui_event_bus_publish(&moui_bus, &(moui_event_t){
            .type = MOUI_EVT_VALUE_CHANGED, .id = w->event_id,
            .value = dd->selected, .sender = w,
        });
        return true;
    case MOUI_EV_ENCODER_BACK:
        dd->open = false;
        return true;
    default:
        return false;
    }
}

static void dropdown_on_focus(moui_widget_t *w, bool gained)
{
    moui_widget_dropdown_t *dd = (moui_widget_dropdown_t *)w;
    if (!gained) dd->open = false;
}

static const moui_widget_vtable_t dropdown_vtable = {
    .draw     = dropdown_draw,
    .on_event = dropdown_on_event,
    .on_focus = dropdown_on_focus,
};

void moui_dropdown_init(moui_widget_dropdown_t *dd, const moui_font_t *font)
{
    moui_widget_init(&dd->base, &dropdown_vtable);
    dd->font = font;
    dd->item_h = font ? font->line_height + 6 : 16;
    dd->visible_count = 4;
    dd->open = false;
}

void moui_dropdown_set_options(moui_widget_dropdown_t *dd, const char **opts, uint8_t count)
{
    if (count > MOUI_DROPDOWN_MAX_OPTIONS) count = MOUI_DROPDOWN_MAX_OPTIONS;
    for (int i = 0; i < count; i++) dd->options[i] = opts[i];
    dd->option_count = count;
    dd->selected = 0;
}
