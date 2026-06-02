#include "moui_grid.h"

static void grid_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_grid_t *g = (moui_grid_t *)w;
    moui_draw_push_clip(ctx, &w->bounds);
    for (int i = 0; i < g->child_count; i++) {
        moui_widget_draw(g->children[i], ctx);
    }
    moui_draw_pop_clip(ctx);
}

static const moui_widget_vtable_t grid_vtable = {
    .draw = grid_draw,
};

void moui_grid_init(moui_grid_t *g, uint8_t cols, uint8_t rows)
{
    moui_widget_init(&g->base, &grid_vtable);
    g->cols = cols > 0 ? cols : 1;
    g->rows = rows > 0 ? rows : 1;
    g->padding = 0;
    g->gap = 0;
    g->child_count = 0;
}

void moui_grid_add(moui_grid_t *g, moui_widget_t *child)
{
    if (g->child_count >= MOUI_GRID_MAX_CHILDREN) return;
    if (g->child_count >= g->cols * g->rows) return;
    g->children[g->child_count++] = child;
    child->parent = &g->base;
}

void moui_grid_layout(moui_grid_t *g)
{
    moui_rect_t b = g->base.bounds;
    int ox = b.x + g->padding;
    int oy = b.y + g->padding;
    int avail_w = b.w - 2 * g->padding;
    int avail_h = b.h - 2 * g->padding;

    int total_gap_x = (g->cols > 1) ? g->gap * (g->cols - 1) : 0;
    int total_gap_y = (g->rows > 1) ? g->gap * (g->rows - 1) : 0;
    int cell_w = (avail_w - total_gap_x) / g->cols;
    int cell_h = (avail_h - total_gap_y) / g->rows;
    if (cell_w < 1) cell_w = 1;
    if (cell_h < 1) cell_h = 1;

    for (int i = 0; i < g->child_count; i++) {
        int col = i % g->cols;
        int row = i / g->cols;
        moui_widget_t *ch = g->children[i];
        ch->bounds.x = ox + col * (cell_w + g->gap);
        ch->bounds.y = oy + row * (cell_h + g->gap);
        ch->bounds.w = (ch->w_hint >= 0) ? ch->w_hint : cell_w;
        ch->bounds.h = (ch->h_hint >= 0) ? ch->h_hint : cell_h;
    }
}
