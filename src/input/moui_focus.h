#ifndef MOUI_FOCUS_H
#define MOUI_FOCUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../widget/moui_widget.h"

typedef struct {
    moui_widget_t *focused;
    bool          wrap;
} moui_focus_group_t;

void moui_focus_group_init(moui_focus_group_t *fg);
void moui_focus_group_add(moui_focus_group_t *fg, moui_widget_t *w);
void moui_focus_group_remove(moui_focus_group_t *fg, moui_widget_t *w);
void moui_focus_group_reset(moui_focus_group_t *fg);
void moui_focus_next(moui_focus_group_t *fg);
void moui_focus_prev(moui_focus_group_t *fg);
void moui_focus_set(moui_focus_group_t *fg, moui_widget_t *w);
void moui_focus_activate_first(moui_focus_group_t *fg);
bool moui_focus_dispatch(moui_focus_group_t *fg, const moui_input_event_t *ev);


#ifdef __cplusplus
}
#endif
#endif
