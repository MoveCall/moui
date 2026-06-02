#ifndef MOUI_WIDGET_RING_H
#define MOUI_WIDGET_RING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

typedef struct {
    moui_widget_t     base;
    const moui_font_t *font;
    int32_t          value;
    int32_t          max_val;
    int              thickness;
    const char      *label;
} moui_widget_ring_t;

void moui_ring_init(moui_widget_ring_t *r, const moui_font_t *font, int32_t max_val);

#ifdef __cplusplus
}
#endif

#endif
