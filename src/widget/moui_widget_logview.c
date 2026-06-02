#include "moui_widget_logview.h"
#include "../core/moui_theme.h"
#include <stdio.h>
#include <string.h>

static void logview_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_logview_t *lv = (moui_widget_logview_t *)w;
    moui_rect_t b = w->bounds;
    const moui_font_t *f = lv->font ? lv->font : &moui_font_ascii_6x8;

    if (lv->show_border) {
        moui_draw_rect(ctx, &b, moui_theme_fg());
    }

    if (lv->count == 0) return;

    int line_h = f->line_height + 2;
    int max_visible = (b.h - 4) / line_h;
    if (max_visible < 1) max_visible = 1;

    int visible_count = lv->count < max_visible ? lv->count : max_visible;
    int start_idx = (lv->count > visible_count) ? (lv->count - visible_count) : 0;

    int y = b.y + 2;
    for (int i = 0; i < visible_count; i++) {
        int idx = (lv->head + start_idx + i) % MOUI_LOGVIEW_MAX_LINES;
        moui_font_draw_str(ctx, f, b.x + 4, y, lv->lines[idx], moui_theme_fg());
        y += line_h;
    }
}

static const moui_widget_vtable_t lv_vtable = {
    .draw = logview_draw,
};

void moui_logview_init(moui_widget_logview_t *lv, const moui_font_t *font)
{
    moui_widget_init(&lv->base, &lv_vtable);
    lv->font = font;
    lv->head = 0;
    lv->count = 0;
    lv->auto_scroll = true;
    lv->show_border = true;
    memset(lv->lines, 0, sizeof(lv->lines));
}

void moui_logview_add_line(moui_widget_logview_t *lv, const char *line)
{
    if (!line) return;

    if (lv->count < MOUI_LOGVIEW_MAX_LINES) {
        snprintf(lv->lines[lv->count], MOUI_LOGVIEW_LINE_LEN, "%s", line);
        lv->count++;
    } else {
        snprintf(lv->lines[lv->head], MOUI_LOGVIEW_LINE_LEN, "%s", line);
        lv->head = (lv->head + 1) % MOUI_LOGVIEW_MAX_LINES;
    }
}

void moui_logview_clear(moui_widget_logview_t *lv)
{
    lv->head = 0;
    lv->count = 0;
    memset(lv->lines, 0, sizeof(lv->lines));
}
