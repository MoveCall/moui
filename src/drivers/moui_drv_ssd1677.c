#include "moui_drv_ssd1677.h"
#include <string.h>
#include <stdlib.h>

#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
// prev_buf is CPU-only (dirty-rect diff) → may live in PSRAM to save internal RAM.
// tx_buf / gray planes feed SPI DMA → must stay in internal DMA-capable RAM
// (DMA straight from PSRAM is unreliable here and shows up as stripes).
#if defined(CONFIG_SPIRAM)
#define DRV_CALLOC(n, sz) heap_caps_calloc(n, sz, MALLOC_CAP_SPIRAM)
#else
#define DRV_CALLOC(n, sz) heap_caps_calloc(n, sz, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL)
#endif
#define DRV_MALLOC(sz)    heap_caps_malloc(sz, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL)
#define DRV_FREE(p)       heap_caps_free(p)
#else
#define DRV_CALLOC(n, sz) calloc(n, sz)
#define DRV_MALLOC(sz)    malloc(sz)
#define DRV_FREE(p)       free(p)
#endif

/*
 * SSD1677 E-Paper controller driver.
 * Supports software LUT (GxEPD2 tables) or IC-internal LUT ROM.
 */

/* ── Software LUT tables (from GxEPD2, for ED037TC1 3.7") ─────────────── */

static const uint8_t lut_full[] = {
    0x2A, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x2A, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x03, 0x0A, 0x00, 0x02, 0x06, 0x0A, 0x05, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x22, 0x22, 0x22, 0x22, 0x22
};

static const uint8_t lut_partial[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0A, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x05, 0x05, 0x00, 0x05, 0x03, 0x05, 0x05, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x22, 0x22, 0x22, 0x22, 0x22
};

/* ── 4-level grayscale LUT (from GxEPD2_4G, for GDEQ0426T82 / SSD1677) ─ */

static const uint8_t lut_4g_default[] = {
    /* VS L0 (red=0, black=0) white */
    0x80, 0x48, 0x4A, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* VS L1 (red=1, black=0) light grey */
    0x0A, 0x48, 0x68, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* VS L2 (red=0, black=1) dark grey */
    0x88, 0x48, 0x60, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* VS L3 (red=1, black=1) black */
    0xA8, 0x48, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* VS L4 (vcom) */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* TP0-TP9 timing/repeat */
    0x07, 0x1E, 0x1C, 0x02, 0x00,
    0x05, 0x01, 0x05, 0x01, 0x02,
    0x08, 0x01, 0x01, 0x04, 0x04,
    0x00, 0x02, 0x01, 0x02, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01,
    /* frame rate */
    0x22, 0x22, 0x22, 0x22, 0x22,
    /* VGH, VSH1, VSH2, VSL, VCOM */
    0x17, 0x41, 0xA8, 0x32, 0x30,
    /* reserved */
    0x00, 0x00,
};

#define LUT_4G_DATA_LEN   105
#define LUT_4G_TOTAL_LEN  112

/* ── Helpers ───────────────────────────────────────────────────────────── */

static inline void cmd(moui_drv_ssd1677_t *drv, uint8_t c)
{
    drv->cfg.write_cmd(c, drv->cfg.user);
}

static inline void data_byte(moui_drv_ssd1677_t *drv, uint8_t d)
{
    drv->cfg.write_data_byte(d, drv->cfg.user);
}

static inline void data(moui_drv_ssd1677_t *drv, const uint8_t *d, uint32_t len)
{
    drv->cfg.write_data(d, len, drv->cfg.user);
}

static inline void ssd1677_wait(moui_drv_ssd1677_t *drv)
{
    drv->cfg.wait_busy(drv->cfg.user);
}

/* ── Update sequence builder ──────────────────────────────────────────── */

static void update_trigger(moui_drv_ssd1677_t *drv, uint8_t seq)
{
    cmd(drv, 0x22);
    data_byte(drv, seq);
    cmd(drv, 0x20);
    ssd1677_wait(drv);
}

#define SEQ_POWER_ON  (SSD1677_SEQ_ENABLE_CLOCK | SSD1677_SEQ_ENABLE_ANALOG)
#define SEQ_POWER_OFF (SSD1677_SEQ_DISABLE_ANALOG | SSD1677_SEQ_DISABLE_CLOCK)
#define SEQ_FULL      (SEQ_POWER_ON | SSD1677_SEQ_LOAD_LUT | SSD1677_SEQ_DISPLAY | SEQ_POWER_OFF)
#define SEQ_PARTIAL   (SEQ_POWER_ON | SSD1677_SEQ_LOAD_LUT | SSD1677_SEQ_DISPLAY | SSD1677_SEQ_MODE2 | SEQ_POWER_OFF)

/* ── RAM area ─────────────────────────────────────────────────────────── */

static void set_ram_area(moui_drv_ssd1677_t *drv,
                         uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    if (drv->cfg.mirror_y) {
        y = (uint16_t)(drv->phys_h - y - h);

        cmd(drv, 0x11);
        data_byte(drv, 0x01);

        cmd(drv, 0x44);
        data_byte(drv, x & 0xFF);
        data_byte(drv, x >> 8);
        data_byte(drv, (x + w - 1) & 0xFF);
        data_byte(drv, (x + w - 1) >> 8);

        cmd(drv, 0x45);
        data_byte(drv, (y + h - 1) & 0xFF);
        data_byte(drv, (y + h - 1) >> 8);
        data_byte(drv, y & 0xFF);
        data_byte(drv, y >> 8);

        cmd(drv, 0x4E);
        data_byte(drv, x & 0xFF);
        data_byte(drv, x >> 8);

        cmd(drv, 0x4F);
        data_byte(drv, (y + h - 1) & 0xFF);
        data_byte(drv, (y + h - 1) >> 8);
    } else {
        cmd(drv, 0x11);
        data_byte(drv, 0x03);

        cmd(drv, 0x44);
        data_byte(drv, x & 0xFF);
        data_byte(drv, x >> 8);
        data_byte(drv, (x + w - 1) & 0xFF);
        data_byte(drv, (x + w - 1) >> 8);

        cmd(drv, 0x45);
        data_byte(drv, y & 0xFF);
        data_byte(drv, y >> 8);
        data_byte(drv, (y + h - 1) & 0xFF);
        data_byte(drv, (y + h - 1) >> 8);

        cmd(drv, 0x4E);
        data_byte(drv, x & 0xFF);
        data_byte(drv, x >> 8);

        cmd(drv, 0x4F);
        data_byte(drv, y & 0xFF);
        data_byte(drv, y >> 8);
    }
}

/* ── LUT management ───────────────────────────────────────────────────── */

static void load_sw_lut_full(moui_drv_ssd1677_t *drv)
{
    cmd(drv, 0x3C);
    data_byte(drv, 0x01);
    cmd(drv, 0x32);
    data(drv, lut_full, sizeof(lut_full));
    drv->using_partial_mode = false;
}

static void load_sw_lut_partial(moui_drv_ssd1677_t *drv)
{
    cmd(drv, 0x3C);
    data_byte(drv, 0xC0);
    cmd(drv, 0x32);
    data(drv, lut_partial, sizeof(lut_partial));
    drv->using_partial_mode = true;
}

/* ── Update functions ─────────────────────────────────────────────────── */

static void ensure_analog(moui_drv_ssd1677_t *drv)
{
    if (!drv->analog_enabled) {
        update_trigger(drv, SEQ_POWER_ON);
        drv->analog_enabled = true;
    }
}

static void update_full(moui_drv_ssd1677_t *drv)
{
    if (drv->cfg.use_internal_lut) {
        cmd(drv, 0x21);
        data_byte(drv, 0x40);
        data_byte(drv, 0x00);
        if (drv->cfg.fast_temp) {
            update_trigger(drv, 0xD7);
        } else {
            ensure_analog(drv);
            update_trigger(drv, SSD1677_SEQ_LOAD_TEMP | SSD1677_SEQ_LOAD_LUT |
                                SSD1677_SEQ_DISPLAY | SEQ_POWER_OFF);
        }
    } else {
        if (drv->using_partial_mode) load_sw_lut_full(drv);
        update_trigger(drv, SEQ_FULL);
    }
    drv->analog_enabled = false;
    drv->using_partial_mode = false;
}

static void update_partial(moui_drv_ssd1677_t *drv)
{
    if (drv->cfg.use_internal_lut) {
        cmd(drv, 0x21);
        data_byte(drv, 0x00);
        data_byte(drv, 0x00);
        if (drv->cfg.fast_temp) {
            update_trigger(drv, 0xDF);
        } else {
            ensure_analog(drv);
            update_trigger(drv, SSD1677_SEQ_LOAD_LUT | SSD1677_SEQ_DISPLAY |
                                SSD1677_SEQ_MODE2);
            drv->analog_enabled = true;
        }
    } else {
        if (!drv->using_partial_mode) load_sw_lut_partial(drv);
        ensure_analog(drv);
        update_trigger(drv, SSD1677_SEQ_LOAD_LUT | SSD1677_SEQ_DISPLAY |
                            SSD1677_SEQ_MODE2);
        drv->analog_enabled = true;
    }
    drv->using_partial_mode = true;
}

/* ── Buffer inversion (Moui 1=black → SSD1677 0=black) ────────────────── */

static void invert_to_tx(moui_drv_ssd1677_t *drv, const uint8_t *src, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
        drv->tx_buf[i] = ~src[i];
}

/* ── 2bpp → dual 1bpp plane split (for grayscale mode) ────────────────── */

static void split_gray_planes(const uint8_t *gray2bpp, uint8_t *plane1, uint8_t *plane2,
                               int w, int h)
{
    uint32_t mono_size = ((uint32_t)w * h + 7) / 8;
    memset(plane1, 0x00, mono_size);
    memset(plane2, 0x00, mono_size);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int gray_pos = y * w + x;
            int gray_idx = gray_pos >> 2;
            int gray_shift = 6 - (gray_pos & 3) * 2;
            uint8_t val = (gray2bpp[gray_idx] >> gray_shift) & 0x03;

            int mono_idx = gray_pos >> 3;
            uint8_t mono_mask = 0x80 >> (gray_pos & 7);

            /* 4G LUT mapping:
             *   L0: (0x24=0, 0x26=0) → white
             *   L1: (0x24=0, 0x26=1) → light grey
             *   L2: (0x24=1, 0x26=0) → dark grey
             *   L3: (0x24=1, 0x26=1) → black
             * plane1 → 0x24 = bit1 of val, plane2 → 0x26 = bit0 of val */
            if (val & 0x02) plane1[mono_idx] |= mono_mask;
            if (val & 0x01) plane2[mono_idx] |= mono_mask;
        }
    }
}

/* ── Dirty-rect diff ──────────────────────────────────────────────────── */

typedef struct { int x, y, w, h; } dirty_rect_t;

static dirty_rect_t calc_dirty_rect(const uint8_t *cur, const uint8_t *prev,
                                    int width, int height)
{
    int stride = width / 8;
    int min_y = height, max_y = -1;
    int min_xb = stride, max_xb = -1;

    for (int y = 0; y < height; y++) {
        const uint8_t *row_c = cur  + y * stride;
        const uint8_t *row_p = prev + y * stride;
        for (int xb = 0; xb < stride; xb++) {
            if (row_c[xb] != row_p[xb]) {
                if (y < min_y)  min_y  = y;
                if (y > max_y)  max_y  = y;
                if (xb < min_xb) min_xb = xb;
                if (xb > max_xb) max_xb = xb;
            }
        }
    }

    if (max_y < 0) return (dirty_rect_t){0, 0, 0, 0};

    return (dirty_rect_t){
        .x = min_xb * 8,
        .y = min_y,
        .w = (max_xb - min_xb + 1) * 8,
        .h = max_y - min_y + 1,
    };
}

/* ── hw_flush — called by moui_backend_fb_t ───────────────────────────── */

static void ssd1677_hw_flush(const uint8_t *pixels, uint32_t len, void *user)
{
    moui_drv_ssd1677_t *drv = (moui_drv_ssd1677_t *)user;
    const int pw = drv->phys_w;
    const int ph = drv->phys_h;
    uint32_t mono_len = (uint32_t)(pw * ph + 7) / 8;

    if (drv->cfg.grayscale && drv->plane1 && drv->plane2) {
        if (!drv->initial_refresh && drv->prev_buf &&
            memcmp(pixels, drv->prev_buf, len) == 0)
            return;

        split_gray_planes(pixels, drv->plane1, drv->plane2, pw, ph);

        set_ram_area(drv, 0, 0, pw, ph);
        cmd(drv, 0x24);
        data(drv, drv->plane1, mono_len);

        set_ram_area(drv, 0, 0, pw, ph);
        cmd(drv, 0x26);
        data(drv, drv->plane2, mono_len);

        cmd(drv, 0x21);
        data_byte(drv, 0x00);
        data_byte(drv, 0x00);

        update_trigger(drv, SEQ_POWER_ON | SSD1677_SEQ_DISPLAY | SEQ_POWER_OFF);
        drv->analog_enabled = false;
        drv->initial_refresh = false;
        memcpy(drv->prev_buf, pixels, len);
        return;
    }

    uint32_t phys_len = mono_len;

    invert_to_tx(drv, pixels, len);

    const uint8_t *send_buf = drv->tx_buf;
    uint32_t send_len = phys_len;

    if (drv->initial_refresh || !drv->prev_buf) {
        set_ram_area(drv, 0, 0, pw, ph);
        cmd(drv, 0x26);
        data(drv, send_buf, send_len);
        set_ram_area(drv, 0, 0, pw, ph);
        cmd(drv, 0x24);
        data(drv, send_buf, send_len);
        update_full(drv);
        drv->initial_refresh = false;
        if (drv->prev_buf)
            memcpy(drv->prev_buf, pixels, len);
        return;
    }

    dirty_rect_t dr = calc_dirty_rect(pixels, drv->prev_buf, pw, ph);
    if (dr.w == 0 && dr.h == 0) return;

    /* One-shot full-screen dirty rect: force partial waveform over the
     * whole panel. Useful for testing/handling local dirty rects that
     * affect other regions on rotated screens. */
    if (drv->force_partial && drv->force_full_partial_once) {
        dr = (dirty_rect_t){0, 0, pw, ph};
        drv->force_full_partial_once = false;
    }

    int total_pixels = pw * ph;
    int dirty_pixels = dr.w * dr.h;
    bool use_full = (dirty_pixels * 2 > total_pixels) && !drv->force_partial;

    if (drv->force_partial && drv->partial_count >= 20) {
        use_full = true;
        drv->partial_count = 0;
    }

    if (use_full) {
        set_ram_area(drv, 0, 0, pw, ph);
        cmd(drv, 0x24);
        data(drv, send_buf, send_len);
        update_partial(drv);

        set_ram_area(drv, 0, 0, pw, ph);
        cmd(drv, 0x26);
        data(drv, send_buf, send_len);
        set_ram_area(drv, 0, 0, pw, ph);
        cmd(drv, 0x24);
        data(drv, send_buf, send_len);
        update_full(drv);
        drv->partial_count = 0;
    } else {
        if (drv->cfg.anti_ghosting) {
            /* Anti-ghosting: invert-then-restore to reduce residual image */
            int stride_full = pw / 8;
            int stride_dr = dr.w / 8;

            set_ram_area(drv, dr.x, dr.y, dr.w, dr.h);
            cmd(drv, 0x24);
            for (int y = 0; y < dr.h; y++) {
                int src_off = (dr.y + y) * stride_full + dr.x / 8;
                for (int b = 0; b < stride_dr; b++)
                    data_byte(drv, ~send_buf[src_off + b]);
            }
            update_partial(drv);
        }

        int stride_full = pw / 8;
        int stride_dr = dr.w / 8;

        set_ram_area(drv, dr.x, dr.y, dr.w, dr.h);
        cmd(drv, 0x24);
        for (int y = 0; y < dr.h; y++) {
            int src_off = (dr.y + y) * stride_full + dr.x / 8;
            data(drv, send_buf + src_off, stride_dr);
        }
        update_partial(drv);

        set_ram_area(drv, dr.x, dr.y, dr.w, dr.h);
        cmd(drv, 0x26);
        for (int y = 0; y < dr.h; y++) {
            int src_off = (dr.y + y) * stride_full + dr.x / 8;
            data(drv, send_buf + src_off, stride_dr);
        }

        drv->partial_count++;
    }

    memcpy(drv->prev_buf, pixels, len);
}

/* ── Sleep / Wakeup ───────────────────────────────────────────────────── */

static void ssd1677_sleep(moui_backend_t *be)
{
    moui_drv_ssd1677_t *drv = (moui_drv_ssd1677_t *)be;

    if (drv->analog_enabled) {
        update_trigger(drv, SEQ_POWER_OFF);
        drv->analog_enabled = false;
    }

    cmd(drv, 0x10);
    data_byte(drv, drv->cfg.light_sleep ? 0x01 : 0x03);
}

static void ssd1677_wakeup(moui_backend_t *be);

/* ── Init sequence ────────────────────────────────────────────────────── */

static void ssd1677_init_display(moui_drv_ssd1677_t *drv)
{
    const int w = drv->phys_w;
    const int h = drv->phys_h;

    drv->cfg.hw_reset(drv->cfg.user);

    cmd(drv, 0x12);
    ssd1677_wait(drv);

    cmd(drv, 0x01);
    data_byte(drv, (h - 1) & 0xFF);
    data_byte(drv, ((h - 1) >> 8) & 0xFF);
    data_byte(drv, drv->cfg.gate_scan_dir);

    if (drv->cfg.grayscale) {
        cmd(drv, 0x0C);
        data_byte(drv, 0xAE);
        data_byte(drv, 0xC7);
        data_byte(drv, 0xC3);
        data_byte(drv, 0xC0);
        data_byte(drv, 0x80);

        cmd(drv, 0x3C);
        data_byte(drv, 0x00);

        cmd(drv, 0x18);
        data_byte(drv, 0x80);

        cmd(drv, 0x11);
        data_byte(drv, 0x03);

        set_ram_area(drv, 0, 0, w, h);

        const uint8_t *lut = drv->cfg.gray_lut ? drv->cfg.gray_lut : lut_4g_default;
        /* Custom LUT must provide the 105 data bytes plus VCOM/DV/BS bytes
         * (indices 105-109). Fall back to the built-in table if too short. */
        if (drv->cfg.gray_lut && drv->cfg.gray_lut_len < LUT_4G_TOTAL_LEN)
            lut = lut_4g_default;

        cmd(drv, 0x32);
        data(drv, lut, LUT_4G_DATA_LEN);

        cmd(drv, 0x03);
        data_byte(drv, lut[105]);

        cmd(drv, 0x04);
        data_byte(drv, lut[106]);
        data_byte(drv, lut[107]);
        data_byte(drv, lut[108]);

        cmd(drv, 0x2C);
        data_byte(drv, lut[109]);
    } else {
        if (!drv->cfg.use_otp_voltages) {
            cmd(drv, 0x03);
            data_byte(drv, 0x00);

            cmd(drv, 0x04);
            data_byte(drv, 0x41);
            data_byte(drv, 0xA8);
            data_byte(drv, 0x32);
        }

        cmd(drv, 0x11);
        data_byte(drv, 0x03);

        cmd(drv, 0x0C);
        data_byte(drv, 0xAE);
        data_byte(drv, 0xC7);
        data_byte(drv, 0xC3);
        data_byte(drv, drv->cfg.use_otp_voltages ? 0x80 : 0xC0);
        data_byte(drv, 0xC0);

        cmd(drv, 0x18);
        data_byte(drv, 0x80);

        if (!drv->cfg.use_otp_voltages) {
            cmd(drv, 0x2C);
            data_byte(drv, 0x44);

            cmd(drv, 0x37);
            data_byte(drv, 0x00);
            data_byte(drv, 0xFF);
            data_byte(drv, 0xFF);
            data_byte(drv, 0xFF);
            data_byte(drv, 0xFF);
            data_byte(drv, 0x4F);
            data_byte(drv, 0xFF);
            data_byte(drv, 0xFF);
            data_byte(drv, 0xFF);
            data_byte(drv, 0xFF);
        }

        cmd(drv, 0x3C);
        data_byte(drv, 0x01);

        if (drv->cfg.fast_temp) {
            cmd(drv, 0x1A);
            data_byte(drv, drv->cfg.fast_temp);
        }

        set_ram_area(drv, 0, 0, w, h);

        if (!drv->cfg.use_internal_lut)
            load_sw_lut_full(drv);
    }

    drv->analog_enabled = false;
}

static void ssd1677_wakeup(moui_backend_t *be)
{
    moui_drv_ssd1677_t *drv = (moui_drv_ssd1677_t *)be;

    if (drv->cfg.light_sleep) {
        ssd1677_init_display(drv);
        if (drv->prev_buf) {
            uint32_t buf_size = (uint32_t)(drv->phys_w * drv->phys_h + 7) / 8;
            invert_to_tx(drv, drv->prev_buf, buf_size);
            set_ram_area(drv, 0, 0, drv->phys_w, drv->phys_h);
            cmd(drv, 0x26);
            data(drv, drv->tx_buf, buf_size);
        }
    } else {
        ssd1677_init_display(drv);
    }
    drv->initial_refresh = true;
}

/* ── Public API ───────────────────────────────────────────────────────── */

int moui_drv_ssd1677_init(moui_drv_ssd1677_t *drv, const moui_ssd1677_cfg_t *cfg)
{
    memset(drv, 0, sizeof(*drv));
    drv->cfg = *cfg;
    if (drv->cfg.width  == 0) drv->cfg.width  = 280;
    if (drv->cfg.height == 0) drv->cfg.height = 480;

    drv->phys_w = drv->cfg.width;
    drv->phys_h = drv->cfg.height;

    uint32_t buf_size = (uint32_t)(drv->phys_w * drv->phys_h + 7) / 8;
    uint32_t fb_size = drv->cfg.grayscale
        ? ((uint32_t)drv->phys_w * drv->phys_h + 3) / 4
        : buf_size;

    drv->prev_buf = (uint8_t *)DRV_CALLOC(1, fb_size);
    if (!drv->prev_buf) return -1;

    if (drv->cfg.grayscale) {
        drv->tx_buf = NULL;
        drv->plane1 = (uint8_t *)DRV_MALLOC(buf_size);
        drv->plane2 = (uint8_t *)DRV_MALLOC(buf_size);
        if (!drv->plane1 || !drv->plane2) {
            DRV_FREE(drv->prev_buf);
            DRV_FREE(drv->plane1);
            DRV_FREE(drv->plane2);
            return -1;
        }
    } else {
        drv->tx_buf = (uint8_t *)DRV_MALLOC(buf_size);
        if (!drv->tx_buf) {
            DRV_FREE(drv->prev_buf);
            return -1;
        }
        drv->plane1 = NULL;
        drv->plane2 = NULL;
    }

    drv->initial_refresh = true;
    drv->using_partial_mode = false;
    drv->analog_enabled = false;

    ssd1677_init_display(drv);

    moui_pixfmt_t fmt = drv->cfg.grayscale ? MOUI_PIXFMT_GRAY2_HMSB : MOUI_PIXFMT_MONO_HMSB;
    int ret = moui_backend_fb_init(&drv->fb_be, drv->phys_w, drv->phys_h,
                                  fmt, ssd1677_hw_flush, drv);
    if (ret != 0) {
        DRV_FREE(drv->prev_buf);
        DRV_FREE(drv->tx_buf);
        DRV_FREE(drv->plane1);
        DRV_FREE(drv->plane2);
        return ret;
    }

    drv->fb_be.base.sleep  = ssd1677_sleep;
    drv->fb_be.base.wakeup = ssd1677_wakeup;

    return 0;
}

void moui_drv_ssd1677_deinit(moui_drv_ssd1677_t *drv)
{
    moui_backend_fb_deinit(&drv->fb_be);
    DRV_FREE(drv->prev_buf);
    DRV_FREE(drv->tx_buf);
    DRV_FREE(drv->plane1);
    DRV_FREE(drv->plane2);
    drv->prev_buf = NULL;
    drv->tx_buf = NULL;
    drv->plane1 = NULL;
    drv->plane2 = NULL;
}

moui_backend_t *moui_drv_ssd1677_backend(moui_drv_ssd1677_t *drv)
{
    return &drv->fb_be.base;
}

void moui_drv_ssd1677_set_partial(moui_drv_ssd1677_t *drv, bool force)
{
    drv->force_partial = force;
    drv->partial_count = 0;
}

void moui_drv_ssd1677_force_full_partial_once(moui_drv_ssd1677_t *drv)
{
    drv->force_full_partial_once = true;
}
