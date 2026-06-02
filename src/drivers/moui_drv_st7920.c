#include "moui_drv_st7920.h"
#include <string.h>

/*
 * ST7920 init sequence (SPI serial mode, extended instruction set).
 *
 * The controller must be in serial SPI mode (PSB pin low).
 * Sequence: function set basic → function set extended → graphic display ON.
 */
static const uint8_t st7920_init_seq[] = {
    0x30,  /* function set: basic instruction set, 8-bit bus */
    0x34,  /* function set: extended instruction set, graphic mode off */
    0x36,  /* function set: extended instruction set, graphic display ON */
};

/*
 * ST7920 graphic RAM layout (128×64):
 *   - 32 rows in upper half, 32 rows in lower half.
 *   - Each row is 16 bytes (128 pixels, MSB-first, horizontal).
 *   - Upper half: GDRAM Y = 0x80+row, X = 0x80 (word address 0).
 *   - Lower half: GDRAM Y = 0x80+row, X = 0x88 (word address 8, maps to col 128+).
 *
 * Wire buffer is HMSB: row-major, each row = width/8 bytes.
 * bytes_per_row = 128/8 = 16 for a standard 128-wide panel.
 */
static void st7920_hw_flush(const uint8_t *data, uint32_t len, void *user)
{
    moui_drv_st7920_t *drv = (moui_drv_st7920_t *)user;
    const int bytes_per_row = drv->cfg.width / 8;
    const int rows = drv->cfg.height < 64 ? drv->cfg.height : 64;
    const int upper = rows < 32 ? rows : 32;
    (void)len;

    /* Upper half: rows 0–31 (or fewer if panel height < 32) */
    for (int row = 0; row < upper; row++) {
        drv->cfg.write_cmd((uint8_t)(0x80 | row), drv->cfg.user);  /* Y address */
        drv->cfg.write_cmd(0x80, drv->cfg.user);                    /* X address: upper half */
        drv->cfg.write_data(data + row * bytes_per_row, (uint32_t)bytes_per_row, drv->cfg.user);
    }

    /* Lower half: rows 32–63 → sent as rows 0–31 at X=0x88 */
    if (rows > 32) {
        for (int row = 0; row < rows - 32; row++) {
            drv->cfg.write_cmd((uint8_t)(0x80 | row), drv->cfg.user);  /* Y address */
            drv->cfg.write_cmd(0x88, drv->cfg.user);                    /* X address: lower half */
            drv->cfg.write_data(data + (row + 32) * bytes_per_row, (uint32_t)bytes_per_row, drv->cfg.user);
        }
    }
}

int moui_drv_st7920_init(moui_drv_st7920_t *drv, const moui_st7920_cfg_t *cfg)
{
    memset(drv, 0, sizeof(*drv));
    drv->cfg = *cfg;
    if (drv->cfg.width == 0)  drv->cfg.width  = 128;
    if (drv->cfg.height == 0) drv->cfg.height = 64;
    if (drv->cfg.width % 8 != 0) return -1;
    if (drv->cfg.height > 64) drv->cfg.height = 64;

    for (int i = 0; i < (int)sizeof(st7920_init_seq); i++)
        cfg->write_cmd(st7920_init_seq[i], cfg->user);

    return moui_backend_fb_init(&drv->fb_be, drv->cfg.width, drv->cfg.height,
                               MOUI_PIXFMT_MONO_HMSB, st7920_hw_flush, drv);
}

void moui_drv_st7920_deinit(moui_drv_st7920_t *drv)
{
    moui_backend_fb_deinit(&drv->fb_be);
}

moui_backend_t *moui_drv_st7920_backend(moui_drv_st7920_t *drv)
{
    return &drv->fb_be.base;
}
