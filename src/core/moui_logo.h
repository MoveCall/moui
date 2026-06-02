#ifndef MOUI_LOGO_H
#define MOUI_LOGO_H

#include "moui_draw.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 16x16 Pixel Mono Logo for Moui Framework
 * Display frame containing geometric 'M'
 */
static const uint8_t moui_logo_16x16[32] = {
    0xFF, 0xFE, /* 11111111 11111110 - Top Frame */
    0x80, 0x02, /* 10000000 00000010 - Outer Frame */
    0x8F, 0xF2, /* 10001111 11110010 - Inner Frame */
    0x88, 0x12, /* 10001000 00010010 */
    0x89, 0x92, /* 10001001 10010010 - M Peak Left/Right */
    0x8A, 0x52, /* 10001010 01010010 - M Peak Center */
    0x8C, 0x32, /* 10001100 00110010 - M Center Join */
    0x88, 0x12, /* 10001000 00010010 - M Legs */
    0x88, 0x12, /* 10001000 00010010 */
    0x88, 0x12, /* 10001000 00010010 */
    0x8F, 0xF2, /* 10001111 11110010 - Inner Frame Bottom */
    0x80, 0x02, /* 10000000 00000010 */
    0xFF, 0xFE, /* 11111111 11111110 - Bottom Frame */
    0x00, 0x00,
    0x07, 0xE0, /* Stand Base */
    0x0F, 0xF0  /* Stand Foot */
};

/**
 * Draw 16x16 Moui Logo Icon
 */
static inline void moui_draw_logo_16(moui_draw_ctx_t *ctx, int x, int y, moui_color_t color)
{
    moui_draw_bitmap(ctx, x, y, 16, 16, moui_logo_16x16, color);
}

/**
 * Draw Scaled Moui Logo Banner (Display Frame + "Moui UI")
 */
static inline void moui_draw_logo_banner(moui_draw_ctx_t *ctx, int x, int y, const moui_font_t *font, moui_color_t color)
{
    /* Display Frame */
    moui_rect_t frame = { (int16_t)x, (int16_t)y, 22, 16 };
    moui_draw_rounded_rect(ctx, &frame, 3, color);
    /* 'M' geometry inside frame */
    moui_draw_vline(ctx, x + 4, y + 4, 8, color);
    moui_draw_vline(ctx, x + 17, y + 4, 8, color);
    moui_draw_pixel(ctx, x + 6, y + 6, color);
    moui_draw_pixel(ctx, x + 8, y + 8, color);
    moui_draw_pixel(ctx, x + 10, y + 10, color);
    moui_draw_pixel(ctx, x + 12, y + 8, color);
    moui_draw_pixel(ctx, x + 14, y + 6, color);
    /* Stand */
    moui_draw_hline(ctx, x + 7, y + 16, 8, color);
    moui_draw_hline(ctx, x + 5, y + 17, 12, color);

    /* Text */
    if (font) {
        moui_font_draw_str(ctx, font, x + 26, y + 4, "Moui UI", color);
    }
}

#ifdef __cplusplus
}
#endif

#endif /* MOUI_LOGO_H */
