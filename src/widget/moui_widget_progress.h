#ifndef MOUI_WIDGET_PROGRESS_H
#define MOUI_WIDGET_PROGRESS_H

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
    bool             show_pct;
} moui_widget_progress_t;

void moui_progress_init(moui_widget_progress_t *p, const moui_font_t *font, int32_t max_val);


#ifdef __cplusplus
}
#endif
#endif
