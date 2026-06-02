#include "moui_widget_statusbar.h"
#include "../core/moui_theme.h"
#include <string.h>
#include <stdio.h>

static void draw_battery_icon(moui_draw_ctx_t *ctx, int x, int y, uint8_t pct)
{
    moui_draw_rect(ctx, &(moui_rect_t){x, y + 1, 14, 8}, moui_theme_fg());
    moui_draw_fill_rect(ctx, &(moui_rect_t){x + 14, y + 3, 2, 4}, moui_theme_fg());

    int fill = (int)(pct * 11 / 100);
    if (fill > 0)
        moui_draw_fill_rect(ctx, &(moui_rect_t){x + 1, y + 2, fill, 6}, moui_theme_fg());
}

static void statusbar_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_statusbar_t *sb = (moui_widget_statusbar_t *)w;
    if (!sb->font) return;

    moui_draw_fill_rect(ctx, &w->bounds, moui_theme_fg());

    if (sb->title) {
        moui_font_draw_str(ctx, sb->font, w->bounds.x + 3, w->bounds.y + 3,
                          sb->title, moui_theme_bg());
    }

    int rx = w->bounds.x + w->bounds.w;

    if (sb->show_battery) {
        rx -= 20;
        draw_battery_icon(ctx, rx, w->bounds.y + 2, sb->battery_pct);
    }

    if (sb->right_text[0]) {
        int tw = moui_font_measure_str(sb->font, sb->right_text);
        rx -= tw + 4;
        moui_font_draw_str(ctx, sb->font, rx, w->bounds.y + 3,
                          sb->right_text, moui_theme_bg());
    }
}

static const moui_widget_vtable_t statusbar_vtable = {
    .draw = statusbar_draw,
};

void moui_statusbar_init(moui_widget_statusbar_t *sb, const moui_font_t *font, const char *title)
{
    moui_widget_init(&sb->base, &statusbar_vtable);
    sb->font = font;
    sb->title = title;
    sb->right_text[0] = '\0';
    sb->battery_pct = 100;
    sb->show_battery = true;
    sb->base.bounds = (moui_rect_t){0, 0, MOUI_DISP_W, moui_theme.statusbar_h};
    sb->base.enabled = 0;
}

void moui_statusbar_set_battery(moui_widget_statusbar_t *sb, uint8_t pct)
{
    sb->battery_pct = pct > 100 ? 100 : pct;
}

void moui_statusbar_set_right(moui_widget_statusbar_t *sb, const char *text)
{
    if (text)
        snprintf(sb->right_text, sizeof(sb->right_text), "%s", text);
    else
        sb->right_text[0] = '\0';
}
