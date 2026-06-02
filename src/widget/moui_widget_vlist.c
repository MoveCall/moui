#include "moui_widget_vlist.h"
#include "../core/moui_draw.h"
#include "../input/moui_input.h"
#include <string.h>

static int16_t get_item_h(const moui_vlist_t *vl, uint32_t idx)
{
    if (vl->height_fn) return vl->height_fn(vl, idx, vl->user_data);
    return vl->item_height > 0 ? (int16_t)vl->item_height : 24;
}

int32_t moui_vlist_get_total_height(const moui_vlist_t *vl)
{
    if (!vl || !vl->count_fn) return 0;
    uint32_t count = vl->count_fn(vl, vl->user_data);
    if (count == 0) return 0;
    if (!vl->height_fn) {
        return (int32_t)count * get_item_h(vl, 0);
    }
    int32_t total = 0;
    for (uint32_t i = 0; i < count; i++) {
        total += get_item_h(vl, i);
    }
    return total;
}

static void moui_vlist_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_vlist_t *vl = (moui_vlist_t *)w;
    if (!vl || !vl->count_fn || !vl->bind_fn || vl->pool_size == 0) return;

    uint32_t total_count = vl->count_fn(vl, vl->user_data);
    if (total_count == 0) return;

    /* Clamp scroll offset FIRST, before computing visible range */
    int32_t total_h = moui_vlist_get_total_height(vl);
    int32_t max_scroll = total_h - w->bounds.h;
    if (max_scroll < 0) max_scroll = 0;
    if (vl->scroll_offset > max_scroll) vl->scroll_offset = max_scroll;
    if (vl->scroll_offset < 0) vl->scroll_offset = 0;

    int32_t vis_y0 = vl->scroll_offset;
    int32_t vis_y1 = vl->scroll_offset + w->bounds.h;

    /* Draw container border / background */
    moui_draw_rect(ctx, &w->bounds, MOUI_BLACK);

    /* Push clip to widget bounds — prevent cells from rendering outside */
    moui_rect_t inner_clip = {
        (int16_t)(w->bounds.x + 1),
        (int16_t)(w->bounds.y + 1),
        (uint16_t)(w->bounds.w - 2),
        (uint16_t)(w->bounds.h - 2)
    };
    moui_draw_push_clip(ctx, &inner_clip);

    /* Render visible items using recycled cell pool */
    int32_t current_y = 0;
    uint8_t cell_idx = 0;

    for (uint32_t i = 0; i < total_count; i++) {
        int16_t h = get_item_h(vl, i);
        int32_t item_y0 = current_y;
        int32_t item_y1 = current_y + h;

        /* Early exit: past viewport, no more visible items */
        if (item_y0 >= vis_y1) break;

        /* Check overlap with visible viewport */
        if (item_y1 > vis_y0) {
            if (cell_idx < vl->pool_size && vl->cell_pool[cell_idx]) {
                moui_widget_t *cell = vl->cell_pool[cell_idx];

                /* Position cell inside viewport */
                int16_t screen_y = w->bounds.y + 1 + (int16_t)(item_y0 - vl->scroll_offset);
                cell->bounds = (moui_rect_t){
                    (int16_t)(w->bounds.x + 2),
                    screen_y,
                    (int16_t)(w->bounds.w - 12), /* Leave space for scrollbar */
                    (uint16_t)(h - 1)
                };

                /* Bind cell with item data */
                vl->bind_fn(vl, cell, i, vl->user_data);

                /* Selection highlight */
                if (i == vl->selected_index) {
                    moui_rect_t sel_box = {
                        (int16_t)(w->bounds.x + 1),
                        (int16_t)(screen_y - 1),
                        (uint16_t)(w->bounds.w - 11),
                        (uint16_t)h
                    };
                    moui_draw_rect(ctx, &sel_box, MOUI_BLACK);
                }

                /* Draw cell widget */
                if (cell->vtable && cell->vtable->draw) {
                    cell->vtable->draw(cell, ctx);
                }

                cell_idx++;
            }
        }
        current_y += h;
    }

    /* Pop clip before drawing scrollbar (scrollbar is on the border edge) */
    moui_draw_pop_clip(ctx);

    /* Scrollbar Indicator on Right Side */
    if (total_h > w->bounds.h && max_scroll > 0) {
        int sb_x = w->bounds.x + w->bounds.w - 6;
        int sb_y = w->bounds.y + 2;
        int sb_h = w->bounds.h - 4;
        moui_draw_vline(ctx, sb_x + 2, sb_y, sb_h, MOUI_BLACK);

        int thumb_h = (sb_h * w->bounds.h) / total_h;
        if (thumb_h < 6) thumb_h = 6;
        int thumb_y = sb_y + (int)((int64_t)vl->scroll_offset * (sb_h - thumb_h) / max_scroll);

        moui_rect_t thumb_rect = { (int16_t)sb_x, (int16_t)thumb_y, 5, (uint16_t)thumb_h };
        moui_draw_fill_rect(ctx, &thumb_rect, MOUI_BLACK);
    }
}

static bool moui_vlist_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_vlist_t *vl = (moui_vlist_t *)w;
    if (!vl || !vl->count_fn || !ev) return false;

    uint32_t count = vl->count_fn(vl, vl->user_data);
    if (count == 0) return false;

    if (ev->type == MOUI_EV_ENCODER_CW) {
        if (vl->selected_index + 1 < count) {
            vl->selected_index++;
            moui_vlist_scroll_to_index(vl, vl->selected_index);
            if (vl->on_select) vl->on_select(vl, vl->selected_index);
            w->dirty = 1;
            return true;
        }
    } else if (ev->type == MOUI_EV_ENCODER_CCW) {
        if (vl->selected_index > 0) {
            vl->selected_index--;
            moui_vlist_scroll_to_index(vl, vl->selected_index);
            if (vl->on_select) vl->on_select(vl, vl->selected_index);
            w->dirty = 1;
            return true;
        }
    } else if (ev->type == MOUI_EV_ENCODER_PRESS) {
        if (vl->on_select) {
            vl->on_select(vl, vl->selected_index);
            return true;
        }
    }
    return false;
}

static const moui_widget_vtable_t moui_vlist_vtable = {
    .draw     = moui_vlist_draw,
    .on_event = moui_vlist_event
};

void moui_vlist_init(moui_vlist_t *vl,
                    int16_t item_height,
                    moui_vlist_count_fn count_fn,
                    moui_vlist_bind_fn bind_fn,
                    void *user_data)
{
    if (!vl) return;
    memset(vl, 0, sizeof(*vl));
    moui_widget_init(&vl->base, &moui_vlist_vtable);

    vl->item_height    = item_height > 0 ? item_height : 24;
    vl->count_fn       = count_fn;
    vl->bind_fn        = bind_fn;
    vl->user_data      = user_data;
    vl->scroll_offset  = 0;
    vl->selected_index = 0;
    vl->pool_size      = 0;
}

bool moui_vlist_add_cell(moui_vlist_t *vl, moui_widget_t *cell)
{
    if (!vl || !cell || vl->pool_size >= MOUI_VLIST_MAX_POOL_SIZE) return false;
    vl->cell_pool[vl->pool_size++] = cell;
    return true;
}

void moui_vlist_scroll(moui_vlist_t *vl, int32_t delta)
{
    if (!vl) return;
    vl->scroll_offset += delta;
    int32_t total_h = moui_vlist_get_total_height(vl);
    int32_t max_scroll = total_h - vl->base.bounds.h;
    if (max_scroll < 0) max_scroll = 0;

    if (vl->scroll_offset < 0) vl->scroll_offset = 0;
    if (vl->scroll_offset > max_scroll) vl->scroll_offset = max_scroll;
    vl->base.dirty = 1;
}

void moui_vlist_scroll_to_index(moui_vlist_t *vl, uint32_t index)
{
    if (!vl || !vl->count_fn) return;
    uint32_t count = vl->count_fn(vl, vl->user_data);
    if (index >= count) return;

    int32_t item_y0 = 0;
    if (!vl->height_fn) {
        item_y0 = (int32_t)index * vl->item_height;
    } else {
        for (uint32_t i = 0; i < index; i++) {
            item_y0 += get_item_h(vl, i);
        }
    }
    int16_t item_h = get_item_h(vl, index);
    int32_t item_y1 = item_y0 + item_h;

    int32_t vis_y0 = vl->scroll_offset;
    int32_t vis_y1 = vl->scroll_offset + vl->base.bounds.h;

    if (item_y0 < vis_y0) {
        vl->scroll_offset = item_y0;
    } else if (item_y1 > vis_y1) {
        vl->scroll_offset = item_y1 - vl->base.bounds.h;
    }
    vl->base.dirty = 1;
}

void moui_vlist_set_selected(moui_vlist_t *vl, uint32_t index)
{
    if (!vl || !vl->count_fn) return;
    uint32_t count = vl->count_fn(vl, vl->user_data);
    if (index >= count) return;

    vl->selected_index = index;
    moui_vlist_scroll_to_index(vl, index);
}
