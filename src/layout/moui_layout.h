#ifndef MOUI_LAYOUT_H
#define MOUI_LAYOUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../widget/moui_widget.h"

typedef enum {
    MOUI_STACK_VERTICAL,
    MOUI_STACK_HORIZONTAL,
} moui_stack_dir_t;

typedef struct {
    moui_widget_t    base;
    moui_stack_dir_t direction;
    uint8_t         padding;
    uint8_t         gap;
    moui_widget_t   *children[MOUI_CONTAINER_MAX_CHILDREN];
    uint8_t         child_count;
} moui_container_t;

void moui_container_init(moui_container_t *c, moui_stack_dir_t dir);
void moui_container_add(moui_container_t *c, moui_widget_t *child);
void moui_container_layout(moui_container_t *c);
void moui_container_draw(moui_container_t *c, moui_draw_ctx_t *ctx);


#ifdef __cplusplus
}
#endif
#endif
