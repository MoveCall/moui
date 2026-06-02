#ifndef MOUI_WIDGET_TREEVIEW_H
#define MOUI_WIDGET_TREEVIEW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"
#include "../font/moui_font.h"

#define MOUI_TREEVIEW_MAX_NODES 32

typedef struct {
    const char *label;
    uint8_t     depth;
    bool        expanded;
    bool        has_children;
} moui_tree_node_t;

typedef struct moui_widget_treeview moui_widget_treeview_t;

struct moui_widget_treeview {
    moui_widget_t     base;
    const moui_font_t *font;
    moui_tree_node_t  nodes[MOUI_TREEVIEW_MAX_NODES];
    uint16_t          node_count;
    uint16_t          selected;

    void (*on_select)(moui_widget_treeview_t *tv, uint16_t node_idx);
};

void moui_treeview_init(moui_widget_treeview_t *tv, const moui_font_t *font);
void moui_treeview_add_node(moui_widget_treeview_t *tv, const char *label, uint8_t depth, bool has_children);

#ifdef __cplusplus
}
#endif
#endif
