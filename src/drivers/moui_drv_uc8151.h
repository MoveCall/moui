#ifndef MOUI_DRV_UC8151_H
#define MOUI_DRV_UC8151_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../backend/moui_backend_fb.h"

/*
 * UC8151 / UC8151D E-Paper controller driver (configurable size, SPI)
 *
 * Typical panel: 296×128 (2.9" e-paper). Wire format: HMSB (row-major).
 * Framebuffer stride = (width + 7) / 8 bytes per row.
 *
 * cfg.wait_busy must block until the BUSY pin goes low (controller ready).
 * cfg.width / cfg.height are the panel pixel dimensions.
 */

typedef struct {
    void (*write_cmd)(uint8_t cmd, void *user);
    void (*write_data)(const uint8_t *data, uint32_t len, void *user);
    void (*wait_busy)(void *user);
    void *user;
    int width;
    int height;
} moui_uc8151_cfg_t;

typedef struct {
    moui_backend_fb_t fb_be;
    moui_uc8151_cfg_t cfg;
} moui_drv_uc8151_t;

int moui_drv_uc8151_init(moui_drv_uc8151_t *drv, const moui_uc8151_cfg_t *cfg);
void moui_drv_uc8151_deinit(moui_drv_uc8151_t *drv);
moui_backend_t *moui_drv_uc8151_backend(moui_drv_uc8151_t *drv);

#ifdef __cplusplus
}
#endif

#endif
