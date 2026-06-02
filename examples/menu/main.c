/*
 * Moui Example: Settings Menu
 *
 * Shows how to build a multi-level settings menu
 * with various input widgets.
 *
 * Build: link with moui_framework + moui_fonts
 */

#include "moui.h"

static moui_screen_t          scr_main, scr_display, scr_input;
static moui_widget_label_t    main_title, disp_title, input_title;
static moui_widget_list_t     main_list;
static moui_widget_slider_t   brightness, contrast;
static moui_widget_switch_t   dark_mode;
static moui_widget_stepper_t  volume;
static moui_widget_checklist_t features;
static moui_widget_label_t    lbl_bright, lbl_contrast, lbl_dark, lbl_vol, lbl_feat;
static moui_widget_button_t   save_btn;

static moui_screen_mgr_t *s_mgr;

static const char *menu_items[] = { "Display", "Input", "About" };
static const char *feat_items[] = { "Vibration", "Sound", "LED" };
static const char *array_adapter(uint16_t idx, void *data) { return ((const char **)data)[idx]; }

static void push_display(void) { moui_screen_push(s_mgr, &scr_display); }
static void push_input_scr(void) { moui_screen_push(s_mgr, &scr_input); }

static void on_menu_select(moui_widget_list_t *l, uint16_t idx)
{
    (void)l;
    switch (idx) {
    case 0: push_display(); break;
    case 1: push_input_scr(); break;
    case 2: moui_popup_show_toast(&s_mgr->popup, "v1.0", 1500); break;
    }
}

static void on_dark_toggle(moui_widget_button_t *b, bool state)
{
    (void)b;
    moui_theme.invert_colors = state;
}

static void on_save(moui_widget_button_t *b)
{
    (void)b;
    moui_popup_show_toast(&s_mgr->popup, "Saved!", 1000);
}

void setup_menu_example(moui_screen_mgr_t *mgr)
{
    s_mgr = mgr;

    /* ── Main Menu ── */
    moui_screen_init(&scr_main);
    moui_label_init(&main_title, "Settings", &moui_font_ascii_6x8);
    main_title.inverted = true;
    main_title.base.bounds = (moui_rect_t){0, 0, MOUI_DISP_W, 16};
    moui_screen_add_widget(&scr_main, &main_title.base);

    moui_list_init(&main_list, &moui_font_ascii_6x8);
    moui_list_set_adapter(&main_list, array_adapter, (void *)menu_items, 3);
    main_list.on_select = on_menu_select;
    main_list.capturing = true;
    main_list.base.bounds = (moui_rect_t){0, 18, MOUI_DISP_W, 200};
    moui_screen_add_widget(&scr_main, &main_list.base);
    moui_focus_group_add(&scr_main.focus, &main_list.base);

    /* ── Display Settings ── */
    moui_screen_init(&scr_display);
    scr_display.enter_trans = MOUI_TRANS_SLIDE_LEFT;
    int y = 0;

    moui_label_init(&disp_title, "Display", &moui_font_ascii_6x8);
    disp_title.inverted = true;
    disp_title.base.bounds = (moui_rect_t){0, y, MOUI_DISP_W, 16};
    moui_screen_add_widget(&scr_display, &disp_title.base);
    y += 20;

    moui_label_init(&lbl_bright, "Brightness", &moui_font_ascii_6x8);
    lbl_bright.base.bounds = (moui_rect_t){4, y, 80, 10};
    moui_screen_add_widget(&scr_display, &lbl_bright.base);
    y += 12;

    moui_slider_init(&brightness, &moui_font_ascii_6x8, 0, 100, 5);
    brightness.value = 70;
    brightness.base.bounds = (moui_rect_t){4, y, 160, 18};
    moui_screen_add_widget(&scr_display, &brightness.base);
    y += 22;

    moui_label_init(&lbl_contrast, "Contrast", &moui_font_ascii_6x8);
    lbl_contrast.base.bounds = (moui_rect_t){4, y, 80, 10};
    moui_screen_add_widget(&scr_display, &lbl_contrast.base);
    y += 12;

    moui_slider_init(&contrast, &moui_font_ascii_6x8, 0, 100, 5);
    contrast.value = 50;
    contrast.base.bounds = (moui_rect_t){4, y, 160, 18};
    moui_screen_add_widget(&scr_display, &contrast.base);
    y += 24;

    moui_label_init(&lbl_dark, "Dark Mode", &moui_font_ascii_6x8);
    lbl_dark.base.bounds = (moui_rect_t){4, y, 80, 10};
    moui_screen_add_widget(&scr_display, &lbl_dark.base);

    moui_switch_init(&dark_mode);
    dark_mode.base.bounds = (moui_rect_t){120, y - 2, 36, 16};
    moui_screen_add_widget(&scr_display, &dark_mode.base);

    moui_focus_group_add(&scr_display.focus, &brightness.base);
    moui_focus_group_add(&scr_display.focus, &contrast.base);
    moui_focus_group_add(&scr_display.focus, &dark_mode.base);

    /* ── Input Settings ── */
    moui_screen_init(&scr_input);
    scr_input.enter_trans = MOUI_TRANS_SLIDE_LEFT;
    y = 0;

    moui_label_init(&input_title, "Input", &moui_font_ascii_6x8);
    input_title.inverted = true;
    input_title.base.bounds = (moui_rect_t){0, y, MOUI_DISP_W, 16};
    moui_screen_add_widget(&scr_input, &input_title.base);
    y += 20;

    moui_label_init(&lbl_vol, "Volume", &moui_font_ascii_6x8);
    lbl_vol.base.bounds = (moui_rect_t){4, y, 60, 10};
    moui_screen_add_widget(&scr_input, &lbl_vol.base);
    y += 12;

    moui_stepper_init(&volume, &moui_font_ascii_6x8, 0, 100, 10);
    volume.value = 50;
    volume.base.bounds = (moui_rect_t){4, y, 160, 20};
    moui_screen_add_widget(&scr_input, &volume.base);
    y += 24;

    moui_label_init(&lbl_feat, "Features", &moui_font_ascii_6x8);
    lbl_feat.base.bounds = (moui_rect_t){4, y, 60, 10};
    moui_screen_add_widget(&scr_input, &lbl_feat.base);
    y += 12;

    moui_checklist_init(&features, &moui_font_ascii_6x8);
    moui_checklist_set_items(&features, feat_items, 3);
    features.checked = 0x03;
    features.base.bounds = (moui_rect_t){4, y, 160, 3 * features.item_h};
    moui_screen_add_widget(&scr_input, &features.base);
    y += 3 * features.item_h + 8;

    moui_button_init(&save_btn, "[ Save ]", &moui_font_ascii_6x8, MOUI_BTN_PUSH);
    save_btn.on_click = on_save;
    save_btn.base.bounds = (moui_rect_t){30, y, 108, 20};
    moui_screen_add_widget(&scr_input, &save_btn.base);

    moui_focus_group_add(&scr_input.focus, &volume.base);
    moui_focus_group_add(&scr_input.focus, &features.base);
    moui_focus_group_add(&scr_input.focus, &save_btn.base);

    moui_screen_push(mgr, &scr_main);
}

/*
 * Usage:
 *   setup_menu_example(&mgr);
 *   // then run mgr tick loop
 */
