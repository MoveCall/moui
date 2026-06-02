#include "moui_widget_icon_bar.h"
#include "../core/moui_theme.h"
#include <stdio.h>

static void icon_bar_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_icon_bar_t *ib = (moui_widget_icon_bar_t *)w;
    moui_rect_t b = w->bounds;
    const moui_font_t *f = ib->font ? ib->font : &moui_font_ascii_6x8;

    int rx = b.x + b.w - 24;
    int ry = b.y + (b.h - 12) / 2;

    /* Draw Battery Body */
    moui_draw_rect(ctx, &(moui_rect_t){(int16_t)rx, (int16_t)ry, 20, 11}, moui_theme_fg());
    moui_draw_fill_rect(ctx, &(moui_rect_t){(int16_t)(rx + 20), (int16_t)(ry + 3), 2, 5}, moui_theme_fg());

    /* Battery Fill Level */
    uint8_t fill_w = (ib->battery_pct * 16) / 100;
    if (fill_w > 0) {
        moui_draw_fill_rect(ctx, &(moui_rect_t){(int16_t)(rx + 2), (int16_t)(ry + 2), fill_w, 7}, moui_theme_fg());
    }

    /* Battery Percentage Text */
    if (ib->show_pct_text) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d%%", ib->battery_pct);
        int tw = moui_font_measure_str(f, buf);
        moui_font_draw_str(ctx, f, rx - tw - 4, b.y + (b.h - f->line_height) / 2, buf, moui_theme_fg());
    }

    /* Draw WiFi Signal Bars */
    if (ib->rssi_bars >= 0) {
        int wx = b.x + 4;
        int wy = b.y + (b.h - 10) / 2;
        for (int i = 0; i < 4; i++) {
            int bh = 3 + i * 2;
            int bx = wx + i * 4;
            int by = wy + (10 - bh);
            moui_rect_t r = { (int16_t)bx, (int16_t)by, 3, (int16_t)bh };
            if (i < ib->rssi_bars)
                moui_draw_fill_rect(ctx, &r, moui_theme_fg());
            else
                moui_draw_rect(ctx, &r, moui_theme_fg());
        }
    }
}

static const moui_widget_vtable_t ib_vtable = {
    .draw = icon_bar_draw,
};

void moui_icon_bar_init(moui_widget_icon_bar_t *ib, const moui_font_t *font)
{
    moui_widget_init(&ib->base, &ib_vtable);
    ib->font = font;
    ib->battery_pct = 100;
    ib->charging = false;
    ib->rssi_bars = 4;
    ib->ble_connected = false;
    ib->show_pct_text = true;
}

void moui_icon_bar_set_battery(moui_widget_icon_bar_t *ib, uint8_t pct, bool charging)
{
    ib->battery_pct = pct > 100 ? 100 : pct;
    ib->charging = charging;
}

void moui_icon_bar_set_wifi(moui_widget_icon_bar_t *ib, int8_t bars)
{
    ib->rssi_bars = bars > 4 ? 4 : bars;
}
