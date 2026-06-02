#ifndef MOUI_WIDGET_TEXTINPUT_H
#define MOUI_WIDGET_TEXTINPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

typedef struct moui_widget_textinput moui_widget_textinput_t;

struct moui_widget_textinput {
    moui_widget_t     base;
    const moui_font_t *font;
    char             text[MOUI_TEXTINPUT_MAX_LEN + 1];
    uint8_t          cursor;
    uint8_t          len;
    bool             editing;
    bool             char_select;
    char             current_char;
    const char      *charset;

    void (*on_done)(moui_widget_textinput_t *ti);
};

void moui_textinput_init(moui_widget_textinput_t *ti, const moui_font_t *font);
void moui_textinput_set_text(moui_widget_textinput_t *ti, const char *text);


#ifdef __cplusplus
}
#endif
#endif
