#ifndef MOUI_WIDGET_DROPDOWN_H
#define MOUI_WIDGET_DROPDOWN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

typedef struct moui_widget_dropdown moui_widget_dropdown_t;

struct moui_widget_dropdown {
    moui_widget_t     base;
    const char      *options[MOUI_DROPDOWN_MAX_OPTIONS];
    uint8_t          option_count;
    uint8_t          selected;
    const moui_font_t *font;
    bool             open;
    uint8_t          item_h;
    uint8_t          visible_count;

    void (*on_change)(moui_widget_dropdown_t *dd, uint8_t idx);
};

void moui_dropdown_init(moui_widget_dropdown_t *dd, const moui_font_t *font);
void moui_dropdown_set_options(moui_widget_dropdown_t *dd, const char **opts, uint8_t count);


#ifdef __cplusplus
}
#endif
#endif
