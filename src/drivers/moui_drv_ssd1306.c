#include "moui_drv_ssd1306.h"
#include <string.h>

static const uint8_t ssd1306_init_128x64[] = {
    0xAE,       /* display off */
    0xD5, 0x80, /* clock div */
    0xA8, 0x3F, /* multiplex 64 */
    0xD3, 0x00, /* display offset 0 */
    0x40,       /* start line 0 */
    0x8D, 0x14, /* charge pump enable */
    0x20, 0x00, /* horizontal addressing */
    0xA1,       /* segment remap */
    0xC8,       /* COM scan dec */
    0xDA, 0x12, /* COM pins */
    0x81, 0xCF, /* contrast */
    0xD9, 0xF1, /* pre-charge */
    0xDB, 0x40, /* VCOMH deselect */
    0xA4,       /* display from RAM */
    0xA6,       /* normal (not inverted) */
    0xAF,       /* display on */
};

static const uint8_t ssd1306_init_128x32[] = {
    0xAE,
    0xD5, 0x80,
    0xA8, 0x1F, /* multiplex 32 */
    0xD3, 0x00,
    0x40,
    0x8D, 0x14,
    0x20, 0x00,
    0xA1,
    0xC8,
    0xDA, 0x02, /* COM pins for 32 rows */
    0x81, 0x8F,
    0xD9, 0xF1,
    0xDB, 0x40,
    0xA4,
    0xA6,
    0xAF,
};

static void ssd1306_hw_flush(const uint8_t *data, uint32_t len, void *user)
{
    moui_drv_ssd1306_t *drv = (moui_drv_ssd1306_t *)user;
    /* Set column and page address to full screen */
    drv->cfg.write_cmd(0x21, drv->cfg.user); /* column addr */
    drv->cfg.write_cmd(0x00, drv->cfg.user);
    drv->cfg.write_cmd((uint8_t)(drv->cfg.width - 1), drv->cfg.user);
    drv->cfg.write_cmd(0x22, drv->cfg.user); /* page addr */
    drv->cfg.write_cmd(0x00, drv->cfg.user);
    drv->cfg.write_cmd((uint8_t)(drv->cfg.height / 8 - 1), drv->cfg.user);
    drv->cfg.write_data(data, len, drv->cfg.user);
}

int moui_drv_ssd1306_init(moui_drv_ssd1306_t *drv, const moui_ssd1306_cfg_t *cfg)
{
    memset(drv, 0, sizeof(*drv));
    drv->cfg = *cfg;
    if (drv->cfg.width == 0) drv->cfg.width = 128;
    if (drv->cfg.height == 0) drv->cfg.height = 64;

    /* Send init sequence */
    const uint8_t *seq;
    int seq_len;
    if (drv->cfg.height <= 32) {
        seq = ssd1306_init_128x32;
        seq_len = (int)sizeof(ssd1306_init_128x32);
    } else {
        seq = ssd1306_init_128x64;
        seq_len = (int)sizeof(ssd1306_init_128x64);
    }
    for (int i = 0; i < seq_len; i++) {
        cfg->write_cmd(seq[i], cfg->user);
    }

    /* SSD1306 uses VMSB (page mode) wire format */
    int ret = moui_backend_fb_init(&drv->fb_be, drv->cfg.width, drv->cfg.height,
                                  MOUI_PIXFMT_MONO_VMSB, ssd1306_hw_flush, drv);
    return ret;
}

void moui_drv_ssd1306_deinit(moui_drv_ssd1306_t *drv)
{
    moui_backend_fb_deinit(&drv->fb_be);
}

moui_backend_t *moui_drv_ssd1306_backend(moui_drv_ssd1306_t *drv)
{
    return &drv->fb_be.base;
}
