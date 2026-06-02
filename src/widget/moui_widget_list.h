#ifndef MOUI_WIDGET_LIST_H
#define MOUI_WIDGET_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

typedef struct moui_widget_list moui_widget_list_t;

typedef const char *(*moui_list_adapter_fn)(uint16_t index, void *user_data);

struct moui_widget_list {
    moui_widget_t        base;
    moui_list_adapter_fn adapter;
    void                *adapter_data;
    uint16_t             item_count;
    uint16_t             selected;
    const moui_font_t   *font;
    uint8_t              item_h;
    bool                 loop;
    bool                 capturing;
    bool                 dragging;
    int16_t              drag_start_y;
    int16_t              drag_base_scroll;

    void (*on_select)(moui_widget_list_t *list, uint16_t idx);
};

void moui_list_init(moui_widget_list_t *l, const moui_font_t *font);
void moui_list_set_adapter(moui_widget_list_t *l, moui_list_adapter_fn fn,
                           void *user_data, uint16_t count);


#ifdef __cplusplus
}
#endif
#endif
