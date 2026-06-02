/*
 * widget_test — full-component coverage screens for the moui framework.
 *
 * A main menu (List) selects among several test screens; each screen packs a
 * group of widgets. Everything here uses only the public moui API so it can be
 * driven by 7-key navigation on the OnePage C61 E-Paper.
 */

#include "widget_test.h"
#include "tst_board.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define TST_W 480
#define TST_H 800

static moui_screen_mgr_t *s_mgr;

/* ── Main menu ── */

static const char *s_menu_items[] = {
    "Forms",
    "Widgets",
    "Data Display",
    "Text Input",
    "Graphics",
    "CJK Fonts",
    "Scaled Font",
    "QR Code",
};

static moui_screen_t          s_main_scr;
static moui_widget_label_t    s_main_title;
static moui_widget_list_t     s_main_list;

static const char *menu_label(uint16_t idx, void *ud)
{
    (void)ud;
    if (idx < sizeof(s_menu_items) / sizeof(s_menu_items[0]))
        return s_menu_items[idx];
    return NULL;
}

void wt_show_test(int idx);   /* defined below */

static void main_list_select(moui_widget_list_t *l, uint16_t idx)
{
    (void)l;
    extern void wt_show_test(int idx);
    wt_show_test((int)idx);
}

static void setup_main_screen(void)
{
    moui_screen_init(&s_main_scr);

    moui_label_init(&s_main_title, "Moui Widget Test", &moui_font_cn_16);
    s_main_title.inverted = true;
    s_main_title.base.bounds = (moui_rect_t){0, 0, TST_W, 24};
    moui_screen_add_widget(&s_main_scr, &s_main_title.base);

    moui_list_init(&s_main_list, &moui_font_cn_16);
    s_main_list.adapter = menu_label;
    s_main_list.item_count = sizeof(s_menu_items) / sizeof(s_menu_items[0]);
    s_main_list.item_h = 34;
    s_main_list.on_select = main_list_select;
    s_main_list.base.bounds = (moui_rect_t){0, 26, TST_W, 800 - 26};
    moui_screen_add_widget(&s_main_scr, &s_main_list.base);
}

/* ── Forms screen: slider / switch / radio / spinner / stepper / checklist / button ── */

static moui_screen_t           s_forms_scr;
static moui_widget_label_t     s_forms_title, s_forms_hint;
static moui_widget_slider_t    s_f_slider;
static moui_widget_switch_t    s_f_switch;
static moui_widget_radio_t     s_f_radio;
static moui_widget_spinner_t   s_f_spinner;
static moui_widget_stepper_t   s_f_stepper;
static moui_widget_checklist_t s_f_checklist;
static moui_widget_button_t    s_f_btn;
static moui_widget_progress_t  s_f_progress;

static const char *s_radio_opts[] = { "Low", "Med", "High", "Max" };
static const char *s_check_items[] = { "WiFi", "BT", "NFC", "GPS" };

static void on_slider_change(moui_widget_slider_t *s, int32_t v)
{
    (void)s;
    s_f_progress.value = v;
}

static void on_btn_click(moui_widget_button_t *b)
{
    (void)b;
    moui_popup_show_toast(&s_mgr->popup, "Button OK", 1000);
}

static void setup_forms_screen(void)
{
    moui_screen_init(&s_forms_scr);
    s_forms_scr.enter_trans = MOUI_TRANS_NONE;
    int y = 0;

    moui_label_init(&s_forms_title, "Forms", &moui_font_cn_16);
    s_forms_title.inverted = true;
    s_forms_title.base.bounds = (moui_rect_t){0, y, TST_W, 22};
    moui_screen_add_widget(&s_forms_scr, &s_forms_title.base); y += 26;

    moui_slider_init(&s_f_slider, &moui_font_inter_16, 0, 100, 5);
    s_f_slider.value = 60;
    s_f_slider.on_change = on_slider_change;
    s_f_slider.base.bounds = (moui_rect_t){10, y, TST_W - 20, 24};
    moui_screen_add_widget(&s_forms_scr, &s_f_slider.base); y += 30;

    moui_progress_init(&s_f_progress, &moui_font_inter_16, 100);
    s_f_progress.value = 60;
    s_f_progress.base.bounds = (moui_rect_t){10, y, TST_W - 20, 20};
    moui_screen_add_widget(&s_forms_scr, &s_f_progress.base); y += 26;

    moui_switch_init(&s_f_switch);
    s_f_switch.state = true;
    s_f_switch.base.bounds = (moui_rect_t){10, y, 80, 24};
    moui_screen_add_widget(&s_forms_scr, &s_f_switch.base); y += 30;

    moui_radio_init(&s_f_radio, &moui_font_inter_16);
    moui_radio_set_options(&s_f_radio, s_radio_opts, 4);
    s_f_radio.base.bounds = (moui_rect_t){10, y, 200, 60};
    moui_screen_add_widget(&s_forms_scr, &s_f_radio.base); y += 66;

    moui_spinner_init(&s_f_spinner, &moui_font_inter_16, 0, 100, 1);
    s_f_spinner.value = 10;
    s_f_spinner.format = "%d";
    s_f_spinner.base.bounds = (moui_rect_t){10, y, 100, 24};
    moui_screen_add_widget(&s_forms_scr, &s_f_spinner.base); y += 30;

    moui_stepper_init(&s_f_stepper, &moui_font_inter_16, 0, 20, 1);
    s_f_stepper.value = 5;
    s_f_stepper.base.bounds = (moui_rect_t){10, y, 100, 24};
    moui_screen_add_widget(&s_forms_scr, &s_f_stepper.base); y += 30;

    moui_checklist_init(&s_f_checklist, &moui_font_inter_16);
    moui_checklist_set_items(&s_f_checklist, s_check_items, 4);
    s_f_checklist.base.bounds = (moui_rect_t){10, y, 200, 60};
    moui_screen_add_widget(&s_forms_scr, &s_f_checklist.base); y += 66;

    moui_button_init(&s_f_btn, "[ Press ]", &moui_font_inter_16, MOUI_BTN_PUSH);
    s_f_btn.on_click = on_btn_click;
    s_f_btn.base.bounds = (moui_rect_t){10, y, 120, 22};
    moui_screen_add_widget(&s_forms_scr, &s_f_btn.base); y += 28;

    moui_label_init(&s_forms_hint, "UP/DN nav  ENT select  BK back", &moui_font_ascii_6x8);
    s_forms_hint.base.bounds = (moui_rect_t){6, y, TST_W - 12, 10};
    moui_screen_add_widget(&s_forms_scr, &s_forms_hint.base);
}

/* ── Widgets screen: gauge / ring / statusbar / tab ── */

static moui_screen_t         s_wdgt_scr;
static moui_widget_label_t   s_wdgt_title, s_wdgt_hint;
static moui_widget_gauge_t   s_w_gauge;
static moui_widget_ring_t    s_w_ring;
static moui_widget_statusbar_t s_w_sb;
static moui_widget_tab_t     s_w_tab;
static moui_widget_ring_t    s_w_ring2;
static uint32_t              s_wdgt_last_tick;
static float                 s_wdgt_phase;

static void wdgt_tick(moui_screen_t *s, uint32_t now_ms, float dt)
{
    (void)s; (void)dt;
    if (now_ms - s_wdgt_last_tick < 5000) return;
    s_wdgt_last_tick = now_ms;
    s_wdgt_phase += 0.15f;
    int v = (int)(50 + sinf(s_wdgt_phase) * 50);
    s_w_gauge.value = v;
    s_w_ring.value = v;
    s_w_ring2.value = 100 - v;
    s_w_sb.battery_pct = (uint8_t)v;
    moui_screen_mgr_mark_dirty(s_mgr);
}

static void setup_widgets_screen(void)
{
    moui_screen_init(&s_wdgt_scr);
    s_wdgt_scr.enter_trans = MOUI_TRANS_NONE;
    s_wdgt_scr.on_tick = wdgt_tick;
    int y = 0;

    moui_label_init(&s_wdgt_title, "Widgets", &moui_font_cn_16);
    s_wdgt_title.inverted = true;
    s_wdgt_title.base.bounds = (moui_rect_t){0, y, TST_W, 22};
    moui_screen_add_widget(&s_wdgt_scr, &s_wdgt_title.base); y += 26;

    moui_statusbar_init(&s_w_sb, &moui_font_inter_16, "Status");
    s_w_sb.battery_pct = 80;
    s_w_sb.base.bounds = (moui_rect_t){0, y, TST_W, 20};
    moui_screen_add_widget(&s_wdgt_scr, &s_w_sb.base); y += 26;

    moui_gauge_init(&s_w_gauge, &moui_font_inter_16, 0, 100);
    s_w_gauge.value = 50;
    s_w_gauge.base.bounds = (moui_rect_t){10, y, 220, 100};
    moui_screen_add_widget(&s_wdgt_scr, &s_w_gauge.base);

    moui_ring_init(&s_w_ring, &moui_font_inter_16, 100);
    s_w_ring.value = 50;
    s_w_ring.base.bounds = (moui_rect_t){250, y, 80, 80};
    moui_screen_add_widget(&s_wdgt_scr, &s_w_ring.base);
    y += 106;

    moui_ring_init(&s_w_ring2, &moui_font_inter_16, 100);
    s_w_ring2.value = 50;
    s_w_ring2.base.bounds = (moui_rect_t){10, y, 60, 60};
    moui_screen_add_widget(&s_wdgt_scr, &s_w_ring2.base);
    y += 66;

    /* Tab with three simple pages */
    static moui_widget_label_t tab_pg1, tab_pg2, tab_pg3;
    moui_tab_init(&s_w_tab, &moui_font_inter_16);
    moui_label_init(&tab_pg1, "Page A", &moui_font_inter_16);
    moui_label_init(&tab_pg2, "Page B", &moui_font_inter_16);
    moui_label_init(&tab_pg3, "Page C", &moui_font_inter_16);
    moui_tab_add_page(&s_w_tab, "A", &tab_pg1.base);
    moui_tab_add_page(&s_w_tab, "B", &tab_pg2.base);
    moui_tab_add_page(&s_w_tab, "C", &tab_pg3.base);
    s_w_tab.base.bounds = (moui_rect_t){10, y, TST_W - 20, 40};
    moui_screen_add_widget(&s_wdgt_scr, &s_w_tab.base); y += 46;

    moui_label_init(&s_wdgt_hint, "UP/DN nav  ENT select  BK back", &moui_font_ascii_6x8);
    s_wdgt_hint.base.bounds = (moui_rect_t){6, y, TST_W - 12, 10};
    moui_screen_add_widget(&s_wdgt_scr, &s_wdgt_hint.base);
}

/* ── Data screen: chart / table / btnmatrix / dropdown / calendar ── */

static moui_screen_t           s_data_scr;
static moui_widget_label_t     s_data_title, s_data_hint;
static moui_widget_chart_t     s_d_chart;
static moui_widget_table_t     s_d_table;
static moui_widget_btnmatrix_t s_d_btnmx;
static moui_widget_dropdown_t  s_d_drop;
static moui_widget_calendar_t  s_d_cal;
static uint32_t                s_data_last_tick;
static float                   s_data_phase;

static const char *s_btnmx_labels[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#" };
static const char *s_drop_opts[] = { "Alpha", "Beta", "Gamma", "Delta", "Epsilon" };
static const char *s_table_rows[] = { "01", "Temp", "23", "02", "Humi", "55", "03", "Wind", "12" };

static void data_tick(moui_screen_t *s, uint32_t now_ms, float dt)
{
    (void)s; (void)dt;
    if (now_ms - s_data_last_tick < 5000) return;
    s_data_last_tick = now_ms;
    s_data_phase += 0.12f;
    int16_t v = (int16_t)(50 + sinf(s_data_phase) * 45);
    moui_chart_push(&s_d_chart, v);
    moui_screen_mgr_mark_dirty(s_mgr);
}

static void setup_data_screen(void)
{
    moui_screen_init(&s_data_scr);
    s_data_scr.enter_trans = MOUI_TRANS_NONE;
    s_data_scr.on_tick = data_tick;
    int y = 0;

    moui_label_init(&s_data_title, "Data Display", &moui_font_cn_16);
    s_data_title.inverted = true;
    s_data_title.base.bounds = (moui_rect_t){0, y, TST_W, 22};
    moui_screen_add_widget(&s_data_scr, &s_data_title.base); y += 26;

    moui_chart_init(&s_d_chart, 0, 100);
    s_d_chart.show_frame = true;
    s_d_chart.base.bounds = (moui_rect_t){10, y, TST_W - 20, 100};
    moui_screen_add_widget(&s_data_scr, &s_d_chart.base); y += 106;

    moui_dropdown_init(&s_d_drop, &moui_font_inter_16);
    moui_dropdown_set_options(&s_d_drop, s_drop_opts, 5);
    s_d_drop.base.bounds = (moui_rect_t){10, y, 200, 24};
    moui_screen_add_widget(&s_data_scr, &s_d_drop.base); y += 30;

    moui_btnmatrix_init(&s_d_btnmx, &moui_font_inter_16, 3);
    moui_btnmatrix_set_labels(&s_d_btnmx, s_btnmx_labels, 12);
    s_d_btnmx.base.bounds = (moui_rect_t){10, y, 200, 90};
    moui_screen_add_widget(&s_data_scr, &s_d_btnmx.base); y += 96;

    moui_table_init(&s_d_table, &moui_font_inter_16, 3, 4);
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 3; c++)
            moui_table_set_cell(&s_d_table, (uint8_t)r, (uint8_t)c,
                                s_table_rows[r * 3 + c]);
    s_d_table.base.bounds = (moui_rect_t){10, y, TST_W - 20, 90};
    moui_screen_add_widget(&s_data_scr, &s_d_table.base); y += 96;

    moui_calendar_init(&s_d_cal, &moui_font_inter_16, 2026, 8, 10);
    s_d_cal.base.bounds = (moui_rect_t){10, y, 220, 140};
    moui_screen_add_widget(&s_data_scr, &s_d_cal.base); y += 146;

    moui_label_init(&s_data_hint, "UP/DN nav  ENT select  BK back", &moui_font_ascii_6x8);
    s_data_hint.base.bounds = (moui_rect_t){6, y, TST_W - 12, 10};
    moui_screen_add_widget(&s_data_scr, &s_data_hint.base);
}

/* ── Text screen: label / textinput / keyboard ── */

static moui_screen_t            s_text_scr;
static moui_widget_label_t      s_text_title, s_text_lbl, s_text_hint;
static moui_widget_textinput_t  s_text_input;
static moui_widget_keyboard_t   s_text_kb;

static void kb_on_done(moui_widget_keyboard_t *kb, const char *text)
{
    (void)kb;
    moui_textinput_set_text(&s_text_input, text);
}

static void setup_text_screen(void)
{
    moui_screen_init(&s_text_scr);
    s_text_scr.enter_trans = MOUI_TRANS_NONE;
    int y = 0;

    moui_label_init(&s_text_title, "Text Input", &moui_font_cn_16);
    s_text_title.inverted = true;
    s_text_title.base.bounds = (moui_rect_t){0, y, TST_W, 22};
    moui_screen_add_widget(&s_text_scr, &s_text_title.base); y += 26;

    moui_textinput_init(&s_text_input, &moui_font_inter_16);
    moui_textinput_set_text(&s_text_input, "hello");
    s_text_input.base.bounds = (moui_rect_t){10, y, TST_W - 20, 26};
    moui_screen_add_widget(&s_text_scr, &s_text_input.base); y += 32;

    moui_label_init(&s_text_lbl, "Hello World\nLine 2\nCJK: 深圳测试", &moui_font_cn_16);
    s_text_lbl.base.bounds = (moui_rect_t){10, y, TST_W - 20, 60};
    moui_screen_add_widget(&s_text_scr, &s_text_lbl.base); y += 66;

    moui_keyboard_init(&s_text_kb, &moui_font_inter_16);
    s_text_kb.on_done = kb_on_done;
    s_text_kb.base.bounds = (moui_rect_t){0, y, TST_W, 200};
    moui_screen_add_widget(&s_text_scr, &s_text_kb.base); y += 206;

    moui_label_init(&s_text_hint, "ENT type  BK back", &moui_font_ascii_6x8);
    s_text_hint.base.bounds = (moui_rect_t){6, y, TST_W - 12, 10};
    moui_screen_add_widget(&s_text_scr, &s_text_hint.base);
}

/* ── Graphics screen: image / dots / number / sparkline / marquee / badge ── */

static moui_screen_t          s_gfx_scr;
static moui_widget_label_t    s_gfx_title, s_gfx_hint;
static moui_widget_image_t    s_g_img;
static moui_widget_dots_t     s_g_dots;
static moui_widget_number_t   s_g_number;
static moui_widget_sparkline_t s_g_spark;
static moui_widget_marquee_t  s_g_marquee;
static moui_widget_badge_t    s_g_badge;
static uint32_t               s_gfx_last_tick;
static float                  s_gfx_phase;

static void gfx_tick(moui_screen_t *s, uint32_t now_ms, float dt)
{
    (void)s; (void)dt;
    if (now_ms - s_gfx_last_tick < 5000) return;
    s_gfx_last_tick = now_ms;
    s_gfx_phase += 0.1f;
    s_g_number.value = (int32_t)(s_gfx_phase * 100);
    moui_sparkline_push(&s_g_spark, (int16_t)(50 + sinf(s_gfx_phase) * 45));
    s_g_dots.active = ((uint8_t)(s_gfx_phase) % 4);
    s_g_badge.count = (int)((uint8_t)s_gfx_phase % 30);
    moui_screen_mgr_mark_dirty(s_mgr);
}

static void setup_graphics_screen(void)
{
    moui_screen_init(&s_gfx_scr);
    s_gfx_scr.enter_trans = MOUI_TRANS_NONE;
    s_gfx_scr.on_tick = gfx_tick;
    int y = 0;

    moui_label_init(&s_gfx_title, "Graphics", &moui_font_cn_16);
    s_gfx_title.inverted = true;
    s_gfx_title.base.bounds = (moui_rect_t){0, y, TST_W, 22};
    moui_screen_add_widget(&s_gfx_scr, &s_gfx_title.base); y += 26;

    moui_number_init(&s_g_number, &moui_font_inter_16, 4);
    s_g_number.base.bounds = (moui_rect_t){10, y, 120, 20};
    moui_screen_add_widget(&s_gfx_scr, &s_g_number.base);

    moui_dots_init(&s_g_dots, 4);
    s_g_dots.active = 1;
    s_g_dots.base.bounds = (moui_rect_t){160, y + 6, 80, 10};
    moui_screen_add_widget(&s_gfx_scr, &s_g_dots.base); y += 26;

    moui_sparkline_init(&s_g_spark, 0, 100);
    s_g_spark.base.bounds = (moui_rect_t){10, y, TST_W - 20, 40};
    moui_screen_add_widget(&s_gfx_scr, &s_g_spark.base); y += 46;

    /* 16x16 icon image (from moui_icons) */
    extern const uint8_t moui_icon_menu[];
    moui_image_init(&s_g_img, moui_icon_menu, 16, 16);
    s_g_img.base.bounds = (moui_rect_t){10, y, 32, 32};
    moui_screen_add_widget(&s_gfx_scr, &s_g_img.base); y += 38;

    moui_marquee_init(&s_g_marquee, &moui_font_inter_16, "Scrolling marquee text...");
    s_g_marquee.base.bounds = (moui_rect_t){10, y, TST_W - 20, 22};
    moui_screen_add_widget(&s_gfx_scr, &s_g_marquee.base); y += 28;

    moui_badge_init(&s_g_badge, &moui_font_inter_16);
    s_g_badge.count = 5;
    s_g_badge.base.bounds = (moui_rect_t){10, y, 60, 20};
    moui_screen_add_widget(&s_gfx_scr, &s_g_badge.base); y += 26;

    moui_label_init(&s_gfx_hint, "UP/DN nav  ENT select  BK back", &moui_font_ascii_6x8);
    s_gfx_hint.base.bounds = (moui_rect_t){6, y, TST_W - 12, 10};
    moui_screen_add_widget(&s_gfx_scr, &s_gfx_hint.base);
}

/* ── CJK fonts screen ── */

static moui_screen_t        s_cjk_scr;
static moui_widget_label_t  s_cjk_title, s_cjk_lbl1, s_cjk_lbl2, s_cjk_lbl3, s_cjk_hint;

static void setup_cjk_screen(void)
{
    moui_screen_init(&s_cjk_scr);
    s_cjk_scr.enter_trans = MOUI_TRANS_NONE;
    int y = 0;

    moui_label_init(&s_cjk_title, "CJK Fonts", &moui_font_cn_16);
    s_cjk_title.inverted = true;
    s_cjk_title.base.bounds = (moui_rect_t){0, y, TST_W, 22};
    moui_screen_add_widget(&s_cjk_scr, &s_cjk_title.base); y += 26;

    moui_label_init(&s_cjk_lbl1, "中文测试 深圳 ABC123", &moui_font_cn_16);
    s_cjk_lbl1.base.bounds = (moui_rect_t){10, y, TST_W - 20, 22};
    moui_screen_add_widget(&s_cjk_scr, &s_cjk_lbl1.base); y += 26;

    moui_label_init(&s_cjk_lbl2, "简体混排：温度 25°C / 湿度 60%", &moui_font_cn_16);
    s_cjk_lbl2.base.bounds = (moui_rect_t){10, y, TST_W - 20, 22};
    moui_screen_add_widget(&s_cjk_scr, &s_cjk_lbl2.base); y += 26;

    moui_label_init(&s_cjk_lbl3, "文泉驿：电力 电池 信号 ★☆", &moui_font_wqy_16);
    s_cjk_lbl3.base.bounds = (moui_rect_t){10, y, TST_W - 20, 22};
    moui_screen_add_widget(&s_cjk_scr, &s_cjk_lbl3.base); y += 26;

    moui_label_init(&s_cjk_hint, "BK back", &moui_font_ascii_6x8);
    s_cjk_hint.base.bounds = (moui_rect_t){6, y, TST_W - 12, 10};
    moui_screen_add_widget(&s_cjk_scr, &s_cjk_hint.base);
}

/* ── Scaled Font screen: wqy_16 upscaled 2x/3x via the new API ── */

static moui_screen_t s_scaled_scr;

static void scaled_font_canvas(moui_draw_ctx_t *ctx, void *user)
{
    (void)user;
    moui_draw_fill_rect(ctx, &(moui_rect_t){0, 0, TST_W, TST_H}, MOUI_WHITE);

    moui_font_draw_str(ctx, &moui_font_ascii_6x8, 10, 10, "wqy_16 x1:", MOUI_BLACK);
    moui_font_draw_str(ctx, &moui_font_wqy_16, 80, 6, "深圳测试", MOUI_BLACK);

    moui_font_draw_str(ctx, &moui_font_ascii_6x8, 10, 40, "wqy_16 x2:", MOUI_BLACK);
    moui_font_draw_str_scaled(ctx, &moui_font_wqy_16, 80, 34, "深圳测试", MOUI_BLACK, 2);

    moui_font_draw_str(ctx, &moui_font_ascii_6x8, 10, 80, "wqy_16 x3:", MOUI_BLACK);
    moui_font_draw_str_scaled(ctx, &moui_font_wqy_16, 80, 72, "深圳", MOUI_BLACK, 3);

    moui_font_draw_str(ctx, &moui_font_ascii_6x8, 10, 130, "inter x3:", MOUI_BLACK);
    moui_font_draw_str_scaled(ctx, &moui_font_inter_16, 80, 122, "Hello 123", MOUI_BLACK, 3);
}

static void setup_scaled_font_screen(void)
{
    moui_screen_t *cs = moui_screen_canvas_create(scaled_font_canvas, NULL);
    if (cs) {
        s_scaled_scr = *cs;
    } else {
        moui_screen_init(&s_scaled_scr);
    }
}

/* ── QR screen ── */

static moui_screen_t       s_qr_scr;
static moui_widget_label_t s_qr_title;
static moui_qr_t           s_qr;

static void qr_canvas_draw(moui_draw_ctx_t *ctx, void *user)
{
    (void)user;
    moui_draw_fill_rect(ctx, &(moui_rect_t){0, 0, TST_W, TST_H}, MOUI_WHITE);
    moui_qr_draw(ctx, &s_qr, (TST_W - 29 * 6) / 2, 80, 6);
    moui_font_draw_str(ctx, &moui_font_ascii_6x8,
                       (TST_W - moui_font_measure_str(&moui_font_ascii_6x8, "https://github.com/movecall/moui")) / 2,
                       80 + 29 * 6 + 12, "https://github.com/movecall/moui", MOUI_BLACK);
}

static void setup_qr_screen(void)
{
    moui_qr_encode(&s_qr, "https://github.com/movecall/moui");

    /* use a canvas screen so the QR renders without needing a widget */
    moui_screen_t *cs = moui_screen_canvas_create(qr_canvas_draw, NULL);
    if (cs) {
        s_qr_scr = *cs;
    } else {
        moui_screen_init(&s_qr_scr);
        moui_label_init(&s_qr_title, "QR unavailable", &moui_font_cn_16);
        s_qr_title.inverted = true;
        s_qr_title.base.bounds = (moui_rect_t){0, 0, TST_W, 22};
        moui_screen_add_widget(&s_qr_scr, &s_qr_title.base);
    }
}

/* ── Test registry ── */

static const struct { void (*setup)(void); moui_screen_t *scr; } s_tests[] = {
    { setup_forms_screen,   &s_forms_scr },
    { setup_widgets_screen, &s_wdgt_scr },
    { setup_data_screen,    &s_data_scr },
    { setup_text_screen,    &s_text_scr },
    { setup_graphics_screen,&s_gfx_scr },
    { setup_cjk_screen,     &s_cjk_scr },
    { setup_scaled_font_screen, &s_scaled_scr },
    { setup_qr_screen,      &s_qr_scr },
};

void wt_show_test(int idx)
{
    if (idx < 0 || idx >= (int)(sizeof(s_tests) / sizeof(s_tests[0]))) return;
    s_tests[idx].setup();
    moui_screen_push(s_mgr, s_tests[idx].scr);
}

void widget_test_build(moui_screen_mgr_t *mgr)
{
    s_mgr = mgr;
    setup_main_screen();
    moui_screen_push(mgr, &s_main_scr);
}
