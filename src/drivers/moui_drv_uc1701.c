#include "moui_drv_uc1701.h"
#include <string.h>

static const uint8_t uc1701_init_seq[] = {
    0xE2,       /* system reset */
    0x40,       /* scroll line 0 */
    0xA0,       /* ADC normal */
    0xC8,       /* COM reverse */
    0xA2,       /* bias 1/9 */
    0x2C, 0x2E, 0x2F, /* power control: booster, regulator, follower ON */
    0xF8, 0x00, /* booster ratio 4x */
    0x27,       /* V0 resistor ratio */
    0x81, 0x10, /* electronic volume */
    0xAC, 0x00, /* static indicator off */
    0xA6,       /* normal display */
    0xAF,       /* display on */
};

static void uc1701_hw_flush(const uint8_t *data, uint32_t len, void *user)
{
    moui_drv_uc1701_t *drv = (moui_drv_uc1701_t *)user;
    (void)len;
    for (int page = 0; page < 8; page++) {
        drv->cfg.write_cmd(0xB0 | page, drv->cfg.user);
        drv->cfg.write_cmd(0x00, drv->cfg.user);
        drv->cfg.write_cmd(0x10, drv->cfg.user);
        drv->cfg.write_data(data + page * 128, 128, drv->cfg.user);
    }
}

int moui_drv_uc1701_init(moui_drv_uc1701_t *drv, const moui_uc1701_cfg_t *cfg)
{
    memset(drv, 0, sizeof(*drv));
    drv->cfg = *cfg;
    for (int i = 0; i < (int)sizeof(uc1701_init_seq); i++)
        cfg->write_cmd(uc1701_init_seq[i], cfg->user);
    return moui_backend_fb_init(&drv->fb_be, 128, 64, MOUI_PIXFMT_MONO_VMSB, uc1701_hw_flush, drv);
}

void moui_drv_uc1701_deinit(moui_drv_uc1701_t *drv) { moui_backend_fb_deinit(&drv->fb_be); }
moui_backend_t *moui_drv_uc1701_backend(moui_drv_uc1701_t *drv) { return &drv->fb_be.base; }
