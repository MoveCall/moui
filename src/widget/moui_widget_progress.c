#include "moui_widget_progress.h"
#include "../core/moui_theme.h"
#include "../core/moui_style.h"
#include "../core/moui_theme.h"
#include <stdio.h>

static void progress_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_progress_t *p = (moui_widget_progress_t *)w;
    int x = w->bounds.x, y = w->bounds.y;
    int width = w->bounds.w, height = w->bounds.h;
    uint8_t rad = moui_style_radius(w->style);

    moui_draw_rounded_rect(ctx, &w->bounds, rad, moui_theme_fg());

    if (p->max_val > 0) {
        int fill_w = (int)((float)p->value / p->max_val * (width - 4));
        if (fill_w > width - 4) fill_w = width - 4;
        if (fill_w > 0)
            moui_draw_fill_rounded_rect(ctx, &(moui_rect_t){x + 2, y + 2, fill_w, height - 4},
                                       rad > 1 ? rad - 1 : 0, moui_theme_fg());
    }

    if (p->show_pct && p->font) {
        char buf[8];
        int pct = p->max_val > 0 ? (int)(p->value * 100 / p->max_val) : 0;
        snprintf(buf, sizeof(buf), "%d%%", pct);
        int tw = moui_font_measure_str(p->font, buf);
        int tx = x + (width - tw) / 2;
        int ty = y + (height - p->font->line_height) / 2;
        moui_font_draw_str(ctx, p->font, tx, ty, buf, moui_theme_fg());
    }
}

static const moui_widget_vtable_t progress_vtable = {
    .draw = progress_draw,
};

void moui_progress_init(moui_widget_progress_t *p, const moui_font_t *font, int32_t max_val)
{
    moui_widget_init(&p->base, &progress_vtable);
    p->font = font;
    p->value = 0;
    p->max_val = max_val;
    p->show_pct = true;
    p->base.enabled = 0;
}
