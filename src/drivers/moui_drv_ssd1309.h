#ifndef MOUI_DRV_SSD1309_H
#define MOUI_DRV_SSD1309_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../backend/moui_backend_fb.h"

typedef struct {
    void (*write_cmd)(uint8_t cmd, void *user);
    void (*write_data)(const uint8_t *data, uint32_t len, void *user);
    void *user;
    int width;
    int height;
} moui_ssd1309_cfg_t;

typedef struct {
    moui_backend_fb_t fb_be;
    moui_ssd1309_cfg_t cfg;
} moui_drv_ssd1309_t;

int moui_drv_ssd1309_init(moui_drv_ssd1309_t *drv, const moui_ssd1309_cfg_t *cfg);
void moui_drv_ssd1309_deinit(moui_drv_ssd1309_t *drv);
moui_backend_t *moui_drv_ssd1309_backend(moui_drv_ssd1309_t *drv);

#ifdef __cplusplus
}
#endif

#endif
