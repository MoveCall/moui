#ifndef MOUI_WIDGET_SPINNER_H
#define MOUI_WIDGET_SPINNER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

typedef struct moui_widget_spinner moui_widget_spinner_t;

struct moui_widget_spinner {
    moui_widget_t     base;
    const moui_font_t *font;
    int32_t          value;
    int32_t          min_val, max_val;
    int32_t          step;
    const char      *format;
    bool             capturing;
    bool             wrap;

    void (*on_change)(moui_widget_spinner_t *sp, int32_t new_val);
};

void moui_spinner_init(moui_widget_spinner_t *sp, const moui_font_t *font,
                      int32_t min_val, int32_t max_val, int32_t step);


#ifdef __cplusplus
}
#endif
#endif
