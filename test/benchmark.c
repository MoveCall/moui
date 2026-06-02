#include "../src/moui.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

static moui_fb_t g_fb;
static moui_draw_ctx_t g_ctx;

static double time_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

#define BENCH(name, iterations, code) do { \
    double _start = time_ms(); \
    for (int _i = 0; _i < (iterations); _i++) { code; } \
    double _elapsed = time_ms() - _start; \
    printf("  %-35s %6d iters  %8.2f ms  %6.1f us/iter\n", \
           name, iterations, _elapsed, _elapsed * 1000.0 / (iterations)); \
} while(0)

int main(void)
{
    moui_hal_set_display(&moui_disp_st7305_2p9);
    moui_fb_init(&g_fb, &moui_disp_st7305_2p9);
    moui_draw_ctx_init(&g_ctx, &g_fb);
    moui_theme_set_default();

    printf("=== Moui Performance Benchmark ===\n");
    printf("Display: %d x %d (1-bit)\n\n", MOUI_DISP_W, MOUI_DISP_H);

    printf("[Drawing Primitives]\n");
    BENCH("fb_clear", 10000,
        moui_fb_clear(&g_fb, false));
    BENCH("pixel (single)", 100000,
        moui_draw_pixel(&g_ctx, 84, 192, true));
    BENCH("hline (full width 168px)", 50000,
        moui_draw_hline(&g_ctx, 0, 100, 168, true));
    BENCH("vline (full height 384px)", 50000,
        moui_draw_vline(&g_ctx, 84, 0, 384, true));
    BENCH("line (diagonal 168px)", 10000,
        moui_draw_line(&g_ctx, 0, 0, 167, 383, true));
    BENCH("rect (80x40)", 20000,
        moui_draw_rect(&g_ctx, &(moui_rect_t){10, 10, 80, 40}, true));
    BENCH("fill_rect (80x40)", 10000,
        moui_draw_fill_rect(&g_ctx, &(moui_rect_t){10, 10, 80, 40}, true));
    BENCH("circle (r=30)", 10000,
        moui_draw_circle(&g_ctx, 84, 192, 30, true));
    BENCH("fill_circle (r=30)", 5000,
        moui_draw_fill_circle(&g_ctx, 84, 192, 30, true));
    BENCH("rounded_rect (80x40 r=5)", 10000,
        moui_draw_rounded_rect(&g_ctx, &(moui_rect_t){10, 10, 80, 40}, 5, true));
    BENCH("fill_rounded_rect (80x40 r=5)", 5000,
        moui_draw_fill_rounded_rect(&g_ctx, &(moui_rect_t){10, 10, 80, 40}, 5, true));

    printf("\n[Font Rendering]\n");
    BENCH("ascii 6x8 'Hello World'", 20000,
        moui_font_draw_str(&g_ctx, &moui_font_ascii_6x8, 10, 10, "Hello World", true));
    BENCH("inter_16 'Hello World'", 20000,
        moui_font_draw_str(&g_ctx, &moui_font_inter_16, 10, 10, "Hello World", true));
    BENCH("measure_str 'Testing123'", 100000,
        moui_font_measure_str(&moui_font_ascii_6x8, "Testing123"));

    printf("\n[Mono Effects]\n");
    BENCH("dither_fill_rect (80x40)", 5000,
        moui_draw_fill_rect_dither(&g_ctx, &(moui_rect_t){10, 10, 80, 40}, MOUI_DITHER_50));
    BENCH("pattern_fill_rect (80x40)", 5000,
        moui_draw_fill_rect_pattern(&g_ctx, &(moui_rect_t){10, 10, 80, 40}, MOUI_PAT_CROSS_HATCH, 4));
    BENCH("qr_encode 'Hello'", 1000, {
        moui_qr_t qr; moui_qr_encode(&qr, "Hello"); });

    printf("\n[Framebuffer]\n");
    BENCH("fb_pack_wire (ST7305 4x2)", 5000,
        moui_fb_pack_wire(&g_fb));

    printf("\n[Full Frame Render]\n");
    BENCH("clear + 10 hlines + 5 rects + 3 circles + text", 2000, {
        moui_fb_clear(&g_fb, false);
        for (int j = 0; j < 10; j++) moui_draw_hline(&g_ctx, 0, j*38, 168, true);
        for (int j = 0; j < 5; j++) moui_draw_fill_rect(&g_ctx, &(moui_rect_t){j*30, j*60, 40, 30}, true);
        for (int j = 0; j < 3; j++) moui_draw_fill_circle(&g_ctx, 84, 100+j*80, 20, true);
        moui_font_draw_str(&g_ctx, &moui_font_ascii_6x8, 10, 10, "Benchmark Frame", true);
    });

    moui_fb_deinit(&g_fb);
    printf("\nDone.\n");
    return 0;
}
