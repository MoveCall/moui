#include "moui_widget_treeview.h"
#include "../core/moui_theme.h"
#include <string.h>

static void treeview_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_treeview_t *tv = (moui_widget_treeview_t *)w;
    moui_rect_t b = w->bounds;
    const moui_font_t *f = tv->font ? tv->font : &moui_font_ascii_6x8;

    moui_draw_rect(ctx, &b, moui_theme_fg());

    int y = b.y + 2;
    int line_h = f->line_height + 4;

    for (uint16_t i = 0; i < tv->node_count; i++) {
        if (y + line_h > b.y + b.h) break;

        moui_tree_node_t *node = &tv->nodes[i];
        int indent = 4 + node->depth * 12;

        /* Selection highlight bar */
        if (i == tv->selected && moui_widget_has_state(w, MOUI_STATE_FOCUSED)) {
            moui_rect_t sel_r = { (int16_t)(b.x + 2), (int16_t)y, (int16_t)(b.w - 4), (int16_t)line_h };
            moui_draw_fill_rect(ctx, &sel_r, moui_theme_fg());
        }

        moui_color_t color = (i == tv->selected && moui_widget_has_state(w, MOUI_STATE_FOCUSED))
                             ? moui_theme_bg() : moui_theme_fg();

        /* Node icon [+] / [-] or dot */
        const char *prefix = node->has_children ? (node->expanded ? "[-] " : "[+] ") : "  ";
        moui_font_draw_str(ctx, f, b.x + indent, y + 2, prefix, color);
        int pw = moui_font_measure_str(f, prefix);
        moui_font_draw_str(ctx, f, b.x + indent + pw, y + 2, node->label, color);

        y += line_h;
    }
}

static bool treeview_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_widget_treeview_t *tv = (moui_widget_treeview_t *)w;
    if (!w->enabled || tv->node_count == 0) return false;

    if (ev->type == MOUI_EV_ENCODER_CCW || ev->type == MOUI_EV_GESTURE_SWIPE_DOWN) {
        if (tv->selected > 0) tv->selected--;
        return true;
    }

    if (ev->type == MOUI_EV_ENCODER_CW || ev->type == MOUI_EV_GESTURE_SWIPE_UP) {
        if (tv->selected + 1 < tv->node_count) tv->selected++;
        return true;
    }

    if (ev->type == MOUI_EV_ENCODER_PRESS || ev->type == MOUI_EV_GESTURE_TAP || ev->type == MOUI_EV_POINTER_DOWN) {
        moui_tree_node_t *node = &tv->nodes[tv->selected];
        if (node->has_children) {
            node->expanded = !node->expanded;
        }
        if (tv->on_select) tv->on_select(tv, tv->selected);
        return true;
    }

    return false;
}

static const moui_widget_vtable_t tv_vtable = {
    .draw = treeview_draw,
    .on_event = treeview_event,
};

void moui_treeview_init(moui_widget_treeview_t *tv, const moui_font_t *font)
{
    moui_widget_init(&tv->base, &tv_vtable);
    tv->font = font;
    tv->node_count = 0;
    tv->selected = 0;
    tv->on_select = NULL;
    memset(tv->nodes, 0, sizeof(tv->nodes));
}

void moui_treeview_add_node(moui_widget_treeview_t *tv, const char *label, uint8_t depth, bool has_children)
{
    if (tv->node_count >= MOUI_TREEVIEW_MAX_NODES) return;
    moui_tree_node_t *n = &tv->nodes[tv->node_count++];
    n->label = label;
    n->depth = depth;
    n->expanded = false;
    n->has_children = has_children;
}
