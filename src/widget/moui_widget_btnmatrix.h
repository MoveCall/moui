#ifndef MOUI_WIDGET_BTNMATRIX_H
#define MOUI_WIDGET_BTNMATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

typedef struct moui_widget_btnmatrix moui_widget_btnmatrix_t;

struct moui_widget_btnmatrix {
    moui_widget_t     base;
    const moui_font_t *font;
    const char      *labels[MOUI_BTNMATRIX_MAX_BTNS];
    uint8_t          btn_count;
    uint8_t          cols;
    uint8_t          selected;
    bool             capturing;

    void (*on_click)(moui_widget_btnmatrix_t *bm, uint8_t idx);
};

void moui_btnmatrix_init(moui_widget_btnmatrix_t *bm, const moui_font_t *font, uint8_t cols);
void moui_btnmatrix_set_labels(moui_widget_btnmatrix_t *bm, const char **labels, uint8_t count);


#ifdef __cplusplus
}
#endif
#endif
