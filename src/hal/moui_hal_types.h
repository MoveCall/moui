#ifndef MOUI_HAL_TYPES_H
#define MOUI_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef uint32_t moui_time_ms_t;

/* ---- Display Descriptor ---- */

typedef enum {
    MOUI_PIXEL_FORMAT_MONO_HMSB,
    MOUI_PIXEL_FORMAT_MONO_VMSB,
    MOUI_PIXEL_FORMAT_ST7305_4x2,
    MOUI_PIXEL_FORMAT_GRAY2_HMSB,
} moui_pixel_format_t;

typedef struct {
    uint16_t           width;
    uint16_t           height;
    moui_pixel_format_t pixel_format;
} moui_disp_desc_t;

/* Global display descriptor — set once at init */
extern const moui_disp_desc_t *moui_disp;

/* Convenience accessors (use these instead of old macros) */
static inline uint16_t moui_disp_w(void) { return moui_disp->width; }
static inline uint16_t moui_disp_h(void) { return moui_disp->height; }
static inline uint32_t moui_disp_fb_bytes(void) {
    if (moui_disp->pixel_format == MOUI_PIXEL_FORMAT_GRAY2_HMSB)
        return ((uint32_t)moui_disp->width * moui_disp->height + 3) / 4;
    return ((uint32_t)moui_disp->width * moui_disp->height + 7) / 8;
}

/* Legacy macros — for backward compat, reference runtime values */
#define MOUI_DISP_W        (moui_disp_w())
#define MOUI_DISP_H        (moui_disp_h())
#define MOUI_FB_BYTES      (moui_disp_fb_bytes())

/* Preset display configurations */
extern const moui_disp_desc_t moui_disp_st7305_2p9;
extern const moui_disp_desc_t moui_disp_ssd1306_128x64;
extern const moui_disp_desc_t moui_disp_st7305_2p13;
extern const moui_disp_desc_t moui_disp_st7305_4p2;
extern const moui_disp_desc_t moui_disp_st7306_4p2;
extern const moui_disp_desc_t moui_disp_gdew0154;
extern const moui_disp_desc_t moui_disp_ssd1677_3p7;
extern const moui_disp_desc_t moui_disp_ssd1677_4p26;
extern const moui_disp_desc_t moui_disp_ssd1677_4p26_4g;


#ifdef __cplusplus
}
#endif
#endif
