#ifndef MOUI_LAYOUT_FLEX_H
#define MOUI_LAYOUT_FLEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../widget/moui_widget.h"

#define MOUI_FLEX_MAX_ITEMS 32

typedef enum {
    MOUI_FLEX_DIR_ROW = 0,
    MOUI_FLEX_DIR_COLUMN,
} moui_flex_dir_t;

typedef enum {
    MOUI_FLEX_JUSTIFY_START = 0,
    MOUI_FLEX_JUSTIFY_CENTER,
    MOUI_FLEX_JUSTIFY_END,
    MOUI_FLEX_JUSTIFY_SPACE_BETWEEN,
    MOUI_FLEX_JUSTIFY_SPACE_AROUND,
} moui_flex_justify_t;

typedef enum {
    MOUI_FLEX_ALIGN_START = 0,
    MOUI_FLEX_ALIGN_CENTER,
    MOUI_FLEX_ALIGN_END,
    MOUI_FLEX_ALIGN_STRETCH,
} moui_flex_align_t;

typedef struct {
    moui_widget_t     *items[MOUI_FLEX_MAX_ITEMS];
    uint8_t            count;
    moui_flex_dir_t    dir;
    moui_flex_justify_t justify;
    moui_flex_align_t  align;
    int16_t            gap;
    int16_t            padding;
} moui_flex_t;

void moui_flex_init(moui_flex_t *flex, moui_flex_dir_t dir, moui_flex_justify_t justify, moui_flex_align_t align);
void moui_flex_add(moui_flex_t *flex, moui_widget_t *w);
void moui_flex_layout(moui_flex_t *flex, const moui_rect_t *parent_bounds);

#ifdef __cplusplus
}
#endif
#endif
