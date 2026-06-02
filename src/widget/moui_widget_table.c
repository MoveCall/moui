#include "moui_widget_table.h"
#include "../core/moui_theme.h"
#include <string.h>
#include "../core/moui_theme.h"

static void table_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_table_t *t = (moui_widget_table_t *)w;
    if (!t->font) return;

    int x0 = w->bounds.x, y0 = w->bounds.y;

    for (int r = 0; r <= t->row_count; r++) {
        int y = y0 + r * t->row_h;
        moui_draw_hline(ctx, x0, y, w->bounds.w, moui_theme_fg());
    }

    int cx = x0;
    for (int c = 0; c <= t->col_count; c++) {
        moui_draw_vline(ctx, cx, y0, t->row_count * t->row_h, moui_theme_fg());
        if (c < t->col_count) cx += t->col_w[c];
    }

    cx = x0;
    for (int c = 0; c < t->col_count; c++) {
        for (int r = 0; r < t->row_count; r++) {
            int y = y0 + r * t->row_h;
            if (t->header_row && r == 0) {
                moui_draw_fill_rect(ctx, &(moui_rect_t){cx + 1, y + 1, t->col_w[c] - 1, t->row_h - 1}, moui_theme_fg());
                if (t->cells[r][c])
                    moui_font_draw_str(ctx, t->font, cx + 2, y + (t->row_h - t->font->line_height) / 2,
                                      t->cells[r][c], moui_theme_bg());
            } else {
                if (t->cells[r][c])
                    moui_font_draw_str(ctx, t->font, cx + 2, y + (t->row_h - t->font->line_height) / 2,
                                      t->cells[r][c], moui_theme_fg());
            }
        }
        cx += t->col_w[c];
    }
}

static const moui_widget_vtable_t table_vtable = { .draw = table_draw };

void moui_table_init(moui_widget_table_t *t, const moui_font_t *font, uint8_t cols, uint8_t rows)
{
    moui_widget_init(&t->base, &table_vtable);
    t->font = font;
    t->col_count = cols > MOUI_TABLE_MAX_COLS ? MOUI_TABLE_MAX_COLS : cols;
    t->row_count = rows > MOUI_TABLE_MAX_ROWS ? MOUI_TABLE_MAX_ROWS : rows;
    if (t->col_count == 0) t->col_count = 1;
    if (t->row_count == 0) t->row_count = 1;
    t->row_h = font ? font->line_height + 4 : 12;
    t->header_row = true;
    memset(t->cells, 0, sizeof(t->cells));
    int default_w = 160 / t->col_count;
    for (int i = 0; i < t->col_count; i++) t->col_w[i] = default_w;
    t->base.enabled = 0;
}

void moui_table_set_cell(moui_widget_table_t *t, uint8_t row, uint8_t col, const char *text)
{
    if (row < t->row_count && col < t->col_count)
        t->cells[row][col] = text;
}
