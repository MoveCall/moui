#ifndef MOUI_FB_H
#define MOUI_FB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../hal/moui_hal_types.h"
#include "moui_color.h"

typedef void (*moui_fb_pack_fn_t)(uint8_t *wire, const uint8_t *pixels, uint16_t w, uint16_t h);

typedef struct {
    uint8_t        *pixels;
    uint8_t        *wire;
    uint16_t        width;
    uint16_t        height;
    uint32_t        buf_size;
    uint32_t        wire_size;
    moui_fb_pack_fn_t pack_fn;
} moui_fb_t;

int  moui_fb_init(moui_fb_t *fb, const moui_disp_desc_t *desc);
void moui_fb_deinit(moui_fb_t *fb);
void moui_fb_clear(moui_fb_t *fb, moui_color_t color);

static inline void moui_fb_set_pixel(moui_fb_t *fb, int x, int y, moui_color_t color)
{
    uint16_t w = fb->width;
    uint16_t h = fb->height;
    if ((unsigned)x >= w || (unsigned)y >= h) return;
    int pos = y * w + x;
    if (moui_disp->pixel_format == MOUI_PIXEL_FORMAT_GRAY2_HMSB) {
        int idx = pos >> 2;
        int shift = 6 - (pos & 3) * 2;
        fb->pixels[idx] = (fb->pixels[idx] & ~(0x03 << shift)) | ((color & 0x03) << shift);
    } else {
        int byte_idx = pos >> 3;
        uint8_t bit_mask = 0x80 >> (pos & 7);
        if (color >= MOUI_DGRAY)
            fb->pixels[byte_idx] |= bit_mask;
        else
            fb->pixels[byte_idx] &= ~bit_mask;
    }
}

static inline moui_color_t moui_fb_get_pixel(const moui_fb_t *fb, int x, int y)
{
    uint16_t w = fb->width;
    uint16_t h = fb->height;
    if ((unsigned)x >= w || (unsigned)y >= h) return MOUI_WHITE;
    int pos = y * w + x;
    if (moui_disp->pixel_format == MOUI_PIXEL_FORMAT_GRAY2_HMSB) {
        int idx = pos >> 2;
        int shift = 6 - (pos & 3) * 2;
        return (fb->pixels[idx] >> shift) & 0x03;
    } else {
        int byte_idx = pos >> 3;
        uint8_t bit_mask = 0x80 >> (pos & 7);
        return (fb->pixels[byte_idx] & bit_mask) ? MOUI_BLACK : MOUI_WHITE;
    }
}

void moui_fb_pack_wire(moui_fb_t *fb);


#ifdef __cplusplus
}
#endif
#endif
