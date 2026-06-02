#include "moui_backend_fb.h"
#include "../core/moui_pack.h"
#include <string.h>
#include <stdlib.h>

#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
// Render/rotate/pack buffers are CPU-side. On small-RAM SoCs with PSRAM, keep
// them in PSRAM; this works for display drivers that re-feed data through their
// own internal DMA buffer (e.g. moui_drv_ssd1677's tx_buf). Backends that DMA
// these buffers directly should not take this path.
#if defined(CONFIG_SPIRAM)
#define DMA_CALLOC(n, sz) heap_caps_calloc(n, sz, MALLOC_CAP_SPIRAM)
#define DMA_MALLOC(sz)    heap_caps_malloc(sz, MALLOC_CAP_SPIRAM)
#else
#define DMA_CALLOC(n, sz) heap_caps_calloc(n, sz, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL)
#define DMA_MALLOC(sz)    heap_caps_malloc(sz, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL)
#endif
#define DMA_FREE(p)       heap_caps_free(p)
#else
#define DMA_CALLOC(n, sz) calloc(n, sz)
#define DMA_MALLOC(sz)    malloc(sz)
#define DMA_FREE(p)       free(p)
#endif

/* ── Pixel access ── */

static inline bool is_gray_fmt(const moui_backend_fb_t *fb)
{
    return fb->pixfmt == MOUI_PIXFMT_GRAY2_HMSB;
}

static void fb_set_pixel(moui_backend_t *be, int x, int y, moui_color_t c)
{
    moui_backend_fb_t *fb = (moui_backend_fb_t *)be;
    int w = be->sw_rotate ? be->phys_w : be->width;
    int pos = y * w + x;

    if (is_gray_fmt(fb)) {
        int idx = pos >> 2;
        int shift = 6 - (pos & 3) * 2;
        fb->pixels[idx] = (fb->pixels[idx] & ~(0x03 << shift)) | ((c & 0x03) << shift);
    } else {
        int idx = pos >> 3;
        uint8_t mask = 0x80 >> (pos & 7);
        if (c >= MOUI_DGRAY) fb->pixels[idx] |= mask;
        else                 fb->pixels[idx] &= ~mask;
    }
}

static moui_color_t fb_get_pixel(moui_backend_t *be, int x, int y)
{
    moui_backend_fb_t *fb = (moui_backend_fb_t *)be;
    int w = be->sw_rotate ? be->phys_w : be->width;
    int pos = y * w + x;

    if (is_gray_fmt(fb)) {
        int idx = pos >> 2;
        int shift = 6 - (pos & 3) * 2;
        return (fb->pixels[idx] >> shift) & 0x03;
    } else {
        int idx = pos >> 3;
        uint8_t mask = 0x80 >> (pos & 7);
        return (fb->pixels[idx] & mask) ? MOUI_BLACK : MOUI_WHITE;
    }
}

static void fb_clear(moui_backend_t *be, moui_color_t c)
{
    moui_backend_fb_t *fb = (moui_backend_fb_t *)be;
    uint8_t fill;
    if (is_gray_fmt(fb)) {
        uint8_t v = c & 0x03;
        fill = (v << 6) | (v << 4) | (v << 2) | v;
    } else {
        fill = (c >= MOUI_DGRAY) ? 0xFF : 0x00;
    }
    memset(fb->pixels, fill, fb->pix_size);
}

/* ── Rotation helpers ── */

static inline moui_color_t buf_get_pixel(const uint8_t *buf, int w, int x, int y, bool gray)
{
    int pos = y * w + x;
    if (gray) {
        int idx = pos >> 2;
        int shift = 6 - (pos & 3) * 2;
        return (buf[idx] >> shift) & 0x03;
    } else {
        int idx = pos >> 3;
        int bit = 7 - (pos & 7);
        return (buf[idx] >> bit) & 1 ? MOUI_BLACK : MOUI_WHITE;
    }
}

static inline void buf_set_pixel(uint8_t *buf, int w, int x, int y, moui_color_t v, bool gray)
{
    int pos = y * w + x;
    if (gray) {
        int idx = pos >> 2;
        int shift = 6 - (pos & 3) * 2;
        buf[idx] = (buf[idx] & ~(0x03 << shift)) | ((v & 0x03) << shift);
    } else {
        int idx = pos >> 3;
        int bit = 7 - (pos & 7);
        if (v >= MOUI_DGRAY) buf[idx] |= (1 << bit);
        else                 buf[idx] &= ~(1 << bit);
    }
}

static inline uint8_t bit_reverse_u8(uint8_t b)
{
    b = (uint8_t)(((b & 0xF0) >> 4) | ((b & 0x0F) << 4));
    b = (uint8_t)(((b & 0xCC) >> 2) | ((b & 0x33) << 2));
    b = (uint8_t)(((b & 0xAA) >> 1) | ((b & 0x55) << 1));
    return b;
}

static void rotate_buffer(const uint8_t *src, uint8_t *dst,
                           int src_w, int src_h, moui_rotation_t rot, bool gray)
{
    int dst_w, dst_h;
    if (rot == MOUI_ROTATION_90 || rot == MOUI_ROTATION_270) {
        dst_w = src_h; dst_h = src_w;
    } else {
        dst_w = src_w; dst_h = src_h;
    }

    uint32_t dst_size = gray ? ((uint32_t)dst_w * dst_h + 3) / 4
                             : ((uint32_t)dst_w * dst_h + 7) / 8;

    /* Fast path for 180 degree 1bpp rotation: byte-wise bit-reversal */
    if (rot == MOUI_ROTATION_180 && !gray && (src_w & 7) == 0) {
        for (uint32_t i = 0; i < dst_size; i++) {
            dst[dst_size - 1 - i] = bit_reverse_u8(src[i]);
        }
        return;
    }

    memset(dst, 0, dst_size);

    switch (rot) {
    case MOUI_ROTATION_90:
        for (int sy = 0; sy < src_h; sy++) {
            int dx = src_h - 1 - sy;
            for (int sx = 0; sx < src_w; sx++) {
                moui_color_t v = buf_get_pixel(src, src_w, sx, sy, gray);
                if (v != MOUI_WHITE) buf_set_pixel(dst, dst_w, dx, sx, v, gray);
            }
        }
        break;
    case MOUI_ROTATION_180:
        for (int sy = 0; sy < src_h; sy++) {
            int dy = src_h - 1 - sy;
            for (int sx = 0; sx < src_w; sx++) {
                moui_color_t v = buf_get_pixel(src, src_w, sx, sy, gray);
                if (v != MOUI_WHITE) buf_set_pixel(dst, dst_w, src_w - 1 - sx, dy, v, gray);
            }
        }
        break;
    case MOUI_ROTATION_270:
        for (int sy = 0; sy < src_h; sy++) {
            for (int sx = 0; sx < src_w; sx++) {
                moui_color_t v = buf_get_pixel(src, src_w, sx, sy, gray);
                if (v != MOUI_WHITE) buf_set_pixel(dst, dst_w, sy, src_w - 1 - sx, v, gray);
            }
        }
        break;
    default:
        memcpy(dst, src, dst_size);
        break;
    }
}

/* ── Wire packing ── */

static void fb_flush(moui_backend_t *be)
{
    moui_backend_fb_t *fb = (moui_backend_fb_t *)be;
    const uint8_t *src = fb->pixels;
    int out_w = be->phys_w;
    int out_h = be->phys_h;

    if (!be->sw_rotate && be->rotation != MOUI_ROTATION_0 && fb->rot_buf) {
        rotate_buffer(fb->pixels, fb->rot_buf, be->width, be->height, be->rotation, is_gray_fmt(fb));
        src = fb->rot_buf;
    }

    const uint8_t *out;
    uint32_t len;

    switch (fb->pixfmt) {
    case MOUI_PIXFMT_ST7305_4x2:
        moui_pack_st7305_4x2(fb->wire, src, out_w, out_h);
        out = fb->wire;
        len = fb->wire_size;
        break;
    case MOUI_PIXFMT_MONO_VMSB:
        moui_pack_mono_vmsb(fb->wire, src, out_w, out_h);
        out = fb->wire;
        len = fb->wire_size;
        break;
    case MOUI_PIXFMT_GRAY2_HMSB:
    case MOUI_PIXFMT_MONO_HMSB:
    default:
        out = src;
        len = fb->pix_size;
        break;
    }

    if (fb->hw_flush)
        fb->hw_flush(out, len, fb->hw_user);
}

/* ── Wire size calculation ── */

static uint32_t calc_wire_size(int w, int h, moui_pixfmt_t fmt)
{
    switch (fmt) {
    case MOUI_PIXFMT_ST7305_4x2: return (uint32_t)(w / 4) * (h / 2);
    case MOUI_PIXFMT_MONO_VMSB:  return (uint32_t)w * ((h + 7) / 8);
    case MOUI_PIXFMT_GRAY2_HMSB:
    case MOUI_PIXFMT_MONO_HMSB:
    default:                     return ((uint32_t)w * h + 7) / 8;
    }
}

/* ── Public API ── */

int moui_backend_fb_init(moui_backend_fb_t *fb, int w, int h, moui_pixfmt_t fmt,
                        moui_hw_flush_fn hw_flush, void *hw_user)
{
    memset(fb, 0, sizeof(*fb));
    fb->base.width     = w;
    fb->base.height    = h;
    fb->base.phys_w    = w;
    fb->base.phys_h    = h;
    fb->base.rotation  = MOUI_ROTATION_0;
    fb->base.sw_rotate = false;
    fb->base.set_pixel = fb_set_pixel;
    fb->base.get_pixel = fb_get_pixel;
    fb->base.clear     = fb_clear;
    fb->base.flush     = fb_flush;

    fb->pixfmt    = fmt;
    fb->pix_size = (fmt == MOUI_PIXFMT_GRAY2_HMSB) ? ((uint32_t)w * h + 3) / 4 : ((uint32_t)w * h + 7) / 8;
    fb->wire_size = calc_wire_size(w, h, fmt);

    fb->pixels = (uint8_t *)DMA_CALLOC(1, fb->pix_size);
    if (!fb->pixels) return -1;

    if (fmt != MOUI_PIXFMT_MONO_HMSB && fmt != MOUI_PIXFMT_GRAY2_HMSB) {
        fb->wire = (uint8_t *)DMA_CALLOC(1, fb->wire_size);
        if (!fb->wire) { DMA_FREE(fb->pixels); return -1; }
    }

    fb->hw_flush = hw_flush;
    fb->hw_user  = hw_user;
    return 0;
}

void moui_backend_fb_deinit(moui_backend_fb_t *fb)
{
    DMA_FREE(fb->pixels);
    DMA_FREE(fb->wire);
    DMA_FREE(fb->rot_buf);
    fb->pixels = NULL;
    fb->wire = NULL;
    fb->rot_buf = NULL;
}

void moui_backend_fb_set_rotation(moui_backend_fb_t *fb_arg, moui_rotation_t rot)
{
    moui_backend_fb_t *fb = fb_arg;
    moui_backend_t *be = &fb->base;

    if (rot == be->rotation) return;

    if (rot == MOUI_ROTATION_90 || rot == MOUI_ROTATION_270) {
        be->width  = be->phys_h;
        be->height = be->phys_w;
    } else {
        be->width  = be->phys_w;
        be->height = be->phys_h;
    }

    be->rotation = rot;

    if (be->sw_rotate) {
        fb->pix_size = is_gray_fmt(fb) ? ((uint32_t)be->phys_w * be->phys_h + 3) / 4 : ((uint32_t)be->phys_w * be->phys_h + 7) / 8;
    } else {
        fb->pix_size = is_gray_fmt(fb) ? ((uint32_t)be->width * be->height + 3) / 4
                                       : ((uint32_t)be->width * be->height + 7) / 8;
        if (rot != MOUI_ROTATION_0 && !fb->rot_buf) {
            uint32_t phys_size = is_gray_fmt(fb) ? ((uint32_t)be->phys_w * be->phys_h + 3) / 4
                                                 : ((uint32_t)be->phys_w * be->phys_h + 7) / 8;
            fb->rot_buf = (uint8_t *)DMA_CALLOC(1, phys_size);
        }
    }

    memset(fb->pixels, 0, fb->pix_size);
}

const uint8_t *moui_backend_fb_pixels(const moui_backend_fb_t *fb)
{
    return fb->pixels;
}

uint32_t moui_backend_fb_wire_size(const moui_backend_fb_t *fb)
{
    return fb->wire_size;
}
