#include "sim_display.h"
#include "sim_input.h"
#include "../src/moui.h"
#include "../src/font/moui_font.h"
#include "demo_screens.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * 4.2" ST7305 Mono Display & Touch Simulator (300x400)
 * Full Moui Demo Suite (15 Screens) + Capacitive Touch (Touch/Drag/Paint)
 * Screen Resolution: 300 x 400 (ST7305_4x2)
 */

static sim_display_t     g_display;
static moui_fb_t          g_fb;
static moui_screen_mgr_t  g_mgr;

static void sim_flush(const moui_hal_t *hal, const uint8_t *fb, uint32_t len)
{
    (void)hal; (void)len;
    sim_display_render(&g_display, fb);
}

static moui_time_ms_t sim_get_time(const moui_hal_t *hal) { (void)hal; return SDL_GetTicks(); }
static void sim_delay(const moui_hal_t *hal, uint32_t ms) { (void)hal; SDL_Delay(ms); }
static void sim_log(const moui_hal_t *hal, const char *fmt, ...)
{ (void)hal; va_list a; va_start(a, fmt); vprintf(fmt, a); va_end(a); printf("\n"); }

static const moui_hal_t g_hal = {
    .display_flush = sim_flush,
    .get_time_ms   = sim_get_time,
    .delay_ms      = sim_delay,
    .log           = sim_log,
};

/* ── Interactive Touch Paint Pad Screen ───────────────────────────────── */

static moui_screen_t        scr_paint;
static moui_widget_button_t btn_paint_clear;
static moui_widget_button_t btn_paint_back;
static moui_widget_label_t  lbl_paint_title;
static moui_widget_t        w_paint_canvas;
static uint8_t             *g_canvas_mem = NULL;

static void paint_canvas_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    (void)w;
    int dw = moui_disp_w();
    moui_rect_t r = { 10, 50, (int16_t)(dw - 20), 290 };
    moui_draw_rect(ctx, &r, MOUI_BLACK);

    if (g_canvas_mem) {
        int cw = dw - 22;
        int ch = 288;
        for (int y = 0; y < ch; y++) {
            for (int x = 0; x < cw; x++) {
                int idx = (y * cw + x) >> 3;
                uint8_t mask = 0x80 >> ((y * cw + x) & 7);
                if (g_canvas_mem[idx] & mask)
                    moui_draw_pixel(ctx, 11 + x, 51 + y, MOUI_BLACK);
            }
        }
    }
}

static bool paint_canvas_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    (void)w;
    if (!g_canvas_mem) return false;
    int dw = moui_disp_w();
    int cw = dw - 22;
    int ch = 288;

    if (ev->type == MOUI_EV_POINTER_DOWN || ev->type == MOUI_EV_POINTER_MOVE) {
        int cx = ev->x - 11;
        int cy = ev->y - 51;
        if (cx >= 0 && cx < cw && cy >= 0 && cy < ch) {
            for (int dy = -2; dy <= 2; dy++) {
                for (int dx = -2; dx <= 2; dx++) {
                    int px = cx + dx, py = cy + dy;
                    if (px >= 0 && px < cw && py >= 0 && py < ch) {
                        int idx = (py * cw + px) >> 3;
                        uint8_t mask = 0x80 >> ((py * cw + px) & 7);
                        g_canvas_mem[idx] |= mask;
                    }
                }
            }
            return true;
        }
    }
    return false;
}

static const moui_widget_vtable_t vt_paint = {
    .draw = paint_canvas_draw,
    .on_event = paint_canvas_event
};

static void on_paint_clear(moui_widget_button_t *btn)
{
    (void)btn;
    int dw = moui_disp_w();
    if (g_canvas_mem) memset(g_canvas_mem, 0, ((dw - 22) * 288 + 7) / 8);
}

static void on_paint_back(moui_widget_button_t *btn)
{
    (void)btn;
    moui_screen_pop(&g_mgr);
}

static void setup_paint_screen(void)
{
    int dw = moui_disp_w();
    int cw = dw - 22;
    int ch = 288;
    g_canvas_mem = calloc(1, (cw * ch + 7) / 8);

    moui_screen_init(&scr_paint);
    scr_paint.enter_trans = MOUI_TRANS_SLIDE_LEFT;

    moui_label_init(&lbl_paint_title, "Touch Paint Pad (300x400)", &moui_font_inter_16);
    lbl_paint_title.base.bounds = (moui_rect_t){ 10, 20, (int16_t)(dw - 20), 24 };
    moui_screen_add_widget(&scr_paint, &lbl_paint_title.base);

    moui_widget_init(&w_paint_canvas, &vt_paint);
    w_paint_canvas.bounds = (moui_rect_t){ 10, 50, (int16_t)(dw - 20), (int16_t)ch };
    moui_screen_add_widget(&scr_paint, &w_paint_canvas);

    int btn_w = (dw - 30) / 2;
    moui_button_init(&btn_paint_clear, "Clear", &moui_font_inter_16, MOUI_BTN_PUSH);
    btn_paint_clear.base.bounds = (moui_rect_t){ 10, 350, (int16_t)btn_w, 40 };
    btn_paint_clear.on_click = on_paint_clear;
    moui_screen_add_widget(&scr_paint, &btn_paint_clear.base);

    moui_button_init(&btn_paint_back, "Back", &moui_font_inter_16, MOUI_BTN_PUSH);
    btn_paint_back.base.bounds = (moui_rect_t){ (int16_t)(20 + btn_w), 350, (int16_t)btn_w, 40 };
    btn_paint_back.on_click = on_paint_back;
    moui_screen_add_widget(&scr_paint, &btn_paint_back.base);
}

/* PBM Screenshot Helper */
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
    moui_hal_set_display(&moui_disp_st7305_4p2);
    if (moui_fb_init(&g_fb, &moui_disp_st7305_4p2) < 0) return 1;
    if (sim_display_init(&g_display) < 0) { SDL_Quit(); return 1; }

    moui_screen_mgr_init(&g_mgr, &g_fb, &g_hal);
    demo_init(&g_mgr);
    demo_setup_all_screens();

    moui_screen_push(&g_mgr, demo_get_main_screen());
    moui_screen_mgr_render_once(&g_mgr, 100);

    char path[256];
    snprintf(path, sizeof(path), "%s/st7305_4p2_main.pbm", outdir);
    dump_fb_pbm(&g_fb, path);
    printf("  %s\n", path);

    sim_display_destroy(&g_display);
    SDL_Quit();
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc >= 3 && strcmp(argv[1], "--screenshot") == 0) {
        return run_screenshots(argv[2]);
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    const moui_disp_desc_t *disp_desc = &moui_disp_st7305_4p2;
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

    /* Full Moui Demo Suite Setup */
    demo_init(&g_mgr);
    demo_setup_all_screens();
    setup_paint_screen();

    /* Push main menu screen */
    moui_screen_push(&g_mgr, demo_get_main_screen());

    printf("=========================================================\n");
    printf(" ST7305 4.2\" Reflective LCD (RLCD) Simulator (300x400)   \n");
    printf("=========================================================\n");
    printf(" Full Moui Demo Suite (17 Screens + Touch Paint Pad)    \n");
    printf("  Touch / Mouse Click   : Touch Menu Items & Select      \n");
    printf("  Touch Drag            : Scroll Lists & Drag Sliders    \n");
    printf("  Arrow Keys / Enter    : Rotary Encoder Navigation       \n");
    printf("  R                     : Rotate Screen Orientation      \n");
    printf("  ESC                   : Back / Quit                    \n");
    printf("=========================================================\n");

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
            if (sdl_ev.type == SDL_KEYDOWN && sdl_ev.key.keysym.sym == SDLK_p) {
                moui_screen_push(&g_mgr, &scr_paint);
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

    if (g_canvas_mem) free(g_canvas_mem);
    sim_display_destroy(&g_display);
    SDL_Quit();
    return 0;
}
