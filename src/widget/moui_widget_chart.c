#include "moui_widget_chart.h"
#include "../core/moui_theme.h"
#include <string.h>
#include "../core/moui_theme.h"

static void chart_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_chart_t *c = (moui_widget_chart_t *)w;
    int x = w->bounds.x, y = w->bounds.y;
    int width = w->bounds.w, height = w->bounds.h;

    if (c->show_frame)
        moui_draw_rect(ctx, &w->bounds, moui_theme_fg());

    if (c->data_count == 0 || c->y_max <= c->y_min) return;

    int plot_x = x + 1;
    int plot_w = width - 2;
    int plot_y = y + 1;
    int plot_h = height - 2;

    float range = (float)(c->y_max - c->y_min);
    int count = c->data_count < plot_w ? c->data_count : plot_w;
    if (count > MOUI_CHART_MAX_POINTS) count = MOUI_CHART_MAX_POINTS;

    int prev_py = -1;
    for (int i = 0; i < count; i++) {
        int idx = (c->write_pos - count + i + MOUI_CHART_MAX_POINTS) % MOUI_CHART_MAX_POINTS;
        float norm = (float)(c->data[idx] - c->y_min) / range;
        if (norm < 0.0f) norm = 0.0f;
        if (norm > 1.0f) norm = 1.0f;
        int py = plot_y + plot_h - 1 - (int)(norm * (plot_h - 1));
        int px = plot_x + i;

        if (c->fill_below) {
            moui_draw_vline(ctx, px, py, plot_y + plot_h - py, moui_theme_fg());
        }

        moui_draw_pixel(ctx, px, py, moui_theme_fg());

        if (prev_py >= 0 && i > 0) {
            moui_draw_line(ctx, px - 1, prev_py, px, py, moui_theme_fg());
        }
        prev_py = py;
    }
}

static const moui_widget_vtable_t chart_vtable = {
    .draw = chart_draw,
};

void moui_chart_init(moui_widget_chart_t *c, int16_t y_min, int16_t y_max)
{
    moui_widget_init(&c->base, &chart_vtable);
    c->y_min      = y_min;
    c->y_max      = y_max;
    c->show_frame = true;
    c->fill_below = false;
    c->data_count = 0;
    c->write_pos  = 0;
    c->base.enabled = 0;
}

void moui_chart_push(moui_widget_chart_t *c, int16_t sample)
{
    c->data[c->write_pos] = sample;
    c->write_pos = (c->write_pos + 1) % MOUI_CHART_MAX_POINTS;
    if (c->data_count < MOUI_CHART_MAX_POINTS) c->data_count++;
}

void moui_chart_clear(moui_widget_chart_t *c)
{
    c->data_count = 0;
    c->write_pos  = 0;
    memset(c->data, 0, sizeof(c->data));
}
