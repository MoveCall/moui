#include "moui_draw.h"
#include <string.h>

static inline int max_i(int a, int b) { return a > b ? a : b; }
static inline int min_i(int a, int b) { return a < b ? a : b; }
static inline int abs_i(int a) { return a < 0 ? -a : a; }

/* fb-path pixel dispatch */
static void fb_set(moui_draw_ctx_t *ctx, int x, int y, moui_color_t color)
{
    moui_fb_set_pixel(ctx->fb, x, y, color);
}
static moui_color_t fb_get(moui_draw_ctx_t *ctx, int x, int y)
{
    return moui_fb_get_pixel(ctx->fb, x, y);
}

/* backend-path pixel dispatch (goes through moui_be_set_pixel for rotation support) */
static void be_set(moui_draw_ctx_t *ctx, int x, int y, moui_color_t color)
{
    moui_be_set_pixel(ctx->be, x, y, color);
}
static moui_color_t be_get(moui_draw_ctx_t *ctx, int x, int y)
{
    return moui_be_get_pixel(ctx->be, x, y);
}

void moui_draw_ctx_init(moui_draw_ctx_t *ctx, moui_fb_t *fb)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->fb = fb;
    ctx->set_fn = fb_set;
    ctx->get_fn = fb_get;
    ctx->clip = (moui_rect_t){ 0, 0, MOUI_DISP_W, MOUI_DISP_H };
}

void moui_draw_ctx_init_be(moui_draw_ctx_t *ctx, moui_backend_t *be)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->be = be;
    ctx->set_fn = be_set;
    ctx->get_fn = be_get;
    ctx->clip = (moui_rect_t){ 0, 0, (int16_t)be->width, (int16_t)be->height };
}

static inline bool clip_contains(const moui_rect_t *c, int x, int y)
{
    return x >= c->x && x < c->x + c->w && y >= c->y && y < c->y + c->h;
}

void moui_draw_pixel(moui_draw_ctx_t *ctx, int x, int y, moui_color_t color)
{
    if (!clip_contains(&ctx->clip, x, y)) return;
    if (ctx->xor_mode) color = MOUI_BLACK ^ ctx->get_fn(ctx, x, y);
    ctx->set_fn(ctx, x, y, color);
}

void moui_draw_hline(moui_draw_ctx_t *ctx, int x, int y, int len, moui_color_t color)
{
    if (y < ctx->clip.y || y >= ctx->clip.y + ctx->clip.h) return;
    int x0 = max_i(x, ctx->clip.x);
    int x1 = min_i(x + len, ctx->clip.x + ctx->clip.w);
    if (x0 >= x1) return;

    if (ctx->xor_mode || !ctx->fb
        || moui_disp->pixel_format == MOUI_PIXEL_FORMAT_GRAY2_HMSB) {
        for (int i = x0; i < x1; i++) {
            if (ctx->xor_mode) ctx->set_fn(ctx, i, y, MOUI_BLACK ^ ctx->get_fn(ctx, i, y));
            else                ctx->set_fn(ctx, i, y, color);
        }
        return;
    }

    /* Fast path: direct byte manipulation on fb pixel buffer (1bpp only) */
    uint8_t *pixels = ctx->fb->pixels;
    uint16_t w = ctx->fb->width;
    if (w == 0 || (unsigned)y >= ctx->fb->height || (unsigned)x0 >= w) return;
    if ((unsigned)x1 > w) x1 = w;
    int row_bit = y * w;

    int bit0 = row_bit + x0;
    int bit1 = row_bit + x1;
    int byte0 = bit0 >> 3;
    int byte1 = (bit1 - 1) >> 3;

    if (byte0 == byte1) {
        uint8_t mask = (0xFF >> (bit0 & 7)) & (0xFF << (7 - ((bit1 - 1) & 7)));
        if (color >= MOUI_DGRAY) pixels[byte0] |= mask;
        else   pixels[byte0] &= ~mask;
    } else {
        uint8_t head_mask = 0xFF >> (bit0 & 7);
        uint8_t tail_mask = 0xFF << (7 - ((bit1 - 1) & 7));
        if (color >= MOUI_DGRAY) {
            pixels[byte0] |= head_mask;
            if (byte1 > byte0 + 1) memset(&pixels[byte0 + 1], 0xFF, byte1 - byte0 - 1);
            pixels[byte1] |= tail_mask;
        } else {
            pixels[byte0] &= ~head_mask;
            if (byte1 > byte0 + 1) memset(&pixels[byte0 + 1], 0x00, byte1 - byte0 - 1);
            pixels[byte1] &= ~tail_mask;
        }
    }
}

void moui_draw_vline(moui_draw_ctx_t *ctx, int x, int y, int len, moui_color_t color)
{
    if (x < ctx->clip.x || x >= ctx->clip.x + ctx->clip.w) return;
    int y0 = max_i(y, ctx->clip.y);
    int y1 = min_i(y + len, ctx->clip.y + ctx->clip.h);
    if (y0 >= y1) return;

    if (ctx->xor_mode || !ctx->fb
        || moui_disp->pixel_format == MOUI_PIXEL_FORMAT_GRAY2_HMSB) {
        for (int i = y0; i < y1; i++) {
            if (ctx->xor_mode) ctx->set_fn(ctx, x, i, MOUI_BLACK ^ ctx->get_fn(ctx, x, i));
            else                ctx->set_fn(ctx, x, i, color);
        }
        return;
    }

    /* Fast path: direct byte manipulation on fb pixel buffer (1bpp only) */
    uint8_t *pixels = ctx->fb->pixels;
    uint16_t w = ctx->fb->width;
    if (w == 0 || (unsigned)y0 >= ctx->fb->height || (unsigned)x >= w) return;

    if ((w & 7) == 0) {
        int stride_bytes = w >> 3;
        uint8_t *ptr = &pixels[(y0 * w + x) >> 3];
        uint8_t mask = 0x80 >> (x & 7);
        if (color >= MOUI_DGRAY) {
            for (int i = y0; i < y1; i++, ptr += stride_bytes) {
                *ptr |= mask;
            }
        } else {
            uint8_t inv_mask = ~mask;
            for (int i = y0; i < y1; i++, ptr += stride_bytes) {
                *ptr &= inv_mask;
            }
        }
        return;
    }

    int stride_bit = w;
    int pos = y0 * stride_bit + x;
    if (color >= MOUI_DGRAY) {
        for (int i = y0; i < y1; i++, pos += stride_bit) {
            pixels[pos >> 3] |= (0x80 >> (pos & 7));
        }
    } else {
        for (int i = y0; i < y1; i++, pos += stride_bit) {
            pixels[pos >> 3] &= ~(0x80 >> (pos & 7));
        }
    }
}

void moui_draw_line(moui_draw_ctx_t *ctx, int x0, int y0, int x1, int y1, moui_color_t color)
{
    int dx = abs_i(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs_i(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    for (;;) {
        moui_draw_pixel(ctx, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void moui_draw_rect(moui_draw_ctx_t *ctx, const moui_rect_t *r, moui_color_t color)
{
    if (r->w <= 0 || r->h <= 0) return;
    moui_draw_hline(ctx, r->x, r->y, r->w, color);
    moui_draw_hline(ctx, r->x, r->y + r->h - 1, r->w, color);
    if (r->h > 2) {
        moui_draw_vline(ctx, r->x, r->y + 1, r->h - 2, color);
        moui_draw_vline(ctx, r->x + r->w - 1, r->y + 1, r->h - 2, color);
    }
}

void moui_draw_fill_rect(moui_draw_ctx_t *ctx, const moui_rect_t *r, moui_color_t color)
{
    int y0 = max_i(r->y, ctx->clip.y);
    int y1 = min_i(r->y + r->h, ctx->clip.y + ctx->clip.h);
    for (int y = y0; y < y1; y++) {
        moui_draw_hline(ctx, r->x, y, r->w, color);
    }
}

void moui_draw_circle(moui_draw_ctx_t *ctx, int cx, int cy, int radius, moui_color_t color)
{
    int x = 0, y = radius, d = 3 - 2 * radius;
    while (x <= y) {
        moui_draw_pixel(ctx, cx + x, cy + y, color);
        moui_draw_pixel(ctx, cx - x, cy + y, color);
        moui_draw_pixel(ctx, cx + x, cy - y, color);
        moui_draw_pixel(ctx, cx - x, cy - y, color);
        moui_draw_pixel(ctx, cx + y, cy + x, color);
        moui_draw_pixel(ctx, cx - y, cy + x, color);
        moui_draw_pixel(ctx, cx + y, cy - x, color);
        moui_draw_pixel(ctx, cx - y, cy - x, color);
        if (d < 0) d += 4 * x + 6;
        else { d += 4 * (x - y) + 10; y--; }
        x++;
    }
}

void moui_draw_fill_circle(moui_draw_ctx_t *ctx, int cx, int cy, int radius, moui_color_t color)
{
    int x = 0, y = radius, d = 3 - 2 * radius;
    while (x <= y) {
        moui_draw_hline(ctx, cx - x, cy + y, 2 * x + 1, color);
        moui_draw_hline(ctx, cx - x, cy - y, 2 * x + 1, color);
        if (x != y) {
            moui_draw_hline(ctx, cx - y, cy + x, 2 * y + 1, color);
            moui_draw_hline(ctx, cx - y, cy - x, 2 * y + 1, color);
        }
        if (d < 0) d += 4 * x + 6;
        else { d += 4 * (x - y) + 10; y--; }
        x++;
    }
}

void moui_draw_arc(moui_draw_ctx_t *ctx, int cx, int cy, int radius,
                  int start_deg, int end_deg, moui_color_t color)
{
    if (radius < 1) return;

    /* Normalize degrees to 0-360 */
    start_deg = ((start_deg % 360) + 360) % 360;
    end_deg   = ((end_deg % 360) + 360) % 360;

    /* Draw arc using angle lookup per pixel on the circle */
    int x = 0, y = radius, d = 3 - 2 * radius;
    while (x <= y) {
        /* 8 octant points */
        int pts[8][2] = {
            {cx + x, cy - y}, {cx + y, cy - x},
            {cx + y, cy + x}, {cx + x, cy + y},
            {cx - x, cy + y}, {cx - y, cy + x},
            {cx - y, cy - x}, {cx - x, cy - y},
        };
        /* Approximate angle for each octant point (using atan2-free approach) */
        int angles[8] = {
            (x == 0 && y > 0) ? 0 : (90 - 90 * x / (x + y)),
            90 * x / (x + y),
            90 + 90 * x / (x + y),
            180 - 90 * x / (x + y),
            180 + 90 * x / (x + y),
            270 - 90 * x / (x + y),
            270 + 90 * x / (x + y),
            360 - 90 * x / (x + y),
        };
        for (int i = 0; i < 8; i++) {
            int a = angles[i] % 360;
            bool in_arc;
            if (start_deg <= end_deg)
                in_arc = (a >= start_deg && a <= end_deg);
            else
                in_arc = (a >= start_deg || a <= end_deg);
            if (in_arc)
                moui_draw_pixel(ctx, pts[i][0], pts[i][1], color);
        }
        if (d < 0) d += 4 * x + 6;
        else { d += 4 * (x - y) + 10; y--; }
        x++;
    }
}

void moui_draw_rounded_rect(moui_draw_ctx_t *ctx, const moui_rect_t *r, int rad, moui_color_t color)
{
    if (rad <= 0) { moui_draw_rect(ctx, r, color); return; }
    int x0 = r->x, y0 = r->y, w = r->w, h = r->h;
    moui_draw_hline(ctx, x0 + rad, y0, w - 2 * rad, color);
    moui_draw_hline(ctx, x0 + rad, y0 + h - 1, w - 2 * rad, color);
    moui_draw_vline(ctx, x0, y0 + rad, h - 2 * rad, color);
    moui_draw_vline(ctx, x0 + w - 1, y0 + rad, h - 2 * rad, color);
    int cx0 = x0 + rad, cy0 = y0 + rad;
    int cx1 = x0 + w - 1 - rad, cy1 = y0 + h - 1 - rad;
    int x = 0, y = rad, d = 3 - 2 * rad;
    while (x <= y) {
        moui_draw_pixel(ctx, cx1 + x, cy1 + y, color);
        moui_draw_pixel(ctx, cx0 - x, cy1 + y, color);
        moui_draw_pixel(ctx, cx1 + x, cy0 - y, color);
        moui_draw_pixel(ctx, cx0 - x, cy0 - y, color);
        moui_draw_pixel(ctx, cx1 + y, cy1 + x, color);
        moui_draw_pixel(ctx, cx0 - y, cy1 + x, color);
        moui_draw_pixel(ctx, cx1 + y, cy0 - x, color);
        moui_draw_pixel(ctx, cx0 - y, cy0 - x, color);
        if (d < 0) d += 4 * x + 6;
        else { d += 4 * (x - y) + 10; y--; }
        x++;
    }
}

void moui_draw_fill_rounded_rect(moui_draw_ctx_t *ctx, const moui_rect_t *r, int rad, moui_color_t color)
{
    if (rad <= 0) { moui_draw_fill_rect(ctx, r, color); return; }
    int x0 = r->x, y0 = r->y, w = r->w, h = r->h;
    moui_rect_t center = { x0, y0 + rad, w, h - 2 * rad };
    moui_draw_fill_rect(ctx, &center, color);
    int cx0 = x0 + rad, cx1 = x0 + w - 1 - rad;
    int cy0 = y0 + rad, cy1 = y0 + h - 1 - rad;
    int x = 0, y = rad, d = 3 - 2 * rad;
    while (x <= y) {
        moui_draw_hline(ctx, cx0 - x, cy0 - y, (cx1 - cx0) + 2 * x + 1, color);
        moui_draw_hline(ctx, cx0 - x, cy1 + y, (cx1 - cx0) + 2 * x + 1, color);
        moui_draw_hline(ctx, cx0 - y, cy0 - x, (cx1 - cx0) + 2 * y + 1, color);
        moui_draw_hline(ctx, cx0 - y, cy1 + x, (cx1 - cx0) + 2 * y + 1, color);
        if (d < 0) d += 4 * x + 6;
        else { d += 4 * (x - y) + 10; y--; }
        x++;
    }
}

void moui_draw_bitmap(moui_draw_ctx_t *ctx, int x, int y, int w, int h,
                     const uint8_t *bmp, moui_color_t color)
{
    int bytes_per_row = (w + 7) / 8;
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            int byte_idx = row * bytes_per_row + (col >> 3);
            uint8_t bit_mask = 0x80 >> (col & 7);
            if (bmp[byte_idx] & bit_mask)
                moui_draw_pixel(ctx, x + col, y + row, color);
        }
    }
}

void moui_draw_bitmap_scaled(moui_draw_ctx_t *ctx, int dst_x, int dst_y, int dst_w, int dst_h,
                          int src_w, int src_h, const uint8_t *bmp, moui_color_t color)
{
    if (dst_w <= 0 || dst_h <= 0 || src_w <= 0 || src_h <= 0 || !bmp) return;
    if (dst_w == src_w && dst_h == src_h) {
        moui_draw_bitmap(ctx, dst_x, dst_y, src_w, src_h, bmp, color);
        return;
    }
    int src_bytes_per_row = (src_w + 7) / 8;
    for (int dy = 0; dy < dst_h; dy++) {
        int sy = (dy * src_h) / dst_h;
        for (int dx = 0; dx < dst_w; dx++) {
            int sx = (dx * src_w) / dst_w;
            int byte_idx = sy * src_bytes_per_row + (sx >> 3);
            uint8_t bit_mask = 0x80 >> (sx & 7);
            if (bmp[byte_idx] & bit_mask) {
                moui_draw_pixel(ctx, dst_x + dx, dst_y + dy, color);
            }
        }
    }
}

void moui_draw_bitmap_rle(moui_draw_ctx_t *ctx, int x, int y, int w, int h,
                         const uint8_t *rle_data, uint32_t rle_len, moui_color_t color)
{
    int bytes_per_row = (w + 7) / 8;
    int total_raw = bytes_per_row * h;

    int raw_pos = 0;
    uint32_t src = 0;

    while (src < rle_len && raw_pos < total_raw) {
        uint8_t b;
        int count;

        if (rle_data[src] == 0x80 && src + 2 < rle_len) {
            count = rle_data[src + 1];
            b = rle_data[src + 2];
            src += 3;
        } else {
            b = rle_data[src++];
            count = 1;
        }

        for (int r = 0; r < count && raw_pos < total_raw; r++, raw_pos++) {
            int row = raw_pos / bytes_per_row;
            int col_byte = raw_pos % bytes_per_row;
            for (int bit = 0; bit < 8; bit++) {
                int col = col_byte * 8 + bit;
                if (col >= w) break;
                if (b & (0x80 >> bit))
                    moui_draw_pixel(ctx, x + col, y + row, color);
            }
        }
    }
}

void moui_draw_push_clip(moui_draw_ctx_t *ctx, const moui_rect_t *clip)
{
    if (ctx->clip_sp >= MOUI_CLIP_STACK_DEPTH) return;
    ctx->clip_stack[ctx->clip_sp++] = ctx->clip;
    int x0 = max_i(ctx->clip.x, clip->x);
    int y0 = max_i(ctx->clip.y, clip->y);
    int x1 = min_i(ctx->clip.x + ctx->clip.w, clip->x + clip->w);
    int y1 = min_i(ctx->clip.y + ctx->clip.h, clip->y + clip->h);
    ctx->clip.x = x0;
    ctx->clip.y = y0;
    ctx->clip.w = max_i(0, x1 - x0);
    ctx->clip.h = max_i(0, y1 - y0);
}

void moui_draw_pop_clip(moui_draw_ctx_t *ctx)
{
    if (ctx->clip_sp == 0) return;
    ctx->clip = ctx->clip_stack[--ctx->clip_sp];
}
