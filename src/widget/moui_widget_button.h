#ifndef MOUI_WIDGET_BUTTON_H
#define MOUI_WIDGET_BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

typedef enum {
    MOUI_BTN_PUSH,
    MOUI_BTN_TOGGLE,
    MOUI_BTN_CHECKBOX,
} moui_btn_type_t;

typedef struct moui_widget_button moui_widget_button_t;

struct moui_widget_button {
    moui_widget_t     base;
    const char      *label;
    const moui_font_t *font;
    moui_btn_type_t   type;
    bool             state;

    void (*on_click)(moui_widget_button_t *btn);
    void (*on_toggle)(moui_widget_button_t *btn, bool new_state);
};

void moui_button_init(moui_widget_button_t *b, const char *label,
                     const moui_font_t *font, moui_btn_type_t type);


#ifdef __cplusplus
}
#endif
#endif
