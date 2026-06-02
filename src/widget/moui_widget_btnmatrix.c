#include "moui_widget_btnmatrix.h"
#include "../core/moui_theme.h"
#include "../core/moui_event_bus.h"
#include "../core/moui_theme.h"

static void btnmatrix_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_btnmatrix_t *bm = (moui_widget_btnmatrix_t *)w;
    if (!bm->font || bm->btn_count == 0 || bm->cols == 0) return;

    uint8_t rows = (bm->btn_count + bm->cols - 1) / bm->cols;
    int bw = w->bounds.w / bm->cols;
    int bh = w->bounds.h / rows;

    for (int i = 0; i < bm->btn_count; i++) {
        int r = i / bm->cols, c = i % bm->cols;
        int bx = w->bounds.x + c * bw;
        int by = w->bounds.y + r * bh;

        if (i == bm->selected && bm->capturing) {
            moui_draw_fill_rounded_rect(ctx, &(moui_rect_t){bx + 1, by + 1, bw - 2, bh - 2}, 3, moui_theme_fg());
            int tw = moui_font_measure_str(bm->font, bm->labels[i]);
            moui_font_draw_str(ctx, bm->font, bx + (bw - tw) / 2,
                              by + (bh - bm->font->line_height) / 2, bm->labels[i], moui_theme_bg());
        } else {
            moui_draw_rounded_rect(ctx, &(moui_rect_t){bx + 1, by + 1, bw - 2, bh - 2}, 3, moui_theme_fg());
            int tw = moui_font_measure_str(bm->font, bm->labels[i]);
            moui_font_draw_str(ctx, bm->font, bx + (bw - tw) / 2,
                              by + (bh - bm->font->line_height) / 2, bm->labels[i], moui_theme_fg());
        }
    }

    if (moui_widget_has_state(w, MOUI_STATE_FOCUSED) && !bm->capturing)
        moui_draw_rect(ctx, &w->bounds, moui_theme_fg());
}

static bool btnmatrix_on_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_widget_btnmatrix_t *bm = (moui_widget_btnmatrix_t *)w;
    if (bm->btn_count == 0) return false;

    if (!bm->capturing) {
        if (ev->type == MOUI_EV_ENCODER_PRESS) { bm->capturing = true; return true; }
        return false;
    }

    switch (ev->type) {
    case MOUI_EV_ENCODER_CW:
        if (bm->selected < bm->btn_count - 1) bm->selected++;
        return true;
    case MOUI_EV_ENCODER_CCW:
        if (bm->selected > 0) bm->selected--;
        return true;
    case MOUI_EV_ENCODER_PRESS:
        if (bm->on_click) bm->on_click(bm, bm->selected);
        moui_event_bus_publish(&moui_bus, &(moui_event_t){
            .type = MOUI_EVT_BUTTON_CLICK, .id = w->event_id,
            .value = bm->selected, .sender = w,
        });
        return true;
    case MOUI_EV_ENCODER_BACK:
        bm->capturing = false;
        return true;
    default: return false;
    }
}

static void btnmatrix_on_focus(moui_widget_t *w, bool gained)
{
    moui_widget_btnmatrix_t *bm = (moui_widget_btnmatrix_t *)w;
    if (!gained) bm->capturing = false;
}

static const moui_widget_vtable_t btnmatrix_vtable = {
    .draw = btnmatrix_draw, .on_event = btnmatrix_on_event, .on_focus = btnmatrix_on_focus,
};

void moui_btnmatrix_init(moui_widget_btnmatrix_t *bm, const moui_font_t *font, uint8_t cols)
{
    moui_widget_init(&bm->base, &btnmatrix_vtable);
    bm->font = font;
    bm->cols = cols > 0 ? cols : 3;
    bm->btn_count = 0;
    bm->selected = 0;
    bm->capturing = false;
}

void moui_btnmatrix_set_labels(moui_widget_btnmatrix_t *bm, const char **labels, uint8_t count)
{
    if (count > MOUI_BTNMATRIX_MAX_BTNS) count = MOUI_BTNMATRIX_MAX_BTNS;
    for (int i = 0; i < count; i++) bm->labels[i] = labels[i];
    bm->btn_count = count;
    bm->selected = 0;
}
