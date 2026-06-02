#include "moui_layout.h"
#include <string.h>

static void container_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_container_t *c = (moui_container_t *)w;
    moui_draw_push_clip(ctx, &w->bounds);
    for (int i = 0; i < c->child_count; i++) {
        moui_widget_draw(c->children[i], ctx);
    }
    moui_draw_pop_clip(ctx);
}

static const moui_widget_vtable_t container_vtable = {
    .draw = container_draw,
};

void moui_container_init(moui_container_t *c, moui_stack_dir_t dir)
{
    moui_widget_init(&c->base, &container_vtable);
    c->direction   = dir;
    c->padding     = 0;
    c->gap         = 0;
    c->child_count = 0;
}

void moui_container_add(moui_container_t *c, moui_widget_t *child)
{
    if (c->child_count >= MOUI_CONTAINER_MAX_CHILDREN) return;
    c->children[c->child_count++] = child;
    child->parent = &c->base;
}

void moui_container_layout(moui_container_t *c)
{
    moui_rect_t b = c->base.bounds;
    int x = b.x + c->padding;
    int y = b.y + c->padding;
    int avail_w = b.w - 2 * c->padding;
    int avail_h = b.h - 2 * c->padding;

    if (c->direction == MOUI_STACK_VERTICAL) {
        int total_gap = (c->child_count > 1) ? c->gap * (c->child_count - 1) : 0;
        int remaining = avail_h - total_gap;
        int fixed_used = 0;
        int fill_count = 0;

        for (int i = 0; i < c->child_count; i++) {
            if (c->children[i]->h_hint >= 0) {
                fixed_used += c->children[i]->h_hint;
            } else {
                fill_count++;
            }
        }

        int fill_h = fill_count > 0 ? (remaining - fixed_used) / fill_count : 0;
        if (fill_h < 0) fill_h = 0;

        int cy = y;
        for (int i = 0; i < c->child_count; i++) {
            moui_widget_t *ch = c->children[i];
            ch->bounds.x = x;
            ch->bounds.y = cy;
            ch->bounds.w = (ch->w_hint >= 0) ? ch->w_hint : avail_w;
            ch->bounds.h = (ch->h_hint >= 0) ? ch->h_hint : fill_h;
            cy += ch->bounds.h + c->gap;
        }
    } else {
        int total_gap = (c->child_count > 1) ? c->gap * (c->child_count - 1) : 0;
        int remaining = avail_w - total_gap;
        int fixed_used = 0;
        int fill_count = 0;

        for (int i = 0; i < c->child_count; i++) {
            if (c->children[i]->w_hint >= 0) {
                fixed_used += c->children[i]->w_hint;
            } else {
                fill_count++;
            }
        }

        int fill_w = fill_count > 0 ? (remaining - fixed_used) / fill_count : 0;
        if (fill_w < 0) fill_w = 0;

        int cx = x;
        for (int i = 0; i < c->child_count; i++) {
            moui_widget_t *ch = c->children[i];
            ch->bounds.x = cx;
            ch->bounds.y = y;
            ch->bounds.w = (ch->w_hint >= 0) ? ch->w_hint : fill_w;
            ch->bounds.h = (ch->h_hint >= 0) ? ch->h_hint : avail_h;
            cx += ch->bounds.w + c->gap;
        }
    }

    for (int i = 0; i < c->child_count; i++) {
        moui_widget_t *ch = c->children[i];
        if (ch->vtable == &container_vtable)
            moui_container_layout((moui_container_t *)ch);
    }
}

void moui_container_draw(moui_container_t *c, moui_draw_ctx_t *ctx)
{
    moui_widget_draw(&c->base, ctx);
}
