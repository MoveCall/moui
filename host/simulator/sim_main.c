#include "sim_display.h"
#include "sim_input.h"
#include "../src/moui.h"
#include "demo_screens.h"
#include <stdio.h>
#include <string.h>

static sim_display_t     g_display;
static moui_fb_t          g_fb;
static moui_screen_mgr_t  g_mgr;

static void sim_flush(const moui_hal_t *hal, const uint8_t *fb, uint32_t len)
{ (void)hal; (void)len; sim_display_render(&g_display, fb); }
static moui_time_ms_t sim_get_time(const moui_hal_t *hal) { (void)hal; return SDL_GetTicks(); }
static void sim_delay(const moui_hal_t *hal, uint32_t ms) { (void)hal; SDL_Delay(ms); }
static void sim_log(const moui_hal_t *hal, const char *fmt, ...)
{ (void)hal; va_list a; va_start(a, fmt); vprintf(fmt, a); va_end(a); printf("\n"); }

static const moui_hal_t g_hal = {
    .display_flush = sim_flush, .get_time_ms = sim_get_time,
    .delay_ms = sim_delay, .log = sim_log,
};

static void dump_fb_pbm(const moui_fb_t *fb, const char *path)
{
    int w = moui_disp_w(), h = moui_disp_h();
    FILE *f = fopen(path, "w");
    if (!f) { fprintf(stderr, "Cannot open %s\n", path); return; }
    fprintf(f, "P1\n%d %d\n", w, h);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            fprintf(f, "%d ", moui_fb_get_pixel(fb, x, y) ? 1 : 0);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

static int run_screenshots(const char *outdir)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;
    moui_hal_set_display(&moui_disp_st7305_2p9);
    if (moui_fb_init(&g_fb, &moui_disp_st7305_2p9) < 0) return 1;
    if (sim_display_init(&g_display) < 0) { SDL_Quit(); return 1; }

    moui_screen_mgr_init(&g_mgr, &g_fb, &g_hal);
    demo_init(&g_mgr);
    demo_setup_all_screens();

    /* Push main screen and render */
    moui_screen_push(&g_mgr, demo_get_main_screen());
    moui_screen_mgr_render_once(&g_mgr, 100);

    char path[256];
    snprintf(path, sizeof(path), "%s/main_menu.pbm", outdir);
    dump_fb_pbm(&g_fb, path);
    printf("  %s\n", path);

    /* Simulate selecting each menu item (push each sub-screen) */
    moui_input_event_t press = { .type = MOUI_EV_ENCODER_PRESS, .timestamp_ms = 200 };
    moui_input_event_t cw = { .type = MOUI_EV_ENCODER_CW, .timestamp_ms = 200 };

    const char *page_names[] = {
        "dashboard", "settings", "waveform", "graphics", "stress",
        "widgets1", "widgets2", "widgets3", "widgets4",
        "mono", "image", "features", "icons", "about"
    };

    for (int i = 0; i < 14; i++) {
        /* Navigate to item i and select */
        moui_screen_mgr_init(&g_mgr, &g_fb, &g_hal);
        demo_init(&g_mgr);
        moui_screen_push(&g_mgr, demo_get_main_screen());

        /* Push CW events to scroll to item i, then press */
        for (int j = 0; j < i; j++) {
            moui_input_queue_push(moui_screen_mgr_input(&g_mgr), &cw);
        }
        moui_screen_mgr_tick(&g_mgr, 200 + i * 100, 16.0f);
        moui_input_queue_push(moui_screen_mgr_input(&g_mgr), &press);
        moui_screen_mgr_tick(&g_mgr, 300 + i * 100, 16.0f);
        moui_screen_mgr_tick(&g_mgr, 900 + i * 100, 500.0f);

        /* Render the sub-screen */
        moui_screen_mgr_render_once(&g_mgr, 500 + i * 100);

        snprintf(path, sizeof(path), "%s/%s.pbm", outdir, page_names[i]);
        dump_fb_pbm(&g_fb, path);
        printf("  %s\n", path);
    }

    sim_display_destroy(&g_display);
    SDL_Quit();
    printf("Screenshots: %d pages → %s/\n", 15, outdir);
    return 0;
}

int main(int argc, char *argv[])
{
    /* Headless screenshot mode */
    if (argc >= 3 && strcmp(argv[1], "--screenshot") == 0) {
        return run_screenshots(argv[2]);
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;

    moui_hal_set_display(&moui_disp_st7305_2p9);
    if (moui_fb_init(&g_fb, &moui_disp_st7305_2p9) < 0) return 1;
    if (sim_display_init(&g_display) < 0) { SDL_Quit(); return 1; }

    moui_screen_mgr_init(&g_mgr, &g_fb, &g_hal);
    demo_init(&g_mgr);
    demo_setup_all_screens();
    moui_screen_push(&g_mgr, demo_get_main_screen());

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
        if (elapsed < 20) SDL_Delay(20 - elapsed);
    }

    sim_display_destroy(&g_display);
    SDL_Quit();
    return 0;
}
