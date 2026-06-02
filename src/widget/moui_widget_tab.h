#ifndef MOUI_WIDGET_TAB_H
#define MOUI_WIDGET_TAB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

typedef struct moui_widget_tab moui_widget_tab_t;

struct moui_widget_tab {
    moui_widget_t     base;
    const moui_font_t *font;
    const char      *titles[MOUI_TAB_MAX_PAGES];
    moui_widget_t    *pages[MOUI_TAB_MAX_PAGES];
    uint8_t          page_count;
    uint8_t          active;
    uint8_t          tab_h;
    bool             capturing;

    void (*on_change)(moui_widget_tab_t *t, uint8_t idx);
};

void moui_tab_init(moui_widget_tab_t *t, const moui_font_t *font);
void moui_tab_add_page(moui_widget_tab_t *t, const char *title, moui_widget_t *page);


#ifdef __cplusplus
}
#endif
#endif
