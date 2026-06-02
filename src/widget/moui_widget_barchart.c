#include "moui_widget_barchart.h"
#include "../core/moui_theme.h"
#include <stdio.h>
#include <string.h>

static void barchart_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_barchart_t *bc = (moui_widget_barchart_t *)w;
    moui_rect_t b = w->bounds;
    const moui_font_t *f = bc->font ? bc->font : &moui_font_ascii_6x8;

    moui_draw_rect(ctx, &b, moui_theme_fg());

    if (bc->bar_count == 0) return;

    int label_h = f->line_height + 2;
    int chart_h = b.h - label_h - 4;
    int chart_y0 = b.y + 2 + chart_h;
    int chart_x0 = b.x + 4;
    int chart_w = b.w - 8;

    /* Draw 50% Gridline and Baseline */
    moui_draw_hline(ctx, chart_x0, chart_y0, chart_w, moui_theme_fg());
    if (bc->show_grid && chart_h > 20) {
        moui_draw_hline(ctx, chart_x0, chart_y0 - chart_h / 2, chart_w, moui_theme_fg());
    }

    int seg_w = chart_w / bc->bar_count;
    int bar_w = seg_w > 6 ? seg_w - 4 : seg_w - 1;
    if (bar_w < 1) bar_w = 1;

    int32_t range = bc->max_val - bc->min_val;
    if (range <= 0) range = 1;

    for (int i = 0; i < bc->bar_count; i++) {
        int bx = chart_x0 + i * seg_w + (seg_w - bar_w) / 2;
        int32_t val = bc->values[i];
        if (val < bc->min_val) val = bc->min_val;
        if (val > bc->max_val) val = bc->max_val;

        int bh = (int)(((int64_t)(val - bc->min_val) * (chart_h - 12)) / range);
        if (bh < 1) bh = 1;
        int by = chart_y0 - bh;

        moui_rect_t bar_r = { (int16_t)bx, (int16_t)by, (int16_t)bar_w, (int16_t)bh };
        moui_draw_fill_rect_dither(ctx, &bar_r, bc->dither);
        moui_draw_rect(ctx, &bar_r, moui_theme_fg());

        /* Value text on top of bar */
        if (bc->show_values && by - f->line_height >= b.y + 2) {
            char val_buf[16];
            snprintf(val_buf, sizeof(val_buf), "%d", (int)val);
            int vtw = moui_font_measure_str(f, val_buf);
            int vx = bx + (bar_w - vtw) / 2;
            moui_font_draw_str(ctx, f, vx, by - f->line_height - 1, val_buf, moui_theme_fg());
        }

        /* Label below */
        if (bc->labels[i]) {
            int tw = moui_font_measure_str(f, bc->labels[i]);
            int lx = bx + (bar_w - tw) / 2;
            moui_font_draw_str(ctx, f, lx, chart_y0 + 2, bc->labels[i], moui_theme_fg());
        }
    }
}

static const moui_widget_vtable_t bc_vtable = {
    .draw = barchart_draw,
};

void moui_barchart_init(moui_widget_barchart_t *bc, const moui_font_t *font, int32_t min_val, int32_t max_val)
{
    moui_widget_init(&bc->base, &bc_vtable);
    bc->font = font;
    bc->min_val = min_val;
    bc->max_val = max_val;
    bc->bar_count = 0;
    bc->show_grid = true;
    bc->show_values = false;
    bc->dither = MOUI_DITHER_50;
    memset(bc->values, 0, sizeof(bc->values));
    memset(bc->labels, 0, sizeof(bc->labels));
}

void moui_barchart_set_data(moui_widget_barchart_t *bc, const int32_t *values, const char **labels, uint8_t count)
{
    if (count > MOUI_BARCHART_MAX_BARS) count = MOUI_BARCHART_MAX_BARS;
    bc->bar_count = count;
    for (uint8_t i = 0; i < count; i++) {
        bc->values[i] = values[i];
        bc->labels[i] = labels ? labels[i] : NULL;
    }
}
