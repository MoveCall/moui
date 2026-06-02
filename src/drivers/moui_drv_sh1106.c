#include "moui_drv_sh1106.h"
#include <string.h>

static const uint8_t sh1106_init_seq[] = {
    0xAE,       /* display off */
    0xD5, 0x80, /* clock div */
    0xA8, 0x3F, /* multiplex 64 */
    0xD3, 0x00, /* offset 0 */
    0x40,       /* start line */
    0xAD, 0x8B, /* DC-DC on */
    0xA1,       /* segment remap */
    0xC8,       /* COM scan dec */
    0xDA, 0x12, /* COM pins */
    0x81, 0xFF, /* contrast max */
    0xD9, 0x1F, /* pre-charge */
    0xDB, 0x40, /* VCOMH */
    0xA6,       /* normal display */
    0xAF,       /* display on */
};

static void sh1106_hw_flush(const uint8_t *data, uint32_t len, void *user)
{
    moui_drv_sh1106_t *drv = (moui_drv_sh1106_t *)user;
    (void)len;
    /* SH1106: must send page by page with column offset 2 */
    for (int page = 0; page < 8; page++) {
        drv->cfg.write_cmd(0xB0 | page, drv->cfg.user);         /* set page */
        drv->cfg.write_cmd(0x02, drv->cfg.user);                 /* lower col addr (offset 2) */
        drv->cfg.write_cmd(0x10, drv->cfg.user);                 /* upper col addr */
        drv->cfg.write_data(data + page * 128, 128, drv->cfg.user);
    }
}

int moui_drv_sh1106_init(moui_drv_sh1106_t *drv, const moui_sh1106_cfg_t *cfg)
{
    memset(drv, 0, sizeof(*drv));
    drv->cfg = *cfg;

    for (int i = 0; i < (int)sizeof(sh1106_init_seq); i++)
        cfg->write_cmd(sh1106_init_seq[i], cfg->user);

    return moui_backend_fb_init(&drv->fb_be, 128, 64,
                               MOUI_PIXFMT_MONO_VMSB, sh1106_hw_flush, drv);
}

void moui_drv_sh1106_deinit(moui_drv_sh1106_t *drv)
{
    moui_backend_fb_deinit(&drv->fb_be);
}

moui_backend_t *moui_drv_sh1106_backend(moui_drv_sh1106_t *drv)
{
    return &drv->fb_be.base;
}
