#ifndef MOUI_WIDGET_RADIO_H
#define MOUI_WIDGET_RADIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

typedef struct moui_widget_radio moui_widget_radio_t;

struct moui_widget_radio {
    moui_widget_t     base;
    const char      *options[MOUI_RADIO_MAX_OPTIONS];
    uint8_t          option_count;
    uint8_t          selected;
    const moui_font_t *font;
    uint8_t          item_h;
    bool             capturing;

    void (*on_change)(moui_widget_radio_t *r, uint8_t idx);
};

void moui_radio_init(moui_widget_radio_t *r, const moui_font_t *font);
void moui_radio_set_options(moui_widget_radio_t *r, const char **opts, uint8_t count);


#ifdef __cplusplus
}
#endif
#endif
