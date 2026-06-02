#ifndef MOUI_GRID_H
#define MOUI_GRID_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../widget/moui_widget.h"
#include "../moui_conf.h"

#define MOUI_GRID_MAX_CHILDREN MOUI_CONTAINER_MAX_CHILDREN

typedef struct {
    moui_widget_t    base;
    uint8_t         cols;
    uint8_t         rows;
    uint8_t         padding;
    uint8_t         gap;
    moui_widget_t   *children[MOUI_GRID_MAX_CHILDREN];
    uint8_t         child_count;
} moui_grid_t;

void moui_grid_init(moui_grid_t *g, uint8_t cols, uint8_t rows);
void moui_grid_add(moui_grid_t *g, moui_widget_t *child);
void moui_grid_layout(moui_grid_t *g);

#ifdef __cplusplus
}
#endif

#endif
