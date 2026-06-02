#ifndef MOUI_BACKEND_FB_H
#define MOUI_BACKEND_FB_H

#include "moui_backend.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MOUI_PIXFMT_MONO_HMSB,
    MOUI_PIXFMT_MONO_VMSB,
    MOUI_PIXFMT_ST7305_4x2,
    MOUI_PIXFMT_GRAY2_HMSB,
} moui_pixfmt_t;

typedef void (*moui_hw_flush_fn)(const uint8_t *data, uint32_t len, void *user);

typedef struct {
    moui_backend_t  base;
    uint8_t       *pixels;
    uint8_t       *wire;
    uint8_t       *rot_buf;
    uint32_t       pix_size;
    uint32_t       wire_size;
    moui_pixfmt_t   pixfmt;
    moui_hw_flush_fn hw_flush;
    void           *hw_user;
} moui_backend_fb_t;

int  moui_backend_fb_init(moui_backend_fb_t *fb, int w, int h, moui_pixfmt_t fmt,
                         moui_hw_flush_fn hw_flush, void *hw_user);
void moui_backend_fb_deinit(moui_backend_fb_t *fb);
void moui_backend_fb_set_rotation(moui_backend_fb_t *fb, moui_rotation_t rot);

const uint8_t *moui_backend_fb_pixels(const moui_backend_fb_t *fb);
uint32_t       moui_backend_fb_wire_size(const moui_backend_fb_t *fb);

#ifdef __cplusplus
}
#endif

#endif
