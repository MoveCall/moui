#include "moui_drv_ssd1309.h"
#include <string.h>

static const uint8_t ssd1309_init_seq[] = {
    0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
    0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12,
    0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF,
};

static void ssd1309_hw_flush(const uint8_t *data, uint32_t len, void *user)
{
    moui_drv_ssd1309_t *drv = (moui_drv_ssd1309_t *)user;
    drv->cfg.write_cmd(0x21, drv->cfg.user);
    drv->cfg.write_cmd(0x00, drv->cfg.user);
    drv->cfg.write_cmd((uint8_t)(drv->cfg.width - 1), drv->cfg.user);
    drv->cfg.write_cmd(0x22, drv->cfg.user);
    drv->cfg.write_cmd(0x00, drv->cfg.user);
    drv->cfg.write_cmd((uint8_t)(drv->cfg.height / 8 - 1), drv->cfg.user);
    drv->cfg.write_data(data, len, drv->cfg.user);
}

int moui_drv_ssd1309_init(moui_drv_ssd1309_t *drv, const moui_ssd1309_cfg_t *cfg)
{
    memset(drv, 0, sizeof(*drv));
    drv->cfg = *cfg;
    if (drv->cfg.width == 0) drv->cfg.width = 128;
    if (drv->cfg.height == 0) drv->cfg.height = 64;

    for (int i = 0; i < (int)sizeof(ssd1309_init_seq); i++)
        cfg->write_cmd(ssd1309_init_seq[i], cfg->user);

    return moui_backend_fb_init(&drv->fb_be, drv->cfg.width, drv->cfg.height,
                               MOUI_PIXFMT_MONO_VMSB, ssd1309_hw_flush, drv);
}

void moui_drv_ssd1309_deinit(moui_drv_ssd1309_t *drv) { moui_backend_fb_deinit(&drv->fb_be); }
moui_backend_t *moui_drv_ssd1309_backend(moui_drv_ssd1309_t *drv) { return &drv->fb_be.base; }
