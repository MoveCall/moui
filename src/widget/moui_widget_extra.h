#ifndef MOUI_WIDGET_EXTRA_H
#define MOUI_WIDGET_EXTRA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

/* ── Page Dots (pagination indicator) ── */
typedef struct {
    moui_widget_t base;
    uint8_t total;
    uint8_t active;
    uint8_t dot_r;
    uint8_t gap;
} moui_widget_dots_t;

void moui_dots_init(moui_widget_dots_t *d, uint8_t total);

/* ── Number Display (large digit readout) ── */
typedef struct {
    moui_widget_t     base;
    const moui_font_t *font;
    int32_t          value;
    uint8_t          digits;
    bool             leading_zero;
    const char      *unit;
} moui_widget_number_t;

void moui_number_init(moui_widget_number_t *n, const moui_font_t *font, uint8_t digits);

/* ── Stepper (+/- buttons with value) ── */
typedef struct moui_widget_stepper moui_widget_stepper_t;
struct moui_widget_stepper {
    moui_widget_t     base;
    const moui_font_t *font;
    int32_t          value;
    int32_t          min_val, max_val;
    int32_t          step;
    bool             capturing;
    void (*on_change)(moui_widget_stepper_t *s, int32_t val);
};

void moui_stepper_init(moui_widget_stepper_t *s, const moui_font_t *font,
                      int32_t min_val, int32_t max_val, int32_t step);

/* ── Sparkline (inline mini chart) ── */
#define MOUI_SPARKLINE_MAX 32

typedef struct {
    moui_widget_t base;
    int16_t      data[MOUI_SPARKLINE_MAX];
    uint8_t      count;
    uint8_t      write_pos;
    int16_t      y_min, y_max;
} moui_widget_sparkline_t;

void moui_sparkline_init(moui_widget_sparkline_t *s, int16_t y_min, int16_t y_max);
void moui_sparkline_push(moui_widget_sparkline_t *s, int16_t val);

/* ── Checkbox List (multi-select) ── */
#define MOUI_CHECKLIST_MAX 10

typedef struct moui_widget_checklist moui_widget_checklist_t;
struct moui_widget_checklist {
    moui_widget_t     base;
    const moui_font_t *font;
    const char      *items[MOUI_CHECKLIST_MAX];
    uint16_t         checked;  /* bitmask */
    uint8_t          count;
    uint8_t          cursor;
    uint8_t          item_h;
    bool             capturing;
    void (*on_change)(moui_widget_checklist_t *cl, uint8_t idx, bool checked);
};

void moui_checklist_init(moui_widget_checklist_t *cl, const moui_font_t *font);
void moui_checklist_set_items(moui_widget_checklist_t *cl, const char **items, uint8_t count);
bool moui_checklist_is_checked(const moui_widget_checklist_t *cl, uint8_t idx);

#ifdef __cplusplus
}
#endif

#endif
