#ifndef MOUI_PROPERTY_H
#define MOUI_PROPERTY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../widget/moui_widget.h"

#define MOUI_PROP_MAX_OBSERVERS 8

typedef struct moui_prop_int moui_prop_int_t;

typedef void (*moui_prop_int_cb_t)(moui_prop_int_t *prop, int32_t old_val, int32_t new_val, void *user_data);

typedef struct {
    moui_prop_int_cb_t cb;
    void              *user_data;
    moui_widget_t     *target_widget;
} moui_prop_observer_t;

struct moui_prop_int {
    int32_t              value;
    moui_prop_observer_t observers[MOUI_PROP_MAX_OBSERVERS];
    uint8_t              obs_count;
};

#define MOUI_PROP_INT_INIT(v) { .value = (v), .obs_count = 0 }

void moui_prop_int_init(moui_prop_int_t *prop, int32_t init_val);
void moui_prop_int_set(moui_prop_int_t *prop, int32_t new_val);
int32_t moui_prop_int_get(const moui_prop_int_t *prop);
bool moui_prop_int_observe(moui_prop_int_t *prop, moui_prop_int_cb_t cb, moui_widget_t *target_widget, void *user_data);

#ifdef __cplusplus
}
#endif
#endif
