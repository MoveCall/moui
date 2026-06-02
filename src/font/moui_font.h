#ifndef MOUI_FONT_H
#define MOUI_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../core/moui_draw.h"
#include <stdint.h>

typedef struct {
    uint8_t  width;
    uint8_t  height;
    int8_t   x_offset;
    int8_t   y_offset;
    uint8_t  x_advance;
    uint32_t bitmap_offset;
} moui_glyph_t;

typedef struct moui_font {
    uint8_t           px_size;
    uint8_t           line_height;
    uint8_t           ascii_width;
    uint8_t           bpp;
    const moui_glyph_t *ascii_glyphs;
    uint16_t           cn_count;
    const uint32_t    *cn_codepoints;
    const moui_glyph_t *cn_glyphs;
    const uint8_t     *bitmap_data;
    const struct moui_font *fallback;
} moui_font_t;

extern const moui_font_t moui_font_ascii_6x8;
extern const moui_font_t moui_font_cn_12;
extern const moui_font_t moui_font_cn_16;
extern const moui_font_t moui_font_cn_24;
extern const moui_font_t moui_font_cn_28;
extern const moui_font_t moui_font_cn_32;
extern const moui_font_t moui_font_inter_12;
extern const moui_font_t moui_font_inter_16;
extern const moui_font_t moui_font_inter_24;
extern const moui_font_t moui_font_inter_32;
extern const moui_font_t moui_font_inter_48;
extern const moui_font_t moui_font_inter_16_aa;
extern const moui_font_t moui_font_inter_24_aa;
extern const moui_font_t moui_font_wqy_16;

int moui_font_draw_str(moui_draw_ctx_t *ctx, const moui_font_t *font,
                      int x, int y, const char *utf8_str, moui_color_t color);

int moui_font_draw_str_aa(moui_draw_ctx_t *ctx, const moui_font_t *font,
                         int x, int y, const char *utf8_str, moui_color_t color);

int moui_font_measure_str(const moui_font_t *font, const char *utf8_str);

int moui_font_glyph_advance(const moui_font_t *font, uint32_t codepoint);

uint32_t moui_font_utf8_decode(const char **p);

void moui_font_measure_wrapped(const moui_font_t *font, const char *text,
                              int max_w, int *out_w, int *out_h);

int moui_font_draw_str_wrapped(moui_draw_ctx_t *ctx, const moui_font_t *font,
                              int x, int y, int max_w, const char *text, moui_color_t color);

int moui_font_draw_str_wrapped_ex(moui_draw_ctx_t *ctx, const moui_font_t *font,
                                  int x, int y, int max_w, const char *text,
                                  int start_line, int max_lines, moui_color_t color);

void moui_font_measure_wrapped_ex(const moui_font_t *font, const char *text,
                                  int max_w, int start_line, int max_lines,
                                  int *out_w, int *out_h);

int moui_font_count_wrapped_lines(const moui_font_t *font, const char *text, int max_w);

int moui_font_draw_glyph(moui_draw_ctx_t *ctx, const moui_font_t *font,
                        int x, int y, uint32_t codepoint, moui_color_t color);

/*
 * Integer-scaled text/glyph drawing. `scale` is the zoom factor (>=1); every
 * source pixel becomes a scale x scale block. Useful to render a small point
 * size at a larger display size without a second font. Advance width, offsets
 * and line height are all scaled by `scale`.
 */
int moui_font_draw_glyph_scaled(moui_draw_ctx_t *ctx, const moui_font_t *font,
                               int x, int y, uint32_t codepoint,
                               moui_color_t color, uint8_t scale);

int moui_font_draw_str_scaled(moui_draw_ctx_t *ctx, const moui_font_t *font,
                             int x, int y, const char *utf8_str,
                             moui_color_t color, uint8_t scale);

int moui_font_measure_str_scaled(const moui_font_t *font, const char *utf8_str,
                                uint8_t scale);

int moui_font_glyph_advance_scaled(const moui_font_t *font, uint32_t codepoint,
                                  uint8_t scale);

static inline void moui_font_set_fallback(moui_font_t *font, const moui_font_t *fb)
{
    font->fallback = fb;
}


#ifdef __cplusplus
}
#endif
#endif
