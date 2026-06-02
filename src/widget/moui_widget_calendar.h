#ifndef MOUI_WIDGET_CALENDAR_H
#define MOUI_WIDGET_CALENDAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

typedef struct moui_widget_calendar moui_widget_calendar_t;

struct moui_widget_calendar {
    moui_widget_t     base;
    const moui_font_t *font;
    uint16_t         year;
    uint8_t          month;
    uint8_t          day;
    uint8_t          cursor_day;
    bool             capturing;

    void (*on_select)(moui_widget_calendar_t *cal, uint16_t y, uint8_t m, uint8_t d);
};

void moui_calendar_init(moui_widget_calendar_t *cal, const moui_font_t *font,
                       uint16_t year, uint8_t month, uint8_t day);


#ifdef __cplusplus
}
#endif
#endif
