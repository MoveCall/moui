#include "moui_font.h"
#include <string.h>

static uint32_t utf8_decode(const char **p)
{
    const uint8_t *s = (const uint8_t *)*p;
    uint32_t cp;
    int len;

    if (s[0] < 0x80) {
        cp = s[0]; len = 1;
    } else if ((s[0] & 0xE0) == 0xC0) {
        cp = s[0] & 0x1F; len = 2;
    } else if ((s[0] & 0xF0) == 0xE0) {
        cp = s[0] & 0x0F; len = 3;
    } else if ((s[0] & 0xF8) == 0xF0) {
        cp = s[0] & 0x07; len = 4;
    } else {
        *p += 1;
        return 0xFFFD;
    }

    for (int i = 1; i < len; i++) {
        if ((s[i] & 0xC0) != 0x80) { *p += 1; return 0xFFFD; }
        cp = (cp << 6) | (s[i] & 0x3F);
    }
    *p += len;
    return cp;
}

static const moui_glyph_t *find_glyph_single(const moui_font_t *font, uint32_t cp)
{
    if (cp >= 0x20 && cp <= 0x7E && font->ascii_glyphs)
        return &font->ascii_glyphs[cp - 0x20];

    if (font->cn_count == 0 || !font->cn_codepoints) return NULL;

    int lo = 0, hi = (int)font->cn_count - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (font->cn_codepoints[mid] == cp)
            return &font->cn_glyphs[mid];
        else if (font->cn_codepoints[mid] < cp)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return NULL;
}

static const moui_glyph_t *find_glyph(const moui_font_t *font, uint32_t cp,
                                       const moui_font_t **out_font)
{
    for (const moui_font_t *f = font; f; f = f->fallback) {
        const moui_glyph_t *g = find_glyph_single(f, cp);
        if (g) {
            if (out_font) *out_font = f;
            return g;
        }
    }
    if (out_font) *out_font = font;
    return NULL;
}

static void draw_glyph_aa(moui_draw_ctx_t *ctx, const moui_font_t *font,
                           const moui_glyph_t *g, int x, int y, moui_color_t color);

static void draw_glyph_bitmap(moui_draw_ctx_t *ctx, const moui_font_t *font,
                               const moui_glyph_t *g, int x, int y, moui_color_t color)
{
    int dx = x + g->x_offset;
    int dy = y + g->y_offset;

    /* Fast clip check: skip out-of-bounds glyph completely */
    if (dx + g->width <= ctx->clip.x || dx >= ctx->clip.x + ctx->clip.w ||
        dy + g->height <= ctx->clip.y || dy >= ctx->clip.y + ctx->clip.h) {
        return;
    }

    const uint8_t *bmp = font->bitmap_data + g->bitmap_offset;
    int src_bpr = (g->width + 7) / 8;

    /* Fast path: direct byte-OR into fb pixel buffer (1bpp only) */
    if (ctx->fb && !ctx->xor_mode && color
        && moui_disp->pixel_format != MOUI_PIXEL_FORMAT_GRAY2_HMSB) {
        uint8_t *pixels = ctx->fb->pixels;
        uint16_t disp_w = moui_disp_w();
        uint16_t disp_h = moui_disp_h();

        for (int row = 0; row < g->height; row++) {
            int py = dy + row;
            if (py < ctx->clip.y || py >= ctx->clip.y + ctx->clip.h) continue;
            if ((unsigned)py >= disp_h) continue;

            const uint8_t *src_row = bmp + row * src_bpr;
            int dst_bit0 = py * disp_w + dx;

            for (int col = 0; col < g->width; col++) {
                int px = dx + col;
                if (px < ctx->clip.x || px >= ctx->clip.x + ctx->clip.w) continue;
                if ((unsigned)px >= disp_w) continue;

                if (src_row[col >> 3] & (0x80 >> (col & 7))) {
                    int di = dst_bit0 + col;
                    if (di < 0) continue;
                    pixels[di >> 3] |= (0x80 >> (di & 7));
                }
            }
        }
        return;
    }

    /* Fallback: pixel-by-pixel through draw context */
    for (int row = 0; row < g->height; row++) {
        for (int col = 0; col < g->width; col++) {
            int byte_idx = row * src_bpr + (col >> 3);
            uint8_t bit_mask = 0x80 >> (col & 7);
            if (bmp[byte_idx] & bit_mask)
                moui_draw_pixel(ctx, dx + col, dy + row, color);
        }
    }
}

int moui_font_draw_glyph(moui_draw_ctx_t *ctx, const moui_font_t *font,
                        int x, int y, uint32_t codepoint, moui_color_t color)
{
    if (!font) return 0;
    const moui_font_t *resolved = font;
    const moui_glyph_t *g = find_glyph(font, codepoint, &resolved);
    if (!g) return font->ascii_width ? font->ascii_width : font->px_size / 2;

    if (resolved->bpp >= 2)
        draw_glyph_aa(ctx, resolved, g, x, y, color);
    else
        draw_glyph_bitmap(ctx, resolved, g, x, y, color);
    return g->x_advance ? g->x_advance : g->width;
}

int moui_font_draw_glyph_scaled(moui_draw_ctx_t *ctx, const moui_font_t *font,
                               int x, int y, uint32_t codepoint,
                               moui_color_t color, uint8_t scale)
{
    if (!font) return 0;
    if (scale < 1) scale = 1;

    const moui_font_t *resolved = font;
    const moui_glyph_t *g = find_glyph(font, codepoint, &resolved);
    if (!g) return (font->ascii_width ? font->ascii_width : font->px_size / 2) * scale;

    const uint8_t *bmp = resolved->bitmap_data + g->bitmap_offset;
    int dx = (x + g->x_offset) * scale;
    int dy = (y + g->y_offset) * scale;
    int adv = (g->x_advance ? g->x_advance : g->width) * scale;

    if (resolved->bpp >= 2) {
        /* 2bpp AA: upscale each 2-bit pixel */
        int bytes_per_row = (g->width + 3) / 4;
        for (int row = 0; row < g->height; row++) {
            for (int col = 0; col < g->width; col++) {
                int byte_idx = row * bytes_per_row + (col / 4);
                int shift = 6 - (col % 4) * 2;
                int level = (bmp[byte_idx] >> shift) & 0x03;
                if (level == 0) continue;

                moui_color_t c;
                switch (level) {
                case 3: c = color; break;
                case 2: c = MOUI_DGRAY; break;
                default: c = MOUI_LGRAY; break;
                }
                moui_draw_fill_rect(ctx, &(moui_rect_t){
                    dx + col * scale, dy + row * scale, scale, scale }, c);
            }
        }
        return adv;
    }

    /* 1bpp */
    int src_bpr = (g->width + 7) / 8;
    for (int row = 0; row < g->height; row++) {
        for (int col = 0; col < g->width; col++) {
            int byte_idx = row * src_bpr + (col >> 3);
            uint8_t bit_mask = 0x80 >> (col & 7);
            if (bmp[byte_idx] & bit_mask) {
                moui_draw_fill_rect(ctx, &(moui_rect_t){
                    dx + col * scale, dy + row * scale, scale, scale }, color);
            }
        }
    }
    return adv;
}

int moui_font_draw_str_scaled(moui_draw_ctx_t *ctx, const moui_font_t *font,
                             int x, int y, const char *utf8_str,
                             moui_color_t color, uint8_t scale)
{
    if (!font || !utf8_str) return 0;
    if (scale < 1) scale = 1;

    int cursor_x = x;
    int cursor_y = y;
    while (*utf8_str) {
        uint32_t cp = utf8_decode(&utf8_str);
        if (cp == '\n') {
            cursor_x = x;
            cursor_y += font->line_height * scale;
            continue;
        }
        cursor_x += moui_font_draw_glyph_scaled(ctx, font, cursor_x, cursor_y,
                                                cp, color, scale);
    }
    return cursor_x - x;
}

int moui_font_glyph_advance_scaled(const moui_font_t *font, uint32_t codepoint,
                                  uint8_t scale)
{
    int adv = moui_font_glyph_advance(font, codepoint);
    return (scale < 1 ? 1 : scale) * adv;
}

int moui_font_measure_str_scaled(const moui_font_t *font, const char *utf8_str,
                                uint8_t scale)
{
    if (!font || !utf8_str) return 0;
    if (scale < 1) scale = 1;

    int max_w = 0, line_w = 0;
    while (*utf8_str) {
        uint32_t cp = utf8_decode(&utf8_str);
        if (cp == '\n') {
            if (line_w > max_w) max_w = line_w;
            line_w = 0;
            continue;
        }
        line_w += moui_font_glyph_advance_scaled(font, cp, scale);
    }
    return line_w > max_w ? line_w : max_w;
}

int moui_font_draw_str(moui_draw_ctx_t *ctx, const moui_font_t *font,
                      int x, int y, const char *utf8_str, moui_color_t color)
{
    if (!font || !utf8_str) return 0;
    int cursor_x = x;
    while (*utf8_str) {
        uint32_t cp = utf8_decode(&utf8_str);
        if (cp == '\n') {
            cursor_x = x;
            y += font->line_height;
            continue;
        }
        cursor_x += moui_font_draw_glyph(ctx, font, cursor_x, y, cp, color);
    }
    return cursor_x - x;
}

static void draw_glyph_aa(moui_draw_ctx_t *ctx, const moui_font_t *font,
                           const moui_glyph_t *g, int x, int y, moui_color_t color)
{
    int dx = x + g->x_offset;
    int dy = y + g->y_offset;

    /* Fast clip check: skip out-of-bounds glyph completely */
    if (dx + g->width <= ctx->clip.x || dx >= ctx->clip.x + ctx->clip.w ||
        dy + g->height <= ctx->clip.y || dy >= ctx->clip.y + ctx->clip.h) {
        return;
    }

    const uint8_t *bmp = font->bitmap_data + g->bitmap_offset;
    int bytes_per_row = (g->width + 3) / 4;  /* 2bpp: 4 pixels per byte */

    bool gray_target = moui_disp && moui_disp->pixel_format == MOUI_PIXEL_FORMAT_GRAY2_HMSB;

    for (int row = 0; row < g->height; row++) {
        for (int col = 0; col < g->width; col++) {
            int byte_idx = row * bytes_per_row + (col >> 2);
            int shift = 6 - (col & 3) * 2;
            int level = (bmp[byte_idx] >> shift) & 0x03;

            if (level == 0) continue;

            int px = dx + col;
            int py = dy + row;

            if (gray_target) {
                moui_color_t c;
                switch (level) {
                case 3: c = color; break;
                case 2: c = MOUI_DGRAY; break;
                case 1: c = MOUI_LGRAY; break;
                default: continue;
                }
                moui_draw_pixel(ctx, px, py, c);
            } else {
                switch (level) {
                case 3:
                    moui_draw_pixel(ctx, px, py, color);
                    break;
                case 2:
                    if (((px + py) & 1) || ((px ^ py) & 2))
                        moui_draw_pixel(ctx, px, py, color);
                    break;
                case 1:
                    if (((px + py) & 1) == 0 && ((px ^ py) & 2) == 0)
                        moui_draw_pixel(ctx, px, py, color);
                    break;
                }
            }
        }
    }
}

int moui_font_draw_str_aa(moui_draw_ctx_t *ctx, const moui_font_t *font,
                         int x, int y, const char *utf8_str, moui_color_t color)
{
    if (!font || !utf8_str) return 0;
    int cursor_x = x;
    while (*utf8_str) {
        uint32_t cp = utf8_decode(&utf8_str);
        if (cp == '\n') {
            cursor_x = x;
            y += font->line_height;
            continue;
        }
        const moui_font_t *resolved = font;
        const moui_glyph_t *g = find_glyph(font, cp, &resolved);
        if (g) {
            draw_glyph_aa(ctx, resolved, g, cursor_x, y, color);
            cursor_x += g->x_advance ? g->x_advance : g->width;
        } else {
            cursor_x += font->ascii_width ? font->ascii_width : font->px_size / 2;
        }
    }
    return cursor_x - x;
}

int moui_font_measure_str(const moui_font_t *font, const char *utf8_str)
{
    if (!font || !utf8_str) return 0;
    int max_w = 0, line_w = 0;
    while (*utf8_str) {
        uint32_t cp = utf8_decode(&utf8_str);
        if (cp == '\n') {
            if (line_w > max_w) max_w = line_w;
            line_w = 0;
            continue;
        }
        line_w += moui_font_glyph_advance(font, cp);
    }
    return line_w > max_w ? line_w : max_w;
}

uint32_t moui_font_utf8_decode(const char **p)
{
    return utf8_decode(p);
}

int moui_font_glyph_advance(const moui_font_t *font, uint32_t cp)
{
    const moui_glyph_t *g = find_glyph(font, cp, NULL);
    if (g) return g->x_advance ? g->x_advance : g->width;
    return font->ascii_width ? font->ascii_width : font->px_size / 2;
}

static bool is_cjk_cp(uint32_t cp)
{
    return (cp >= 0x4E00 && cp <= 0x9FFF) ||
           (cp >= 0x3400 && cp <= 0x4DBF) ||
           (cp >= 0x3000 && cp <= 0x303F);
}

void moui_font_measure_wrapped(const moui_font_t *font, const char *text,
                              int max_w, int *out_w, int *out_h)
{
    if (!font || !text) { *out_w = 0; *out_h = 0; return; }

    int cur_w = 0, result_w = 0;
    int lines = 1;
    const char *p = text;

    while (*p) {
        uint32_t cp = utf8_decode(&p);

        if (cp == '\n') {
            if (cur_w > result_w) result_w = cur_w;
            cur_w = 0;
            lines++;
            continue;
        }

        int adv = moui_font_glyph_advance(font, cp);

        if (is_cjk_cp(cp)) {
            if (cur_w + adv > max_w && cur_w > 0) {
                if (cur_w > result_w) result_w = cur_w;
                cur_w = 0;
                lines++;
            }
            cur_w += adv;
            continue;
        }

        if (cp == ' ') {
            if (cur_w + adv > max_w && cur_w > 0) {
                if (cur_w > result_w) result_w = cur_w;
                cur_w = 0;
                lines++;
            } else {
                cur_w += adv;
            }
            continue;
        }

        /* Measure word */
        int word_w = adv;
        const char *word_end = p;
        while (*word_end && *word_end != ' ' && *word_end != '\n') {
            const char *peek = word_end;
            uint32_t ncp = utf8_decode(&peek);
            if (is_cjk_cp(ncp)) break;
            word_w += moui_font_glyph_advance(font, ncp);
            word_end = peek;
        }

        if (cur_w > 0 && cur_w + word_w > max_w) {
            if (cur_w > result_w) result_w = cur_w;
            cur_w = 0;
            lines++;
        }

        cur_w += adv;
        while (p < word_end) {
            cp = utf8_decode(&p);
            cur_w += moui_font_glyph_advance(font, cp);
        }
    }

    if (cur_w > result_w) result_w = cur_w;
    *out_w = result_w;
    *out_h = lines * font->line_height;
}

int moui_font_draw_str_wrapped(moui_draw_ctx_t *ctx, const moui_font_t *font,
                              int x, int y, int max_w, const char *text, moui_color_t color)
{
    if (!font || !text || !ctx) return 0;

    int cur_x = x;
    int cur_y = y;
    const char *p = text;

    while (*p) {
        uint32_t cp = utf8_decode(&p);

        if (cp == '\n') {
            cur_x = x;
            cur_y += font->line_height;
            continue;
        }

        int adv = moui_font_glyph_advance(font, cp);

        if (is_cjk_cp(cp)) {
            if (cur_x - x + adv > max_w && cur_x > x) {
                cur_x = x;
                cur_y += font->line_height;
            }
            moui_font_draw_glyph(ctx, font, cur_x, cur_y, cp, color);
            cur_x += adv;
            continue;
        }

        if (cp == ' ') {
            if (cur_x - x + adv > max_w && cur_x > x) {
                cur_x = x;
                cur_y += font->line_height;
            } else {
                cur_x += adv;
            }
            continue;
        }

        int word_w = adv;
        const char *word_end = p;
        while (*word_end && *word_end != ' ' && *word_end != '\n') {
            const char *peek = word_end;
            uint32_t ncp = utf8_decode(&peek);
            if (is_cjk_cp(ncp)) break;
            word_w += moui_font_glyph_advance(font, ncp);
            word_end = peek;
        }

        if (cur_x > x && cur_x - x + word_w > max_w) {
            cur_x = x;
            cur_y += font->line_height;
        }

        moui_font_draw_glyph(ctx, font, cur_x, cur_y, cp, color);
        cur_x += adv;
        while (p < word_end) {
            cp = utf8_decode(&p);
            adv = moui_font_draw_glyph(ctx, font, cur_x, cur_y, cp, color);
            cur_x += adv;
        }
    }

    return cur_y - y + font->line_height;
}

int moui_font_draw_str_wrapped_ex(moui_draw_ctx_t *ctx, const moui_font_t *font,
                                  int x, int y, int max_w, const char *text,
                                  int start_line, int max_lines, moui_color_t color)
{
    if (!font || !text || !ctx) return 0;

    int cur_x = x;
    int cur_y = y;
    int line = 0;
    int lines_drawn = 0;
    const char *p = text;

    while (*p) {
        uint32_t cp = utf8_decode(&p);

        if (cp == '\n') {
            line++;
            if (max_lines > 0 && line > start_line && lines_drawn >= max_lines)
                break;
            cur_x = x;
            if (line > start_line)
                cur_y += font->line_height;
            continue;
        }

        int adv = moui_font_glyph_advance(font, cp);

        if (is_cjk_cp(cp)) {
            if (cur_x - x + adv > max_w && cur_x > x) {
                line++;
                if (max_lines > 0 && line > start_line && lines_drawn >= max_lines)
                    break;
                cur_x = x;
                if (line > start_line)
                    cur_y += font->line_height;
            }
            if (line >= start_line) {
                if (lines_drawn == 0) lines_drawn = 1;
                moui_font_draw_glyph(ctx, font, cur_x, cur_y, cp, color);
            }
            cur_x += adv;
            continue;
        }

        if (cp == ' ') {
            if (cur_x - x + adv > max_w && cur_x > x) {
                line++;
                if (max_lines > 0 && line > start_line && lines_drawn >= max_lines)
                    break;
                cur_x = x;
                if (line > start_line)
                    cur_y += font->line_height;
            } else {
                cur_x += adv;
            }
            continue;
        }

        int word_w = adv;
        const char *word_end = p;
        while (*word_end && *word_end != ' ' && *word_end != '\n') {
            const char *peek = word_end;
            uint32_t ncp = utf8_decode(&peek);
            if (is_cjk_cp(ncp)) break;
            word_w += moui_font_glyph_advance(font, ncp);
            word_end = peek;
        }

        if (cur_x > x && cur_x - x + word_w > max_w) {
            line++;
            if (max_lines > 0 && line > start_line && lines_drawn >= max_lines)
                break;
            cur_x = x;
            if (line > start_line)
                cur_y += font->line_height;
        }

        if (line >= start_line) {
            if (lines_drawn == 0) lines_drawn = 1;
            moui_font_draw_glyph(ctx, font, cur_x, cur_y, cp, color);
        }
        cur_x += adv;
        while (p < word_end) {
            cp = utf8_decode(&p);
            if (line >= start_line)
                moui_font_draw_glyph(ctx, font, cur_x, cur_y, cp, color);
            cur_x += moui_font_glyph_advance(font, cp);
        }

        if (line >= start_line && lines_drawn == 0) lines_drawn = 1;
    }

    if (line >= start_line) lines_drawn = line - start_line + 1;
    if (max_lines > 0 && lines_drawn > max_lines) lines_drawn = max_lines;
    return lines_drawn;
}

void moui_font_measure_wrapped_ex(const moui_font_t *font, const char *text,
                                  int max_w, int start_line, int max_lines,
                                  int *out_w, int *out_h)
{
    if (!font || !text) { *out_w = 0; *out_h = 0; return; }

    int cur_w = 0, result_w = 0;
    int line = 0;
    int lines_counted = 0;
    const char *p = text;

    while (*p) {
        uint32_t cp = utf8_decode(&p);

        if (cp == '\n') {
            if (line >= start_line) {
                if (cur_w > result_w) result_w = cur_w;
                lines_counted++;
                if (max_lines > 0 && lines_counted >= max_lines) break;
            }
            cur_w = 0;
            line++;
            continue;
        }

        int adv = moui_font_glyph_advance(font, cp);

        if (is_cjk_cp(cp)) {
            if (cur_w + adv > max_w && cur_w > 0) {
                if (line >= start_line) {
                    if (cur_w > result_w) result_w = cur_w;
                    lines_counted++;
                    if (max_lines > 0 && lines_counted >= max_lines) break;
                }
                cur_w = 0;
                line++;
            }
            cur_w += adv;
            continue;
        }

        if (cp == ' ') {
            if (cur_w + adv > max_w && cur_w > 0) {
                if (line >= start_line) {
                    if (cur_w > result_w) result_w = cur_w;
                    lines_counted++;
                    if (max_lines > 0 && lines_counted >= max_lines) break;
                }
                cur_w = 0;
                line++;
            } else {
                cur_w += adv;
            }
            continue;
        }

        int word_w = adv;
        const char *word_end = p;
        while (*word_end && *word_end != ' ' && *word_end != '\n') {
            const char *peek = word_end;
            uint32_t ncp = utf8_decode(&peek);
            if (is_cjk_cp(ncp)) break;
            word_w += moui_font_glyph_advance(font, ncp);
            word_end = peek;
        }

        if (cur_w > 0 && cur_w + word_w > max_w) {
            if (line >= start_line) {
                if (cur_w > result_w) result_w = cur_w;
                lines_counted++;
                if (max_lines > 0 && lines_counted >= max_lines) break;
            }
            cur_w = 0;
            line++;
        }

        cur_w += adv;
        while (p < word_end) {
            cp = utf8_decode(&p);
            cur_w += moui_font_glyph_advance(font, cp);
        }
    }

    if (line >= start_line && cur_w > 0) {
        if (cur_w > result_w) result_w = cur_w;
        lines_counted++;
    }

    if (max_lines > 0 && lines_counted > max_lines) lines_counted = max_lines;
    *out_w = result_w;
    *out_h = lines_counted * font->line_height;
}

int moui_font_count_wrapped_lines(const moui_font_t *font, const char *text, int max_w)
{
    if (!font || !text) return 0;

    int lines = 1;
    int cur_w = 0;
    const char *p = text;

    while (*p) {
        uint32_t cp = utf8_decode(&p);

        if (cp == '\n') { cur_w = 0; lines++; continue; }

        int adv = moui_font_glyph_advance(font, cp);

        if (is_cjk_cp(cp)) {
            if (cur_w + adv > max_w && cur_w > 0) { cur_w = 0; lines++; }
            cur_w += adv;
            continue;
        }

        if (cp == ' ') {
            if (cur_w + adv > max_w && cur_w > 0) { cur_w = 0; lines++; }
            else cur_w += adv;
            continue;
        }

        int word_w = adv;
        const char *word_end = p;
        while (*word_end && *word_end != ' ' && *word_end != '\n') {
            const char *peek = word_end;
            uint32_t ncp = utf8_decode(&peek);
            if (is_cjk_cp(ncp)) break;
            word_w += moui_font_glyph_advance(font, ncp);
            word_end = peek;
        }

        if (cur_w > 0 && cur_w + word_w > max_w) { cur_w = 0; lines++; }

        cur_w += adv;
        while (p < word_end) {
            cp = utf8_decode(&p);
            cur_w += moui_font_glyph_advance(font, cp);
        }
    }

    return lines;
}
