#include "moui_theme.h"

moui_theme_t moui_theme;

void moui_theme_set_default(void)
{
    moui_theme = (moui_theme_t){
        .padding          = 4,
        .gap              = 2,
        .corner_radius    = 4,
        .border_width     = 1,
        .statusbar_h      = 14,
        .list_item_h      = 20,
        .button_h         = 20,
        .slider_h         = 20,
        .tab_h            = 14,
        .focus_indicator_w = 4,
        .focus_invert     = true,
        .invert_colors    = false,
    };
}

void moui_theme_set_compact(void)
{
    moui_theme = (moui_theme_t){
        .padding          = 2,
        .gap              = 1,
        .corner_radius    = 2,
        .border_width     = 1,
        .statusbar_h      = 12,
        .list_item_h      = 16,
        .button_h         = 16,
        .slider_h         = 16,
        .tab_h            = 12,
        .focus_indicator_w = 3,
        .focus_invert     = true,
        .invert_colors    = false,
    };
}

void moui_theme_set_large(void)
{
    moui_theme = (moui_theme_t){
        .padding          = 6,
        .gap              = 4,
        .corner_radius    = 6,
        .border_width     = 2,
        .statusbar_h      = 18,
        .list_item_h      = 28,
        .button_h         = 24,
        .slider_h         = 24,
        .tab_h            = 18,
        .focus_indicator_w = 5,
        .focus_invert     = true,
        .invert_colors    = false,
    };
}
