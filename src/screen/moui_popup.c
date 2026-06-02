#include "moui_popup.h"
#include <string.h>

void moui_popup_init(moui_popup_t *p, const moui_font_t *font)
{
    memset(p, 0, sizeof(*p));
    p->font = font;
}

void moui_popup_show_toast(moui_popup_t *p, const char *msg, uint32_t duration_ms)
{
    p->type = MOUI_POPUP_TOAST;
    p->title = NULL;
    p->message = msg;
    p->visible = true;
    p->duration_ms = duration_ms;
    p->show_time = 0;
}

void moui_popup_show_alert(moui_popup_t *p, const char *title, const char *msg)
{
    p->type = MOUI_POPUP_ALERT;
    p->title = title;
    p->message = msg;
    p->visible = true;
    p->selected = 0;
    p->duration_ms = 0;
}

void moui_popup_show_confirm(moui_popup_t *p, const char *title, const char *msg,
                            void (*on_close)(moui_popup_t *, bool))
{
    p->type = MOUI_POPUP_CONFIRM;
    p->title = title;
    p->message = msg;
    p->visible = true;
    p->selected = 1;
    p->on_close = on_close;
    p->duration_ms = 0;
}

void moui_popup_show_menu(moui_popup_t *p, const char *title,
                         const char **items, uint8_t count,
                         void (*on_select)(moui_popup_t *, int))
{
    p->type = MOUI_POPUP_MENU;
    p->title = title;
    p->message = NULL;
    p->items = items;
    p->item_count = count > MOUI_POPUP_MENU_MAX_ITEMS ? MOUI_POPUP_MENU_MAX_ITEMS : count;
    p->visible = true;
    p->selected = 0;
    p->scroll_top = 0;
    p->on_menu_select = on_select;
    p->duration_ms = 0;
}

bool moui_popup_handle_event(moui_popup_t *p, const moui_input_event_t *ev)
{
    if (!p->visible) return false;
    if (p->type == MOUI_POPUP_TOAST) {
        if (ev->type == MOUI_EV_ENCODER_PRESS || ev->type == MOUI_EV_ENCODER_BACK) {
            p->visible = false;
            return true;
        }
        return true;
    }

    if (p->type == MOUI_POPUP_ALERT) {
        if (ev->type == MOUI_EV_ENCODER_PRESS || ev->type == MOUI_EV_ENCODER_BACK) {
            p->visible = false;
            return true;
        }
        return true;
    }

    if (p->type == MOUI_POPUP_CONFIRM) {
        switch (ev->type) {
        case MOUI_EV_ENCODER_CW:
        case MOUI_EV_ENCODER_CCW:
            p->selected = p->selected ? 0 : 1;
            return true;
        case MOUI_EV_ENCODER_PRESS:
            p->result = (p->selected == 1);
            p->visible = false;
            if (p->on_close) p->on_close(p, p->result);
            return true;
        case MOUI_EV_ENCODER_BACK:
            p->result = false;
            p->visible = false;
            if (p->on_close) p->on_close(p, false);
            return true;
        default:
            return true;
        }
    }

    if (p->type == MOUI_POPUP_MENU) {
        switch (ev->type) {
        case MOUI_EV_ENCODER_CW:
            if (p->item_count > 0 && p->selected < p->item_count - 1) p->selected++;
            return true;
        case MOUI_EV_ENCODER_CCW:
            if (p->selected > 0) p->selected--;
            return true;
        case MOUI_EV_ENCODER_PRESS:
            p->visible = false;
            if (p->on_menu_select) p->on_menu_select(p, p->selected);
            return true;
        case MOUI_EV_ENCODER_BACK:
            p->visible = false;
            if (p->on_menu_select) p->on_menu_select(p, -1);
            return true;
        default:
            return true;
        }
    }

    return false;
}

static void draw_box(moui_draw_ctx_t *ctx, int x, int y, int w, int h)
{
    moui_draw_fill_rect(ctx, &(moui_rect_t){x, y, w, h}, MOUI_WHITE);
    moui_draw_rounded_rect(ctx, &(moui_rect_t){x, y, w, h}, 4, MOUI_BLACK);
    moui_draw_rounded_rect(ctx, &(moui_rect_t){x + 1, y + 1, w - 2, h - 2}, 3, MOUI_BLACK);
}

void moui_popup_draw(moui_popup_t *p, moui_draw_ctx_t *ctx, uint32_t now_ms)
{
    if (!p->visible || !p->font) return;

    if (p->type == MOUI_POPUP_TOAST) {
        if (p->show_time == 0) p->show_time = now_ms;
        if (p->duration_ms > 0 && (now_ms - p->show_time) > p->duration_ms) {
            p->visible = false;
            return;
        }

        int tw = moui_font_measure_str(p->font, p->message);
        int pad = 6;
        int w = tw + pad * 2;
        if (w > MOUI_DISP_W - 8) w = MOUI_DISP_W - 8;
        int h = p->font->line_height + pad * 2;
        int x = (MOUI_DISP_W - w) / 2;
        int y = MOUI_DISP_H - h - 20;

        draw_box(ctx, x, y, w, h);
        moui_font_draw_str(ctx, p->font, x + pad, y + pad, p->message, MOUI_BLACK);
        return;
    }

    if (p->type == MOUI_POPUP_MENU && p->items) {
        int pad = 6;
        int box_w = MOUI_DISP_W - 16;
        int box_x = 8;
        int title_h = p->title ? (p->font->line_height + 4) : 0;
        int item_h = p->font->line_height + 4;
        int max_visible = (MOUI_DISP_H - 40) / item_h;
        if (max_visible > p->item_count) max_visible = p->item_count;

        int menu_h = pad + title_h + item_h * max_visible + pad;
        int menu_y = (MOUI_DISP_H - menu_h) / 2;

        if (p->selected < p->scroll_top)
            p->scroll_top = p->selected;
        if (p->selected >= p->scroll_top + max_visible)
            p->scroll_top = p->selected - max_visible + 1;

        draw_box(ctx, box_x, menu_y, box_w, menu_h);
        int my = menu_y + pad;

        if (p->title) {
            moui_draw_fill_rect(ctx, &(moui_rect_t){box_x + 2, my, box_w - 4, p->font->line_height + 2}, MOUI_BLACK);
            moui_font_draw_str(ctx, p->font, box_x + pad, my + 1, p->title, MOUI_WHITE);
            my += title_h;
        }

        for (int i = 0; i < max_visible; i++) {
            int idx = p->scroll_top + i;
            if (idx >= p->item_count) break;
            if (idx == p->selected) {
                moui_draw_fill_rect(ctx, &(moui_rect_t){box_x + 3, my, box_w - 6, item_h}, MOUI_BLACK);
                moui_font_draw_str(ctx, p->font, box_x + pad + 2, my + 2, p->items[idx], MOUI_WHITE);
            } else {
                moui_font_draw_str(ctx, p->font, box_x + pad + 2, my + 2, p->items[idx], MOUI_BLACK);
            }
            my += item_h;
        }
        return;
    }

    int box_w = MOUI_DISP_W - 16;
    int box_x = 8;
    int pad = 6;
    int inner_w = box_w - pad * 2;
    int cy = pad;

    int title_h = 0;
    if (p->title) title_h = p->font->line_height + 4;
    int msg_h = p->message ? (p->font->line_height * 3 + 4) : 0;
    int btn_h = (p->type == MOUI_POPUP_CONFIRM) ? (p->font->line_height + 12) : (p->font->line_height + 8);
    int box_h = pad + title_h + msg_h + btn_h + pad;
    int box_y = (MOUI_DISP_H - box_h) / 2;

    draw_box(ctx, box_x, box_y, box_w, box_h);

    cy = box_y + pad;

    if (p->title) {
        moui_draw_fill_rect(ctx, &(moui_rect_t){box_x + 2, cy, box_w - 4, p->font->line_height + 2}, MOUI_BLACK);
        moui_font_draw_str(ctx, p->font, box_x + pad, cy + 1, p->title, MOUI_WHITE);
        cy += title_h;
    }

    if (p->message) {
        moui_font_draw_str_wrapped(ctx, p->font, box_x + pad, cy, inner_w, p->message, MOUI_BLACK);
        cy += msg_h;
    }

    if (p->type == MOUI_POPUP_ALERT) {
        int bw = 50, bh = p->font->line_height + 6;
        int bx = box_x + (box_w - bw) / 2;
        moui_draw_fill_rounded_rect(ctx, &(moui_rect_t){bx, cy, bw, bh}, 3, MOUI_BLACK);
        moui_font_draw_str(ctx, p->font, bx + (bw - moui_font_measure_str(p->font, "OK")) / 2,
                          cy + 3, "OK", MOUI_WHITE);
    }

    if (p->type == MOUI_POPUP_CONFIRM) {
        int bw = 50, bh = p->font->line_height + 6;
        int gap = 12;
        int total = bw * 2 + gap;
        int bx0 = box_x + (box_w - total) / 2;
        int bx1 = bx0 + bw + gap;

        if (p->selected == 0) {
            moui_draw_fill_rounded_rect(ctx, &(moui_rect_t){bx0, cy, bw, bh}, 3, MOUI_BLACK);
            moui_font_draw_str(ctx, p->font, bx0 + (bw - moui_font_measure_str(p->font, "No")) / 2,
                              cy + 3, "No", MOUI_WHITE);
        } else {
            moui_draw_rounded_rect(ctx, &(moui_rect_t){bx0, cy, bw, bh}, 3, MOUI_BLACK);
            moui_font_draw_str(ctx, p->font, bx0 + (bw - moui_font_measure_str(p->font, "No")) / 2,
                              cy + 3, "No", MOUI_BLACK);
        }

        if (p->selected == 1) {
            moui_draw_fill_rounded_rect(ctx, &(moui_rect_t){bx1, cy, bw, bh}, 3, MOUI_BLACK);
            moui_font_draw_str(ctx, p->font, bx1 + (bw - moui_font_measure_str(p->font, "Yes")) / 2,
                              cy + 3, "Yes", MOUI_WHITE);
        } else {
            moui_draw_rounded_rect(ctx, &(moui_rect_t){bx1, cy, bw, bh}, 3, MOUI_BLACK);
            moui_font_draw_str(ctx, p->font, bx1 + (bw - moui_font_measure_str(p->font, "Yes")) / 2,
                              cy + 3, "Yes", MOUI_BLACK);
        }
    }
}

void moui_popup_hide(moui_popup_t *p)
{
    if (!p || !p->visible) return;
    p->visible = false;
    if (p->type == MOUI_POPUP_CONFIRM && p->on_close)
        p->on_close(p, false);
    else if (p->type == MOUI_POPUP_MENU && p->on_menu_select)
        p->on_menu_select(p, -1);
}
