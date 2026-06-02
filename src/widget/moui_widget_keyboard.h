#ifndef MOUI_WIDGET_KEYBOARD_H
#define MOUI_WIDGET_KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "moui_widget_btnmatrix.h"
#include "../font/moui_font.h"

typedef struct moui_widget_keyboard moui_widget_keyboard_t;

struct moui_widget_keyboard {
    moui_widget_t          base;
    moui_widget_btnmatrix_t matrix;
    bool                  shift;
    char                  output[64];
    uint8_t               out_len;
    void (*on_key)(moui_widget_keyboard_t *kb, char c);
    void (*on_done)(moui_widget_keyboard_t *kb, const char *text);
};

void moui_keyboard_init(moui_widget_keyboard_t *kb, const moui_font_t *font);

#ifdef __cplusplus
}
#endif

#endif
