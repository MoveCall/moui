#include "moui_drv_st7567.h"
#include <string.h>

static const uint8_t st7567_init_seq[] = {
    0xE2,       /* software system reset */
    0xA3,       /* LCD bias: 1/7 */
    0xA0,       /* ADC select: normal */
    0xC8,       /* COM output scan: reverse */
    0x2F,       /* power control: booster, regulator, follower all on */
    0x26,       /* V0 resistor ratio: 6 */
    0x81, 0x20, /* electronic volume: 32 */
    0xAF,       /* display on */
};

static void st7567_hw_flush(const uint8_t *data, uint32_t len, void *user)
{
    moui_drv_st7567_t *drv = (moui_drv_st7567_t *)user;
    (void)len;
    for (int page = 0; page < 8; page++) {
        drv->cfg.write_cmd(0xB0 | page, drv->cfg.user);  /* page address */
        drv->cfg.write_cmd(0x10, drv->cfg.user);          /* upper column address: 0 */
        drv->cfg.write_cmd(0x00, drv->cfg.user);          /* lower column address: 0 */
        drv->cfg.write_data(data + page * 128, 128, drv->cfg.user);
    }
}

int moui_drv_st7567_init(moui_drv_st7567_t *drv, const moui_st7567_cfg_t *cfg)
{
    memset(drv, 0, sizeof(*drv));
    drv->cfg = *cfg;

    for (int i = 0; i < (int)sizeof(st7567_init_seq); i++)
        cfg->write_cmd(st7567_init_seq[i], cfg->user);

    return moui_backend_fb_init(&drv->fb_be, 128, 64,
                               MOUI_PIXFMT_MONO_VMSB, st7567_hw_flush, drv);
}

void moui_drv_st7567_deinit(moui_drv_st7567_t *drv)
{
    moui_backend_fb_deinit(&drv->fb_be);
}

moui_backend_t *moui_drv_st7567_backend(moui_drv_st7567_t *drv)
{
    return &drv->fb_be.base;
}
