#ifndef MOUI_WIDGET_SWITCH_H
#define MOUI_WIDGET_SWITCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"

typedef struct moui_widget_switch moui_widget_switch_t;
struct moui_widget_switch {
    moui_widget_t base;
    bool         state;
    void (*on_change)(moui_widget_switch_t *sw, bool new_state);
};

void moui_switch_init(moui_widget_switch_t *sw);


#ifdef __cplusplus
}
#endif
#endif
