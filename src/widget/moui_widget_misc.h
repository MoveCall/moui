#ifndef MOUI_WIDGET_MISC_H
#define MOUI_WIDGET_MISC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

/* ---- Marquee (scrolling text) ---- */
typedef struct {
    moui_widget_t     base;
    const moui_font_t *font;
    const char      *text;
    int16_t          offset;
    int16_t          text_w;
    uint8_t          speed;
} moui_widget_marquee_t;

void moui_marquee_init(moui_widget_marquee_t *m, const moui_font_t *font, const char *text);
void moui_marquee_tick(moui_widget_marquee_t *m);

/* ---- Badge (number dot) ---- */
typedef struct {
    moui_widget_t     base;
    const moui_font_t *font;
    int16_t          count;
    bool             dot_only;
} moui_widget_badge_t;

void moui_badge_init(moui_widget_badge_t *b, const moui_font_t *font);

/* ---- Divider (line separator) ---- */
typedef struct {
    moui_widget_t base;
    bool         vertical;
} moui_widget_divider_t;

void moui_divider_init(moui_widget_divider_t *d, bool vertical);

/* ---- Loading spinner ---- */
typedef struct {
    moui_widget_t base;
    float        angle;
    uint8_t      speed;
} moui_widget_loading_t;

void moui_loading_init(moui_widget_loading_t *l);
void moui_loading_tick(moui_widget_loading_t *l, float dt);


#ifdef __cplusplus
}
#endif
#endif
