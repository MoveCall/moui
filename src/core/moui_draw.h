#ifndef MOUI_DRAW_H
#define MOUI_DRAW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_fb.h"
#include "moui_color.h"
#include "../moui_conf.h"
#include "../backend/moui_backend.h"

typedef struct { int16_t x, y; }       moui_point_t;
typedef struct { int16_t x, y, w, h; } moui_rect_t;

static inline bool moui_rect_contains(const moui_rect_t *r, int16_t px, int16_t py) {
    return px >= r->x && px < r->x + r->w && py >= r->y && py < r->y + r->h;
}

static inline bool moui_rect_intersects(const moui_rect_t *a, const moui_rect_t *b) {
    return a->x < b->x + b->w && a->x + a->w > b->x &&
           a->y < b->y + b->h && a->y + a->h > b->y;
}

typedef struct moui_draw_ctx moui_draw_ctx_t;

typedef void (*moui_draw_set_fn)(moui_draw_ctx_t *ctx, int x, int y, moui_color_t color);
typedef moui_color_t (*moui_draw_get_fn)(moui_draw_ctx_t *ctx, int x, int y);

struct moui_draw_ctx {
    moui_fb_t      *fb;
    moui_backend_t *be;
    moui_draw_set_fn set_fn;
    moui_draw_get_fn get_fn;
    moui_rect_t clip;
    moui_rect_t clip_stack[MOUI_CLIP_STACK_DEPTH];
    uint8_t    clip_sp;
    bool       xor_mode;
};

void moui_draw_ctx_init(moui_draw_ctx_t *ctx, moui_fb_t *fb);
void moui_draw_ctx_init_be(moui_draw_ctx_t *ctx, moui_backend_t *be);

void moui_draw_pixel     (moui_draw_ctx_t *ctx, int x, int y, moui_color_t color);
void moui_draw_hline     (moui_draw_ctx_t *ctx, int x, int y, int len, moui_color_t color);
void moui_draw_vline     (moui_draw_ctx_t *ctx, int x, int y, int len, moui_color_t color);
void moui_draw_line      (moui_draw_ctx_t *ctx, int x0, int y0, int x1, int y1, moui_color_t color);
void moui_draw_rect      (moui_draw_ctx_t *ctx, const moui_rect_t *r, moui_color_t color);
void moui_draw_fill_rect (moui_draw_ctx_t *ctx, const moui_rect_t *r, moui_color_t color);
void moui_draw_circle    (moui_draw_ctx_t *ctx, int cx, int cy, int radius, moui_color_t color);
void moui_draw_fill_circle(moui_draw_ctx_t *ctx, int cx, int cy, int radius, moui_color_t color);
void moui_draw_arc(moui_draw_ctx_t *ctx, int cx, int cy, int radius,
                  int start_deg, int end_deg, moui_color_t color);
void moui_draw_rounded_rect(moui_draw_ctx_t *ctx, const moui_rect_t *r, int radius, moui_color_t color);
void moui_draw_fill_rounded_rect(moui_draw_ctx_t *ctx, const moui_rect_t *r, int radius, moui_color_t color);
void moui_draw_bitmap    (moui_draw_ctx_t *ctx, int x, int y, int w, int h,
                         const uint8_t *bmp, moui_color_t color);
void moui_draw_bitmap_scaled(moui_draw_ctx_t *ctx, int dst_x, int dst_y, int dst_w, int dst_h,
                          int src_w, int src_h, const uint8_t *bmp, moui_color_t color);
void moui_draw_bitmap_rle(moui_draw_ctx_t *ctx, int x, int y, int w, int h,
                         const uint8_t *rle_data, uint32_t rle_len, moui_color_t color);

void moui_draw_push_clip(moui_draw_ctx_t *ctx, const moui_rect_t *clip);
void moui_draw_pop_clip (moui_draw_ctx_t *ctx);

static inline void moui_draw_set_xor(moui_draw_ctx_t *ctx, bool xor_on) { ctx->xor_mode = xor_on; }


#ifdef __cplusplus
}
#endif
#endif
