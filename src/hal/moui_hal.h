#ifndef MOUI_HAL_H
#define MOUI_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_hal_types.h"

typedef struct moui_hal moui_hal_t;

struct moui_hal {
    void (*display_flush)(const moui_hal_t *hal, const uint8_t *fb, uint32_t len);
    moui_time_ms_t (*get_time_ms)(const moui_hal_t *hal);
    void (*delay_ms)(const moui_hal_t *hal, uint32_t ms);
    void (*log)(const moui_hal_t *hal, const char *fmt, ...);
    void *priv;
};

void moui_hal_set_display(const moui_disp_desc_t *desc);


#ifdef __cplusplus
}
#endif
#endif
