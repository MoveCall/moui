#include "moui_drv_uc8151.h"
#include <string.h>

/*
 * UC8151 / UC8151D E-Paper controller.
 *
 * Init sequence follows the datasheet power-on recommended flow:
 *   SW reset → panel setting → power setting → power on (wait busy) →
 *   booster soft start → PLL → resolution → VCM_DC → VCOM data interval.
 *
 * Flush sequence: DTM2 data start (0x13) → pixel data (full frame) →
 *   display refresh (0x12) → wait busy.
 *
 * Wire format: HMSB (horizontal, MSB-first). Stride = (width+7)/8 bytes/row.
 * 1 bit per pixel: 0 = black, 1 = white (UC8151 convention).
 */

/* Helper: send one command byte with no data payload */
static inline void uc8151_cmd(moui_drv_uc8151_t *drv, uint8_t cmd)
{
    drv->cfg.write_cmd(cmd, drv->cfg.user);
}

/* Helper: send one command followed by a data byte array */
static inline void uc8151_cmd_data(moui_drv_uc8151_t *drv, uint8_t cmd,
                                   const uint8_t *data, uint32_t len)
{
    drv->cfg.write_cmd(cmd, drv->cfg.user);
    drv->cfg.write_data(data, len, drv->cfg.user);
}

static void uc8151_hw_flush(const uint8_t *data, uint32_t len, void *user)
{
    moui_drv_uc8151_t *drv = (moui_drv_uc8151_t *)user;

    /* DTM2: transmit new pixel data to controller SRAM (B/W plane) */
    drv->cfg.write_cmd(0x13, drv->cfg.user);
    drv->cfg.write_data(data, len, drv->cfg.user);

    /* Display refresh */
    drv->cfg.write_cmd(0x12, drv->cfg.user);
    drv->cfg.wait_busy(drv->cfg.user);
}

int moui_drv_uc8151_init(moui_drv_uc8151_t *drv, const moui_uc8151_cfg_t *cfg)
{
    memset(drv, 0, sizeof(*drv));
    drv->cfg = *cfg;
    if (drv->cfg.width  == 0) drv->cfg.width  = 296;
    if (drv->cfg.height == 0) drv->cfg.height = 128;

    const int w = drv->cfg.width;
    const int h = drv->cfg.height;

    /* Software reset — controller re-initialises all registers */
    uc8151_cmd(drv, 0x12); /* SW_RESET */
    drv->cfg.wait_busy(drv->cfg.user);

    /* Panel setting (PSR): LUT from OTP, KW mode, scan up, shift right, booster on */
    { uint8_t d[] = {0x0F}; uc8151_cmd_data(drv, 0x00, d, sizeof(d)); }

    /* Power setting (PWR): VGH/VGL/VSH/VSL from internal, VSH=+15 V, VSL=-15 V */
    { uint8_t d[] = {0x03, 0x00, 0x2B, 0x2B, 0x03}; uc8151_cmd_data(drv, 0x01, d, sizeof(d)); }

    /* Power on — wait for BUSY low before continuing */
    uc8151_cmd(drv, 0x04); /* PON */
    drv->cfg.wait_busy(drv->cfg.user);

    /* Booster soft-start (BTST): phase A/B/C soft-start current strength */
    { uint8_t d[] = {0x17, 0x17, 0x17}; uc8151_cmd_data(drv, 0x06, d, sizeof(d)); }

    /* PLL control: frame rate ~50 Hz */
    { uint8_t d[] = {0x3C}; uc8151_cmd_data(drv, 0x30, d, sizeof(d)); }

    /* Resolution setting (TRES): panel width and height in pixels */
    {
        uint8_t d[4];
        d[0] = (uint8_t)((w >> 8) & 0xFF);
        d[1] = (uint8_t)(w & 0xFF);
        d[2] = (uint8_t)((h >> 8) & 0xFF);
        d[3] = (uint8_t)(h & 0xFF);
        uc8151_cmd_data(drv, 0x61, d, sizeof(d));
    }

    /* VCM_DC setting */
    { uint8_t d[] = {0x12}; uc8151_cmd_data(drv, 0x82, d, sizeof(d)); }

    /* VCOM and data interval: WBmode VBDF 17, DDX=1, CDI=10 */
    { uint8_t d[] = {0x97}; uc8151_cmd_data(drv, 0x50, d, sizeof(d)); }

    return moui_backend_fb_init(&drv->fb_be, w, h,
                               MOUI_PIXFMT_MONO_HMSB, uc8151_hw_flush, drv);
}

void moui_drv_uc8151_deinit(moui_drv_uc8151_t *drv)
{
    moui_backend_fb_deinit(&drv->fb_be);
}

moui_backend_t *moui_drv_uc8151_backend(moui_drv_uc8151_t *drv)
{
    return &drv->fb_be.base;
}
