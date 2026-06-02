#ifndef MOUI_WIDGET_ROLLER_H
#define MOUI_WIDGET_ROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

#define MOUI_ROLLER_MAX_ITEMS 32

typedef struct moui_widget_roller moui_widget_roller_t;

struct moui_widget_roller {
    moui_widget_t     base;
    const moui_font_t *font;
    const char      *items[MOUI_ROLLER_MAX_ITEMS];
    uint16_t         item_count;
    uint16_t         selected;
    bool             loop;

    void (*on_change)(moui_widget_roller_t *r, uint16_t selected_idx);
};

void moui_roller_init(moui_widget_roller_t *r, const moui_font_t *font);
void moui_roller_set_items(moui_widget_roller_t *r, const char **items, uint16_t count);
void moui_roller_set_selected(moui_widget_roller_t *r, uint16_t index);

#ifdef __cplusplus
}
#endif
#endif
