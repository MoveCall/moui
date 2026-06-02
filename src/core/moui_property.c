#include "moui_property.h"
#include "../screen/moui_screen.h"
#include <string.h>

void moui_prop_int_init(moui_prop_int_t *prop, int32_t init_val)
{
    memset(prop, 0, sizeof(*prop));
    prop->value = init_val;
}

void moui_prop_int_set(moui_prop_int_t *prop, int32_t new_val)
{
    if (!prop || prop->value == new_val) return;
    int32_t old_val = prop->value;
    prop->value = new_val;

    for (uint8_t i = 0; i < prop->obs_count; i++) {
        moui_prop_observer_t *obs = &prop->observers[i];
        if (obs->cb) {
            obs->cb(prop, old_val, new_val, obs->user_data);
        }
    }
}

int32_t moui_prop_int_get(const moui_prop_int_t *prop)
{
    return prop ? prop->value : 0;
}

bool moui_prop_int_observe(moui_prop_int_t *prop, moui_prop_int_cb_t cb, moui_widget_t *target_widget, void *user_data)
{
    if (!prop || prop->obs_count >= MOUI_PROP_MAX_OBSERVERS) return false;
    moui_prop_observer_t *obs = &prop->observers[prop->obs_count++];
    obs->cb = cb;
    obs->target_widget = target_widget;
    obs->user_data = user_data;
    return true;
}
