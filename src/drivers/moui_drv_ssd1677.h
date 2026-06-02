#ifndef MOUI_DRV_SSD1677_H
#define MOUI_DRV_SSD1677_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../backend/moui_backend_fb.h"

/*
 * SSD1677 E-Paper controller driver
 *
 * Supported panels:
 *   - ED037TC1   3.7"  (280x480)
 *   - EPD0426A02 4.26" (800x480, OTP waveforms)
 *
 * Wire format: HMSB (row-major, MSB-first). 1bpp.
 * Polarity: Moui bit=1 → black; SSD1677 bit=0 → black (driver inverts).
 *
 * Features:
 *   - Automatic dirty-rect partial refresh
 *   - Anti-ghosting partial update (invert-then-restore)
 *   - Configurable software vs IC-internal LUT
 *   - Light sleep (RAM preserved) or deep sleep
 *   - Force-partial mode for fast consecutive updates
 */

/* Update sequence command bits (register 0x22) */
#define SSD1677_SEQ_DISABLE_CLOCK   0x01
#define SSD1677_SEQ_DISABLE_ANALOG  0x02
#define SSD1677_SEQ_DISPLAY         0x04
#define SSD1677_SEQ_MODE2           0x08
#define SSD1677_SEQ_LOAD_LUT        0x10
#define SSD1677_SEQ_LOAD_TEMP       0x20
#define SSD1677_SEQ_ENABLE_ANALOG   0x40
#define SSD1677_SEQ_ENABLE_CLOCK    0x80

typedef struct {
    void (*write_cmd)(uint8_t cmd, void *user);
    void (*write_data)(const uint8_t *data, uint32_t len, void *user);
    void (*write_data_byte)(uint8_t data, void *user);
    void (*wait_busy)(void *user);
    void (*hw_reset)(void *user);
    void *user;
    int width;
    int height;
    uint8_t gate_scan_dir;
    bool use_internal_lut;
    bool light_sleep;
    bool anti_ghosting;
    bool grayscale;
    bool use_otp_voltages;
    uint8_t fast_temp;
    bool mirror_y;
    const uint8_t *gray_lut;
    uint8_t gray_lut_len;
} moui_ssd1677_cfg_t;

/*
 * Panel presets — initialize cfg with one macro, then fill in platform callbacks.
 *
 * Usage:
 *   moui_ssd1677_cfg_t cfg = MOUI_SSD1677_PANEL_EPD0426A02;
 *   cfg.write_cmd = my_write_cmd;
 *   cfg.write_data = my_write_data;
 *   ...
 */
#define MOUI_SSD1677_PANEL_ED037TC1 { \
    .width = 280, .height = 480, \
    .gate_scan_dir = 0x00, \
}

#define MOUI_SSD1677_PANEL_EPD0426A02 { \
    .width = 800, .height = 480, \
    .gate_scan_dir = 0x02, \
    .use_internal_lut = true, \
    .use_otp_voltages = true, \
    .fast_temp = 0x5A, \
    .mirror_y = true, \
}

#define MOUI_SSD1677_PANEL_EPD0426A02_4G { \
    .width = 800, .height = 480, \
    .gate_scan_dir = 0x02, \
    .grayscale = true, \
    .mirror_y = true, \
}

typedef struct {
    moui_backend_fb_t fb_be;
    moui_ssd1677_cfg_t cfg;
    uint8_t *prev_buf;
    uint8_t *tx_buf;
    uint8_t *plane1;
    uint8_t *plane2;
    int phys_w, phys_h;
    bool using_partial_mode;
    bool analog_enabled;
    bool initial_refresh;
    bool force_partial;
    int  partial_count;
} moui_drv_ssd1677_t;

int            moui_drv_ssd1677_init(moui_drv_ssd1677_t *drv, const moui_ssd1677_cfg_t *cfg);
void           moui_drv_ssd1677_deinit(moui_drv_ssd1677_t *drv);
moui_backend_t *moui_drv_ssd1677_backend(moui_drv_ssd1677_t *drv);
void           moui_drv_ssd1677_set_partial(moui_drv_ssd1677_t *drv, bool force);

#ifdef __cplusplus
}
#endif

#endif
