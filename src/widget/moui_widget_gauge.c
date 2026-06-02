#include "moui_widget_gauge.h"
#include "../core/moui_theme.h"
#include <math.h>
#include "../core/moui_theme.h"
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979f
#endif

static void gauge_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_gauge_t *g = (moui_widget_gauge_t *)w;
    int cx = w->bounds.x + w->bounds.w / 2;
    int cy = w->bounds.y + w->bounds.h - 8;
    int r  = w->bounds.w / 2 - 4;
    if (r > w->bounds.h - 16) r = w->bounds.h - 16;

    for (int i = 0; i <= g->tick_count; i++) {
        float angle = M_PI + (float)i / g->tick_count * M_PI;
        int x1 = cx + (int)(cosf(angle) * r);
        int y1 = cy + (int)(sinf(angle) * r);
        int x2 = cx + (int)(cosf(angle) * (r - 4));
        int y2 = cy + (int)(sinf(angle) * (r - 4));
        moui_draw_line(ctx, x1, y1, x2, y2, moui_theme_fg());
    }

    moui_draw_circle(ctx, cx, cy, r, moui_theme_fg());
    moui_draw_hline(ctx, cx - r, cy, r * 2 + 1, moui_theme_fg());

    if (g->max_val > g->min_val) {
        float frac = (float)(g->value - g->min_val) / (float)(g->max_val - g->min_val);
        if (frac < 0) frac = 0;
        if (frac > 1) frac = 1;
        float angle = M_PI + frac * M_PI;
        int nx = cx + (int)(cosf(angle) * (r - 8));
        int ny = cy + (int)(sinf(angle) * (r - 8));
        moui_draw_line(ctx, cx, cy, nx, ny, moui_theme_fg());
        moui_draw_fill_circle(ctx, cx, cy, 3, moui_theme_fg());
    }

    if (g->font) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%s", (int)g->value, g->unit ? g->unit : "");
        int tw = moui_font_measure_str(g->font, buf);
        moui_font_draw_str(ctx, g->font, cx - tw / 2, cy - r / 2 - g->font->line_height / 2, buf, moui_theme_fg());

        if (g->label) {
            tw = moui_font_measure_str(g->font, g->label);
            moui_font_draw_str(ctx, g->font, cx - tw / 2, cy + 2, g->label, moui_theme_fg());
        }
    }
}

static const moui_widget_vtable_t gauge_vtable = { .draw = gauge_draw };

void moui_gauge_init(moui_widget_gauge_t *g, const moui_font_t *font,
                    int32_t min_val, int32_t max_val)
{
    moui_widget_init(&g->base, &gauge_vtable);
    g->font = font;
    g->min_val = min_val;
    g->max_val = max_val;
    g->value = min_val;
    g->tick_count = 10;
    g->label = NULL;
    g->unit = NULL;
    g->base.enabled = 0;
}
