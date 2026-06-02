#include "sim_display.h"
#include "sim_input.h"
#include "../src/moui.h"
#include "../src/font/moui_font.h"
#include "demo_screens.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/*
 * SSD1677 4.26" E-Paper simulator
 * Normal mode: B/W demo screens (480x800, 1bpp)
 * --4g mode: Two-page 4-level grayscale demo (480x800, 2bpp)
 */

#define EPD_FULL_REFRESH_MS   1500
#define EPD_PARTIAL_REFRESH_MS 300
#define EPD_FLASH_MS           150
#define EPD_DIRTY_THRESHOLD    50

static sim_display_t     g_display;
static moui_fb_t          g_fb;
static moui_screen_mgr_t  g_mgr;
static uint8_t          *g_prev_fb;
static uint32_t          g_fb_size;
static int               g_partial_count;
static bool              g_4g_mode = false;
static bool              g_screenshot = false;

static void render_solid(uint32_t color)
{
    uint16_t dw = moui_disp_w(), dh = moui_disp_h();
    int npix = dw * dh;
    uint32_t *out = g_display.pixels;
    for (int i = 0; i < npix; i++)
        out[i] = 0xFF000000 | color;
    SDL_UpdateTexture(g_display.texture, NULL, out, dw * sizeof(uint32_t));
    SDL_RenderClear(g_display.renderer);
    SDL_RenderCopy(g_display.renderer, g_display.texture, NULL, NULL);
    SDL_RenderPresent(g_display.renderer);
}

static void epd_flash_effect(void)
{
    render_solid(SIM_FG_COLOR);
    SDL_Delay(EPD_FLASH_MS);
    render_solid(SIM_BG_COLOR);
    SDL_Delay(EPD_FLASH_MS);
}

static void epd_refresh(const uint8_t *wire, bool full)
{
    if (full) {
        epd_flash_effect();
        g_partial_count = 0;
    }
    sim_display_render(&g_display, wire);
    uint32_t delay = full ? (EPD_FULL_REFRESH_MS - EPD_FLASH_MS * 2) : EPD_PARTIAL_REFRESH_MS;
    SDL_Delay(delay);
}

static bool need_full_refresh(const uint8_t *cur, const uint8_t *prev, uint32_t size)
{
    uint32_t diff_bytes = 0;
    for (uint32_t i = 0; i < size; i++) {
        if (cur[i] != prev[i]) diff_bytes++;
    }
    return (diff_bytes * 100 / size) > EPD_DIRTY_THRESHOLD;
}

static void sim_flush(const moui_hal_t *hal, const uint8_t *fb, uint32_t len)
{
    (void)hal;

    if (!g_prev_fb) {
        epd_refresh(fb, true);
        g_prev_fb = malloc(len);
        g_fb_size = len;
        memcpy(g_prev_fb, fb, len);
        return;
    }

    if (memcmp(fb, g_prev_fb, len) == 0)
        return;

    bool full = need_full_refresh(fb, g_prev_fb, len) || g_partial_count >= 10;
    epd_refresh(fb, full);
    if (!full) g_partial_count++;
    memcpy(g_prev_fb, fb, len);
}

static moui_time_ms_t sim_get_time(const moui_hal_t *hal) { (void)hal; return SDL_GetTicks(); }
static void sim_delay(const moui_hal_t *hal, uint32_t ms) { (void)hal; SDL_Delay(ms); }
static void sim_log(const moui_hal_t *hal, const char *fmt, ...)
{ (void)hal; va_list a; va_start(a, fmt); vprintf(fmt, a); va_end(a); printf("\n"); }

static const moui_hal_t g_hal = {
    .display_flush = sim_flush, .get_time_ms = sim_get_time,
    .delay_ms = sim_delay, .log = sim_log,
};

/* ── 4G Mode: Two-page demo ─────────────────────────────────────────────── */

static moui_screen_t  scr_4g_gray;
static moui_screen_t  scr_4g_fonts;
static moui_widget_t  w_gray_canvas;
static moui_widget_t  w_font_canvas;

static void draw_gray_page(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    (void)w;
    int dw = moui_disp_w(), dh = moui_disp_h();

    moui_font_draw_str(ctx, &moui_font_inter_24_aa, 20, 8,
                       "4-Level Grayscale Demo", MOUI_BLACK);

    int y0 = 44;
    int bar_h = 120;
    moui_color_t colors[4] = { MOUI_WHITE, MOUI_LGRAY, MOUI_DGRAY, MOUI_BLACK };
    const char *names[4] = { "WHITE (0)", "LGRAY (1)", "DGRAY (2)", "BLACK (3)" };

    for (int i = 0; i < 4; i++) {
        moui_rect_t r = { 20, y0 + i * bar_h, dw - 40, bar_h - 4 };
        moui_draw_fill_rect(ctx, &r, colors[i]);

        moui_color_t tc = (i < 2) ? MOUI_BLACK : MOUI_WHITE;
        moui_font_draw_str(ctx, &moui_font_inter_16_aa, 40,
                           y0 + i * bar_h + bar_h / 2 - 8, names[i], tc);
    }

    int circle_y = y0 + bar_h * 4 + 20;
    moui_font_draw_str(ctx, &moui_font_inter_16_aa, 20, circle_y,
                       "Gradient circle (4 gray levels):", MOUI_BLACK);

    int cx = dw / 2, cy = circle_y + 70;
    int radius = 50;
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            float dist2 = (float)(dx * dx + dy * dy);
            float r2 = (float)(radius * radius);
            if (dist2 <= r2) {
                int level = (int)(dist2 / r2 * 4.0f);
                if (level > 3) level = 3;
                moui_draw_pixel(ctx, cx + dx, cy + dy, (moui_color_t)level);
            }
        }
    }

    char footer[64];
    snprintf(footer, sizeof(footer), "Page 1/2  |  Up/Down: switch page");
    moui_font_draw_str(ctx, &moui_font_inter_16_aa, 20, dh - 24, footer, MOUI_DGRAY);
}

static void draw_font_page(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    (void)w;
    int dw = moui_disp_w(), dh = moui_disp_h();

    moui_font_draw_str(ctx, &moui_font_inter_24_aa, 20, 4,
                       "Font Anti-Aliasing Comparison", MOUI_BLACK);

    int y = 50;
    int col2 = dw / 2;

    moui_font_draw_str(ctx, &moui_font_inter_16_aa, 20, y, "-- 1bpp (normal) --", MOUI_DGRAY);
    moui_font_draw_str(ctx, &moui_font_inter_16_aa, col2, y, "-- 2bpp (AA) --", MOUI_DGRAY);
    y += 28;

    moui_font_draw_str(ctx, &moui_font_inter_24, 20, y,
                       "Hello World", MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_inter_24_aa, col2, y,
                       "Hello World", MOUI_BLACK);
    y += 34;

    moui_font_draw_str(ctx, &moui_font_inter_16, 20, y,
                       "The quick brown fox", MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_inter_16_aa, col2, y,
                       "The quick brown fox", MOUI_BLACK);
    y += 24;

    moui_font_draw_str(ctx, &moui_font_inter_16, 20, y,
                       "jumps over the lazy", MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_inter_16_aa, col2, y,
                       "jumps over the lazy", MOUI_BLACK);
    y += 24;

    moui_font_draw_str(ctx, &moui_font_inter_16, 20, y,
                       "dog. 0123456789", MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_inter_16_aa, col2, y,
                       "dog. 0123456789", MOUI_BLACK);
    y += 34;

    moui_font_draw_str(ctx, &moui_font_inter_24, 20, y,
                       "ABCDEFGHIJ", MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_inter_24_aa, col2, y,
                       "ABCDEFGHIJ", MOUI_BLACK);
    y += 34;

    moui_font_draw_str(ctx, &moui_font_inter_24, 20, y,
                       "abcdefghij", MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_inter_24_aa, col2, y,
                       "abcdefghij", MOUI_BLACK);
    y += 40;

    moui_rect_t sep = { 10, y, dw - 20, 1 };
    moui_draw_fill_rect(ctx, &sep, MOUI_DGRAY);
    y += 10;

    moui_font_draw_str(ctx, &moui_font_inter_16_aa, 20, y,
                       "AA uses gray pixels (LGRAY/DGRAY) at edges", MOUI_BLACK);
    y += 20;
    moui_font_draw_str(ctx, &moui_font_inter_16_aa, 20, y,
                       "for smooth rendering on 4-level EPD.", MOUI_BLACK);

    char footer[64];
    snprintf(footer, sizeof(footer), "Page 2/2  |  Up/Down: switch page");
    moui_font_draw_str(ctx, &moui_font_inter_16_aa, 20, dh - 24, footer, MOUI_DGRAY);
}

static bool gray_page_event(moui_screen_t *s, const moui_input_event_t *ev)
{
    (void)s;
    if (ev->type == MOUI_EV_ENCODER_CW) {
        moui_screen_replace(&g_mgr, &scr_4g_fonts);
        return true;
    }
    return false;
}

static bool font_page_event(moui_screen_t *s, const moui_input_event_t *ev)
{
    (void)s;
    if (ev->type == MOUI_EV_ENCODER_CCW) {
        moui_screen_replace(&g_mgr, &scr_4g_gray);
        return true;
    }
    return false;
}

static const moui_widget_vtable_t vt_gray = { .draw = draw_gray_page };
static const moui_widget_vtable_t vt_font = { .draw = draw_font_page };

static void setup_4g_screens(void)
{
    int dw = moui_disp_w(), dh = moui_disp_h();

    moui_screen_init(&scr_4g_gray);
    moui_widget_init(&w_gray_canvas, &vt_gray);
    w_gray_canvas.bounds = (moui_rect_t){ 0, 0, dw, dh };
    moui_screen_add_widget(&scr_4g_gray, &w_gray_canvas);
    scr_4g_gray.on_event = gray_page_event;

    moui_screen_init(&scr_4g_fonts);
    moui_widget_init(&w_font_canvas, &vt_font);
    w_font_canvas.bounds = (moui_rect_t){ 0, 0, dw, dh };
    moui_screen_add_widget(&scr_4g_fonts, &w_font_canvas);
    scr_4g_fonts.on_event = font_page_event;
    scr_4g_fonts.enter_trans = MOUI_TRANS_SLIDE_LEFT;
}

/* ── Main ────────────────────────────────────────────────────────────────── */

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--4g") == 0 || strcmp(argv[i], "-4g") == 0)
            g_4g_mode = true;
        if (strcmp(argv[i], "--screenshot") == 0)
            g_screenshot = true;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    const moui_disp_desc_t *disp_desc = g_4g_mode
        ? &moui_disp_ssd1677_4p26_4g
        : &moui_disp_ssd1677_4p26;

    moui_hal_set_display(disp_desc);
    if (moui_fb_init(&g_fb, disp_desc) < 0) {
        fprintf(stderr, "Framebuffer init failed\n");
        return 1;
    }
    if (sim_display_init(&g_display) < 0) {
        SDL_Quit();
        return 1;
    }

    moui_screen_mgr_init(&g_mgr, &g_fb, &g_hal);

    if (g_4g_mode) {
        setup_4g_screens();
        moui_screen_push(&g_mgr, &scr_4g_gray);

        printf("SSD1677 4.26\" EPD Simulator - 4-Level Grayscale (480x800, 2bpp)\n");
        printf("  Up/Down: switch page  |  ESC: quit\n");
    } else {
        demo_init(&g_mgr);
        demo_setup_all_screens();
        moui_screen_push(&g_mgr, demo_get_main_screen());

        printf("SSD1677 4.26\" EPD Simulator (480x800)\n");
        printf("  Arrow keys: navigate  |  Enter: select  |  ESC: back  |  R: rotate\n");
        printf("  Use --4g flag for 4-level grayscale demo\n");
    }

    uint32_t last_tick = SDL_GetTicks();
    bool quit = false;
    bool screenshot_taken = false;

    while (!quit) {
        SDL_Event sdl_ev;
        while (SDL_PollEvent(&sdl_ev)) {
            if (sdl_ev.type == SDL_QUIT) { quit = true; break; }
            if (sdl_ev.type == SDL_KEYDOWN && sdl_ev.key.keysym.sym == SDLK_ESCAPE
                && g_mgr.depth <= 1) {
                quit = true; break;
            }
            if (!g_4g_mode && sdl_ev.type == SDL_KEYDOWN
                && sdl_ev.key.keysym.sym == SDLK_r) {
                sim_display_rotate(&g_display);
                continue;
            }
            sim_input_handle_event(&sdl_ev, moui_screen_mgr_input(&g_mgr));
        }

        uint32_t now = SDL_GetTicks();
        float dt = (float)(now - last_tick);
        if (dt < 1.0f) dt = 1.0f;
        last_tick = now;

        moui_screen_mgr_tick(&g_mgr, now, dt);

        if (g_screenshot && !screenshot_taken) {
            screenshot_taken = true;
            SDL_Surface *srf = SDL_CreateRGBSurfaceFrom(
                g_display.pixels, moui_disp_w(), moui_disp_h(), 32,
                moui_disp_w() * 4, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
            if (srf) {
                SDL_SaveBMP(srf, "epd_4g_page1.bmp");
                printf("Screenshot saved: epd_4g_page1.bmp\n");
                SDL_FreeSurface(srf);
            }
        }

        uint32_t elapsed = SDL_GetTicks() - now;
        if (elapsed < 50) SDL_Delay(50 - elapsed);
    }

    free(g_prev_fb);
    sim_display_destroy(&g_display);
    SDL_Quit();
    return 0;
}
