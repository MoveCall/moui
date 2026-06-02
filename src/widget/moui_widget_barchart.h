#ifndef MOUI_WIDGET_BARCHART_H
#define MOUI_WIDGET_BARCHART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"
#include "../core/moui_dither.h"

#define MOUI_BARCHART_MAX_BARS 16

typedef struct moui_widget_barchart moui_widget_barchart_t;

struct moui_widget_barchart {
    moui_widget_t     base;
    const moui_font_t *font;
    int32_t          values[MOUI_BARCHART_MAX_BARS];
    const char      *labels[MOUI_BARCHART_MAX_BARS];
    uint8_t          bar_count;
    int32_t          min_val;
    int32_t          max_val;
    bool             show_grid;
    bool             show_values;
    moui_dither_pattern_t dither;
};

void moui_barchart_init(moui_widget_barchart_t *bc, const moui_font_t *font, int32_t min_val, int32_t max_val);
void moui_barchart_set_data(moui_widget_barchart_t *bc, const int32_t *values, const char **labels, uint8_t count);

#ifdef __cplusplus
}
#endif
#endif
