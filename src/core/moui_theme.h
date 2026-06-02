#ifndef MOUI_THEME_H
#define MOUI_THEME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "moui_color.h"

struct moui_font;

typedef struct {
    uint8_t padding;
    uint8_t gap;
    uint8_t corner_radius;
    uint8_t border_width;

    uint8_t statusbar_h;
    uint8_t list_item_h;
    uint8_t button_h;
    uint8_t slider_h;
    uint8_t tab_h;

    uint8_t focus_indicator_w;
    bool    focus_invert;

    bool    invert_colors;
    const struct moui_font *default_font;
} moui_theme_t;

extern moui_theme_t moui_theme;

void moui_theme_set_default(void);
void moui_theme_set_compact(void);
void moui_theme_set_large(void);

static inline moui_color_t moui_theme_fg(void) { return moui_theme.invert_colors ? MOUI_WHITE : MOUI_BLACK; }
static inline moui_color_t moui_theme_bg(void) { return moui_theme.invert_colors ? MOUI_BLACK : MOUI_WHITE; }


#ifdef __cplusplus
}
#endif
#endif
