#ifndef MOUI_DRV_ST7567_H
#define MOUI_DRV_ST7567_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../backend/moui_backend_fb.h"

/*
 * ST7567 COG LCD driver (128×64, SPI)
 *
 * Very similar to ST7565. Uses page-mode (VMSB) wire format.
 * Bias setting: 1/7 (0xA3). COM direction: reverse (0xC8).
 */

typedef struct {
    void (*write_cmd)(uint8_t cmd, void *user);
    void (*write_data)(const uint8_t *data, uint32_t len, void *user);
    void *user;
} moui_st7567_cfg_t;

typedef struct {
    moui_backend_fb_t fb_be;
    moui_st7567_cfg_t cfg;
} moui_drv_st7567_t;

int moui_drv_st7567_init(moui_drv_st7567_t *drv, const moui_st7567_cfg_t *cfg);
void moui_drv_st7567_deinit(moui_drv_st7567_t *drv);
moui_backend_t *moui_drv_st7567_backend(moui_drv_st7567_t *drv);

#ifdef __cplusplus
}
#endif

#endif
