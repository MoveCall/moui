#include "moui_drv_st7565.h"
#include <string.h>

static const uint8_t st7565_init_seq[] = {
    0xA0,       /* ADC select: normal */
    0xAE,       /* display off */
    0xC8,       /* COM output scan: reverse */
    0xA2,       /* LCD bias: 1/9 */
    0x2F,       /* power control: all on */
    0x26,       /* resistor ratio: 6 */
    0x81, 0x20, /* electronic volume: 32 */
    0xAF,       /* display on */
};

static void st7565_hw_flush(const uint8_t *data, uint32_t len, void *user)
{
    moui_drv_st7565_t *drv = (moui_drv_st7565_t *)user;
    (void)len;
    for (int page = 0; page < 8; page++) {
        drv->cfg.write_cmd(0xB0 | page, drv->cfg.user);
        drv->cfg.write_cmd(0x10, drv->cfg.user);  /* upper column */
        drv->cfg.write_cmd(0x00, drv->cfg.user);  /* lower column */
        drv->cfg.write_data(data + page * 128, 128, drv->cfg.user);
    }
}

int moui_drv_st7565_init(moui_drv_st7565_t *drv, const moui_st7565_cfg_t *cfg)
{
    memset(drv, 0, sizeof(*drv));
    drv->cfg = *cfg;

    for (int i = 0; i < (int)sizeof(st7565_init_seq); i++)
        cfg->write_cmd(st7565_init_seq[i], cfg->user);

    return moui_backend_fb_init(&drv->fb_be, 128, 64,
                               MOUI_PIXFMT_MONO_VMSB, st7565_hw_flush, drv);
}

void moui_drv_st7565_deinit(moui_drv_st7565_t *drv)
{
    moui_backend_fb_deinit(&drv->fb_be);
}

moui_backend_t *moui_drv_st7565_backend(moui_drv_st7565_t *drv)
{
    return &drv->fb_be.base;
}
