#include "moui_fb.h"
#include "moui_pack.h"
#include <string.h>
#include <stdlib.h>

static uint32_t calc_wire_size(const moui_disp_desc_t *desc)
{
    switch (desc->pixel_format) {
    case MOUI_PIXEL_FORMAT_ST7305_4x2:
        return (desc->width / 4) * (desc->height / 2);
    case MOUI_PIXEL_FORMAT_MONO_VMSB:
        return desc->width * ((desc->height + 7) / 8);
    case MOUI_PIXEL_FORMAT_GRAY2_HMSB:
        return ((uint32_t)desc->width * desc->height + 3) / 4;
    case MOUI_PIXEL_FORMAT_MONO_HMSB:
    default:
        return ((uint32_t)desc->width * desc->height + 7) / 8;
    }
}

static void pack_fn_st7305(uint8_t *w, const uint8_t *p, uint16_t ww, uint16_t hh) { moui_pack_st7305_4x2(w, p, ww, hh); }
static void pack_fn_vmsb(uint8_t *w, const uint8_t *p, uint16_t ww, uint16_t hh)   { moui_pack_mono_vmsb(w, p, ww, hh); }
static void pack_fn_hmsb(uint8_t *w, const uint8_t *p, uint16_t ww, uint16_t hh)   { moui_pack_mono_hmsb(w, p, ww, hh); }

int moui_fb_init(moui_fb_t *fb, const moui_disp_desc_t *desc)
{
    memset(fb, 0, sizeof(*fb));
    fb->width  = desc->width;
    fb->height = desc->height;
    if (desc->pixel_format == MOUI_PIXEL_FORMAT_GRAY2_HMSB)
        fb->buf_size = ((uint32_t)desc->width * desc->height + 3) / 4;
    else
        fb->buf_size = ((uint32_t)desc->width * desc->height + 7) / 8;
    fb->wire_size = calc_wire_size(desc);

    fb->pixels = (uint8_t *)calloc(1, fb->buf_size);
    fb->wire   = (uint8_t *)calloc(1, fb->wire_size);
    if (!fb->pixels || !fb->wire) {
        free(fb->pixels);
        free(fb->wire);
        fb->pixels = NULL;
        fb->wire = NULL;
        return -1;
    }

    switch (desc->pixel_format) {
    case MOUI_PIXEL_FORMAT_ST7305_4x2: fb->pack_fn = pack_fn_st7305; break;
    case MOUI_PIXEL_FORMAT_MONO_VMSB:  fb->pack_fn = pack_fn_vmsb;  break;
    case MOUI_PIXEL_FORMAT_MONO_HMSB:  fb->pack_fn = pack_fn_hmsb;  break;
    case MOUI_PIXEL_FORMAT_GRAY2_HMSB: fb->pack_fn = NULL; break;
    }

    return 0;
}

void moui_fb_deinit(moui_fb_t *fb)
{
    free(fb->pixels);
    free(fb->wire);
    fb->pixels = NULL;
    fb->wire = NULL;
}

void moui_fb_clear(moui_fb_t *fb, moui_color_t color)
{
    memset(fb->pixels, (color >= MOUI_DGRAY) ? 0xFF : 0x00, fb->buf_size);
}

void moui_fb_pack_wire(moui_fb_t *fb)
{
    if (fb->pack_fn)
        fb->pack_fn(fb->wire, fb->pixels, moui_disp_w(), moui_disp_h());
    else
        memcpy(fb->wire, fb->pixels, fb->wire_size);
}
