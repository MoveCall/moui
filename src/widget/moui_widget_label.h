#ifndef MOUI_WIDGET_LABEL_H
#define MOUI_WIDGET_LABEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

typedef struct {
    moui_widget_t     base;
    const char      *text;
    const moui_font_t *font;
    bool              inverted;
    bool              wrap;
} moui_widget_label_t;

void moui_label_init(moui_widget_label_t *l, const char *text, const moui_font_t *font);


#ifdef __cplusplus
}
#endif
#endif
