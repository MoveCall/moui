#ifndef MOUI_WIDGET_TABLE_H
#define MOUI_WIDGET_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

typedef struct {
    moui_widget_t     base;
    const moui_font_t *font;
    const char      *cells[MOUI_TABLE_MAX_ROWS][MOUI_TABLE_MAX_COLS];
    uint8_t          col_count;
    uint8_t          row_count;
    uint8_t          col_w[MOUI_TABLE_MAX_COLS];
    uint8_t          row_h;
    bool             header_row;
} moui_widget_table_t;

void moui_table_init(moui_widget_table_t *t, const moui_font_t *font, uint8_t cols, uint8_t rows);
void moui_table_set_cell(moui_widget_table_t *t, uint8_t row, uint8_t col, const char *text);


#ifdef __cplusplus
}
#endif
#endif
