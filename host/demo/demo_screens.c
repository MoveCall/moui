#include "demo_screens.h"
#include "moui_image_test.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

static moui_screen_mgr_t *s_mgr;

void demo_init(moui_screen_mgr_t *mgr)
{
    s_mgr = mgr;
}

/* ====================================================================
 *  Screen: Main Menu
 * ==================================================================== */
static moui_screen_t       scr_main;
static moui_widget_label_t main_title;
static moui_widget_list_t  main_list;

static void push_dashboard(void);
static void push_settings(void);
static void push_chart(void);
static void push_graphics(void);
static void push_stress(void);
static void push_widgets(void);
static void push_widgets2(void);
static void push_widgets3(void);
static void push_widgets4(void);
static void push_mono(void);
static void push_image(void);
static void push_features(void);
static void push_about(void);
static void push_icons(void);

static moui_screen_t scr_icons;

static const char *main_items[] = {
    "Dashboard",
    "Settings",
    "Dual Waveform",
    "Graphics Demo",
    "Stress Test",
    "Widgets 1",
    "Widgets 2",
    "Widgets 3",
    "Widgets 4",
    "New Widgets (6)",
    "VirtualList (100k)",
    "Chat Demo",
    "App Framework",
    "Architecture (4)",
    "Mono Effects",
    "Image Test",
    "New Features",
    "Icons",
    "About",
};

static const char *main_list_adapter(uint16_t idx, void *data)
{
    return ((const char **)data)[idx];
}

static void push_widgets5(void);
static void push_vlist(void);
static void push_chat(void);
static void push_app(void);
static void push_arch(void);

static void on_main_select(moui_widget_list_t *l, uint16_t idx)
{
    (void)l;
    switch (idx) {
    case 0: push_dashboard(); break;
    case 1: push_settings(); break;
    case 2: push_chart(); break;
    case 3: push_graphics(); break;
    case 4: push_stress(); break;
    case 5: push_widgets(); break;
    case 6: push_widgets2(); break;
    case 7: push_widgets3(); break;
    case 8: push_widgets4(); break;
    case 9: push_widgets5(); break;
    case 10: push_vlist(); break;
    case 11: push_chat(); break;
    case 12: push_app(); break;
    case 13: push_arch(); break;
    case 14: push_mono(); break;
    case 15: push_image(); break;
    case 16: push_features(); break;
    case 17: push_icons(); break;
    case 18: push_about(); break;
    }
}

static void setup_main_screen(void)
{
    moui_screen_init(&scr_main);

    moui_label_init(&main_title, "Moui v1.0", &moui_font_ascii_6x8);
    main_title.inverted = true;
    main_title.base.bounds = (moui_rect_t){0, 0, MOUI_DISP_W, 16};
    moui_screen_add_widget(&scr_main, &main_title.base);

    moui_list_init(&main_list, &moui_font_ascii_6x8);
    moui_list_set_adapter(&main_list, main_list_adapter, (void *)main_items, 16);
    main_list.base.bounds = (moui_rect_t){0, 18, MOUI_DISP_W, MOUI_DISP_H - 18};
    main_list.base.bounds = (moui_rect_t){0, 18, MOUI_DISP_W, MOUI_DISP_H - 18};
    main_list.on_select = on_main_select;
    main_list.capturing = true;
    moui_screen_add_widget(&scr_main, &main_list.base);

    moui_focus_group_add(&scr_main.focus, &main_list.base);
}

/* ====================================================================
 *  Screen: Dashboard
 * ==================================================================== */
static moui_screen_t        scr_dash;
static moui_widget_label_t  dash_title;
static moui_widget_chart_t  dash_temp_chart, dash_humi_chart;
static moui_widget_label_t  dash_temp_label, dash_humi_label, dash_batt_label, dash_status;
static moui_widget_slider_t dash_batt_bar;

static char dash_temp_buf[24], dash_humi_buf[24], dash_batt_buf[24], dash_status_buf[40];
static uint32_t dash_last_update = 0;
static float dash_t = 0.0f;

static void dash_tick(moui_screen_t *s, uint32_t now_ms, float dt)
{
    (void)s; (void)dt;
    if (now_ms - dash_last_update < 60) return;
    dash_last_update = now_ms;
    dash_t += 0.06f;

    float temp = 23.0f + sinf(dash_t * 0.5f) * 4.0f + sinf(dash_t * 1.3f) * 1.5f;
    float humi = 55.0f + cosf(dash_t * 0.3f) * 12.0f + sinf(dash_t * 2.1f) * 3.0f;
    float batt = 85.0f - dash_t * 0.015f;
    if (batt < 10.0f) batt = 10.0f + sinf(dash_t) * 2.0f;

    snprintf(dash_temp_buf, sizeof(dash_temp_buf), "Temp: %.1f C", temp);
    snprintf(dash_humi_buf, sizeof(dash_humi_buf), "Humi: %.1f %%", humi);
    snprintf(dash_batt_buf, sizeof(dash_batt_buf), "Batt: %.0f%%", batt);

    uint32_t sec = now_ms / 1000;
    snprintf(dash_status_buf, sizeof(dash_status_buf), "Up %02d:%02d:%02d  FPS:50",
             (int)(sec / 3600), (int)((sec / 60) % 60), (int)(sec % 60));

    moui_chart_push(&dash_temp_chart, (int16_t)(temp * 10));
    moui_chart_push(&dash_humi_chart, (int16_t)(humi * 10));
    dash_batt_bar.value = (int32_t)batt;
}

static void setup_dashboard_screen(void)
{
    moui_screen_init(&scr_dash);
    scr_dash.enter_trans = MOUI_TRANS_SLIDE_LEFT;
    scr_dash.on_tick = dash_tick;

    moui_label_init(&dash_title, "Dashboard", &moui_font_ascii_6x8);
    dash_title.inverted = true;
    dash_title.base.bounds = (moui_rect_t){0, 0, MOUI_DISP_W, 16};
    moui_screen_add_widget(&scr_dash, &dash_title.base);

    snprintf(dash_temp_buf, sizeof(dash_temp_buf), "Temp: 25.0 C");
    moui_label_init(&dash_temp_label, dash_temp_buf, &moui_font_ascii_6x8);
    dash_temp_label.base.bounds = (moui_rect_t){4, 20, MOUI_DISP_W - 8, 12};
    moui_screen_add_widget(&scr_dash, &dash_temp_label.base);

    moui_chart_init(&dash_temp_chart, 150, 350);
    dash_temp_chart.base.bounds = (moui_rect_t){2, 34, MOUI_DISP_W - 4, 55};
    dash_temp_chart.show_frame = true;
    moui_screen_add_widget(&scr_dash, &dash_temp_chart.base);

    snprintf(dash_humi_buf, sizeof(dash_humi_buf), "Humi: 55.0 %%");
    moui_label_init(&dash_humi_label, dash_humi_buf, &moui_font_ascii_6x8);
    dash_humi_label.base.bounds = (moui_rect_t){4, 94, MOUI_DISP_W - 8, 12};
    moui_screen_add_widget(&scr_dash, &dash_humi_label.base);

    moui_chart_init(&dash_humi_chart, 300, 800);
    dash_humi_chart.base.bounds = (moui_rect_t){2, 108, MOUI_DISP_W - 4, 55};
    dash_humi_chart.show_frame = true;
    dash_humi_chart.fill_below = true;
    moui_screen_add_widget(&scr_dash, &dash_humi_chart.base);

    snprintf(dash_batt_buf, sizeof(dash_batt_buf), "Batt: 85%%");
    moui_label_init(&dash_batt_label, dash_batt_buf, &moui_font_ascii_6x8);
    dash_batt_label.base.bounds = (moui_rect_t){4, 170, 80, 12};
    moui_screen_add_widget(&scr_dash, &dash_batt_label.base);

    moui_slider_init(&dash_batt_bar, &moui_font_ascii_6x8, 0, 100, 1);
    dash_batt_bar.value = 85;
    dash_batt_bar.show_value = false;
    dash_batt_bar.base.bounds = (moui_rect_t){4, 184, MOUI_DISP_W - 8, 14};
    dash_batt_bar.base.enabled = 0;
    moui_screen_add_widget(&scr_dash, &dash_batt_bar.base);

    snprintf(dash_status_buf, sizeof(dash_status_buf), "Up 00:00:00  FPS:50");
    moui_label_init(&dash_status, dash_status_buf, &moui_font_ascii_6x8);
    dash_status.base.bounds = (moui_rect_t){4, 204, MOUI_DISP_W - 8, 12};
    moui_screen_add_widget(&scr_dash, &dash_status.base);
}

/* ====================================================================
 *  Screen: Settings
 * ==================================================================== */
static moui_screen_t         scr_settings;
static moui_widget_label_t   set_title, set_lbl_bright, set_lbl_contrast, set_lbl_rate;
static moui_widget_slider_t  set_brightness, set_contrast, set_refresh_rate;
static moui_widget_button_t  set_wifi, set_led, set_auto_sleep, set_invert, set_save;
static moui_widget_label_t   set_hint;

static void on_save_click(moui_widget_button_t *b)
{
    (void)b;
    moui_popup_show_toast(&s_mgr->popup, "Settings saved!", 1500);
}

static void setup_settings_screen(void)
{
    moui_screen_init(&scr_settings);
    scr_settings.enter_trans = MOUI_TRANS_SLIDE_LEFT;
    int y = 0;

    moui_label_init(&set_title, "Settings", &moui_font_ascii_6x8);
    set_title.inverted = true;
    set_title.base.bounds = (moui_rect_t){0, y, MOUI_DISP_W, 16};
    moui_screen_add_widget(&scr_settings, &set_title.base); y += 20;

    moui_label_init(&set_lbl_bright, "Brightness", &moui_font_ascii_6x8);
    set_lbl_bright.base.bounds = (moui_rect_t){6, y, 100, 10};
    moui_screen_add_widget(&scr_settings, &set_lbl_bright.base); y += 12;
    moui_slider_init(&set_brightness, &moui_font_ascii_6x8, 0, 100, 5);
    set_brightness.value = 70;
    set_brightness.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 20};
    moui_screen_add_widget(&scr_settings, &set_brightness.base); y += 24;

    moui_label_init(&set_lbl_contrast, "Contrast", &moui_font_ascii_6x8);
    set_lbl_contrast.base.bounds = (moui_rect_t){6, y, 100, 10};
    moui_screen_add_widget(&scr_settings, &set_lbl_contrast.base); y += 12;
    moui_slider_init(&set_contrast, &moui_font_ascii_6x8, 0, 100, 5);
    set_contrast.value = 50;
    set_contrast.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 20};
    moui_screen_add_widget(&scr_settings, &set_contrast.base); y += 24;

    moui_label_init(&set_lbl_rate, "Refresh Hz", &moui_font_ascii_6x8);
    set_lbl_rate.base.bounds = (moui_rect_t){6, y, 100, 10};
    moui_screen_add_widget(&scr_settings, &set_lbl_rate.base); y += 12;
    moui_slider_init(&set_refresh_rate, &moui_font_ascii_6x8, 1, 51, 1);
    set_refresh_rate.value = 32;
    set_refresh_rate.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 20};
    moui_screen_add_widget(&scr_settings, &set_refresh_rate.base); y += 28;

    moui_button_init(&set_wifi, "WiFi", &moui_font_ascii_6x8, MOUI_BTN_TOGGLE);
    set_wifi.base.bounds = (moui_rect_t){4, y, (MOUI_DISP_W - 12) / 2, 18};
    moui_screen_add_widget(&scr_settings, &set_wifi.base);
    moui_button_init(&set_led, "LED", &moui_font_ascii_6x8, MOUI_BTN_TOGGLE);
    set_led.base.bounds = (moui_rect_t){4 + (MOUI_DISP_W - 12) / 2 + 4, y, (MOUI_DISP_W - 12) / 2, 18};
    moui_screen_add_widget(&scr_settings, &set_led.base); y += 22;

    moui_button_init(&set_auto_sleep, "Auto Sleep", &moui_font_ascii_6x8, MOUI_BTN_CHECKBOX);
    set_auto_sleep.state = true;
    set_auto_sleep.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 12, 18};
    moui_screen_add_widget(&scr_settings, &set_auto_sleep.base); y += 22;
    moui_button_init(&set_invert, "Invert Display", &moui_font_ascii_6x8, MOUI_BTN_CHECKBOX);
    set_invert.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 12, 18};
    moui_screen_add_widget(&scr_settings, &set_invert.base); y += 26;

    moui_button_init(&set_save, "[ Save ]", &moui_font_ascii_6x8, MOUI_BTN_PUSH);
    set_save.on_click = on_save_click;
    set_save.base.bounds = (moui_rect_t){20, y, MOUI_DISP_W - 40, 20};
    moui_screen_add_widget(&scr_settings, &set_save.base); y += 24;

    moui_label_init(&set_hint, "R=Rotate ESC=Back", &moui_font_ascii_6x8);
    set_hint.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 10};
    moui_screen_add_widget(&scr_settings, &set_hint.base);

    moui_focus_group_add(&scr_settings.focus, &set_brightness.base);
    moui_focus_group_add(&scr_settings.focus, &set_contrast.base);
    moui_focus_group_add(&scr_settings.focus, &set_refresh_rate.base);
    moui_focus_group_add(&scr_settings.focus, &set_wifi.base);
    moui_focus_group_add(&scr_settings.focus, &set_led.base);
    moui_focus_group_add(&scr_settings.focus, &set_auto_sleep.base);
    moui_focus_group_add(&scr_settings.focus, &set_invert.base);
    moui_focus_group_add(&scr_settings.focus, &set_save.base);
}

/* ====================================================================
 *  Screen: Dual Waveform
 * ==================================================================== */
static moui_screen_t        scr_chart;
static moui_widget_label_t  chart_title, chart_lbl1, chart_lbl2, chart_hint;
static moui_widget_chart_t  chart_wave1, chart_wave2;
static uint32_t chart_last_push = 0;
static float chart_phase = 0.0f;

static void chart_tick(moui_screen_t *s, uint32_t now_ms, float dt)
{
    (void)s; (void)dt;
    if (now_ms - chart_last_push < 30) return;
    chart_last_push = now_ms;
    chart_phase += 0.1f;
    int16_t v1 = (int16_t)(sinf(chart_phase) * 40 + 50 + sinf(chart_phase * 3.7f) * 10);
    int16_t v2 = (int16_t)(cosf(chart_phase * 0.7f) * 35 + 50 + sinf(chart_phase * 2.3f) * 8);
    moui_chart_push(&chart_wave1, v1);
    moui_chart_push(&chart_wave2, v2);
}

static void setup_chart_screen(void)
{
    moui_screen_init(&scr_chart);
    scr_chart.enter_trans = MOUI_TRANS_SLIDE_UP;
    scr_chart.on_tick = chart_tick;

    moui_label_init(&chart_title, "Dual Waveform", &moui_font_ascii_6x8);
    chart_title.inverted = true;
    chart_title.base.bounds = (moui_rect_t){0, 0, MOUI_DISP_W, 16};
    moui_screen_add_widget(&scr_chart, &chart_title.base);

    moui_label_init(&chart_lbl1, "CH-A Sine+Harmonic", &moui_font_ascii_6x8);
    chart_lbl1.base.bounds = (moui_rect_t){4, 20, MOUI_DISP_W - 8, 10};
    moui_screen_add_widget(&scr_chart, &chart_lbl1.base);
    moui_chart_init(&chart_wave1, 0, 100);
    chart_wave1.base.bounds = (moui_rect_t){2, 32, MOUI_DISP_W - 4, 80};
    chart_wave1.show_frame = true;
    moui_screen_add_widget(&scr_chart, &chart_wave1.base);

    moui_label_init(&chart_lbl2, "CH-B Cosine (fill)", &moui_font_ascii_6x8);
    chart_lbl2.base.bounds = (moui_rect_t){4, 118, MOUI_DISP_W - 8, 10};
    moui_screen_add_widget(&scr_chart, &chart_lbl2.base);
    moui_chart_init(&chart_wave2, 0, 100);
    chart_wave2.base.bounds = (moui_rect_t){2, 130, MOUI_DISP_W - 4, 80};
    chart_wave2.show_frame = true;
    chart_wave2.fill_below = true;
    moui_screen_add_widget(&scr_chart, &chart_wave2.base);

    moui_label_init(&chart_hint, "ESC=Back  R=Rotate", &moui_font_ascii_6x8);
    chart_hint.base.bounds = (moui_rect_t){4, 218, MOUI_DISP_W - 8, 10};
    moui_screen_add_widget(&scr_chart, &chart_hint.base);
}

/* ====================================================================
 *  Screen: Graphics Demo
 * ==================================================================== */
static moui_screen_t       scr_gfx;
static moui_widget_label_t gfx_title, gfx_fps_label;
static char gfx_fps_buf[32];
static uint32_t gfx_frame_count = 0, gfx_last_fps_time = 0;
static float gfx_anim_t = 0.0f;

static void gfx_tick(moui_screen_t *s, uint32_t now_ms, float dt)
{
    (void)s;
    gfx_anim_t += dt * 0.003f;
    gfx_frame_count++;
    if (now_ms - gfx_last_fps_time >= 1000) {
        float fps = (float)gfx_frame_count * 1000.0f / (float)(now_ms - gfx_last_fps_time);
        snprintf(gfx_fps_buf, sizeof(gfx_fps_buf), "FPS: %.1f  t=%.1f", fps, gfx_anim_t);
        gfx_frame_count = 0;
        gfx_last_fps_time = now_ms;
    }
}

static void gfx_draw_custom(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    (void)w;
    float t = gfx_anim_t;
    int cx = MOUI_DISP_W / 2;
    int base_y = 30;

    for (int i = 0; i < 5; i++) {
        float phase = t + i * 0.7f;
        int r = 12 + i * 5;
        int ox = (int)(sinf(phase) * (50 - i * 8));
        int oy = base_y + 30 + i * 35;
        moui_draw_circle(ctx, cx + ox, oy, r, MOUI_BLACK);
        if (i % 2 == 0)
            moui_draw_fill_circle(ctx, cx + ox, oy, r / 2, MOUI_BLACK);
    }

    for (int i = 0; i < 8; i++) {
        float angle = t * 2.0f + i * 0.785f;
        int x1 = cx + (int)(cosf(angle) * 60);
        int y1 = 280 + (int)(sinf(angle) * 40);
        int x2 = cx + (int)(cosf(angle + 1.57f) * 60);
        int y2 = 280 + (int)(sinf(angle + 1.57f) * 40);
        moui_draw_line(ctx, x1, y1, x2, y2, MOUI_BLACK);
    }

    int box_x = 10 + (int)(sinf(t * 1.5f) * 30 + 30);
    int box_w = 40 + (int)(sinf(t * 2.3f) * 20);
    moui_draw_fill_rounded_rect(ctx, &(moui_rect_t){box_x, 330, box_w, 20}, 5, MOUI_BLACK);
    moui_draw_rounded_rect(ctx, &(moui_rect_t){box_x + 5, 355, box_w - 10, 16}, 4, MOUI_BLACK);
}

static moui_widget_t gfx_canvas;
static const moui_widget_vtable_t gfx_canvas_vtable = { .draw = gfx_draw_custom };

static void setup_graphics_screen(void)
{
    moui_screen_init(&scr_gfx);
    scr_gfx.enter_trans = MOUI_TRANS_SLIDE_LEFT;
    scr_gfx.on_tick = gfx_tick;

    moui_label_init(&gfx_title, "Graphics Demo", &moui_font_ascii_6x8);
    gfx_title.inverted = true;
    gfx_title.base.bounds = (moui_rect_t){0, 0, MOUI_DISP_W, 16};
    moui_screen_add_widget(&scr_gfx, &gfx_title.base);

    snprintf(gfx_fps_buf, sizeof(gfx_fps_buf), "FPS: --");
    moui_label_init(&gfx_fps_label, gfx_fps_buf, &moui_font_ascii_6x8);
    gfx_fps_label.base.bounds = (moui_rect_t){4, 18, MOUI_DISP_W - 8, 10};
    moui_screen_add_widget(&scr_gfx, &gfx_fps_label.base);

    moui_widget_init(&gfx_canvas, &gfx_canvas_vtable);
    gfx_canvas.bounds = (moui_rect_t){0, 28, MOUI_DISP_W, MOUI_DISP_H - 28};
    moui_screen_add_widget(&scr_gfx, &gfx_canvas);
}

/* ====================================================================
 *  Screen: Stress Test
 * ==================================================================== */
static moui_screen_t       scr_stress;
static moui_widget_label_t stress_title, stress_info;
static char stress_info_buf[40];
static float stress_t = 0.0f;
static uint32_t stress_frame = 0, stress_last_fps = 0;

static void stress_tick(moui_screen_t *s, uint32_t now_ms, float dt)
{
    (void)s;
    stress_t += dt * 0.002f;
    stress_frame++;
    if (now_ms - stress_last_fps >= 1000) {
        float fps = (float)stress_frame * 1000.0f / (float)(now_ms - stress_last_fps);
        snprintf(stress_info_buf, sizeof(stress_info_buf), "FPS:%.0f  43 prims/frame", fps);
        stress_frame = 0;
        stress_last_fps = now_ms;
    }
}

static void stress_draw_custom(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    (void)w;
    float t = stress_t;
    int center_x = MOUI_DISP_W / 2;

    for (int i = 0; i < 20; i++) {
        float a = t + i * 0.314f;
        int cx2 = center_x + (int)(sinf(a * 1.1f) * (center_x - 24));
        int cy = 120 + (int)(cosf(a * 0.9f) * 80);
        int r = 5 + (i % 10);
        moui_draw_circle(ctx, cx2, cy, r, MOUI_BLACK);
    }

    for (int i = 0; i < 15; i++) {
        float a = t * 1.5f + i * 0.42f;
        int x1 = center_x + (int)(cosf(a) * (center_x - 14));
        int y1 = 250 + (int)(sinf(a) * 50);
        int x2 = center_x + (int)(cosf(a + 2.0f) * (center_x - 14));
        int y2 = 250 + (int)(sinf(a + 2.0f) * 50);
        moui_draw_line(ctx, x1, y1, x2, y2, MOUI_BLACK);
    }

    for (int i = 0; i < 8; i++) {
        float a = t * 0.8f + i * 0.785f;
        int x = 20 + (int)(sinf(a) * (center_x - 30) + (center_x - 30));
        int w2 = 20 + (int)(sinf(a * 2.0f) * 10);
        int y = 330 + i * 6;
        moui_draw_fill_rect(ctx, &(moui_rect_t){x, y, w2, 4}, MOUI_BLACK);
    }
}

static moui_widget_t stress_canvas;
static const moui_widget_vtable_t stress_canvas_vtable = { .draw = stress_draw_custom };

static void setup_stress_screen(void)
{
    moui_screen_init(&scr_stress);
    scr_stress.enter_trans = MOUI_TRANS_SLIDE_UP;
    scr_stress.on_tick = stress_tick;

    moui_label_init(&stress_title, "Stress Test", &moui_font_ascii_6x8);
    stress_title.inverted = true;
    stress_title.base.bounds = (moui_rect_t){0, 0, MOUI_DISP_W, 16};
    moui_screen_add_widget(&scr_stress, &stress_title.base);

    snprintf(stress_info_buf, sizeof(stress_info_buf), "FPS:--  43 prims/frame");
    moui_label_init(&stress_info, stress_info_buf, &moui_font_ascii_6x8);
    stress_info.base.bounds = (moui_rect_t){4, 18, MOUI_DISP_W - 8, 10};
    moui_screen_add_widget(&scr_stress, &stress_info.base);

    moui_widget_init(&stress_canvas, &stress_canvas_vtable);
    stress_canvas.bounds = (moui_rect_t){0, 30, MOUI_DISP_W, MOUI_DISP_H - 30};
    moui_screen_add_widget(&scr_stress, &stress_canvas);
}

/* ====================================================================
 *  Screen: Widgets Demo — Page 1 (StatusBar/Progress/Radio/Spinner/
 *  Dropdown/Switch/Gauge)
 * ==================================================================== */
static moui_screen_t           scr_widgets;
static moui_widget_statusbar_t wdg_statusbar;
static moui_widget_progress_t  wdg_progress;
static moui_widget_radio_t     wdg_radio;
static moui_widget_spinner_t   wdg_spinner;
static moui_widget_dropdown_t  wdg_dropdown;
static moui_widget_switch_t    wdg_switch;
static moui_widget_gauge_t     wdg_gauge;
static moui_widget_label_t     wdg_lbl1, wdg_lbl2, wdg_lbl3, wdg_lbl4, wdg_lbl5;
static moui_widget_loading_t   wdg_loading;

static uint32_t wdg_last_tick = 0;
static int32_t  wdg_progress_val = 0;
static char     wdg_time_buf[8];

static const char *wdg_radio_opts[] = { "Low", "Med", "High" };
static const char *wdg_dd_opts[] = { "Option A", "Option B", "Option C", "Option D" };

static void widgets_tick(moui_screen_t *s, uint32_t now_ms, float dt)
{
    (void)s; (void)dt;
    if (now_ms - wdg_last_tick < 80) return;
    wdg_last_tick = now_ms;

    wdg_progress_val = (wdg_progress_val + 1) % 101;
    wdg_progress.value = wdg_progress_val;
    wdg_gauge.value = wdg_progress_val;

    uint32_t sec = now_ms / 1000;
    snprintf(wdg_time_buf, sizeof(wdg_time_buf), "%02d:%02d",
             (int)((sec / 60) % 60), (int)(sec % 60));
    moui_statusbar_set_right(&wdg_statusbar, wdg_time_buf);
    moui_statusbar_set_battery(&wdg_statusbar, (uint8_t)(100 - wdg_progress_val));

    moui_loading_tick(&wdg_loading, dt);
}

static void setup_widgets_screen(void)
{
    moui_screen_init(&scr_widgets);
    scr_widgets.enter_trans = MOUI_TRANS_SLIDE_LEFT;
    scr_widgets.on_tick = widgets_tick;
    int y = 0;

    moui_statusbar_init(&wdg_statusbar, &moui_font_ascii_6x8, "Widgets 1");
    wdg_statusbar.base.bounds = (moui_rect_t){0, y, MOUI_DISP_W, 14};
    moui_screen_add_widget(&scr_widgets, &wdg_statusbar.base);
    y += 16;

    moui_label_init(&wdg_lbl1, "Progress + Loading", &moui_font_ascii_6x8);
    wdg_lbl1.base.bounds = (moui_rect_t){4, y, 130, 10};
    moui_screen_add_widget(&scr_widgets, &wdg_lbl1.base);
    y += 11;

    moui_progress_init(&wdg_progress, &moui_font_ascii_6x8, 100);
    wdg_progress.base.bounds = (moui_rect_t){4, y, 130, 14};
    moui_screen_add_widget(&scr_widgets, &wdg_progress.base);

    moui_loading_init(&wdg_loading);
    wdg_loading.base.bounds = (moui_rect_t){140, y - 2, 18, 18};
    moui_screen_add_widget(&scr_widgets, &wdg_loading.base);
    y += 20;

    moui_label_init(&wdg_lbl2, "Dropdown", &moui_font_ascii_6x8);
    wdg_lbl2.base.bounds = (moui_rect_t){4, y, 80, 10};
    moui_screen_add_widget(&scr_widgets, &wdg_lbl2.base);
    y += 11;

    moui_dropdown_init(&wdg_dropdown, &moui_font_ascii_6x8);
    moui_dropdown_set_options(&wdg_dropdown, wdg_dd_opts, 4);
    wdg_dropdown.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 18};
    moui_screen_add_widget(&scr_widgets, &wdg_dropdown.base);
    y += 22;

    moui_label_init(&wdg_lbl3, "Switch + Radio", &moui_font_ascii_6x8);
    wdg_lbl3.base.bounds = (moui_rect_t){4, y, 120, 10};
    moui_screen_add_widget(&scr_widgets, &wdg_lbl3.base);
    y += 11;

    moui_switch_init(&wdg_switch);
    wdg_switch.base.bounds = (moui_rect_t){4, y, 36, 16};
    moui_screen_add_widget(&scr_widgets, &wdg_switch.base);

    moui_radio_init(&wdg_radio, &moui_font_ascii_6x8);
    moui_radio_set_options(&wdg_radio, wdg_radio_opts, 3);
    wdg_radio.base.bounds = (moui_rect_t){50, y, 110, 3 * wdg_radio.item_h};
    moui_screen_add_widget(&scr_widgets, &wdg_radio.base);
    y += 3 * wdg_radio.item_h + 4;

    moui_label_init(&wdg_lbl4, "Spinner", &moui_font_ascii_6x8);
    wdg_lbl4.base.bounds = (moui_rect_t){4, y, 80, 10};
    moui_screen_add_widget(&scr_widgets, &wdg_lbl4.base);
    y += 11;

    moui_spinner_init(&wdg_spinner, &moui_font_ascii_6x8, 0, 100, 5);
    wdg_spinner.value = 50;
    wdg_spinner.format = "%d%%";
    wdg_spinner.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 18};
    moui_screen_add_widget(&scr_widgets, &wdg_spinner.base);
    y += 22;

    moui_label_init(&wdg_lbl5, "Gauge", &moui_font_ascii_6x8);
    wdg_lbl5.base.bounds = (moui_rect_t){4, y, 80, 10};
    moui_screen_add_widget(&scr_widgets, &wdg_lbl5.base);
    y += 11;

    moui_gauge_init(&wdg_gauge, &moui_font_ascii_6x8, 0, 100);
    wdg_gauge.label = "Speed";
    wdg_gauge.unit = "%";
    int gx = (MOUI_DISP_W - 128) / 2;
    if (gx < 4) gx = 4;
    wdg_gauge.base.bounds = (moui_rect_t){(int16_t)gx, y, 128, 70};
    moui_screen_add_widget(&scr_widgets, &wdg_gauge.base);

    moui_focus_group_add(&scr_widgets.focus, &wdg_dropdown.base);
    moui_focus_group_add(&scr_widgets.focus, &wdg_switch.base);
    moui_focus_group_add(&scr_widgets.focus, &wdg_radio.base);
    moui_focus_group_add(&scr_widgets.focus, &wdg_spinner.base);
}

/* ====================================================================
 *  Screen: Widgets 2 — Tab/Table/Calendar/BtnMatrix/TextInput/
 *  Marquee/Badge/Divider/Icons
 * ==================================================================== */
static moui_screen_t           scr_widgets2;
static moui_widget_label_t     w2_title;
static moui_widget_tab_t       w2_tab;
static moui_widget_label_t     w2_tab_p1, w2_tab_p2, w2_tab_p3;
static moui_widget_textinput_t w2_textinput;
static moui_widget_btnmatrix_t w2_btnmatrix;
static moui_widget_marquee_t   w2_marquee;
static moui_widget_badge_t     w2_badge;
static moui_widget_divider_t   w2_divider;
static moui_widget_label_t     w2_lbl1, w2_lbl2, w2_lbl3, w2_lbl4, w2_lbl5;

static const char *w2_bm_labels[] = {"1","2","3","4","5","6","7","8","9","*","0","#"};

static void w2_tick(moui_screen_t *s, uint32_t now_ms, float dt)
{
    (void)s; (void)now_ms; (void)dt;
    moui_marquee_tick(&w2_marquee);
}

static void wdg_draw_icons2(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    int x = w->bounds.x + 4;
    int y = w->bounds.y;
    const moui_icon_id_t icon_ids[] = {
        MOUI_ICON_WIFI, MOUI_ICON_BLUETOOTH, MOUI_ICON_SETTINGS, MOUI_ICON_HOME,
        MOUI_ICON_TIMER, MOUI_ICON_ERROR, MOUI_ICON_CHECK, MOUI_ICON_CLOSE,
        MOUI_ICON_ARROW_UP, MOUI_ICON_ARROW_DOWN, MOUI_ICON_ARROW_LEFT, MOUI_ICON_ARROW_RIGHT,
    };
    for (int i = 0; i < 12; i++) {
        moui_draw_icon(ctx, x + (i % 6) * 26, y + (i / 6) * 20, icon_ids[i], MOUI_BLACK);
    }
}

static moui_widget_t w2_icon_canvas;
static const moui_widget_vtable_t w2_icon_vtable = { .draw = wdg_draw_icons2 };

static void setup_widgets2_screen(void)
{
    moui_screen_init(&scr_widgets2);
    scr_widgets2.enter_trans = MOUI_TRANS_SLIDE_LEFT;
    scr_widgets2.on_tick = w2_tick;
    int y = 0;

    moui_label_init(&w2_title, "Widgets 2", &moui_font_ascii_6x8);
    w2_title.inverted = true;
    w2_title.base.bounds = (moui_rect_t){0, y, MOUI_DISP_W, 16};
    moui_screen_add_widget(&scr_widgets2, &w2_title.base);
    y += 18;

    moui_label_init(&w2_lbl1, "Tab Pages", &moui_font_ascii_6x8);
    w2_lbl1.base.bounds = (moui_rect_t){4, y, 80, 10};
    moui_screen_add_widget(&scr_widgets2, &w2_lbl1.base);
    y += 11;

    moui_tab_init(&w2_tab, &moui_font_ascii_6x8);
    moui_label_init(&w2_tab_p1, "  Tab A content", &moui_font_ascii_6x8);
    moui_label_init(&w2_tab_p2, "  Tab B content", &moui_font_ascii_6x8);
    moui_label_init(&w2_tab_p3, "  Tab C content", &moui_font_ascii_6x8);
    moui_tab_add_page(&w2_tab, "A", &w2_tab_p1.base);
    moui_tab_add_page(&w2_tab, "B", &w2_tab_p2.base);
    moui_tab_add_page(&w2_tab, "C", &w2_tab_p3.base);
    w2_tab.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 36};
    moui_screen_add_widget(&scr_widgets2, &w2_tab.base);
    y += 40;

    moui_label_init(&w2_lbl2, "TextInput", &moui_font_ascii_6x8);
    w2_lbl2.base.bounds = (moui_rect_t){4, y, 80, 10};
    moui_screen_add_widget(&scr_widgets2, &w2_lbl2.base);
    y += 11;

    moui_textinput_init(&w2_textinput, &moui_font_ascii_6x8);
    moui_textinput_set_text(&w2_textinput, "Hello");
    w2_textinput.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 18};
    moui_screen_add_widget(&scr_widgets2, &w2_textinput.base);
    y += 22;

    moui_label_init(&w2_lbl3, "ButtonMatrix", &moui_font_ascii_6x8);
    w2_lbl3.base.bounds = (moui_rect_t){4, y, 100, 10};
    moui_screen_add_widget(&scr_widgets2, &w2_lbl3.base);
    y += 11;

    moui_btnmatrix_init(&w2_btnmatrix, &moui_font_ascii_6x8, 3);
    moui_btnmatrix_set_labels(&w2_btnmatrix, w2_bm_labels, 12);
    w2_btnmatrix.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 64};
    moui_screen_add_widget(&scr_widgets2, &w2_btnmatrix.base);
    y += 68;

    moui_label_init(&w2_lbl4, "Marquee + Badge", &moui_font_ascii_6x8);
    w2_lbl4.base.bounds = (moui_rect_t){4, y, 120, 10};
    moui_screen_add_widget(&scr_widgets2, &w2_lbl4.base);
    y += 11;

    moui_marquee_init(&w2_marquee, &moui_font_ascii_6x8, "This is a long scrolling marquee text for demo...");
    w2_marquee.base.bounds = (moui_rect_t){4, y, 140, 12};
    moui_screen_add_widget(&scr_widgets2, &w2_marquee.base);

    moui_badge_init(&w2_badge, &moui_font_ascii_6x8);
    w2_badge.count = 5;
    w2_badge.base.bounds = (moui_rect_t){150, y, 14, 14};
    moui_screen_add_widget(&scr_widgets2, &w2_badge.base);
    y += 18;

    moui_divider_init(&w2_divider, false);
    w2_divider.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 3};
    moui_screen_add_widget(&scr_widgets2, &w2_divider.base);
    y += 6;

    moui_label_init(&w2_lbl5, "Icons (13 x 16px)", &moui_font_ascii_6x8);
    w2_lbl5.base.bounds = (moui_rect_t){4, y, 140, 10};
    moui_screen_add_widget(&scr_widgets2, &w2_lbl5.base);
    y += 12;

    moui_widget_init(&w2_icon_canvas, &w2_icon_vtable);
    w2_icon_canvas.bounds = (moui_rect_t){0, y, MOUI_DISP_W, 44};
    moui_screen_add_widget(&scr_widgets2, &w2_icon_canvas);

    moui_focus_group_add(&scr_widgets2.focus, &w2_tab.base);
    moui_focus_group_add(&scr_widgets2.focus, &w2_textinput.base);
    moui_focus_group_add(&scr_widgets2.focus, &w2_btnmatrix.base);
}

/* ====================================================================
 *  Screen: Mono Effects (Dithering / Pattern Fills / QR Code)
 * ==================================================================== */
static moui_screen_t       scr_mono;
static moui_widget_label_t mono_title;

static void mono_draw_section_hdr(moui_draw_ctx_t *ctx, int y, const char *title)
{
    int tw = moui_font_measure_str(&moui_font_ascii_6x8, title);
    int lx = tw + 10;
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, 4, y + 1, title, MOUI_BLACK);
    moui_draw_hline(ctx, lx, y + 4, MOUI_DISP_W - lx - 4, MOUI_BLACK);
}

static void mono_draw_custom(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    (void)w;
    int y0 = 20;

    /* ─── Section 1: Dithering Gradient ─── */
    mono_draw_section_hdr(ctx, y0, "Dithering");
    y0 += 12;

    /* Gradient bar: solid → 75% → 50% → 25% → white */
    int bar_x = 8, bar_w = MOUI_DISP_W - 16, bar_h = 16;
    int seg_w = bar_w / 5;
    moui_draw_fill_rect(ctx, &(moui_rect_t){bar_x, y0, seg_w, bar_h}, MOUI_BLACK);
    moui_draw_fill_rect_dither(ctx, &(moui_rect_t){bar_x + seg_w, y0, seg_w, bar_h}, MOUI_DITHER_75);
    moui_draw_fill_rect_dither(ctx, &(moui_rect_t){bar_x + seg_w*2, y0, seg_w, bar_h}, MOUI_DITHER_50);
    moui_draw_fill_rect_dither(ctx, &(moui_rect_t){bar_x + seg_w*3, y0, seg_w, bar_h}, MOUI_DITHER_25);
    /* last segment stays white */
    moui_draw_rect(ctx, &(moui_rect_t){bar_x, y0, bar_w, bar_h}, MOUI_BLACK);
    y0 += bar_h + 2;

    /* Labels below gradient */
    const char *grad_lbl[] = {"100%","75%","50%","25%","0%"};
    for (int i = 0; i < 5; i++) {
        int tw = moui_font_measure_str(&moui_font_ascii_6x8, grad_lbl[i]);
        moui_font_draw_str(ctx, &moui_font_ascii_6x8,
            bar_x + i * seg_w + (seg_w - tw) / 2, y0, grad_lbl[i], MOUI_BLACK);
    }
    y0 += 12;

    /* Dither pattern circles row */
    const char *dither_names[] = {"Chk","H","V","Diag"};
    moui_dither_pattern_t dpat[] = {MOUI_DITHER_CHECKER, MOUI_DITHER_HLINE, MOUI_DITHER_VLINE, MOUI_DITHER_DIAG};
    for (int i = 0; i < 4; i++) {
        int cx = 24 + i * 38;
        int cy = y0 + 14;
        moui_draw_fill_circle_dither(ctx, cx, cy, 12, dpat[i]);
        moui_draw_circle(ctx, cx, cy, 12, MOUI_BLACK);
        int tw = moui_font_measure_str(&moui_font_ascii_6x8, dither_names[i]);
        moui_font_draw_str(ctx, &moui_font_ascii_6x8, cx - tw / 2, cy + 15, dither_names[i], MOUI_BLACK);
    }
    y0 += 42;

    /* ─── Section 2: Pattern Fills ─── */
    mono_draw_section_hdr(ctx, y0, "Patterns");
    y0 += 12;

    const char *pat_names[] = {"Dots","HLine","VLine","Grid","Diag\\","Diag/","Cross","Brick"};
    int pw = 36, ph = 24, gap = 4;
    int cols = 4;
    int grid_w = cols * pw + (cols - 1) * gap;
    int ox = (MOUI_DISP_W - grid_w) / 2;

    for (int i = 0; i < 8; i++) {
        int col = i % cols;
        int row = i / cols;
        int rx = ox + col * (pw + gap);
        int ry = y0 + row * (ph + 14 + gap);
        moui_rect_t r = {rx, ry, pw, ph};
        moui_draw_fill_rect_pattern(ctx, &r, (moui_pattern_t)i, 4);
        moui_draw_rounded_rect(ctx, &r, 2, MOUI_BLACK);
        int tw = moui_font_measure_str(&moui_font_ascii_6x8, pat_names[i]);
        moui_font_draw_str(ctx, &moui_font_ascii_6x8, rx + (pw - tw) / 2, ry + ph + 2, pat_names[i], MOUI_BLACK);
    }
    y0 += 2 * (ph + 14 + gap) + 2;

    /* ─── Section 3: QR Code ─── */
    mono_draw_section_hdr(ctx, y0, "QR Code");
    y0 += 12;

    moui_qr_t qr;
    if (moui_qr_encode(&qr, "https://github.com/moui")) {
        int scale = 2;
        int qr_px = qr.size * scale;
        int quiet = 4 * scale;  /* quiet zone drawn by moui_qr_draw */
        int frame_pad = quiet + 3;  /* quiet zone + margin to frame border */
        int frame_w = qr_px + frame_pad * 2;
        int frame_h = qr_px + frame_pad * 2 + 16;
        int fx = (MOUI_DISP_W - frame_w) / 2;
        int fy = y0;

        /* Decorative frame */
        moui_draw_rounded_rect(ctx, &(moui_rect_t){fx, fy, frame_w, frame_h}, 4, MOUI_BLACK);
        moui_draw_rounded_rect(ctx, &(moui_rect_t){fx+1, fy+1, frame_w-2, frame_h-2}, 3, MOUI_BLACK);

        /* QR code centered in frame */
        int qx = fx + frame_pad;
        int qy = fy + frame_pad;
        moui_qr_draw(ctx, &qr, qx, qy, scale);

        /* Caption below QR inside frame */
        const char *cap = "Scan Me";
        int tw = moui_font_measure_str(&moui_font_ascii_6x8, cap);
        moui_font_draw_str(ctx, &moui_font_ascii_6x8,
            fx + (frame_w - tw) / 2, fy + frame_pad + qr_px + 4, cap, MOUI_BLACK);

        y0 = fy + frame_h + 4;
    }

    /* Footer info */
    const char *footer = "1-bit mono rendering";
    int ftw = moui_font_measure_str(&moui_font_ascii_6x8, footer);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, (MOUI_DISP_W - ftw) / 2, y0, footer, MOUI_BLACK);
}

static moui_widget_t mono_canvas;
static const moui_widget_vtable_t mono_canvas_vtable = { .draw = mono_draw_custom };

static void setup_mono_screen(void)
{
    moui_screen_init(&scr_mono);
    scr_mono.enter_trans = MOUI_TRANS_SLIDE_LEFT;

    moui_label_init(&mono_title, "Mono Effects", &moui_font_ascii_6x8);
    mono_title.inverted = true;
    mono_title.base.bounds = (moui_rect_t){0, 0, MOUI_DISP_W, 16};
    moui_screen_add_widget(&scr_mono, &mono_title.base);

    moui_widget_init(&mono_canvas, &mono_canvas_vtable);
    mono_canvas.bounds = (moui_rect_t){0, 16, MOUI_DISP_W, MOUI_DISP_H - 16};
    moui_screen_add_widget(&scr_mono, &mono_canvas);
}

/* ====================================================================
 *  Screen: Widgets 3 — Calendar / Table / Scroll
 * ==================================================================== */
static moui_screen_t            scr_widgets3;
static moui_widget_label_t      w3_title, w3_lbl1, w3_lbl2;
static moui_widget_calendar_t   w3_calendar;
static moui_widget_table_t      w3_table;
static moui_widget_scroll_t     w3_scroll;
static moui_widget_label_t      w3_scroll_items[8];
static char                    w3_scroll_bufs[8][24];

static void setup_widgets3_screen(void)
{
    moui_screen_init(&scr_widgets3);
    scr_widgets3.enter_trans = MOUI_TRANS_SLIDE_LEFT;
    int y = 0;

    moui_label_init(&w3_title, "Widgets 3", &moui_font_ascii_6x8);
    w3_title.inverted = true;
    w3_title.base.bounds = (moui_rect_t){0, y, MOUI_DISP_W, 16};
    moui_screen_add_widget(&scr_widgets3, &w3_title.base);
    y += 18;

    /* Calendar */
    moui_label_init(&w3_lbl1, "Calendar", &moui_font_ascii_6x8);
    w3_lbl1.base.bounds = (moui_rect_t){4, y, 80, 10};
    moui_screen_add_widget(&scr_widgets3, &w3_lbl1.base);
    y += 11;

    moui_calendar_init(&w3_calendar, &moui_font_ascii_6x8, 2026, 5, 22);
    w3_calendar.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 110};
    moui_screen_add_widget(&scr_widgets3, &w3_calendar.base);
    y += 114;

    /* Table */
    moui_label_init(&w3_lbl2, "Table", &moui_font_ascii_6x8);
    w3_lbl2.base.bounds = (moui_rect_t){4, y, 80, 10};
    moui_screen_add_widget(&scr_widgets3, &w3_lbl2.base);
    y += 11;

    moui_table_init(&w3_table, &moui_font_ascii_6x8, 3, 5);
    w3_table.header_row = true;
    w3_table.col_w[0] = 40;
    w3_table.col_w[1] = 56;
    w3_table.col_w[2] = 56;
    moui_table_set_cell(&w3_table, 0, 0, "Name");
    moui_table_set_cell(&w3_table, 0, 1, "Value");
    moui_table_set_cell(&w3_table, 0, 2, "Unit");
    moui_table_set_cell(&w3_table, 1, 0, "Temp");
    moui_table_set_cell(&w3_table, 1, 1, "23.5");
    moui_table_set_cell(&w3_table, 1, 2, "C");
    moui_table_set_cell(&w3_table, 2, 0, "Humi");
    moui_table_set_cell(&w3_table, 2, 1, "55.2");
    moui_table_set_cell(&w3_table, 2, 2, "%");
    moui_table_set_cell(&w3_table, 3, 0, "Press");
    moui_table_set_cell(&w3_table, 3, 1, "1013");
    moui_table_set_cell(&w3_table, 3, 2, "hPa");
    moui_table_set_cell(&w3_table, 4, 0, "Light");
    moui_table_set_cell(&w3_table, 4, 1, "420");
    moui_table_set_cell(&w3_table, 4, 2, "lux");
    w3_table.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 12, 5 * w3_table.row_h + 2};
    moui_screen_add_widget(&scr_widgets3, &w3_table.base);
    y += 5 * w3_table.row_h + 6;

    /* Scroll container with items */
    moui_scroll_init(&w3_scroll);
    w3_scroll.show_scrollbar = true;
    w3_scroll.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 80};
    moui_scroll_set_content_height(&w3_scroll, 8 * 14);
    for (int i = 0; i < 8; i++) {
        snprintf(w3_scroll_bufs[i], sizeof(w3_scroll_bufs[i]), "Scroll item #%d", i + 1);
        moui_label_init(&w3_scroll_items[i], w3_scroll_bufs[i], &moui_font_ascii_6x8);
        w3_scroll_items[i].base.bounds = (moui_rect_t){0, i * 14, 140, 12};
        moui_scroll_add(&w3_scroll, &w3_scroll_items[i].base);
    }
    moui_screen_add_widget(&scr_widgets3, &w3_scroll.base);

    moui_focus_group_add(&scr_widgets3.focus, &w3_calendar.base);
    moui_focus_group_add(&scr_widgets3.focus, &w3_scroll.base);
}

/* ====================================================================
 *  Screen: Widgets 4 — Ring / Dots / Number / Stepper / Sparkline /
 *  Checklist
 * ==================================================================== */
static moui_screen_t            scr_widgets4;
static moui_widget_label_t      w4_title;
static moui_widget_ring_t       w4_ring;
static moui_widget_dots_t       w4_dots;
static moui_widget_number_t     w4_number;
static moui_widget_stepper_t    w4_stepper;
static moui_widget_sparkline_t  w4_sparkline;
static moui_widget_checklist_t  w4_checklist;
static moui_widget_label_t      w4_lbl1, w4_lbl2;

static uint32_t w4_last_tick = 0;
static float    w4_t = 0;

static const char *w4_check_items[] = {"WiFi", "BLE", "GPS", "NFC"};

static void widgets4_tick(moui_screen_t *s, uint32_t now_ms, float dt)
{
    (void)s; (void)dt;
    if (now_ms - w4_last_tick < 100) return;
    w4_last_tick = now_ms;
    w4_t += 0.1f;

    w4_ring.value = (int32_t)(50.0f + sinf(w4_t) * 50.0f);
    w4_number.value = w4_ring.value;
    w4_dots.active = ((int)(w4_t * 2) % w4_dots.total);
    moui_sparkline_push(&w4_sparkline, (int16_t)(50 + sinf(w4_t * 1.3f) * 40));
}

static void setup_widgets4_screen(void)
{
    moui_screen_init(&scr_widgets4);
    scr_widgets4.enter_trans = MOUI_TRANS_SLIDE_LEFT;
    scr_widgets4.on_tick = widgets4_tick;
    int y = 0;

    moui_label_init(&w4_title, "Widgets 4", &moui_font_ascii_6x8);
    w4_title.inverted = true;
    w4_title.base.bounds = (moui_rect_t){0, y, MOUI_DISP_W, 16};
    moui_screen_add_widget(&scr_widgets4, &w4_title.base);
    y += 18;

    /* Ring + Number side by side */
    moui_ring_init(&w4_ring, &moui_font_ascii_6x8, 100);
    w4_ring.label = "CPU";
    w4_ring.base.bounds = (moui_rect_t){4, y, 70, 70};
    moui_screen_add_widget(&scr_widgets4, &w4_ring.base);

    moui_number_init(&w4_number, &moui_font_inter_16, 3);
    w4_number.unit = "%";
    w4_number.base.bounds = (moui_rect_t){80, y, 80, 30};
    moui_screen_add_widget(&scr_widgets4, &w4_number.base);

    /* Sparkline */
    moui_label_init(&w4_lbl1, "Sparkline", &moui_font_ascii_6x8);
    w4_lbl1.base.bounds = (moui_rect_t){80, y + 32, 60, 10};
    moui_screen_add_widget(&scr_widgets4, &w4_lbl1.base);

    moui_sparkline_init(&w4_sparkline, 0, 100);
    w4_sparkline.base.bounds = (moui_rect_t){80, y + 44, 78, 24};
    moui_screen_add_widget(&scr_widgets4, &w4_sparkline.base);
    y += 74;

    /* Page dots */
    moui_dots_init(&w4_dots, 5);
    w4_dots.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 14};
    moui_screen_add_widget(&scr_widgets4, &w4_dots.base);
    y += 18;

    /* Stepper */
    moui_label_init(&w4_lbl2, "Stepper", &moui_font_ascii_6x8);
    w4_lbl2.base.bounds = (moui_rect_t){4, y, 60, 10};
    moui_screen_add_widget(&scr_widgets4, &w4_lbl2.base);
    y += 12;

    moui_stepper_init(&w4_stepper, &moui_font_ascii_6x8, 0, 100, 5);
    w4_stepper.value = 50;
    w4_stepper.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 20};
    moui_screen_add_widget(&scr_widgets4, &w4_stepper.base);
    y += 24;

    /* Checklist */
    moui_checklist_init(&w4_checklist, &moui_font_ascii_6x8);
    moui_checklist_set_items(&w4_checklist, w4_check_items, 4);
    w4_checklist.checked = 0x01; /* WiFi pre-checked */
    w4_checklist.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 4 * w4_checklist.item_h};
    moui_screen_add_widget(&scr_widgets4, &w4_checklist.base);

    moui_focus_group_add(&scr_widgets4.focus, &w4_stepper.base);
    moui_focus_group_add(&scr_widgets4.focus, &w4_checklist.base);
}

/* ====================================================================
 *  Screen: Widgets 5 — New 6 Components (TimePicker, BarChart, Roller, IconBar, TreeView, LogView)
 * ==================================================================== */
static moui_screen_t           scr_widgets5;
static moui_widget_label_t     w5_title;
static moui_widget_icon_bar_t  w5_iconbar;
static moui_widget_label_t     w5_lbl1, w5_lbl2, w5_lbl3;
static moui_widget_time_picker_t w5_tp;
static moui_widget_roller_t    w5_roller;
static moui_widget_barchart_t  w5_barchart;
static moui_widget_treeview_t  w5_tree;
static moui_widget_logview_t   w5_log;

static const char *w5_roller_opts[] = { "Mode A", "Mode B", "Mode C", "Mode D" };
static int32_t w5_bar_vals[5] = { 45, 80, 60, 95, 30 };
static const char *w5_bar_lbls[5] = { "M", "T", "W", "T", "F" };

static uint32_t w5_last_tick = 0;
static uint32_t w5_log_cnt = 0;
static char w5_log_buf[32];

static void widgets5_tick(moui_screen_t *s, uint32_t now_ms, float dt)
{
    (void)s; (void)dt;
    if (now_ms - w5_last_tick < 800) return;
    w5_last_tick = now_ms;
    w5_log_cnt++;

    snprintf(w5_log_buf, sizeof(w5_log_buf), "Log #%02d event ok", (int)(w5_log_cnt % 100));
    moui_logview_add_line(&w5_log, w5_log_buf);

    int8_t bars = (w5_log_cnt % 5);
    moui_icon_bar_set_wifi(&w5_iconbar, bars);
}

static void setup_widgets5_screen(void)
{
    moui_screen_init(&scr_widgets5);
    scr_widgets5.enter_trans = MOUI_TRANS_SLIDE_LEFT;
    scr_widgets5.on_tick = widgets5_tick;
    int y = 0;
    int w = MOUI_DISP_W;

    int iconbar_w = 95;
    moui_label_init(&w5_title, "New Widgets (6)", &moui_font_ascii_6x8);
    w5_title.inverted = true;
    w5_title.base.bounds = (moui_rect_t){0, (int16_t)y, (int16_t)(w - iconbar_w), 16};
    moui_screen_add_widget(&scr_widgets5, &w5_title.base);

    moui_icon_bar_init(&w5_iconbar, &moui_font_ascii_6x8);
    moui_icon_bar_set_battery(&w5_iconbar, 90, false);
    moui_icon_bar_set_wifi(&w5_iconbar, 4);
    w5_iconbar.base.bounds = (moui_rect_t){(int16_t)(w - iconbar_w), (int16_t)y, (int16_t)iconbar_w, 16};
    moui_screen_add_widget(&scr_widgets5, &w5_iconbar.base);
    y += 18;

    int half_w = (w - 12) / 2;

    moui_label_init(&w5_lbl1, "TimePicker & Roller", &moui_font_ascii_6x8);
    w5_lbl1.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 10};
    moui_screen_add_widget(&scr_widgets5, &w5_lbl1.base);
    y += 11;

    moui_time_picker_init(&w5_tp, &moui_font_ascii_6x8);
    moui_time_picker_set_time(&w5_tp, 14, 30, 45);
    w5_tp.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)half_w, 42};
    moui_screen_add_widget(&scr_widgets5, &w5_tp.base);

    moui_roller_init(&w5_roller, &moui_font_ascii_6x8);
    moui_roller_set_items(&w5_roller, w5_roller_opts, 4);
    w5_roller.base.bounds = (moui_rect_t){(int16_t)(8 + half_w), (int16_t)y, (int16_t)half_w, 42};
    moui_screen_add_widget(&scr_widgets5, &w5_roller.base);
    y += 46;

    moui_label_init(&w5_lbl2, "BarChart Histogram", &moui_font_ascii_6x8);
    w5_lbl2.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 10};
    moui_screen_add_widget(&scr_widgets5, &w5_lbl2.base);
    y += 11;

    moui_barchart_init(&w5_barchart, &moui_font_ascii_6x8, 0, 100);
    moui_barchart_set_data(&w5_barchart, w5_bar_vals, w5_bar_lbls, 5);
    w5_barchart.show_values = true;
    w5_barchart.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 105};
    moui_screen_add_widget(&scr_widgets5, &w5_barchart.base);
    y += 110;

    moui_label_init(&w5_lbl3, "TreeView & LogView Console", &moui_font_ascii_6x8);
    w5_lbl3.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 10};
    moui_screen_add_widget(&scr_widgets5, &w5_lbl3.base);
    y += 11;

    int bot_h = MOUI_DISP_H - y - 4;
    if (bot_h < 85) bot_h = 85;

    moui_treeview_init(&w5_tree, &moui_font_ascii_6x8);
    moui_treeview_add_node(&w5_tree, "System", 0, true);
    moui_treeview_add_node(&w5_tree, "Network", 1, true);
    moui_treeview_add_node(&w5_tree, "WiFi", 2, false);
    moui_treeview_add_node(&w5_tree, "BLE", 2, false);
    moui_treeview_add_node(&w5_tree, "Display", 1, false);
    w5_tree.nodes[0].expanded = true;
    w5_tree.nodes[1].expanded = true;
    w5_tree.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)half_w, (int16_t)bot_h};
    moui_screen_add_widget(&scr_widgets5, &w5_tree.base);

    moui_logview_init(&w5_log, &moui_font_ascii_6x8);
    moui_logview_add_line(&w5_log, "Init logview ok");
    moui_logview_add_line(&w5_log, "ST7305 4.2\" boot");
    w5_log.base.bounds = (moui_rect_t){(int16_t)(8 + half_w), (int16_t)y, (int16_t)half_w, (int16_t)bot_h};
    moui_screen_add_widget(&scr_widgets5, &w5_log.base);

    moui_focus_group_add(&scr_widgets5.focus, &w5_tp.base);
    moui_focus_group_add(&scr_widgets5.focus, &w5_roller.base);
    moui_focus_group_add(&scr_widgets5.focus, &w5_tree.base);
}

/* ====================================================================
 *  Screen: Architecture Features (Flexbox, Anchor, Property, Timeline)
 * ==================================================================== */
static moui_screen_t           scr_arch;
static moui_widget_label_t     arch_title;
static moui_widget_icon_bar_t  arch_iconbar;
static moui_widget_label_t     arch_lbl1, arch_lbl2, arch_lbl3;

/* Flexbox items */
static moui_widget_button_t    arch_btn_f1, arch_btn_f2, arch_btn_f3;
static moui_flex_t             arch_flex;

/* Property Binding items */
static moui_widget_slider_t    arch_slider;
static moui_widget_progress_t  arch_progress;
static moui_widget_label_t     arch_prop_lbl;
static moui_prop_int_t         arch_prop;
static char                    arch_prop_str[32];

/* Timeline items */
static moui_widget_button_t    arch_btn_play;
static moui_widget_button_t    arch_anim_b1, arch_anim_b2, arch_anim_b3;
static moui_timeline_t         arch_tl;

static void on_arch_prop_change(moui_prop_int_t *prop, int32_t old_val, int32_t new_val, void *user_data)
{
    (void)prop; (void)old_val; (void)user_data;
    arch_progress.value = new_val;
    snprintf(arch_prop_str, sizeof(arch_prop_str), "Value: %d%%", (int)new_val);
}

static void on_arch_slider_change(moui_widget_slider_t *s, int32_t val)
{
    (void)s;
    moui_prop_int_set(&arch_prop, val);
}

static void on_arch_play_click(moui_widget_button_t *b)
{
    (void)b;
    moui_timeline_start(&arch_tl, 0);
}

static void arch_tick(moui_screen_t *s, uint32_t now_ms, float dt)
{
    (void)s; (void)dt;
    moui_timeline_update(&arch_tl, now_ms, (struct moui_screen_mgr_t *)s_mgr);
}

static void setup_arch_screen(void)
{
    moui_screen_init(&scr_arch);
    scr_arch.enter_trans = MOUI_TRANS_SLIDE_LEFT;
    scr_arch.on_tick = arch_tick;
    int y = 0;
    int w = MOUI_DISP_W;

    int iconbar_w = 95;
    moui_label_init(&arch_title, "Architecture (4)", &moui_font_ascii_6x8);
    arch_title.inverted = true;
    arch_title.base.bounds = (moui_rect_t){0, (int16_t)y, (int16_t)(w - iconbar_w), 16};
    moui_screen_add_widget(&scr_arch, &arch_title.base);

    moui_icon_bar_init(&arch_iconbar, &moui_font_ascii_6x8);
    moui_icon_bar_set_battery(&arch_iconbar, 98, true);
    moui_icon_bar_set_wifi(&arch_iconbar, 4);
    arch_iconbar.base.bounds = (moui_rect_t){(int16_t)(w - iconbar_w), (int16_t)y, (int16_t)iconbar_w, 16};
    moui_screen_add_widget(&scr_arch, &arch_iconbar.base);
    y += 20;

    /* 1. Flexbox Layout Engine */
    moui_label_init(&arch_lbl1, "1. Flexbox Layout (Auto Spacing)", &moui_font_ascii_6x8);
    arch_lbl1.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 10};
    moui_screen_add_widget(&scr_arch, &arch_lbl1.base);
    y += 12;

    moui_button_init(&arch_btn_f1, "[ Flex A ]", &moui_font_ascii_6x8, MOUI_BTN_PUSH);
    moui_button_init(&arch_btn_f2, "[ Flex B ]", &moui_font_ascii_6x8, MOUI_BTN_PUSH);
    moui_button_init(&arch_btn_f3, "[ Flex C ]", &moui_font_ascii_6x8, MOUI_BTN_PUSH);
    arch_btn_f1.base.bounds = (moui_rect_t){0, 0, 75, 22};
    arch_btn_f2.base.bounds = (moui_rect_t){0, 0, 75, 22};
    arch_btn_f3.base.bounds = (moui_rect_t){0, 0, 75, 22};

    moui_flex_init(&arch_flex, MOUI_FLEX_DIR_ROW, MOUI_FLEX_JUSTIFY_SPACE_BETWEEN, MOUI_FLEX_ALIGN_CENTER);
    arch_flex.padding = 4;
    moui_flex_add(&arch_flex, &arch_btn_f1.base);
    moui_flex_add(&arch_flex, &arch_btn_f2.base);
    moui_flex_add(&arch_flex, &arch_btn_f3.base);

    moui_rect_t flex_box = {0, (int16_t)y, (int16_t)w, 30};
    moui_flex_layout(&arch_flex, &flex_box);

    moui_screen_add_widget(&scr_arch, &arch_btn_f1.base);
    moui_screen_add_widget(&scr_arch, &arch_btn_f2.base);
    moui_screen_add_widget(&scr_arch, &arch_btn_f3.base);
    y += 34;

    /* 2. Reactive Property Observer */
    moui_label_init(&arch_lbl2, "2. Reactive Data-Binding Observer", &moui_font_ascii_6x8);
    arch_lbl2.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 10};
    moui_screen_add_widget(&scr_arch, &arch_lbl2.base);
    y += 12;

    moui_prop_int_init(&arch_prop, 65);
    moui_prop_int_observe(&arch_prop, on_arch_prop_change, NULL, NULL);

    moui_slider_init(&arch_slider, &moui_font_ascii_6x8, 0, 100, 5);
    arch_slider.value = 65;
    arch_slider.on_change = on_arch_slider_change;
    arch_slider.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 20};
    moui_screen_add_widget(&scr_arch, &arch_slider.base);
    y += 24;

    moui_progress_init(&arch_progress, &moui_font_ascii_6x8, 100);
    arch_progress.value = 65;
    arch_progress.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 100), 16};
    moui_screen_add_widget(&scr_arch, &arch_progress.base);

    snprintf(arch_prop_str, sizeof(arch_prop_str), "Value: 65%%");
    moui_label_init(&arch_prop_lbl, arch_prop_str, &moui_font_ascii_6x8);
    arch_prop_lbl.base.bounds = (moui_rect_t){(int16_t)(w - 92), (int16_t)y, 88, 16};
    moui_screen_add_widget(&scr_arch, &arch_prop_lbl.base);
    y += 26;

    /* 3. Timeline Keyframe Animation Engine */
    moui_label_init(&arch_lbl3, "3. Timeline Multi-Anim Engine", &moui_font_ascii_6x8);
    arch_lbl3.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 10};
    moui_screen_add_widget(&scr_arch, &arch_lbl3.base);
    y += 12;

    moui_button_init(&arch_btn_play, "[ Replay Timeline Anim ]", &moui_font_ascii_6x8, MOUI_BTN_PUSH);
    arch_btn_play.on_click = on_arch_play_click;
    arch_btn_play.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 22};
    moui_screen_add_widget(&scr_arch, &arch_btn_play.base);
    y += 28;

    moui_button_init(&arch_anim_b1, "Card #1", &moui_font_ascii_6x8, MOUI_BTN_PUSH);
    moui_button_init(&arch_anim_b2, "Card #2", &moui_font_ascii_6x8, MOUI_BTN_PUSH);
    moui_button_init(&arch_anim_b3, "Card #3", &moui_font_ascii_6x8, MOUI_BTN_PUSH);

    arch_anim_b1.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 24};
    arch_anim_b2.base.bounds = (moui_rect_t){4, (int16_t)(y + 28), (int16_t)(w - 8), 24};
    arch_anim_b3.base.bounds = (moui_rect_t){4, (int16_t)(y + 56), (int16_t)(w - 8), 24};

    moui_screen_add_widget(&scr_arch, &arch_anim_b1.base);
    moui_screen_add_widget(&scr_arch, &arch_anim_b2.base);
    moui_screen_add_widget(&scr_arch, &arch_anim_b3.base);

    moui_timeline_init(&arch_tl);
    moui_timeline_add(&arch_tl, &arch_anim_b1.base, MOUI_TL_TARGET_X, -200, 4, 0, 400, moui_ease_out_cubic);
    moui_timeline_add(&arch_tl, &arch_anim_b2.base, MOUI_TL_TARGET_X, -200, 4, 200, 400, moui_ease_out_cubic);
    moui_timeline_add(&arch_tl, &arch_anim_b3.base, MOUI_TL_TARGET_X, -200, 4, 400, 400, moui_ease_out_cubic);
    moui_timeline_start(&arch_tl, 0);

    moui_focus_group_add(&scr_arch.focus, &arch_btn_f1.base);
    moui_focus_group_add(&scr_arch.focus, &arch_slider.base);
    moui_focus_group_add(&scr_arch.focus, &arch_btn_play.base);
}

/* ====================================================================
 *  Screen: Image Test — full-screen 300x384 test card
 * ==================================================================== */
static moui_screen_t       scr_image;
static moui_widget_label_t img_title;

static void img_draw_custom(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    /* Full-screen scaled bitmap rendering filling entire 300x384 canvas */
    moui_draw_bitmap_scaled(ctx, w->bounds.x, w->bounds.y, w->bounds.w, w->bounds.h,
                           MOUI_IMAGE_TEST_W, MOUI_IMAGE_TEST_H, moui_image_test, MOUI_BLACK);
}

static moui_widget_t img_canvas;
static const moui_widget_vtable_t img_canvas_vtable = { .draw = img_draw_custom };

static void setup_image_screen(void)
{
    moui_screen_init(&scr_image);
    scr_image.enter_trans = MOUI_TRANS_SLIDE_LEFT;

    moui_label_init(&img_title, "Shenzhen CR Tower (300x384)", &moui_font_ascii_6x8);
    img_title.inverted = true;
    img_title.base.bounds = (moui_rect_t){0, 0, MOUI_DISP_W, 16};
    moui_screen_add_widget(&scr_image, &img_title.base);

    moui_widget_init(&img_canvas, &img_canvas_vtable);
    img_canvas.bounds = (moui_rect_t){0, 16, MOUI_DISP_W, (int16_t)(MOUI_DISP_H - 16)};
    moui_screen_add_widget(&scr_image, &img_canvas);
}

/* ====================================================================
 *  Screen: New Features — Dark mode, Grid, Prop font, Wrap label
 * ==================================================================== */
static moui_screen_t         scr_features;
static moui_widget_label_t   feat_title;
static moui_widget_button_t  feat_dark_btn;
static moui_widget_label_t   feat_grid_lbl;
static moui_grid_t           feat_grid;
static moui_widget_label_t   feat_grid_items[9];
static char                 feat_grid_bufs[9][4];
static moui_widget_label_t   feat_font_lbl;
static moui_widget_label_t   feat_wrap_lbl;

static void on_dark_toggle(moui_widget_button_t *b, bool state)
{
    (void)b;
    moui_theme.invert_colors = state;
}

static void setup_features_screen(void)
{
    moui_screen_init(&scr_features);
    scr_features.enter_trans = MOUI_TRANS_SLIDE_LEFT;
    int y = 0;

    moui_label_init(&feat_title, "New Features", &moui_font_ascii_6x8);
    feat_title.inverted = true;
    feat_title.base.bounds = (moui_rect_t){0, y, MOUI_DISP_W, 16};
    moui_screen_add_widget(&scr_features, &feat_title.base);
    y += 18;

    /* Dark mode toggle */
    moui_button_init(&feat_dark_btn, "Dark Mode", &moui_font_ascii_6x8, MOUI_BTN_TOGGLE);
    feat_dark_btn.on_toggle = on_dark_toggle;
    feat_dark_btn.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 18};
    moui_screen_add_widget(&scr_features, &feat_dark_btn.base);
    y += 22;

    /* Grid layout demo */
    moui_label_init(&feat_grid_lbl, "Grid Layout (3x3):", &moui_font_ascii_6x8);
    feat_grid_lbl.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 10};
    moui_screen_add_widget(&scr_features, &feat_grid_lbl.base);
    y += 12;

    moui_grid_init(&feat_grid, 3, 3);
    feat_grid.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 12, 78};
    feat_grid.gap = 2;
    feat_grid.padding = 2;
    for (int i = 0; i < 9; i++) {
        snprintf(feat_grid_bufs[i], sizeof(feat_grid_bufs[i]), "%d", i + 1);
        moui_label_init(&feat_grid_items[i], feat_grid_bufs[i], &moui_font_inter_16);
        feat_grid_items[i].inverted = true;
        moui_grid_add(&feat_grid, &feat_grid_items[i].base);
    }
    moui_grid_layout(&feat_grid);
    moui_screen_add_widget(&scr_features, &feat_grid.base);
    y += 82;

    /* Proportional font */
    moui_label_init(&feat_font_lbl, "Inter 16px proportional", &moui_font_inter_16);
    feat_font_lbl.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 18};
    moui_screen_add_widget(&scr_features, &feat_font_lbl.base);
    y += 22;

    /* Label auto-wrap */
    moui_label_init(&feat_wrap_lbl,
        "This text demonstrates auto word-wrap. Long sentences break at word boundaries.",
        &moui_font_ascii_6x8);
    feat_wrap_lbl.wrap = true;
    feat_wrap_lbl.base.bounds = (moui_rect_t){4, y, MOUI_DISP_W - 8, 60};
    moui_screen_add_widget(&scr_features, &feat_wrap_lbl.base);

    moui_focus_group_add(&scr_features.focus, &feat_dark_btn.base);
}

/* ====================================================================
 *  Screen: About
 * ==================================================================== */
static moui_screen_t        scr_about;
static moui_widget_label_t  about_title;
static moui_widget_label_t  about_l1, about_l2, about_l3, about_l4;
static moui_widget_label_t  about_l5, about_l6, about_l7, about_l8, about_l9;
static moui_widget_button_t about_ok;

static void on_confirm_exit(moui_popup_t *p, bool confirmed)
{
    (void)p;
    if (confirmed) moui_screen_pop(s_mgr);
}

static void on_about_ok(moui_widget_button_t *b)
{
    (void)b;
    moui_popup_show_confirm(&s_mgr->popup, "Exit", "Return to menu?", on_confirm_exit);
}

static char s_about_res_buf[32];
static char s_about_disp_buf[32];

static void setup_about_screen(void)
{
    moui_screen_init(&scr_about);
    scr_about.enter_trans = MOUI_TRANS_SLIDE_UP;
    int y = 0;
    int w = MOUI_DISP_W;

    moui_label_init(&about_title, "About", &moui_font_ascii_6x8);
    about_title.inverted = true;
    about_title.base.bounds = (moui_rect_t){0, (int16_t)y, (int16_t)w, 16};
    moui_screen_add_widget(&scr_about, &about_title.base); y += 24;

    moui_label_init(&about_l1, "Moui Framework", &moui_font_ascii_6x8);
    about_l1.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 10}; y += 14;
    moui_screen_add_widget(&scr_about, &about_l1.base);

    moui_label_init(&about_l2, "\xe8\xbd\xbb\xe9\x87\x8f\xe5\x8d\x95\xe8\x89\xb2\xe5\xb1\x8f UI", &moui_font_cn_12);
    about_l2.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 14}; y += 18;
    moui_screen_add_widget(&scr_about, &about_l2.base);

    moui_label_init(&about_l3, "----------------", &moui_font_ascii_6x8);
    about_l3.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 10}; y += 14;
    moui_screen_add_widget(&scr_about, &about_l3.base);

    const char *disp_name = "ST7305";
    if (moui_disp) {
        if (moui_disp->width == 480) disp_name = "SSD1677 (4.26\")";
        else if (moui_disp->width == 300) disp_name = "ST7305 (4.2\")";
        else if (moui_disp->width == 168) disp_name = "ST7305 (2.9\")";
        else if (moui_disp->width == 128) disp_name = "SSD1306 (0.96\")";
    }
    snprintf(s_about_disp_buf, sizeof(s_about_disp_buf), "Display: %s", disp_name);
    moui_label_init(&about_l4, s_about_disp_buf, &moui_font_ascii_6x8);
    about_l4.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 10}; y += 14;
    moui_screen_add_widget(&scr_about, &about_l4.base);

    const char *fmt_mode = "mono";
    if (moui_disp && moui_disp->pixel_format == MOUI_PIXEL_FORMAT_GRAY2_HMSB)
        fmt_mode = "4-gray";
    snprintf(s_about_res_buf, sizeof(s_about_res_buf), "Res: %dx%d %s", MOUI_DISP_W, MOUI_DISP_H, fmt_mode);
    moui_label_init(&about_l5, s_about_res_buf, &moui_font_ascii_6x8);
    about_l5.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 10}; y += 14;
    moui_screen_add_widget(&scr_about, &about_l5.base);

    moui_label_init(&about_l6, "MCU: ESP32-C61 / S3", &moui_font_ascii_6x8);
    about_l6.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 10}; y += 14;
    moui_screen_add_widget(&scr_about, &about_l6.base);

    moui_label_init(&about_l7, "SPI: 40MHz DMA", &moui_font_ascii_6x8);
    about_l7.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 10}; y += 14;
    moui_screen_add_widget(&scr_about, &about_l7.base);

    moui_label_init(&about_l8, "UI FW: ~22KB code", &moui_font_ascii_6x8);
    about_l8.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 10}; y += 14;
    moui_screen_add_widget(&scr_about, &about_l8.base);

    moui_label_init(&about_l9, "RAM: ~18KB total", &moui_font_ascii_6x8);
    about_l9.base.bounds = (moui_rect_t){4, (int16_t)y, (int16_t)(w - 8), 10}; y += 24;
    moui_screen_add_widget(&scr_about, &about_l9.base);

    int btn_w = 88;
    int btn_x = (w - btn_w) / 2;
    moui_button_init(&about_ok, "[ OK ]", &moui_font_ascii_6x8, MOUI_BTN_PUSH);
    about_ok.on_click = on_about_ok;
    about_ok.base.bounds = (moui_rect_t){(int16_t)btn_x, (int16_t)y, (int16_t)btn_w, 20};
    moui_screen_add_widget(&scr_about, &about_ok.base);
    moui_focus_group_add(&scr_about.focus, &about_ok.base);
}

/* ==================================================================== */

static void push_dashboard(void) { moui_screen_push(s_mgr, &scr_dash); }
static void push_settings(void)  { moui_screen_push(s_mgr, &scr_settings); }
static void push_chart(void)     { moui_screen_push(s_mgr, &scr_chart); }
static void push_graphics(void)  { moui_screen_push(s_mgr, &scr_gfx); }
static void push_stress(void)    { moui_screen_push(s_mgr, &scr_stress); }
static void push_widgets(void)   { moui_screen_push(s_mgr, &scr_widgets); }
static void push_widgets2(void)  { moui_screen_push(s_mgr, &scr_widgets2); }
static void push_widgets3(void)  { moui_screen_push(s_mgr, &scr_widgets3); }
static void push_widgets4(void)  { moui_screen_push(s_mgr, &scr_widgets4); }
static void push_widgets5(void)  { moui_screen_push(s_mgr, &scr_widgets5); }
static void push_arch(void)      { moui_screen_push(s_mgr, &scr_arch); }
static void push_mono(void)      { moui_screen_push(s_mgr, &scr_mono); }
static void push_image(void)     { moui_screen_push(s_mgr, &scr_image); }
static void push_features(void)  { moui_screen_push(s_mgr, &scr_features); }
static void push_about(void)     { moui_screen_push(s_mgr, &scr_about); }
static void push_icons(void)     { moui_screen_push(s_mgr, &scr_icons); }


/* ═══════════════════ Icons Gallery ═══════════════════ */

static moui_widget_label_t icons_title;
static moui_widget_t icons_canvas;

static void icons_draw_all(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    int ox = w->bounds.x + 6;
    int oy = w->bounds.y;

    /* 1. Featured 2x Scaled Icons Row (32x32 px each) */
    moui_icon_id_t featured[] = {
        MOUI_ICON_FACE_SMILE, MOUI_ICON_ROCKET, MOUI_ICON_HEART, MOUI_ICON_THUMBS_UP,
        MOUI_ICON_WIFI, MOUI_ICON_BATTERY_FULL, MOUI_ICON_SETTINGS
    };
    int feat_count = (int)(sizeof(featured) / sizeof(featured[0]));
    int fx = ox;
    for (int i = 0; i < feat_count && fx + 36 < MOUI_DISP_W; i++) {
        moui_draw_icon_scaled(ctx, fx, oy, featured[i], 2, MOUI_BLACK);
        moui_rect_t ibox = { (int16_t)(fx - 2), (int16_t)(oy - 2), 36, 36 };
        moui_draw_rect(ctx, &ibox, MOUI_BLACK);
        fx += 40;
    }
    oy += 40;

    moui_draw_hline(ctx, 4, oy, MOUI_DISP_W - 8, MOUI_BLACK);
    oy += 6;

    /* 2. Full 108 Material Mono Icons Grid */
    int cell_w = 22;
    int cell_h = 22;
    int cols = (MOUI_DISP_W - 8) / cell_w;
    if (cols < 4) cols = 4;

    for (int i = 0; i < MOUI_ICON_COUNT; i++) {
        int col = i % cols;
        int row = i / cols;
        int x = ox + col * cell_w;
        int y = oy + row * cell_h;
        moui_draw_icon(ctx, x, y, (moui_icon_id_t)i, MOUI_BLACK);
    }
}

static const moui_widget_vtable_t icons_canvas_vt = { .draw = icons_draw_all };

static void setup_icons_screen(void)
{
    moui_screen_init(&scr_icons);
    moui_label_init(&icons_title, "Material Mono Icons (128)", &moui_font_ascii_6x8);
    icons_title.inverted = true;
    icons_title.base.bounds = (moui_rect_t){0, 0, MOUI_DISP_W, 16};
    moui_screen_add_widget(&scr_icons, &icons_title.base);

    moui_widget_init(&icons_canvas, &icons_canvas_vt);
    icons_canvas.bounds = (moui_rect_t){0, 18, MOUI_DISP_W, MOUI_DISP_H - 18};
    moui_screen_add_widget(&scr_icons, &icons_canvas);
}

/* ═══════════════════ Virtualized List (100k) ═══════════════════ */

static moui_screen_t        scr_vlist;
static moui_widget_label_t  vlist_title;
static moui_widget_label_t  vlist_info;
static moui_vlist_t         vlist_demo;
static moui_widget_label_t  vlist_cell_pool[20];

static uint32_t demo_vlist_count(const moui_vlist_t *vl, void *ud)
{
    (void)vl; (void)ud;
    return 100000; /* 100,000 items */
}

static void demo_vlist_bind(const moui_vlist_t *vl, moui_widget_t *cell, uint32_t index, void *ud)
{
    (void)vl; (void)ud;
    moui_widget_label_t *lbl = (moui_widget_label_t *)cell;
    static char buf[64];
    const char *types[] = {"[FILE]", "[LOG]", "[SYS]", "[DATA]"};
    snprintf(buf, sizeof(buf), "%s #%06u — Moui Item", types[index % 4], (unsigned int)index);
    lbl->text = buf;
}

static void push_vlist(void) { moui_screen_push(s_mgr, &scr_vlist); }

static void setup_vlist_screen(void)
{
    moui_screen_init(&scr_vlist);
    scr_vlist.enter_trans = MOUI_TRANS_SLIDE_LEFT;

    moui_label_init(&vlist_title, "VirtualList (100,000 Items)", &moui_font_ascii_6x8);
    vlist_title.inverted = true;
    vlist_title.base.bounds = (moui_rect_t){0, 0, MOUI_DISP_W, 16};
    moui_screen_add_widget(&scr_vlist, &vlist_title.base);

    moui_label_init(&vlist_info, "RAM: <1KB | Recycled Pool: 20 Cells", &moui_font_ascii_6x8);
    vlist_info.base.bounds = (moui_rect_t){4, 18, MOUI_DISP_W - 8, 10};
    moui_screen_add_widget(&scr_vlist, &vlist_info.base);

    moui_vlist_init(&vlist_demo, 22, demo_vlist_count, demo_vlist_bind, NULL);
    vlist_demo.base.bounds = (moui_rect_t){4, 30, MOUI_DISP_W - 8, (int16_t)(MOUI_DISP_H - 34)};

    for (int i = 0; i < 20; i++) {
        moui_label_init(&vlist_cell_pool[i], "", &moui_font_ascii_6x8);
        moui_vlist_add_cell(&vlist_demo, &vlist_cell_pool[i].base);
    }

    moui_screen_add_widget(&scr_vlist, &vlist_demo.base);
    moui_focus_group_add(&scr_vlist.focus, &vlist_demo.base);
}

/* ═══════════════════ Chat Demo (WeChat-like) ═══════════════════ */

typedef struct {
    const char *text;
    const char *sender;     /* display name */
    const char *time;       /* HH:MM */
    bool        is_self;    /* true = right bubble, false = left bubble */
    bool        is_system;  /* true = center system message */
} chat_msg_t;

static const chat_msg_t s_chat_msgs[] = {
    { NULL,                                  NULL,     "10:30", false, true  }, /* system: time */
    { "Hey! Moui EPD UI is amazing!",        "Alice",  "10:31", false, false },
    { "Thanks! It runs on ESP32",            "Me",     "10:31", true,  false },
    { "How much RAM does it use?",           "Alice",  "10:32", false, false },
    { "The whole UI framework < 8KB RAM",    "Me",     "10:32", true,  false },
    { "Wow that's incredible for a full UI", "Alice",  "10:33", false, false },
    { "And the VirtualList can handle 100k items with only 1KB!", "Me", "10:33", true, false },
    { NULL,                                  NULL,     "10:35", false, true  }, /* system: time */
    { "Can it display photos?",              "Alice",  "10:35", false, false },
    { "Yes! 1bpp dithered bitmaps, full screen 300x400", "Me", "10:35", true, false },
    { "What about fonts?",                   "Alice",  "10:36", false, false },
    { "ASCII 6x8, Inter 12-48, Chinese 12-32, all bitmap fonts", "Me", "10:36", true, false },
    { "Does it support animation?",          "Alice",  "10:37", false, false },
    { "Yes! Timeline keyframe animation with easing curves", "Me", "10:37", true, false },
    { "How many widgets?",                   "Alice",  "10:38", false, false },
    { "30 widget types: buttons, sliders, charts, calendar, keyboard...", "Me", "10:38", true, false },
    { NULL,                                  NULL,     "10:40", false, true  },
    { "I want to use it in my project!",     "Alice",  "10:40", false, false },
    { "Go ahead! It's designed for EPD and OLED displays", "Me", "10:40", true, false },
    { "What displays are supported?",        "Alice",  "10:41", false, false },
    { "SSD1306 OLED, SSD1677, UC8151, ST7305 EPD drivers built-in", "Me", "10:41", true, false },
    { "This chat window is also using Moui right?", "Alice", "10:42", false, false },
    { "Yes! It's built on moui_vlist with cell recycling :)", "Me", "10:42", true, false },
};

#define CHAT_MSG_COUNT (sizeof(s_chat_msgs) / sizeof(s_chat_msgs[0]))

/* Bubble layout constants */
#define CHAT_BUBBLE_MAX_W   200  /* max bubble text width */
#define CHAT_BUBBLE_PAD_X   6    /* horizontal padding inside bubble */
#define CHAT_BUBBLE_PAD_Y   4    /* vertical padding inside bubble */
#define CHAT_BUBBLE_RADIUS  4    /* bubble corner radius */
#define CHAT_AVATAR_SIZE    16   /* avatar icon area */
#define CHAT_MARGIN         4    /* margin from screen edge */
#define CHAT_GAP            3    /* gap between messages */
#define CHAT_SYSTEM_H       18   /* system message row height */
#define CHAT_HEADER_H       24   /* top bar height */

static moui_screen_t       scr_chat;
static moui_widget_label_t chat_title;
static moui_vlist_t        chat_vlist;

/* Chat cell widget — custom draw for bubble rendering */
typedef struct {
    moui_widget_t  base;
    uint32_t       msg_index;
} chat_cell_t;

#define CHAT_CELL_POOL_SIZE 20
static chat_cell_t chat_cells[CHAT_CELL_POOL_SIZE];

static const moui_font_t *chat_font(void) { return &moui_font_ascii_6x8; }

/* Calculate bubble text width (clamped) */
static int chat_text_w(const char *text)
{
    int w = moui_font_measure_str(chat_font(), text);
    if (w > CHAT_BUBBLE_MAX_W) w = CHAT_BUBBLE_MAX_W;
    return w;
}

/* Calculate number of wrapped lines */
static int chat_text_lines(const char *text)
{
    return moui_font_count_wrapped_lines(chat_font(), text, CHAT_BUBBLE_MAX_W);
}

/* Height callback */
static int16_t chat_height_fn(const moui_vlist_t *vl, uint32_t index, void *ud)
{
    (void)vl; (void)ud;
    if (index >= CHAT_MSG_COUNT) return 0;
    const chat_msg_t *msg = &s_chat_msgs[index];
    if (msg->is_system) return CHAT_SYSTEM_H;
    int lines = chat_text_lines(msg->text);
    int text_h = lines * (chat_font()->line_height > 0 ? chat_font()->line_height : chat_font()->px_size);
    return (int16_t)(text_h + CHAT_BUBBLE_PAD_Y * 2 + CHAT_GAP + 10); /* +10 for sender name line */
}

static uint32_t chat_count_fn(const moui_vlist_t *vl, void *ud)
{
    (void)vl; (void)ud;
    return CHAT_MSG_COUNT;
}

static void chat_cell_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    chat_cell_t *cc = (chat_cell_t *)w;
    if (cc->msg_index >= CHAT_MSG_COUNT) return;
    const chat_msg_t *msg = &s_chat_msgs[cc->msg_index];

    int bx = w->bounds.x;
    int by = w->bounds.y;
    int bw = w->bounds.w;
    (void)bw;

    if (msg->is_system) {
        /* Center system time label */
        int tw = moui_font_measure_str(chat_font(), msg->time);
        int cx = bx + (MOUI_DISP_W - 8) / 2 - tw / 2;
        /* Draw dashed separator lines */
        for (int dx = bx + 10; dx < cx - 4; dx += 4)
            moui_draw_hline(ctx, dx, by + CHAT_SYSTEM_H / 2, 2, MOUI_BLACK);
        for (int dx = cx + tw + 4; dx < bx + MOUI_DISP_W - 18; dx += 4)
            moui_draw_hline(ctx, dx, by + CHAT_SYSTEM_H / 2, 2, MOUI_BLACK);
        moui_font_draw_str(ctx, chat_font(), cx, by + 5, msg->time, MOUI_BLACK);
        return;
    }

    int lines = chat_text_lines(msg->text);
    int line_h = chat_font()->line_height > 0 ? chat_font()->line_height : chat_font()->px_size;
    int text_h = lines * line_h;
    int bubble_w_text;
    if (lines == 1) {
        bubble_w_text = chat_text_w(msg->text);
    } else {
        bubble_w_text = CHAT_BUBBLE_MAX_W;
    }
    int bubble_w = bubble_w_text + CHAT_BUBBLE_PAD_X * 2;
    int bubble_h = text_h + CHAT_BUBBLE_PAD_Y * 2;

    int name_y = by + 1;
    int bubble_y = by + 10;

    if (msg->is_self) {
        /* ─── Right-aligned (self) ─── */
        int bubble_x = bx + MOUI_DISP_W - 16 - CHAT_MARGIN - CHAT_AVATAR_SIZE - 2 - bubble_w;

        /* Avatar: filled circle with initial */
        int av_x = bx + MOUI_DISP_W - 16 - CHAT_MARGIN - CHAT_AVATAR_SIZE;
        int av_cy = bubble_y + bubble_h / 2;
        moui_rect_t av_rect = {(int16_t)av_x, (int16_t)(av_cy - 7), 14, 14};
        moui_draw_fill_rounded_rect(ctx, &av_rect, 7, MOUI_BLACK);
        /* Draw "M" for "Me" in white (inverted) */
        moui_draw_pixel(ctx, av_x + 3, av_cy - 3, MOUI_WHITE);
        moui_draw_pixel(ctx, av_x + 4, av_cy - 2, MOUI_WHITE);
        moui_draw_pixel(ctx, av_x + 5, av_cy - 3, MOUI_WHITE);
        moui_draw_pixel(ctx, av_x + 6, av_cy - 2, MOUI_WHITE);
        moui_draw_pixel(ctx, av_x + 7, av_cy - 3, MOUI_WHITE);
        for (int dy = -3; dy <= 2; dy++) {
            moui_draw_pixel(ctx, av_x + 3, av_cy + dy, MOUI_WHITE);
            moui_draw_pixel(ctx, av_x + 7, av_cy + dy, MOUI_WHITE);
        }

        /* Sender name (right-aligned) */
        int name_w = moui_font_measure_str(chat_font(), msg->sender);
        moui_font_draw_str(ctx, chat_font(), bubble_x + bubble_w - name_w, name_y, msg->sender, MOUI_BLACK);

        /* Filled bubble */
        moui_rect_t br = {(int16_t)bubble_x, (int16_t)bubble_y, (uint16_t)bubble_w, (uint16_t)bubble_h};
        moui_draw_fill_rounded_rect(ctx, &br, CHAT_BUBBLE_RADIUS, MOUI_BLACK);

        /* Bubble tail (small triangle on right) */
        moui_draw_pixel(ctx, bubble_x + bubble_w, bubble_y + 4, MOUI_BLACK);
        moui_draw_pixel(ctx, bubble_x + bubble_w + 1, bubble_y + 5, MOUI_BLACK);
        moui_draw_pixel(ctx, bubble_x + bubble_w, bubble_y + 5, MOUI_BLACK);
        moui_draw_pixel(ctx, bubble_x + bubble_w, bubble_y + 6, MOUI_BLACK);

        /* Text (white on black) — use inverted XOR mode */
        moui_font_draw_str_wrapped(ctx, chat_font(),
            bubble_x + CHAT_BUBBLE_PAD_X, bubble_y + CHAT_BUBBLE_PAD_Y,
            bubble_w_text, msg->text, MOUI_WHITE);

        /* Time stamp */
        moui_font_draw_str(ctx, chat_font(), bubble_x - 30, bubble_y + bubble_h - 8, msg->time, MOUI_BLACK);

    } else {
        /* ─── Left-aligned (other) ─── */
        int av_x = bx + CHAT_MARGIN;
        int bubble_x = av_x + CHAT_AVATAR_SIZE + 2;

        /* Avatar: circle outline with "A" */
        int av_cy = bubble_y + bubble_h / 2;
        moui_rect_t av_rect = {(int16_t)av_x, (int16_t)(av_cy - 7), 14, 14};
        moui_draw_rounded_rect(ctx, &av_rect, 7, MOUI_BLACK);
        /* Draw "A" inside */
        moui_font_draw_str(ctx, chat_font(), av_x + 4, av_cy - 4, "A", MOUI_BLACK);

        /* Sender name */
        moui_font_draw_str(ctx, chat_font(), bubble_x + 2, name_y, msg->sender, MOUI_BLACK);

        /* Outlined bubble */
        moui_rect_t br = {(int16_t)bubble_x, (int16_t)bubble_y, (uint16_t)bubble_w, (uint16_t)bubble_h};
        moui_draw_rounded_rect(ctx, &br, CHAT_BUBBLE_RADIUS, MOUI_BLACK);

        /* Bubble tail (small triangle on left) */
        moui_draw_pixel(ctx, bubble_x - 1, bubble_y + 4, MOUI_BLACK);
        moui_draw_pixel(ctx, bubble_x - 2, bubble_y + 5, MOUI_BLACK);
        moui_draw_pixel(ctx, bubble_x - 1, bubble_y + 5, MOUI_BLACK);
        moui_draw_pixel(ctx, bubble_x - 1, bubble_y + 6, MOUI_BLACK);

        /* Text (black on white) */
        moui_font_draw_str_wrapped(ctx, chat_font(),
            bubble_x + CHAT_BUBBLE_PAD_X, bubble_y + CHAT_BUBBLE_PAD_Y,
            bubble_w_text, msg->text, MOUI_BLACK);

        /* Time stamp */
        moui_font_draw_str(ctx, chat_font(), bubble_x + bubble_w + 3, bubble_y + bubble_h - 8, msg->time, MOUI_BLACK);
    }
}

static const moui_widget_vtable_t chat_cell_vt = { .draw = chat_cell_draw };

static void chat_bind_fn(const moui_vlist_t *vl, moui_widget_t *cell, uint32_t index, void *ud)
{
    (void)vl; (void)ud;
    chat_cell_t *cc = (chat_cell_t *)cell;
    cc->msg_index = index;
}

static void push_chat(void) { moui_screen_push(s_mgr, &scr_chat); }

/* Bottom bar: fake input field */
static moui_widget_t chat_input_bar;

static void chat_input_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    /* Background fill white */
    moui_draw_fill_rect(ctx, &w->bounds, MOUI_WHITE);
    /* Top border */
    moui_draw_hline(ctx, w->bounds.x, w->bounds.y, w->bounds.w, MOUI_BLACK);
    /* Input box */
    moui_rect_t inp = {
        (int16_t)(w->bounds.x + 4),
        (int16_t)(w->bounds.y + 4),
        (uint16_t)(w->bounds.w - 50),
        (uint16_t)(w->bounds.h - 8)
    };
    moui_draw_rounded_rect(ctx, &inp, 3, MOUI_BLACK);
    moui_font_draw_str(ctx, chat_font(), inp.x + 4, inp.y + 3, "Type a message...", MOUI_BLACK);

    /* Send button */
    moui_rect_t send_btn = {
        (int16_t)(w->bounds.x + w->bounds.w - 42),
        (int16_t)(w->bounds.y + 4),
        38,
        (uint16_t)(w->bounds.h - 8)
    };
    moui_draw_fill_rounded_rect(ctx, &send_btn, 3, MOUI_BLACK);
    moui_font_draw_str(ctx, chat_font(), send_btn.x + 6, send_btn.y + 3, "Send", MOUI_WHITE);
}

static const moui_widget_vtable_t chat_input_vt = { .draw = chat_input_draw };

static void setup_chat_screen(void)
{
    moui_screen_init(&scr_chat);
    scr_chat.enter_trans = MOUI_TRANS_SLIDE_LEFT;

    /* Top bar: contact name */
    moui_label_init(&chat_title, "< Alice", &moui_font_ascii_6x8);
    chat_title.inverted = true;
    chat_title.base.bounds = (moui_rect_t){0, 0, MOUI_DISP_W, CHAT_HEADER_H};
    moui_screen_add_widget(&scr_chat, &chat_title.base);

    /* Bottom: input bar */
    int input_h = 28;
    moui_widget_init(&chat_input_bar, &chat_input_vt);
    chat_input_bar.bounds = (moui_rect_t){0, (int16_t)(MOUI_DISP_H - input_h), MOUI_DISP_W, (uint16_t)input_h};
    moui_screen_add_widget(&scr_chat, &chat_input_bar);

    /* Chat message list using VirtualList */
    int list_y = CHAT_HEADER_H;
    int list_h = MOUI_DISP_H - CHAT_HEADER_H - input_h;

    moui_vlist_init(&chat_vlist, 40, chat_count_fn, chat_bind_fn, NULL);
    chat_vlist.height_fn = chat_height_fn;
    chat_vlist.base.bounds = (moui_rect_t){0, (int16_t)list_y, MOUI_DISP_W, (int16_t)list_h};

    for (int i = 0; i < CHAT_CELL_POOL_SIZE; i++) {
        memset(&chat_cells[i], 0, sizeof(chat_cells[i]));
        moui_widget_init(&chat_cells[i].base, &chat_cell_vt);
        moui_vlist_add_cell(&chat_vlist, &chat_cells[i].base);
    }

    /* Auto-scroll to bottom (last message) */
    chat_vlist.selected_index = CHAT_MSG_COUNT - 1;
    moui_vlist_scroll_to_index(&chat_vlist, CHAT_MSG_COUNT - 1);

    moui_screen_add_widget(&scr_chat, &chat_vlist.base);
    moui_focus_group_add(&scr_chat.focus, &chat_vlist.base);
}

/* ═══════════════════ App Framework Demo ═══════════════════ */

#define APP_TAB_COUNT     4
#define APP_STATUSBAR_H  16
#define APP_TABBAR_H     36
#define APP_CONTENT_Y    APP_STATUSBAR_H
#define APP_CONTENT_H    (MOUI_DISP_H - APP_STATUSBAR_H - APP_TABBAR_H)

static moui_screen_t scr_app;
static uint8_t app_active_tab = 0;

/* Tab definitions */
static const moui_icon_id_t app_tab_icons[APP_TAB_COUNT] = {
    MOUI_ICON_HOME,
    MOUI_ICON_MESSAGE,
    MOUI_ICON_SEARCH,
    MOUI_ICON_PERSON,
};
static const char *app_tab_labels[APP_TAB_COUNT] = {
    "Home", "Chat", "Find", "Me"
};

/* ─── Status Bar widget ─── */
static moui_widget_t app_statusbar;

static void app_statusbar_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_draw_fill_rect(ctx, &w->bounds, MOUI_BLACK);
    /* Time */
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, w->bounds.x + 4, w->bounds.y + 4, "14:12", MOUI_WHITE);
    /* WiFi icon */
    moui_draw_icon(ctx, w->bounds.x + w->bounds.w - 54, w->bounds.y, MOUI_ICON_WIFI, MOUI_WHITE);
    /* Battery icon */
    moui_draw_icon(ctx, w->bounds.x + w->bounds.w - 36, w->bounds.y, MOUI_ICON_BATTERY_FULL, MOUI_WHITE);
    /* Battery percentage */
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, w->bounds.x + w->bounds.w - 20, w->bounds.y + 4, "85", MOUI_WHITE);
}
static const moui_widget_vtable_t app_statusbar_vt = { .draw = app_statusbar_draw };

/* ─── Bottom Tab Bar widget ─── */
static moui_widget_t app_tabbar;

static void app_tabbar_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    /* Background */
    moui_draw_fill_rect(ctx, &w->bounds, MOUI_WHITE);
    /* Top separator */
    moui_draw_hline(ctx, w->bounds.x, w->bounds.y, w->bounds.w, MOUI_BLACK);

    int tab_w = w->bounds.w / APP_TAB_COUNT;

    for (int i = 0; i < APP_TAB_COUNT; i++) {
        int tx = w->bounds.x + i * tab_w;
        int icon_x = tx + (tab_w - MOUI_ICON_W) / 2;
        int icon_y = w->bounds.y + 4;
        int label_w = moui_font_measure_str(&moui_font_ascii_6x8, app_tab_labels[i]);
        int label_x = tx + (tab_w - label_w) / 2;
        int label_y = w->bounds.y + 22;

        if (i == app_active_tab) {
            /* Active tab: filled indicator bar + bold icon */
            moui_rect_t indicator = {(int16_t)(tx + 8), (int16_t)(w->bounds.y + 1), (uint16_t)(tab_w - 16), 2};
            moui_draw_fill_rect(ctx, &indicator, MOUI_BLACK);
            moui_draw_icon(ctx, icon_x, icon_y, app_tab_icons[i], MOUI_BLACK);
            moui_font_draw_str(ctx, &moui_font_ascii_6x8, label_x, label_y, app_tab_labels[i], MOUI_BLACK);
        } else {
            /* Inactive tab: lighter appearance (draw icon normally, use dithered look) */
            moui_draw_icon(ctx, icon_x, icon_y, app_tab_icons[i], MOUI_BLACK);
            moui_font_draw_str(ctx, &moui_font_ascii_6x8, label_x, label_y, app_tab_labels[i], MOUI_BLACK);
            /* Dim effect: overlay white dots on icon to simulate lighter shade */
            for (int dy = icon_y; dy < icon_y + 16; dy += 2)
                for (int dx = icon_x; dx < icon_x + 16; dx += 2)
                    moui_draw_pixel(ctx, dx, dy, MOUI_WHITE);
        }
    }
}
static const moui_widget_vtable_t app_tabbar_vt = { .draw = app_tabbar_draw };

/* ─── Content Area widget (custom draw per active tab) ─── */
static moui_widget_t app_content;

/* Home page content */
static void draw_home_page(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    int x = w->bounds.x;
    int y = w->bounds.y;
    int cw = w->bounds.w;

    /* Welcome banner */
    moui_rect_t banner = {(int16_t)(x + 8), (int16_t)(y + 8), (uint16_t)(cw - 16), 44};
    moui_draw_fill_rounded_rect(ctx, &banner, 6, MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + 16, y + 14, "Welcome to Moui!", MOUI_WHITE);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + 16, y + 28, "EPD UI Framework v1.0", MOUI_WHITE);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + 16, y + 38, "30 Widgets | 128 Icons | <8KB RAM", MOUI_WHITE);

    /* Quick stats cards */
    int card_w = (cw - 24) / 2;
    int card_y = y + 60;

    /* Card 1: Temperature */
    moui_rect_t c1 = {(int16_t)(x + 8), (int16_t)card_y, (uint16_t)card_w, 60};
    moui_draw_rounded_rect(ctx, &c1, 4, MOUI_BLACK);
    moui_draw_icon(ctx, x + 14, card_y + 6, MOUI_ICON_THERMOMETER, MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + 34, card_y + 10, "Temp", MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_inter_24, x + 20, card_y + 26, "23.5", MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + card_w - 16, card_y + 46, "C", MOUI_BLACK);

    /* Card 2: Humidity */
    moui_rect_t c2 = {(int16_t)(x + card_w + 16), (int16_t)card_y, (uint16_t)card_w, 60};
    moui_draw_rounded_rect(ctx, &c2, 4, MOUI_BLACK);
    moui_draw_icon(ctx, x + card_w + 22, card_y + 6, MOUI_ICON_HUMIDITY, MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + card_w + 42, card_y + 10, "Humid", MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_inter_24, x + card_w + 28, card_y + 26, "65", MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + 2 * card_w, card_y + 46, "%", MOUI_BLACK);

    /* Card 3: Steps */
    int card_y2 = card_y + 68;
    moui_rect_t c3 = {(int16_t)(x + 8), (int16_t)card_y2, (uint16_t)card_w, 60};
    moui_draw_rounded_rect(ctx, &c3, 4, MOUI_BLACK);
    moui_draw_icon(ctx, x + 14, card_y2 + 6, MOUI_ICON_SPEED, MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + 34, card_y2 + 10, "Steps", MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_inter_24, x + 14, card_y2 + 26, "8,241", MOUI_BLACK);

    /* Card 4: Heart Rate */
    moui_rect_t c4 = {(int16_t)(x + card_w + 16), (int16_t)card_y2, (uint16_t)card_w, 60};
    moui_draw_rounded_rect(ctx, &c4, 4, MOUI_BLACK);
    moui_draw_icon(ctx, x + card_w + 22, card_y2 + 6, MOUI_ICON_HEART, MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + card_w + 42, card_y2 + 10, "Heart", MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_inter_24, x + card_w + 28, card_y2 + 26, "72", MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + 2 * card_w, card_y2 + 46, "bpm", MOUI_BLACK);

    /* Recent activity list */
    int list_y = card_y2 + 70;
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + 10, list_y, "Recent Activity", MOUI_BLACK);
    moui_draw_hline(ctx, x + 8, list_y + 10, cw - 16, MOUI_BLACK);
    moui_draw_icon(ctx, x + 10, list_y + 14, MOUI_ICON_NOTIFICATION, MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + 30, list_y + 18, "System update available", MOUI_BLACK);
    moui_draw_icon(ctx, x + 10, list_y + 34, MOUI_ICON_MESSAGE, MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + 30, list_y + 38, "Alice: Hey, are you there?", MOUI_BLACK);
    moui_draw_icon(ctx, x + 10, list_y + 54, MOUI_ICON_ALARM, MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + 30, list_y + 58, "Reminder: Meeting at 3PM", MOUI_BLACK);
}

/* Chat list page */
static void draw_chat_page(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    int x = w->bounds.x;
    int y = w->bounds.y;
    int cw = w->bounds.w;

    /* Page title */
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + 10, y + 6, "Messages", MOUI_BLACK);
    moui_draw_hline(ctx, x, y + 16, cw, MOUI_BLACK);

    /* Chat contacts list */
    const char *names[] = {"Alice", "Bob", "Charlie", "Diana", "Eve"};
    const char *msgs[] = {
        "Yes! Built on moui_vlist :)",
        "See you tomorrow!",
        "The PR looks good",
        "Happy birthday!",
        "Check this photo"
    };
    const char *times[] = {"10:42", "09:15", "Yest.", "Mon", "Sun"};
    const moui_icon_id_t avatars[] = {MOUI_ICON_PERSON, MOUI_ICON_FACE_SMILE, MOUI_ICON_STAR, MOUI_ICON_HEART, MOUI_ICON_CAMERA};

    for (int i = 0; i < 5; i++) {
        int row_y = y + 20 + i * 52;

        /* Avatar circle */
        moui_rect_t av = {(int16_t)(x + 10), (int16_t)(row_y + 6), 28, 28};
        moui_draw_rounded_rect(ctx, &av, 14, MOUI_BLACK);
        moui_draw_icon(ctx, x + 16, row_y + 12, avatars[i], MOUI_BLACK);

        /* Name + time */
        moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + 46, row_y + 8, names[i], MOUI_BLACK);
        int tw = moui_font_measure_str(&moui_font_ascii_6x8, times[i]);
        moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + cw - tw - 10, row_y + 8, times[i], MOUI_BLACK);

        /* Last message preview */
        moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + 46, row_y + 22, msgs[i], MOUI_BLACK);

        /* Unread badge for first 2 */
        if (i < 2) {
            moui_rect_t badge = {(int16_t)(x + cw - 20), (int16_t)(row_y + 20), 14, 12};
            moui_draw_fill_rounded_rect(ctx, &badge, 6, MOUI_BLACK);
            moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + cw - 17, row_y + 22, i == 0 ? "3" : "1", MOUI_WHITE);
        }

        /* Separator */
        if (i < 4) moui_draw_hline(ctx, x + 44, row_y + 42, cw - 54, MOUI_BLACK);
    }
}

/* Discover page */
static void draw_discover_page(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    int x = w->bounds.x;
    int y = w->bounds.y;
    int cw = w->bounds.w;

    /* Search bar */
    moui_rect_t search = {(int16_t)(x + 8), (int16_t)(y + 6), (uint16_t)(cw - 16), 22};
    moui_draw_rounded_rect(ctx, &search, 11, MOUI_BLACK);
    moui_draw_icon(ctx, x + 14, y + 9, MOUI_ICON_SEARCH, MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + 34, y + 13, "Search...", MOUI_BLACK);

    /* Category grid (2x3) */
    const char *cats[] = {"Nearby", "Scan", "Music", "Books", "Games", "Tools"};
    const moui_icon_id_t cat_icons[] = {MOUI_ICON_GPS, MOUI_ICON_QRCODE, MOUI_ICON_MUSIC, MOUI_ICON_BOOK, MOUI_ICON_STAR, MOUI_ICON_SETTINGS};

    int grid_y = y + 36;
    int cell_w = (cw - 24) / 3;
    int cell_h = 56;

    for (int i = 0; i < 6; i++) {
        int col = i % 3;
        int row = i / 3;
        int cx = x + 8 + col * (cell_w + 4);
        int cy = grid_y + row * (cell_h + 4);

        moui_rect_t cell = {(int16_t)cx, (int16_t)cy, (uint16_t)cell_w, (uint16_t)cell_h};
        moui_draw_rounded_rect(ctx, &cell, 4, MOUI_BLACK);
        moui_draw_icon(ctx, cx + (cell_w - 16) / 2, cy + 8, cat_icons[i], MOUI_BLACK);
        int lw = moui_font_measure_str(&moui_font_ascii_6x8, cats[i]);
        moui_font_draw_str(ctx, &moui_font_ascii_6x8, cx + (cell_w - lw) / 2, cy + 32, cats[i], MOUI_BLACK);
    }

    /* Trending section */
    int trend_y = grid_y + 2 * (cell_h + 4) + 10;
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + 10, trend_y, "Trending", MOUI_BLACK);
    moui_draw_hline(ctx, x + 8, trend_y + 10, cw - 16, MOUI_BLACK);

    const char *trending[] = {"#MouiFramework", "#ESP32", "#EPD_Display", "#EInk_UI"};
    for (int i = 0; i < 4; i++) {
        moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + 16, trend_y + 16 + i * 14, trending[i], MOUI_BLACK);
        moui_draw_icon(ctx, x + cw - 26, trend_y + 14 + i * 14, MOUI_ICON_ROCKET, MOUI_BLACK);
    }
}

/* Profile page */
static void draw_profile_page(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    int x = w->bounds.x;
    int y = w->bounds.y;
    int cw = w->bounds.w;

    /* Avatar + name card */
    int av_size = 48;
    int av_x = x + (cw - av_size) / 2;
    moui_rect_t av = {(int16_t)av_x, (int16_t)(y + 12), (uint16_t)av_size, (uint16_t)av_size};
    moui_draw_rounded_rect(ctx, &av, 24, MOUI_BLACK);
    moui_draw_icon_scaled(ctx, av_x + 8, y + 20, MOUI_ICON_PERSON, 2, MOUI_BLACK);

    int name_w = moui_font_measure_str(&moui_font_ascii_6x8, "Developer");
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + (cw - name_w) / 2, y + 66, "Developer", MOUI_BLACK);
    int id_w = moui_font_measure_str(&moui_font_ascii_6x8, "ID: moui_dev_001");
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + (cw - id_w) / 2, y + 78, "ID: moui_dev_001", MOUI_BLACK);

    /* Stats row */
    int stats_y = y + 96;
    moui_draw_hline(ctx, x + 8, stats_y, cw - 16, MOUI_BLACK);
    int col_w = cw / 3;
    const char *stat_labels[] = {"Projects", "Stars", "Days"};
    const char *stat_vals[] = {"12", "128", "365"};
    for (int i = 0; i < 3; i++) {
        int cx = x + i * col_w + col_w / 2;
        int vw = moui_font_measure_str(&moui_font_ascii_6x8, stat_vals[i]);
        moui_font_draw_str(ctx, &moui_font_ascii_6x8, cx - vw / 2, stats_y + 8, stat_vals[i], MOUI_BLACK);
        int lw = moui_font_measure_str(&moui_font_ascii_6x8, stat_labels[i]);
        moui_font_draw_str(ctx, &moui_font_ascii_6x8, cx - lw / 2, stats_y + 20, stat_labels[i], MOUI_BLACK);
    }
    moui_draw_hline(ctx, x + 8, stats_y + 34, cw - 16, MOUI_BLACK);

    /* Menu items */
    int menu_y = stats_y + 42;
    const char *menu_items[] = {"Favorites", "Settings", "About", "Logout"};
    const moui_icon_id_t menu_icons[] = {MOUI_ICON_BOOKMARK, MOUI_ICON_SETTINGS, MOUI_ICON_INFO, MOUI_ICON_POWER_OFF};

    for (int i = 0; i < 4; i++) {
        int iy = menu_y + i * 30;
        moui_draw_icon(ctx, x + 16, iy + 2, menu_icons[i], MOUI_BLACK);
        moui_font_draw_str(ctx, &moui_font_ascii_6x8, x + 40, iy + 6, menu_items[i], MOUI_BLACK);
        moui_draw_icon(ctx, x + cw - 24, iy + 2, MOUI_ICON_FORWARD, MOUI_BLACK);
        if (i < 3) moui_draw_hline(ctx, x + 40, iy + 24, cw - 56, MOUI_BLACK);
    }
}

/* Content area draw dispatcher */
static void app_content_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    /* Clear content area */
    moui_draw_fill_rect(ctx, &w->bounds, MOUI_WHITE);

    /* Clip to content area */
    moui_draw_push_clip(ctx, &w->bounds);

    switch (app_active_tab) {
    case 0: draw_home_page(w, ctx); break;
    case 1: draw_chat_page(w, ctx); break;
    case 2: draw_discover_page(w, ctx); break;
    case 3: draw_profile_page(w, ctx); break;
    }

    moui_draw_pop_clip(ctx);
}

static bool app_content_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    if (!ev) return false;

    if (ev->type == MOUI_EV_ENCODER_CW) {
        if (app_active_tab < APP_TAB_COUNT - 1) {
            app_active_tab++;
            w->dirty = 1;
            app_tabbar.dirty = 1;
            return true;
        }
    } else if (ev->type == MOUI_EV_ENCODER_CCW) {
        if (app_active_tab > 0) {
            app_active_tab--;
            w->dirty = 1;
            app_tabbar.dirty = 1;
            return true;
        }
    }
    return false;
}

static const moui_widget_vtable_t app_content_vt = {
    .draw = app_content_draw,
    .on_event = app_content_event
};

static void push_app(void) { moui_screen_push(s_mgr, &scr_app); }

static void setup_app_screen(void)
{
    moui_screen_init(&scr_app);
    scr_app.enter_trans = MOUI_TRANS_SLIDE_LEFT;

    /* Status bar */
    moui_widget_init(&app_statusbar, &app_statusbar_vt);
    app_statusbar.bounds = (moui_rect_t){0, 0, MOUI_DISP_W, APP_STATUSBAR_H};
    moui_screen_add_widget(&scr_app, &app_statusbar);

    /* Content area */
    moui_widget_init(&app_content, &app_content_vt);
    app_content.bounds = (moui_rect_t){0, APP_CONTENT_Y, MOUI_DISP_W, APP_CONTENT_H};
    moui_screen_add_widget(&scr_app, &app_content);
    moui_focus_group_add(&scr_app.focus, &app_content);

    /* Bottom tab bar */
    moui_widget_init(&app_tabbar, &app_tabbar_vt);
    app_tabbar.bounds = (moui_rect_t){0, (int16_t)(MOUI_DISP_H - APP_TABBAR_H), MOUI_DISP_W, APP_TABBAR_H};
    moui_screen_add_widget(&scr_app, &app_tabbar);
}


void demo_setup_all_screens(void)
{
    setup_main_screen();
    setup_dashboard_screen();
    setup_settings_screen();
    setup_chart_screen();
    setup_graphics_screen();
    setup_stress_screen();
    setup_widgets_screen();
    setup_widgets2_screen();
    setup_mono_screen();
    setup_widgets3_screen();
    setup_widgets4_screen();
    setup_widgets5_screen();
    setup_vlist_screen();
    setup_chat_screen();
    setup_app_screen();
    setup_arch_screen();
    setup_image_screen();
    setup_features_screen();
    setup_icons_screen();
    setup_about_screen();
}

moui_screen_t *demo_get_main_screen(void)
{
    return &scr_main;
}
