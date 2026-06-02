#ifndef MOUI_DRV_SH1106_H
#define MOUI_DRV_SH1106_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../backend/moui_backend_fb.h"

/*
 * SH1106 OLED driver template (128×64, page addressing, 132-column controller)
 *
 * Same usage pattern as SSD1306 driver.
 * Key difference: SH1106 uses page-by-page addressing (no horizontal scroll mode),
 * and the visible 128 columns start at column offset 2 in the 132-column buffer.
 */

typedef struct {
    void (*write_cmd)(uint8_t cmd, void *user);
    void (*write_data)(const uint8_t *data, uint32_t len, void *user);
    void *user;
} moui_sh1106_cfg_t;

typedef struct {
    moui_backend_fb_t fb_be;
    moui_sh1106_cfg_t cfg;
} moui_drv_sh1106_t;

int moui_drv_sh1106_init(moui_drv_sh1106_t *drv, const moui_sh1106_cfg_t *cfg);
void moui_drv_sh1106_deinit(moui_drv_sh1106_t *drv);
moui_backend_t *moui_drv_sh1106_backend(moui_drv_sh1106_t *drv);

#ifdef __cplusplus
}
#endif

#endif
