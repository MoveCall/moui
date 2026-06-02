#ifndef MOUI_BACKEND_H
#define MOUI_BACKEND_H

#include "../moui_conf.h"
#include "../core/moui_color.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MOUI_ROTATION_0   = 0,
    MOUI_ROTATION_90  = 1,
    MOUI_ROTATION_180 = 2,
    MOUI_ROTATION_270 = 3,
} moui_rotation_t;

typedef struct moui_backend moui_backend_t;

struct moui_backend {
    void (*set_pixel)(moui_backend_t *be, int x, int y, moui_color_t c);
    moui_color_t (*get_pixel)(moui_backend_t *be, int x, int y);
    void (*clear)(moui_backend_t *be, moui_color_t c);
    void (*flush)(moui_backend_t *be);
    void (*sleep)(moui_backend_t *be);
    void (*wakeup)(moui_backend_t *be);

    int width;
    int height;
    int phys_w;
    int phys_h;
    moui_rotation_t rotation;
    bool sw_rotate;
};

static inline void moui_be_set_pixel(moui_backend_t *be, int x, int y, moui_color_t c)
{
    if (x < 0 || x >= be->width || y < 0 || y >= be->height) return;
    int px = x, py = y;
    if (be->sw_rotate) {
        switch (be->rotation) {
        case MOUI_ROTATION_90:  px = be->phys_w - 1 - y; py = x; break;
        case MOUI_ROTATION_180: px = be->phys_w - 1 - x; py = be->phys_h - 1 - y; break;
        case MOUI_ROTATION_270: px = y; py = be->phys_h - 1 - x; break;
        default: break;
        }
    }
    be->set_pixel(be, px, py, c);
}

static inline moui_color_t moui_be_get_pixel(moui_backend_t *be, int x, int y)
{
    if (x < 0 || x >= be->width || y < 0 || y >= be->height) return MOUI_WHITE;
    int px = x, py = y;
    if (be->sw_rotate) {
        switch (be->rotation) {
        case MOUI_ROTATION_90:  px = be->phys_w - 1 - y; py = x; break;
        case MOUI_ROTATION_180: px = be->phys_w - 1 - x; py = be->phys_h - 1 - y; break;
        case MOUI_ROTATION_270: px = y; py = be->phys_h - 1 - x; break;
        default: break;
        }
    }
    return be->get_pixel(be, px, py);
}

static inline void moui_be_clear(moui_backend_t *be, moui_color_t c)
{
    be->clear(be, c);
}

static inline void moui_be_flush(moui_backend_t *be)
{
    be->flush(be);
}

static inline void moui_be_sleep(moui_backend_t *be)
{
    if (be->sleep) be->sleep(be);
}

static inline void moui_be_wakeup(moui_backend_t *be)
{
    if (be->wakeup) be->wakeup(be);
}

#ifdef __cplusplus
}
#endif

#endif
