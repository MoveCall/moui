/*
 * Moui Smartwatch — Premium Edition
 *
 * Design philosophy: Braun/Dieter Rams "Less, but better"
 * - 48px time digits as hero element
 * - Strict 8px grid system
 * - Generous whitespace
 * - Precise geometric details
 * - 200x200 1.54" full-reflective display
 */

#include "../../src/moui.h"
#include "../simulator/sim_display.h"
#include "../simulator/sim_input.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

#define W 200
#define H 200
#define CX 100
#define CY 100
#define PI 3.14159f
#define PAD 16

static sim_display_t    g_display;
static moui_fb_t         g_fb;
static moui_screen_mgr_t g_mgr;

static void sim_flush(const moui_hal_t *h, const uint8_t *fb, uint32_t len)
{ (void)h; (void)len; sim_display_render(&g_display, fb); }
static moui_time_ms_t sim_time(const moui_hal_t *h) { (void)h; return SDL_GetTicks(); }
static void sim_delay(const moui_hal_t *h, uint32_t ms) { (void)h; SDL_Delay(ms); }
static void sim_log(const moui_hal_t *h, const char *f, ...)
{ (void)h; va_list a; va_start(a,f); vprintf(f,a); va_end(a); printf("\n"); }

static const moui_hal_t g_hal = {
    .display_flush=sim_flush, .get_time_ms=sim_time,
    .delay_ms=sim_delay, .log=sim_log,
};

/* ── Simulated data ── */
static int g_hour=10, g_min=24, g_sec=37;
static int g_steps=6842, g_batt=82;
static float g_heart=72, g_time_f=0;
static int g_temp=23;

static void sim_tick(uint32_t now_ms)
{
    g_time_f = (float)now_ms / 1000.0f;
    g_sec = (37 + (int)(g_time_f)) % 60;
    g_min = (24 + (int)(g_time_f / 60)) % 60;
    g_hour = (10 + (int)(g_time_f / 3600)) % 24;
    g_steps = 6842 + (int)(g_time_f * 1.5f);
    g_batt = 82 - (int)(g_time_f * 0.005f) % 15;
    g_heart = 72 + sinf(g_time_f * 0.4f) * 12;
    g_temp = 23 + (int)(sinf(g_time_f * 0.08f) * 4);
}

/* ══════════════════════════════════════════
 *  Helpers
 * ══════════════════════════════════════════ */
static void draw_str_center(moui_draw_ctx_t *ctx, const moui_font_t *f,
                            int y, const char *s, moui_color_t color)
{
    int tw = moui_font_measure_str(f, s);
    moui_font_draw_str(ctx, f, CX - tw / 2, y, s, color);
}

static void draw_str_right(moui_draw_ctx_t *ctx, const moui_font_t *f,
                           int x_right, int y, const char *s, moui_color_t color)
{
    int tw = moui_font_measure_str(f, s);
    moui_font_draw_str(ctx, f, x_right - tw, y, s, color);
}

/* Filled triangle pointer (for clock hands) */
static void draw_thick_hand(moui_draw_ctx_t *ctx, int cx, int cy,
                            float angle, int length, int width, moui_color_t color)
{
    float sa = sinf(angle), ca = cosf(angle);
    /* Tip */
    int tx = cx + (int)(sa * length);
    int ty = cy - (int)(ca * length);
    /* Base left/right */
    float perp = angle + PI / 2;
    int bx1 = cx + (int)(sinf(perp) * width);
    int by1 = cy - (int)(cosf(perp) * width);
    int bx2 = cx - (int)(sinf(perp) * width);
    int by2 = cy + (int)(cosf(perp) * width);

    /* Fill triangle with horizontal lines */
    for (int t = 0; t <= 20; t++) {
        float frac = t / 20.0f;
        int x1 = bx1 + (int)((tx - bx1) * frac);
        int y1 = by1 + (int)((ty - by1) * frac);
        int x2 = bx2 + (int)((tx - bx2) * frac);
        int y2 = by2 + (int)((ty - by2) * frac);
        moui_draw_line(ctx, x1, y1, x2, y2, color);
    }
}

/* ══════════════════════════════════════════
 *  Watch Faces
 * ══════════════════════════════════════════ */
static moui_screen_t scr_face;
static int cur_face = 0;
static moui_widget_dots_t face_dots;
static char tbuf[16], dbuf[24];

/* ─── Face 1: Typography (Braun digital style) ─── */
static void face_typography(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    (void)w;
    moui_color_t on = MOUI_WHITE;

    /* Hero time — 48px, vertically centered */
    snprintf(tbuf, sizeof(tbuf), "%02d:%02d", g_hour, g_min);
    draw_str_center(ctx, &moui_font_inter_48, CY - 30, tbuf, on);

    /* Seconds — 24px, right-aligned below colon */
    snprintf(dbuf, sizeof(dbuf), "%02d", g_sec);
    draw_str_right(ctx, &moui_font_inter_24, W - PAD, CY - 24, dbuf, on);

    /* Thin separator line */
    moui_draw_hline(ctx, PAD + 20, CY + 26, W - PAD * 2 - 40, on);

    /* Date — elegant, centered below separator */
    snprintf(dbuf, sizeof(dbuf), "TUESDAY  27  MAY");
    draw_str_center(ctx, &moui_font_ascii_6x8, CY + 36, dbuf, on);

    /* Bottom complications row */
    int by = H - 32;
    char buf[12];

    /* Heart */
    moui_draw_fill_circle(ctx, PAD + 6, by + 4, 3, on);
    moui_draw_fill_circle(ctx, PAD + 12, by + 4, 3, on);
    snprintf(buf, sizeof(buf), "%d", (int)g_heart);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, PAD + 20, by, buf, on);

    /* Steps */
    snprintf(buf, sizeof(buf), "%d", g_steps);
    draw_str_center(ctx, &moui_font_ascii_6x8, by, buf, on);

    /* Battery — small bar */
    int bx = W - PAD - 28;
    moui_draw_rect(ctx, &(moui_rect_t){bx, by + 1, 20, 7}, on);
    moui_draw_fill_rect(ctx, &(moui_rect_t){bx + 1, by + 2, g_batt * 18 / 100, 5}, on);
    moui_draw_fill_rect(ctx, &(moui_rect_t){bx + 20, by + 3, 2, 3}, on);
}

/* ─── Face 2: Analog (Braun AW50 inspired) ─── */
static void face_analog(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    (void)w;
    moui_color_t on = MOUI_WHITE;
    int r = 92;

    /* Outer ring — single thin circle */
    moui_draw_circle(ctx, CX, CY, r, on);

    /* Hour marks — Braun style: 12 thick, others thin */
    for (int i = 0; i < 60; i++) {
        float a = i * 6.0f * PI / 180.0f;
        int len, thick;
        if (i % 15 == 0) { len = 12; thick = 2; }        /* 12/3/6/9 */
        else if (i % 5 == 0) { len = 8; thick = 1; }     /* 5-min marks */
        else { len = 3; thick = 0; }                       /* 1-min dots */

        if (thick == 0) {
            int dx = CX + (int)(sinf(a) * (r - 6));
            int dy = CY - (int)(cosf(a) * (r - 6));
            moui_draw_pixel(ctx, dx, dy, on);
        } else {
            int x1 = CX + (int)(sinf(a) * (r - 3));
            int y1 = CY - (int)(cosf(a) * (r - 3));
            int x2 = CX + (int)(sinf(a) * (r - 3 - len));
            int y2 = CY - (int)(cosf(a) * (r - 3 - len));
            moui_draw_line(ctx, x1, y1, x2, y2, on);
            if (thick == 2) {
                moui_draw_line(ctx, x1+1, y1, x2+1, y2, on);
                moui_draw_line(ctx, x1-1, y1, x2-1, y2, on);
            }
        }
    }

    /* Hour hand — filled triangle */
    float ha = ((g_hour % 12) + g_min / 60.0f) * 30.0f * PI / 180.0f;
    draw_thick_hand(ctx, CX, CY, ha, 42, 4, on);

    /* Minute hand — thinner, longer */
    float ma = (g_min + g_sec / 60.0f) * 6.0f * PI / 180.0f;
    draw_thick_hand(ctx, CX, CY, ma, 64, 3, on);

    /* Second hand — hairline with counterweight */
    float sa = g_sec * 6.0f * PI / 180.0f;
    moui_draw_line(ctx, CX - (int)(sinf(sa) * 18), CY + (int)(cosf(sa) * 18),
                  CX + (int)(sinf(sa) * 75), CY - (int)(cosf(sa) * 75), on);
    /* Counterweight circle */
    moui_draw_fill_circle(ctx, CX - (int)(sinf(sa) * 14), CY + (int)(cosf(sa) * 14), 3, on);

    /* Center pin */
    moui_draw_fill_circle(ctx, CX, CY, 4, on);

    /* Date window at 3 o'clock — refined */
    int dx = CX + 30, dy = CY - 6;
    moui_draw_rounded_rect(ctx, &(moui_rect_t){dx, dy, 20, 13}, 2, on);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, dx + 4, dy + 2, "27", on);
}

/* ─── Face 3: Dashboard (Apple Infograph refined) ─── */
static void face_dashboard(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    (void)w;
    moui_color_t on = MOUI_WHITE;
    char buf[12];

    /* Time — 48px hero */
    snprintf(tbuf, sizeof(tbuf), "%02d:%02d", g_hour, g_min);
    draw_str_center(ctx, &moui_font_inter_48, 8, tbuf, on);

    /* Thin line */
    moui_draw_hline(ctx, PAD, 62, W - PAD * 2, on);

    /* Three-column complications */
    int cy = 72;
    int col_w = (W - PAD * 2) / 3;

    /* Col 1: Steps with mini ring */
    int c1x = PAD + col_w / 2;
    moui_draw_circle(ctx, c1x, cy + 20, 18, on);
    int fill_deg = g_steps * 360 / 10000;
    moui_draw_arc(ctx, c1x, cy + 20, 18, 270, (270 + fill_deg) % 360, on);
    moui_draw_arc(ctx, c1x, cy + 20, 17, 270, (270 + fill_deg) % 360, on);
    moui_draw_arc(ctx, c1x, cy + 20, 16, 270, (270 + fill_deg) % 360, on);
    snprintf(buf, sizeof(buf), "%d.%dk", g_steps / 1000, (g_steps % 1000) / 100);
    int bw = moui_font_measure_str(&moui_font_ascii_6x8, buf);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, c1x - bw/2, cy + 16, buf, on);
    draw_str_center(ctx, &moui_font_ascii_6x8, cy + 46, "STEPS", on);

    /* Col 2: Heart rate */
    int c2x = CX;
    snprintf(buf, sizeof(buf), "%d", (int)g_heart);
    bw = moui_font_measure_str(&moui_font_inter_24, buf);
    moui_font_draw_str(ctx, &moui_font_inter_24, c2x - bw/2, cy + 10, buf, on);
    draw_str_center(ctx, &moui_font_ascii_6x8, cy + 46, "BPM", on);

    /* Col 3: Temperature */
    int c3x = W - PAD - col_w / 2;
    snprintf(buf, sizeof(buf), "%d", g_temp);
    bw = moui_font_measure_str(&moui_font_inter_24, buf);
    moui_font_draw_str(ctx, &moui_font_inter_24, c3x - bw/2, cy + 10, buf, on);
    moui_draw_circle(ctx, c3x + bw/2 + 4, cy + 12, 2, on);
    int lx = W - PAD - col_w / 2;
    bw = moui_font_measure_str(&moui_font_ascii_6x8, "TEMP");
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, lx - bw/2, cy + 46, "TEMP", on);

    /* Bottom: battery bar full width */
    int bar_y = H - 24;
    moui_draw_rounded_rect(ctx, &(moui_rect_t){PAD, bar_y, W - PAD*2, 8}, 3, on);
    int fill_w = g_batt * (W - PAD*2 - 4) / 100;
    if (fill_w > 0)
        moui_draw_fill_rounded_rect(ctx, &(moui_rect_t){PAD+2, bar_y+2, fill_w, 4}, 1, on);
    snprintf(buf, sizeof(buf), "%d%%", g_batt);
    draw_str_center(ctx, &moui_font_ascii_6x8, bar_y + 10, buf, on);
}

/* Face dispatch + interaction */
static void face_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    switch (cur_face) {
    case 0: face_typography(w, ctx); break;
    case 1: face_analog(w, ctx); break;
    case 2: face_dashboard(w, ctx); break;
    }
}

static void push_menu(void);
static bool face_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    (void)w;
    if (ev->type == MOUI_EV_ENCODER_CW)  { cur_face = (cur_face + 1) % 3; face_dots.active = cur_face; return true; }
    if (ev->type == MOUI_EV_ENCODER_CCW) { cur_face = (cur_face + 2) % 3; face_dots.active = cur_face; return true; }
    if (ev->type == MOUI_EV_ENCODER_PRESS) { push_menu(); return true; }
    return false;
}

static const moui_widget_vtable_t face_vt = { .draw = face_draw, .on_event = face_event };
static moui_widget_t face_canvas;

static void face_tick(moui_screen_t *s, uint32_t now_ms, float dt)
{ (void)s; (void)dt; sim_tick(now_ms); }

/* ══════════════════════════════════════════
 *  Menu
 * ══════════════════════════════════════════ */
static moui_screen_t scr_menu;
static moui_widget_list_t menu_list;
static void push_weather(void), push_fitness(void), push_timer(void), push_settings(void);
static const char *menu_items[] = {"Weather", "Activity", "Timer", "Settings"};
static const char *array_adapter(uint16_t idx, void *data) { return ((const char **)data)[idx]; }

static void on_menu(moui_widget_list_t *l, uint16_t i) {
    (void)l;
    switch(i) { case 0: push_weather(); break; case 1: push_fitness(); break;
                case 2: push_timer(); break; case 3: push_settings(); break; }
}

/* ══════════════════════════════════════════
 *  Timer (Stopwatch)
 * ══════════════════════════════════════════ */
static moui_screen_t scr_timer;
static moui_widget_label_t tmr_title;
static moui_widget_button_t tmr_start, tmr_lap;
static bool tmr_running = false;
static uint32_t tmr_start_ms = 0, tmr_elapsed = 0;
static char tmr_buf[16];
static moui_widget_label_t tmr_display;
static const char *tmr_lap_strs[8];
static char tmr_lap_bufs[8][20];
static uint8_t tmr_lap_n = 0;
static moui_widget_list_t tmr_laps;

static const char *tmr_lap_adapter(uint16_t idx, void *data) {
    (void)data;
    return tmr_lap_strs[idx];
}

static void on_tmr_start(moui_widget_button_t *b) {
    (void)b;
    if (!tmr_running) { tmr_running = true; tmr_start_ms = SDL_GetTicks() - tmr_elapsed; }
    else { tmr_running = false; tmr_elapsed = SDL_GetTicks() - tmr_start_ms; }
}
static void on_tmr_lap(moui_widget_button_t *b) {
    (void)b;
    if (!tmr_running && tmr_elapsed > 0 && tmr_lap_n < 8) {
        int ms = tmr_elapsed;
        snprintf(tmr_lap_bufs[tmr_lap_n], 20, "Lap %d    %d:%02d.%02d",
                 tmr_lap_n+1, ms/60000, (ms/1000)%60, (ms/10)%100);
        tmr_lap_strs[tmr_lap_n] = tmr_lap_bufs[tmr_lap_n];
        tmr_lap_n++;
        moui_list_set_adapter(&tmr_laps, tmr_lap_adapter, NULL, tmr_lap_n);
        tmr_elapsed = 0;
    }
}
static void tmr_tick(moui_screen_t *s, uint32_t now_ms, float dt) {
    (void)s; (void)now_ms; (void)dt;
    if (tmr_running) tmr_elapsed = SDL_GetTicks() - tmr_start_ms;
    int ms = tmr_elapsed;
    snprintf(tmr_buf, sizeof(tmr_buf), "%d:%02d.%02d", ms/60000, (ms/1000)%60, (ms/10)%100);
}

/* ══════════════════════════════════════════
 *  Weather / Activity / Settings (simpler)
 * ══════════════════════════════════════════ */
static moui_screen_t scr_weather, scr_fitness, scr_set;
static moui_widget_label_t wth_title, fit_title, set_title;
static moui_widget_number_t wth_temp;
static moui_widget_chart_t wth_chart;
static moui_widget_ring_t fit_ring;
static moui_widget_number_t fit_num;
static moui_widget_label_t fit_cal, fit_dist;
static char fit_cal_buf[16], fit_dist_buf[16];
static moui_widget_label_t set_l1, set_l2, set_l3;
static moui_widget_slider_t set_bright;
static moui_widget_switch_t set_haptic;
static moui_widget_radio_t set_face;
static const char *face_opts[] = {"Type", "Analog", "Dash"};

static void wth_tick(moui_screen_t *s, uint32_t now_ms, float dt)
{ (void)s; (void)now_ms; (void)dt; wth_temp.value = g_temp; }

static void fit_tick(moui_screen_t *s, uint32_t now_ms, float dt) {
    (void)s; (void)now_ms; (void)dt;
    fit_ring.value = g_steps % 10000; fit_num.value = g_steps;
    snprintf(fit_cal_buf, sizeof(fit_cal_buf), "%d cal", g_steps / 20);
    snprintf(fit_dist_buf, sizeof(fit_dist_buf), "%.1f km", g_steps * 0.0007f);
}

static void on_face_ch(moui_widget_radio_t *r, uint8_t i) { (void)r; cur_face = i; face_dots.active = i; }

/* ══════════════════════════════════════════
 *  Setup
 * ══════════════════════════════════════════ */
static void setup_all(void)
{
    moui_theme.invert_colors = true;

    /* ── Face ── */
    moui_screen_init(&scr_face);
    scr_face.on_tick = face_tick;
    moui_widget_init(&face_canvas, &face_vt);
    face_canvas.bounds = (moui_rect_t){0, 0, W, H - 10};
    face_canvas.enabled = 1;
    moui_screen_add_widget(&scr_face, &face_canvas);
    moui_dots_init(&face_dots, 3);
    face_dots.dot_r = 2; face_dots.gap = 6;
    face_dots.base.bounds = (moui_rect_t){0, H - 10, W, 10};
    moui_screen_add_widget(&scr_face, &face_dots.base);
    moui_focus_group_add(&scr_face.focus, &face_canvas);

    /* ── Menu — clean centered list ── */
    moui_screen_init(&scr_menu);
    scr_menu.enter_trans = MOUI_TRANS_SLIDE_UP;
    moui_list_init(&menu_list, &moui_font_inter_24);
    moui_list_set_adapter(&menu_list, array_adapter, (void *)menu_items, 4);
    menu_list.on_select = on_menu;
    menu_list.capturing = true;
    menu_list.base.bounds = (moui_rect_t){0, 30, W, H - 40};
    moui_screen_add_widget(&scr_menu, &menu_list.base);
    moui_focus_group_add(&scr_menu.focus, &menu_list.base);

    /* ── Weather — clean card layout ── */
    moui_screen_init(&scr_weather);
    scr_weather.enter_trans = MOUI_TRANS_SLIDE_LEFT;
    scr_weather.on_tick = wth_tick;

    moui_label_init(&wth_title, "Weather", &moui_font_inter_16);
    wth_title.base.bounds = (moui_rect_t){PAD, 12, 80, 18};
    moui_screen_add_widget(&scr_weather, &wth_title.base);

    moui_number_init(&wth_temp, &moui_font_inter_48, 2);
    wth_temp.value = 23; wth_temp.unit = " "; wth_temp.leading_zero = false;
    wth_temp.base.bounds = (moui_rect_t){PAD, 38, 130, 52};
    moui_screen_add_widget(&scr_weather, &wth_temp.base);

    moui_chart_init(&wth_chart, 15, 35);
    wth_chart.show_frame = true; wth_chart.fill_below = true;
    int temps[] = {23,25,22,20,24,26,23,21,25,24,22,26};
    for (int i = 0; i < 12; i++) moui_chart_push(&wth_chart, temps[i]*10);
    wth_chart.base.bounds = (moui_rect_t){PAD, 102, W - PAD*2, H - 114};
    moui_screen_add_widget(&scr_weather, &wth_chart.base);

    /* ── Activity — ring hero, data below ── */
    moui_screen_init(&scr_fitness);
    scr_fitness.enter_trans = MOUI_TRANS_SLIDE_LEFT;
    scr_fitness.on_tick = fit_tick;

    moui_label_init(&fit_title, "Activity", &moui_font_inter_16);
    fit_title.base.bounds = (moui_rect_t){PAD, 12, 80, 18};
    moui_screen_add_widget(&scr_fitness, &fit_title.base);

    moui_ring_init(&fit_ring, &moui_font_inter_24, 10000);
    fit_ring.label = ""; fit_ring.thickness = 6;
    fit_ring.base.bounds = (moui_rect_t){CX - 50, 38, 100, 100};
    moui_screen_add_widget(&scr_fitness, &fit_ring.base);

    moui_number_init(&fit_num, &moui_font_inter_16, 5);
    fit_num.leading_zero = false; fit_num.value = g_steps;
    fit_num.base.bounds = (moui_rect_t){PAD, 146, W - PAD*2, 20};
    moui_screen_add_widget(&scr_fitness, &fit_num.base);

    snprintf(fit_cal_buf, sizeof(fit_cal_buf), "342 cal");
    moui_label_init(&fit_cal, fit_cal_buf, &moui_font_inter_16);
    fit_cal.base.bounds = (moui_rect_t){PAD, 172, 90, 18};
    moui_screen_add_widget(&scr_fitness, &fit_cal.base);

    snprintf(fit_dist_buf, sizeof(fit_dist_buf), "4.8 km");
    moui_label_init(&fit_dist, fit_dist_buf, &moui_font_inter_16);
    fit_dist.base.bounds = (moui_rect_t){W - PAD - 70, 172, 70, 18};
    moui_screen_add_widget(&scr_fitness, &fit_dist.base);

    /* ── Timer — bold time, round buttons ── */
    moui_screen_init(&scr_timer);
    scr_timer.enter_trans = MOUI_TRANS_SLIDE_LEFT;
    scr_timer.on_tick = tmr_tick;

    moui_label_init(&tmr_title, "Timer", &moui_font_inter_16);
    tmr_title.base.bounds = (moui_rect_t){PAD, 12, 60, 18};
    moui_screen_add_widget(&scr_timer, &tmr_title.base);

    snprintf(tmr_buf, sizeof(tmr_buf), "0:00.00");
    moui_label_init(&tmr_display, tmr_buf, &moui_font_inter_48);
    tmr_display.base.bounds = (moui_rect_t){4, 40, W - 8, 52};
    moui_screen_add_widget(&scr_timer, &tmr_display.base);

    moui_button_init(&tmr_start, "Start", &moui_font_inter_16, MOUI_BTN_PUSH);
    tmr_start.on_click = on_tmr_start;
    tmr_start.base.bounds = (moui_rect_t){PAD, 102, 78, 30};
    moui_screen_add_widget(&scr_timer, &tmr_start.base);

    moui_button_init(&tmr_lap, "Lap", &moui_font_inter_16, MOUI_BTN_PUSH);
    tmr_lap.on_click = on_tmr_lap;
    tmr_lap.base.bounds = (moui_rect_t){W - PAD - 78, 102, 78, 30};
    moui_screen_add_widget(&scr_timer, &tmr_lap.base);

    moui_list_init(&tmr_laps, &moui_font_inter_16);
    tmr_laps.base.bounds = (moui_rect_t){PAD, 140, W - PAD*2, H - 148};
    tmr_laps.base.enabled = 0;
    moui_screen_add_widget(&scr_timer, &tmr_laps.base);

    moui_focus_group_add(&scr_timer.focus, &tmr_start.base);
    moui_focus_group_add(&scr_timer.focus, &tmr_lap.base);

    /* ── Settings ── use ScrollView for content, generous spacing */
    moui_screen_init(&scr_set);
    scr_set.enter_trans = MOUI_TRANS_SLIDE_LEFT;

    moui_label_init(&set_title, "Settings", &moui_font_inter_24);
    set_title.base.bounds = (moui_rect_t){PAD, 16, 120, 28};
    moui_screen_add_widget(&scr_set, &set_title.base);

    /* Brightness row */
    moui_label_init(&set_l1, "Brightness", &moui_font_inter_16);
    set_l1.base.bounds = (moui_rect_t){PAD, 56, 100, 18};
    moui_screen_add_widget(&scr_set, &set_l1.base);

    moui_slider_init(&set_bright, &moui_font_ascii_6x8, 0, 100, 5);
    set_bright.value = 70;
    set_bright.base.bounds = (moui_rect_t){PAD, 78, W - PAD*2, 20};
    moui_screen_add_widget(&scr_set, &set_bright.base);

    /* Haptic row — label left, switch right, vertically centered */
    moui_label_init(&set_l2, "Haptic", &moui_font_inter_16);
    set_l2.base.bounds = (moui_rect_t){PAD, 114, 80, 18};
    moui_screen_add_widget(&scr_set, &set_l2.base);

    moui_switch_init(&set_haptic);
    set_haptic.base.bounds = (moui_rect_t){W - PAD - 40, 114, 40, 16};
    moui_screen_add_widget(&scr_set, &set_haptic.base);

    /* Face selection */
    moui_label_init(&set_l3, "Watch Face", &moui_font_inter_16);
    set_l3.base.bounds = (moui_rect_t){PAD, 148, 100, 18};
    moui_screen_add_widget(&scr_set, &set_l3.base);

    moui_radio_init(&set_face, &moui_font_inter_16);
    moui_radio_set_options(&set_face, face_opts, 3);
    set_face.on_change = on_face_ch;
    set_face.base.bounds = (moui_rect_t){PAD + 8, 170, W - PAD*2 - 8, 3 * set_face.item_h};
    moui_screen_add_widget(&scr_set, &set_face.base);

    moui_focus_group_add(&scr_set.focus, &set_bright.base);
    moui_focus_group_add(&scr_set.focus, &set_haptic.base);
    moui_focus_group_add(&scr_set.focus, &set_face.base);
}

static void push_menu(void)    { moui_screen_push(&g_mgr, &scr_menu); }
static void push_weather(void) { moui_screen_push(&g_mgr, &scr_weather); }
static void push_fitness(void) { moui_screen_push(&g_mgr, &scr_fitness); }
static void push_timer(void)   { moui_screen_push(&g_mgr, &scr_timer); }
static void push_settings(void){ moui_screen_push(&g_mgr, &scr_set); }

/* ══════════════════════════════════════════
 *  Main
 * ══════════════════════════════════════════ */
int main(int argc, char *argv[])
{
    (void)argc; (void)argv;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;
    moui_hal_set_display(&moui_disp_gdew0154);
    if (moui_fb_init(&g_fb, &moui_disp_gdew0154) < 0) return 1;
    if (sim_display_init(&g_display) < 0) { SDL_Quit(); return 1; }

    moui_screen_mgr_init(&g_mgr, &g_fb, &g_hal);
    setup_all();
    moui_screen_push(&g_mgr, &scr_face);

    uint32_t last = SDL_GetTicks();
    bool quit = false;
    while (!quit) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { quit = true; break; }
            if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE && g_mgr.depth <= 1) { quit = true; break; }
            if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_r) { sim_display_rotate(&g_display); continue; }
            sim_input_handle_event(&ev, moui_screen_mgr_input(&g_mgr));
        }
        uint32_t now = SDL_GetTicks();
        float dt = (float)(now - last); if (dt < 1) dt = 1;
        last = now;
        moui_screen_mgr_tick(&g_mgr, now, dt);
        uint32_t el = SDL_GetTicks() - now;
        if (el < 20) SDL_Delay(20 - el);
    }
    sim_display_destroy(&g_display);
    SDL_Quit();
    return 0;
}
