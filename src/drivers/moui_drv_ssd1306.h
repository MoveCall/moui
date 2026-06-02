#ifndef MOUI_DRV_SSD1306_H
#define MOUI_DRV_SSD1306_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../backend/moui_backend_fb.h"

/*
 * SSD1306 OLED driver template (128×64 / 128×32)
 *
 * Usage:
 *   1. Implement moui_ssd1306_write_cmd() and moui_ssd1306_write_data()
 *      for your platform (I2C or SPI).
 *   2. Call moui_drv_ssd1306_init() to get a moui_backend_t*.
 *   3. Pass the backend to moui_draw_ctx_init_be().
 */

typedef struct {
    void (*write_cmd)(uint8_t cmd, void *user);
    void (*write_data)(const uint8_t *data, uint32_t len, void *user);
    void *user;
    int width;
    int height;
} moui_ssd1306_cfg_t;

typedef struct {
    moui_backend_fb_t fb_be;
    moui_ssd1306_cfg_t cfg;
} moui_drv_ssd1306_t;

int moui_drv_ssd1306_init(moui_drv_ssd1306_t *drv, const moui_ssd1306_cfg_t *cfg);
void moui_drv_ssd1306_deinit(moui_drv_ssd1306_t *drv);
moui_backend_t *moui_drv_ssd1306_backend(moui_drv_ssd1306_t *drv);

#ifdef __cplusplus
}
#endif

#endif
