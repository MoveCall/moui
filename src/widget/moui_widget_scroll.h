#ifndef MOUI_WIDGET_SCROLL_H
#define MOUI_WIDGET_SCROLL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"

typedef struct moui_widget_scroll moui_widget_scroll_t;

struct moui_widget_scroll {
    moui_widget_t     base;
    moui_widget_t    *children[MOUI_SCROLL_MAX_CHILDREN];
    uint8_t          child_count;
    int16_t          content_h;
    int16_t          scroll_step;
    bool             capturing;
    bool             show_scrollbar;
    bool             dragging;
    int16_t          drag_start_y;
    int16_t          drag_base_scroll;
};

void moui_scroll_init(moui_widget_scroll_t *s);
void moui_scroll_add(moui_widget_scroll_t *s, moui_widget_t *child);
void moui_scroll_set_content_height(moui_widget_scroll_t *s, int16_t h);


#ifdef __cplusplus
}
#endif
#endif
