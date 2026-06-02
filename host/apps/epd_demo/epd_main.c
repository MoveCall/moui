#include "sim_display.h"
#include "sim_input.h"
#include "../src/moui.h"
#include "demo_screens.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * E-Paper simulator — mimics real SSD1677 refresh behavior:
 *   - Compares current frame vs previous to detect changes
 *   - Small change → partial refresh (300ms, no flash)
 *   - Large change or first frame → full refresh (1.5s, black flash)
 *   - No change → skip refresh entirely (zero CPU, like real EPD)
 */

#define EPD_FULL_REFRESH_MS   1500
#define EPD_PARTIAL_REFRESH_MS 300
#define EPD_FLASH_MS           150
#define EPD_DIRTY_THRESHOLD    50   /* >50% changed pixels → full refresh */

static sim_display_t     g_display;
static moui_fb_t          g_fb;
static moui_screen_mgr_t  g_mgr;
static uint8_t          *g_prev_fb;
static uint32_t          g_fb_size;
static int               g_partial_count;

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

static bool fb_changed(const uint8_t *cur, const uint8_t *prev, uint32_t size)
{
    return memcmp(cur, prev, size) != 0;
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

    if (!fb_changed(fb, g_prev_fb, len))
        return;

    /* Every 10 partial refreshes, force a full refresh to clear ghosting */
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

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    moui_hal_set_display(&moui_disp_ssd1677_3p7);
    if (moui_fb_init(&g_fb, &moui_disp_ssd1677_3p7) < 0) {
        fprintf(stderr, "Framebuffer init failed\n");
        return 1;
    }
    if (sim_display_init(&g_display) < 0) {
        SDL_Quit();
        return 1;
    }

    moui_screen_mgr_init(&g_mgr, &g_fb, &g_hal);
    demo_init(&g_mgr);
    demo_setup_all_screens();
    moui_screen_push(&g_mgr, demo_get_main_screen());

    printf("EPD 3.7\" Simulator (280x480)\n");
    printf("  Arrow keys: navigate  |  Enter: select  |  ESC: back  |  R: rotate\n");
    printf("  Simulating e-paper refresh delays\n");

    uint32_t last_tick = SDL_GetTicks();
    bool quit = false;

    while (!quit) {
        SDL_Event sdl_ev;
        while (SDL_PollEvent(&sdl_ev)) {
            if (sdl_ev.type == SDL_QUIT) { quit = true; break; }
            if (sdl_ev.type == SDL_KEYDOWN && sdl_ev.key.keysym.sym == SDLK_ESCAPE
                && g_mgr.depth <= 1) {
                quit = true; break;
            }
            if (sdl_ev.type == SDL_KEYDOWN && sdl_ev.key.keysym.sym == SDLK_r) {
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

        uint32_t elapsed = SDL_GetTicks() - now;
        if (elapsed < 50) SDL_Delay(50 - elapsed);
    }

    free(g_prev_fb);
    sim_display_destroy(&g_display);
    SDL_Quit();
    return 0;
}
