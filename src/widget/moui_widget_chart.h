#ifndef MOUI_WIDGET_CHART_H
#define MOUI_WIDGET_CHART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"

#define MOUI_CHART_MAX_POINTS 168

typedef struct {
    moui_widget_t  base;
    int16_t       data[MOUI_CHART_MAX_POINTS];
    uint16_t      data_count;
    uint16_t      write_pos;
    int16_t       y_min, y_max;
    bool          show_frame;
    bool          fill_below;
} moui_widget_chart_t;

void moui_chart_init(moui_widget_chart_t *c, int16_t y_min, int16_t y_max);
void moui_chart_push(moui_widget_chart_t *c, int16_t sample);
void moui_chart_clear(moui_widget_chart_t *c);


#ifdef __cplusplus
}
#endif
#endif
