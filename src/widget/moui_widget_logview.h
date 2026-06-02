#ifndef MOUI_WIDGET_LOGVIEW_H
#define MOUI_WIDGET_LOGVIEW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

#define MOUI_LOGVIEW_MAX_LINES 16
#define MOUI_LOGVIEW_LINE_LEN  48

typedef struct moui_widget_logview moui_widget_logview_t;

struct moui_widget_logview {
    moui_widget_t     base;
    const moui_font_t *font;
    char             lines[MOUI_LOGVIEW_MAX_LINES][MOUI_LOGVIEW_LINE_LEN];
    uint8_t          head;
    uint8_t          count;
    bool             auto_scroll;
    bool             show_border;
};

void moui_logview_init(moui_widget_logview_t *lv, const moui_font_t *font);
void moui_logview_add_line(moui_widget_logview_t *lv, const char *line);
void moui_logview_clear(moui_widget_logview_t *lv);

#ifdef __cplusplus
}
#endif
#endif
