#include "moui_layout_flex.h"
#include <string.h>

void moui_flex_init(moui_flex_t *flex, moui_flex_dir_t dir, moui_flex_justify_t justify, moui_flex_align_t align)
{
    memset(flex, 0, sizeof(*flex));
    flex->dir = dir;
    flex->justify = justify;
    flex->align = align;
    flex->gap = 4;
    flex->padding = 4;
}

void moui_flex_add(moui_flex_t *flex, moui_widget_t *w)
{
    if (flex->count < MOUI_FLEX_MAX_ITEMS && w) {
        flex->items[flex->count++] = w;
    }
}

void moui_flex_layout(moui_flex_t *flex, const moui_rect_t *parent_bounds)
{
    if (!flex || flex->count == 0 || !parent_bounds) return;

    int p = flex->padding;
    int avail_w = parent_bounds->w - p * 2;
    int avail_h = parent_bounds->h - p * 2;
    if (avail_w < 0) avail_w = 0;
    if (avail_h < 0) avail_h = 0;

    int total_item_main = 0;

    /* Calculate total size along main axis */
    for (uint8_t i = 0; i < flex->count; i++) {
        moui_widget_t *w = flex->items[i];
        if (flex->dir == MOUI_FLEX_DIR_ROW) {
            total_item_main += w->bounds.w;
        } else {
            total_item_main += w->bounds.h;
        }
    }

    int total_gaps = (flex->count > 1) ? (flex->count - 1) * flex->gap : 0;
    int free_space = (flex->dir == MOUI_FLEX_DIR_ROW ? avail_w : avail_h) - total_item_main - total_gaps;

    int start_pos = (flex->dir == MOUI_FLEX_DIR_ROW ? parent_bounds->x : parent_bounds->y) + p;
    int step_gap = flex->gap;

    if (free_space > 0) {
        if (flex->justify == MOUI_FLEX_JUSTIFY_CENTER) {
            start_pos += free_space / 2;
        } else if (flex->justify == MOUI_FLEX_JUSTIFY_END) {
            start_pos += free_space;
        } else if (flex->justify == MOUI_FLEX_JUSTIFY_SPACE_BETWEEN && flex->count > 1) {
            step_gap += free_space / (flex->count - 1);
        } else if (flex->justify == MOUI_FLEX_JUSTIFY_SPACE_AROUND && flex->count > 0) {
            int space_each = free_space / flex->count;
            start_pos += space_each / 2;
            step_gap += space_each;
        }
    }

    int curr_pos = start_pos;
    for (uint8_t i = 0; i < flex->count; i++) {
        moui_widget_t *w = flex->items[i];

        if (flex->dir == MOUI_FLEX_DIR_ROW) {
            w->bounds.x = (int16_t)curr_pos;
            curr_pos += w->bounds.w + step_gap;

            /* Align Cross Axis (Y) */
            if (flex->align == MOUI_FLEX_ALIGN_CENTER) {
                w->bounds.y = parent_bounds->y + p + (avail_h - w->bounds.h) / 2;
            } else if (flex->align == MOUI_FLEX_ALIGN_END) {
                w->bounds.y = parent_bounds->y + p + avail_h - w->bounds.h;
            } else if (flex->align == MOUI_FLEX_ALIGN_STRETCH) {
                w->bounds.y = parent_bounds->y + p;
                w->bounds.h = (int16_t)avail_h;
            } else {
                w->bounds.y = parent_bounds->y + p;
            }
        } else {
            w->bounds.y = (int16_t)curr_pos;
            curr_pos += w->bounds.h + step_gap;

            /* Align Cross Axis (X) */
            if (flex->align == MOUI_FLEX_ALIGN_CENTER) {
                w->bounds.x = parent_bounds->x + p + (avail_w - w->bounds.w) / 2;
            } else if (flex->align == MOUI_FLEX_ALIGN_END) {
                w->bounds.x = parent_bounds->x + p + avail_w - w->bounds.w;
            } else if (flex->align == MOUI_FLEX_ALIGN_STRETCH) {
                w->bounds.x = parent_bounds->x + p;
                w->bounds.w = (int16_t)avail_w;
            } else {
                w->bounds.x = parent_bounds->x + p;
            }
        }
    }
}
