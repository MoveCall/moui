#ifndef MOUI_WIDGET_GAUGE_H
#define MOUI_WIDGET_GAUGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

typedef struct {
    moui_widget_t     base;
    const moui_font_t *font;
    int32_t          value;
    int32_t          min_val, max_val;
    const char      *label;
    const char      *unit;
    uint8_t          tick_count;
} moui_widget_gauge_t;

void moui_gauge_init(moui_widget_gauge_t *g, const moui_font_t *font,
                    int32_t min_val, int32_t max_val);


#ifdef __cplusplus
}
#endif
#endif
