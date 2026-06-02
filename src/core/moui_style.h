#ifndef MOUI_STYLE_H
#define MOUI_STYLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "../font/moui_font.h"
#include "moui_theme.h"

typedef struct {
    const moui_font_t *font;
    uint8_t  padding;
    uint8_t  corner_radius;
    uint8_t  border_width;
    bool     invert_on_focus;
    bool     show_border;
} moui_style_t;

extern const moui_style_t moui_style_default;
extern const moui_style_t moui_style_flat;
extern const moui_style_t moui_style_outlined;

static inline uint8_t moui_style_pad(const moui_style_t *s)
{
    if (s && s->padding) return s->padding;
    return moui_theme.padding;
}

static inline uint8_t moui_style_radius(const moui_style_t *s)
{
    if (s && s->corner_radius) return s->corner_radius;
    return moui_theme.corner_radius;
}

static inline const moui_font_t *moui_style_font(const moui_style_t *s)
{
    if (s && s->font) return s->font;
    if (moui_theme.default_font) return (const moui_font_t *)moui_theme.default_font;
    return &moui_font_ascii_6x8;
}

static inline bool moui_style_invert_focus(const moui_style_t *s)
{
    if (s) return s->invert_on_focus;
    return moui_theme.focus_invert;
}

static inline bool moui_style_show_border(const moui_style_t *s)
{
    return s ? s->show_border : true;
}


#ifdef __cplusplus
}
#endif
#endif
