#ifndef MOUI_DRV_ST7920_H
#define MOUI_DRV_ST7920_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../backend/moui_backend_fb.h"

/*
 * ST7920 graphic LCD driver (128×64, SPI serial mode)
 *
 * Uses extended instruction set for graphic RAM access.
 * Wire format: HMSB (horizontal, MSB-first), 16 bytes per row.
 * Upper half: rows 0–31 at Y=0x80+row, X=0x80.
 * Lower half: rows 0–31 at Y=0x80+row, X=0x88.
 * cfg.width and cfg.height control the framebuffer dimensions.
 */

typedef struct {
    void (*write_cmd)(uint8_t cmd, void *user);
    void (*write_data)(const uint8_t *data, uint32_t len, void *user);
    void *user;
    int width;
    int height;
} moui_st7920_cfg_t;

typedef struct {
    moui_backend_fb_t fb_be;
    moui_st7920_cfg_t cfg;
} moui_drv_st7920_t;

int moui_drv_st7920_init(moui_drv_st7920_t *drv, const moui_st7920_cfg_t *cfg);
void moui_drv_st7920_deinit(moui_drv_st7920_t *drv);
moui_backend_t *moui_drv_st7920_backend(moui_drv_st7920_t *drv);

#ifdef __cplusplus
}
#endif

#endif
