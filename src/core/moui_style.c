#include "moui_style.h"

const moui_style_t moui_style_default = {
    .font            = NULL,
    .padding         = 4,
    .corner_radius   = 4,
    .border_width    = 1,
    .invert_on_focus = true,
    .show_border     = true,
};

const moui_style_t moui_style_flat = {
    .font            = NULL,
    .padding         = 2,
    .corner_radius   = 0,
    .border_width    = 0,
    .invert_on_focus = true,
    .show_border     = false,
};

const moui_style_t moui_style_outlined = {
    .font            = NULL,
    .padding         = 4,
    .corner_radius   = 6,
    .border_width    = 2,
    .invert_on_focus = false,
    .show_border     = true,
};
