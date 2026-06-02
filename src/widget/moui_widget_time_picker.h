#ifndef MOUI_WIDGET_TIME_PICKER_H
#define MOUI_WIDGET_TIME_PICKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

typedef struct moui_widget_time_picker moui_widget_time_picker_t;

struct moui_widget_time_picker {
    moui_widget_t     base;
    const moui_font_t *font;
    uint8_t          hour;
    uint8_t          min;
    uint8_t          sec;
    uint8_t          active_col; /* 0: hour, 1: min, 2: sec */
    bool             show_sec;

    void (*on_change)(moui_widget_time_picker_t *tp, uint8_t h, uint8_t m, uint8_t s);
};

void moui_time_picker_init(moui_widget_time_picker_t *tp, const moui_font_t *font);
void moui_time_picker_set_time(moui_widget_time_picker_t *tp, uint8_t h, uint8_t m, uint8_t s);
void moui_time_picker_draw(moui_widget_t *w, moui_draw_ctx_t *ctx);
bool moui_time_picker_event(moui_widget_t *w, const moui_input_event_t *ev);

#ifdef __cplusplus
}
#endif
#endif
