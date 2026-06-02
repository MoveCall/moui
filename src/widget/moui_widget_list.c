#include "moui_widget_list.h"
#include "../anim/moui_ease.h"
#include "../core/moui_theme.h"
#include "../core/moui_event_bus.h"
#include <string.h>

static inline int list_max_scroll(moui_widget_t *w, moui_widget_list_t *l)
{
    int ms = (int)l->item_count * l->item_h - w->bounds.h;
    return ms > 0 ? ms : 0;
}

static void list_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_list_t *l = (moui_widget_list_t *)w;
    if (!l->font || l->item_count == 0 || !l->adapter) return;

    moui_draw_push_clip(ctx, &w->bounds);

    int x = w->bounds.x;
    int y = w->bounds.y;
    int width = w->bounds.w;
    int scroll = (int)w->anim_val[0];

    int first = scroll / l->item_h;
    int last  = (scroll + w->bounds.h) / l->item_h + 1;
    if (first < 0) first = 0;
    if (last > (int)l->item_count) last = (int)l->item_count;

    for (int i = first; i < last; i++) {
        int iy = y + i * l->item_h - scroll;
        bool is_selected = (i == (int)l->selected);
        const char *text = l->adapter((uint16_t)i, l->adapter_data);

        if (is_selected) {
            /* Highlight selected item with solid inverted background */
            moui_draw_fill_rect(ctx, &(moui_rect_t){x + 4, iy + 2, width - 8, l->item_h - 4}, moui_theme_fg());
            if (text) {
                moui_font_draw_str(ctx, l->font, x + 16,
                                  iy + (l->item_h - l->font->line_height) / 2,
                                  text, moui_theme_bg());
            }
        } else {
            if (text) {
                moui_font_draw_str(ctx, l->font, x + 16,
                                  iy + (l->item_h - l->font->line_height) / 2,
                                  text, moui_theme_fg());
            }
        }
    }

    if (moui_widget_has_state(w, MOUI_STATE_FOCUSED) && moui_style_show_border(w->style))
        moui_draw_rect(ctx, &w->bounds, moui_theme_fg());

    moui_draw_pop_clip(ctx);
}

static void list_update_scroll(moui_widget_t *w, moui_widget_list_t *l)
{
    w->anim_tgt[1] = (float)((int)l->selected * l->item_h);
    int visible = w->bounds.h / l->item_h;
    int scroll_target = (int)l->selected - visible / 2;
    if (scroll_target < 0) scroll_target = 0;
    int max_s = list_max_scroll(w, l);
    float st = (float)(scroll_target * l->item_h);
    if (st > (float)max_s) st = (float)max_s;
    w->anim_tgt[0] = st;

    /* Instant snap for E-Paper / Mono screens: avoid multi-frame interpolation flashing */
    w->anim_val[0] = w->anim_tgt[0];
    w->anim_val[1] = w->anim_tgt[1];
}

static bool list_on_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_widget_list_t *l = (moui_widget_list_t *)w;
    if (l->item_count == 0 || l->item_h == 0) return false;
    int max_s = list_max_scroll(w, l);

    /* Touch events bypass the capturing gate */
    if (ev->type == MOUI_EV_GESTURE_SWIPE_DOWN) {
        int page = w->bounds.h / l->item_h;
        if (page < 1) page = 1;
        uint16_t tgt = l->selected + (uint16_t)page;
        l->selected = tgt < l->item_count ? tgt : l->item_count - 1;
        list_update_scroll(w, l);
        return true;
    }
    if (ev->type == MOUI_EV_GESTURE_SWIPE_UP) {
        int page = w->bounds.h / l->item_h;
        if (page < 1) page = 1;
        l->selected = l->selected >= (uint16_t)page ? l->selected - (uint16_t)page : 0;
        list_update_scroll(w, l);
        return true;
    }
    if (ev->type == MOUI_EV_POINTER_DOWN) {
        l->drag_start_y = ev->y;
        l->drag_base_scroll = (int16_t)w->anim_val[0];
        l->dragging = true;
        return true;
    }
    if (ev->type == MOUI_EV_POINTER_MOVE && l->dragging) {
        int dy = l->drag_start_y - ev->y;
        float target = (float)(l->drag_base_scroll + dy);
        if (target < 0) target = 0;
        if (target > (float)max_s) target = (float)max_s;
        w->anim_tgt[0] = target;
        w->anim_val[0] = target;
        return true;
    }
    if (ev->type == MOUI_EV_POINTER_UP && l->dragging) {
        l->dragging = false;
        /* Sync selected to nearest item at current scroll */
        int scroll = (int)w->anim_val[0];
        int center_y = scroll + w->bounds.h / 2;
        uint16_t nearest = (uint16_t)(center_y / l->item_h);
        if (nearest >= l->item_count) nearest = l->item_count - 1;
        l->selected = nearest;
        list_update_scroll(w, l);
        return true;
    }
    if (ev->type == MOUI_EV_GESTURE_TAP) {
        /* Tap to select item at touch y */
        int scroll = (int)w->anim_val[0];
        int rel_y = ev->y - w->bounds.y + scroll;
        uint16_t idx = (uint16_t)(rel_y / l->item_h);
        if (idx < l->item_count) {
            l->selected = idx;
            list_update_scroll(w, l);
            if (l->on_select)
                l->on_select(l, l->selected);
            moui_event_bus_publish(&moui_bus, &(moui_event_t){
                .type = MOUI_EVT_LIST_SELECT, .id = w->event_id,
                .value = l->selected, .sender = w,
            });
        }
        return true;
    }

    switch (ev->type) {
    case MOUI_EV_ENCODER_CW:
        if (l->selected < l->item_count - 1)
            l->selected++;
        else if (l->loop)
            l->selected = 0;
        list_update_scroll(w, l);
        return true;
    case MOUI_EV_ENCODER_CCW:
        if (l->selected > 0)
            l->selected--;
        else if (l->loop)
            l->selected = l->item_count - 1;
        list_update_scroll(w, l);
        return true;
    case MOUI_EV_ENCODER_PRESS:
        if (l->on_select)
            l->on_select(l, l->selected);
        moui_event_bus_publish(&moui_bus, &(moui_event_t){
            .type = MOUI_EVT_LIST_SELECT, .id = w->event_id,
            .value = l->selected, .sender = w,
        });
        return true;
    case MOUI_EV_ENCODER_BACK:
        return false;   /* allow screen pop */
    default:
        return false;
    }
}

static void list_on_focus(moui_widget_t *w, bool gained)
{
    moui_widget_list_t *l = (moui_widget_list_t *)w;
    if (gained) {
        if (w->focus_next == w)
            l->capturing = true;
    } else {
        l->capturing = false;
    }
}

static const moui_widget_vtable_t list_vtable = {
    .draw     = list_draw,
    .on_event = list_on_event,
    .on_focus = list_on_focus,
};

void moui_list_init(moui_widget_list_t *l, const moui_font_t *font)
{
    moui_widget_init(&l->base, &list_vtable);
    l->font       = font;
    l->item_h     = moui_theme.list_item_h ? moui_theme.list_item_h : (font ? font->line_height + 10 : 20);
    l->loop       = true;
    l->capturing  = false;
    l->dragging   = false;
    l->adapter    = NULL;
    l->adapter_data = NULL;
    l->item_count = 0;
    l->selected   = 0;
}

void moui_list_set_adapter(moui_widget_list_t *l, moui_list_adapter_fn fn,
                           void *user_data, uint16_t count)
{
    l->adapter      = fn;
    l->adapter_data = user_data;
    l->item_count   = count;
    l->selected     = 0;
    l->base.anim_val[0] = 0;
    l->base.anim_val[1] = 0;
    l->base.anim_tgt[0] = 0;
    l->base.anim_tgt[1] = 0;
}
