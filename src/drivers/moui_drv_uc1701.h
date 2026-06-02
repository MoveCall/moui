#ifndef MOUI_DRV_UC1701_H
#define MOUI_DRV_UC1701_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../backend/moui_backend_fb.h"

typedef struct {
    void (*write_cmd)(uint8_t cmd, void *user);
    void (*write_data)(const uint8_t *data, uint32_t len, void *user);
    void *user;
} moui_uc1701_cfg_t;

typedef struct {
    moui_backend_fb_t fb_be;
    moui_uc1701_cfg_t cfg;
} moui_drv_uc1701_t;

int moui_drv_uc1701_init(moui_drv_uc1701_t *drv, const moui_uc1701_cfg_t *cfg);
void moui_drv_uc1701_deinit(moui_drv_uc1701_t *drv);
moui_backend_t *moui_drv_uc1701_backend(moui_drv_uc1701_t *drv);

#ifdef __cplusplus
}
#endif

#endif
