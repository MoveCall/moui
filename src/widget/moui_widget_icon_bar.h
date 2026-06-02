#ifndef MOUI_WIDGET_ICON_BAR_H
#define MOUI_WIDGET_ICON_BAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

typedef struct moui_widget_icon_bar moui_widget_icon_bar_t;

struct moui_widget_icon_bar {
    moui_widget_t     base;
    const moui_font_t *font;
    uint8_t          battery_pct;
    bool             charging;
    int8_t           rssi_bars; /* 0..4, -1 for disabled */
    bool             ble_connected;
    bool             show_pct_text;
};

void moui_icon_bar_init(moui_widget_icon_bar_t *ib, const moui_font_t *font);
void moui_icon_bar_set_battery(moui_widget_icon_bar_t *ib, uint8_t pct, bool charging);
void moui_icon_bar_set_wifi(moui_widget_icon_bar_t *ib, int8_t bars);

#ifdef __cplusplus
}
#endif
#endif
