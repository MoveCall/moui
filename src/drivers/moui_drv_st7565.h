#ifndef MOUI_DRV_ST7565_H
#define MOUI_DRV_ST7565_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../backend/moui_backend_fb.h"

/*
 * ST7565 mono LCD driver template (128×64, SPI only)
 *
 * Common in: Nokia-style LCDs, COG displays, instrument panels.
 * Wire format: page mode (VMSB), same as SSD1306.
 */

typedef struct {
    void (*write_cmd)(uint8_t cmd, void *user);
    void (*write_data)(const uint8_t *data, uint32_t len, void *user);
    void *user;
} moui_st7565_cfg_t;

typedef struct {
    moui_backend_fb_t fb_be;
    moui_st7565_cfg_t cfg;
} moui_drv_st7565_t;

int moui_drv_st7565_init(moui_drv_st7565_t *drv, const moui_st7565_cfg_t *cfg);
void moui_drv_st7565_deinit(moui_drv_st7565_t *drv);
moui_backend_t *moui_drv_st7565_backend(moui_drv_st7565_t *drv);

#ifdef __cplusplus
}
#endif

#endif
