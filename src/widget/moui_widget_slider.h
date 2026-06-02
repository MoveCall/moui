#ifndef MOUI_WIDGET_SLIDER_H
#define MOUI_WIDGET_SLIDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

typedef struct moui_widget_slider moui_widget_slider_t;

struct moui_widget_slider {
    moui_widget_t     base;
    int32_t          value;
    int32_t          min_val, max_val;
    int32_t          step;
    const moui_font_t *font;
    bool             show_value;
    bool             capturing;

    void (*on_change)(moui_widget_slider_t *s, int32_t new_val);
};

void moui_slider_init(moui_widget_slider_t *s, const moui_font_t *font,
                     int32_t min_val, int32_t max_val, int32_t step);


#ifdef __cplusplus
}
#endif
#endif
