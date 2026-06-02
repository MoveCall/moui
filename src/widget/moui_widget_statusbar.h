#ifndef MOUI_WIDGET_STATUSBAR_H
#define MOUI_WIDGET_STATUSBAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

typedef struct moui_widget_statusbar moui_widget_statusbar_t;

struct moui_widget_statusbar {
    moui_widget_t     base;
    const moui_font_t *font;
    const char      *title;
    char             right_text[16];
    uint8_t          battery_pct;
    bool             show_battery;
};

void moui_statusbar_init(moui_widget_statusbar_t *sb, const moui_font_t *font, const char *title);
void moui_statusbar_set_battery(moui_widget_statusbar_t *sb, uint8_t pct);
void moui_statusbar_set_right(moui_widget_statusbar_t *sb, const char *text);


#ifdef __cplusplus
}
#endif
#endif
