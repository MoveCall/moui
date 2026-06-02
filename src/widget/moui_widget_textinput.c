#include "moui_widget_textinput.h"
#include "../core/moui_theme.h"
#include <string.h>
#include "../core/moui_theme.h"

static const char default_charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .-_@#!?";

static int charset_index(const char *cs, char c)
{
    for (int i = 0; cs[i]; i++) {
        if (cs[i] == c) return i;
    }
    return 0;
}

static void textinput_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_textinput_t *ti = (moui_widget_textinput_t *)w;
    if (!ti->font) return;

    uint8_t rad = moui_style_radius(w->style);

    if (moui_widget_has_state(w, MOUI_STATE_FOCUSED)) {
        moui_draw_rounded_rect(ctx, &w->bounds, rad, moui_theme_fg());
        moui_draw_rounded_rect(ctx, &(moui_rect_t){
            w->bounds.x + 1, w->bounds.y + 1,
            w->bounds.w - 2, w->bounds.h - 2}, rad > 0 ? rad - 1 : 0, moui_theme_fg());
    } else {
        moui_draw_rounded_rect(ctx, &w->bounds, rad, moui_theme_fg());
    }

    int tx = w->bounds.x + 4;
    int ty = w->bounds.y + (w->bounds.h - ti->font->line_height) / 2;

    for (int i = 0; i <= ti->len && i <= MOUI_TEXTINPUT_MAX_LEN; i++) {
        char buf[2] = {0, 0};
        if (i < ti->len) {
            buf[0] = ti->text[i];
        } else if (ti->editing && i == ti->cursor) {
            buf[0] = ti->char_select ? ti->current_char : '_';
        } else {
            break;
        }

        int cw = moui_font_measure_str(ti->font, buf);

        if (ti->editing && i == ti->cursor) {
            moui_draw_fill_rect(ctx, &(moui_rect_t){tx, ty - 1, cw + 1, ti->font->line_height + 2}, moui_theme_fg());
            moui_font_draw_str(ctx, ti->font, tx, ty, buf, moui_theme_bg());
        } else {
            moui_font_draw_str(ctx, ti->font, tx, ty, buf, moui_theme_fg());
        }
        tx += cw;
    }
}

static bool textinput_on_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_widget_textinput_t *ti = (moui_widget_textinput_t *)w;
    const char *cs = ti->charset ? ti->charset : default_charset;
    int cs_len = (int)strlen(cs);
    if (cs_len == 0) return false;

    if (!ti->editing) {
        if (ev->type == MOUI_EV_ENCODER_PRESS) {
            ti->editing = true;
            ti->char_select = true;
            ti->cursor = ti->len;
            ti->current_char = cs[0];
            return true;
        }
        return false;
    }

    if (ti->char_select) {
        switch (ev->type) {
        case MOUI_EV_ENCODER_CW: {
            int idx = charset_index(cs, ti->current_char);
            ti->current_char = cs[(idx + 1) % cs_len];
            return true;
        }
        case MOUI_EV_ENCODER_CCW: {
            int idx = charset_index(cs, ti->current_char);
            ti->current_char = cs[(idx - 1 + cs_len) % cs_len];
            return true;
        }
        case MOUI_EV_ENCODER_PRESS:
            if (ti->cursor < MOUI_TEXTINPUT_MAX_LEN) {
                ti->text[ti->cursor] = ti->current_char;
                ti->cursor++;
                ti->len = ti->cursor;
                ti->text[ti->len] = '\0';
                ti->current_char = cs[0];
            }
            return true;
        case MOUI_EV_ENCODER_BACK:
            ti->char_select = false;
            ti->editing = false;
            return true;
        default:
            return false;
        }
    }

    return false;
}

static void textinput_on_focus(moui_widget_t *w, bool gained)
{
    moui_widget_textinput_t *ti = (moui_widget_textinput_t *)w;
    if (!gained) {
        ti->editing = false;
        ti->char_select = false;
    }
}

static const moui_widget_vtable_t textinput_vtable = {
    .draw     = textinput_draw,
    .on_event = textinput_on_event,
    .on_focus = textinput_on_focus,
};

void moui_textinput_init(moui_widget_textinput_t *ti, const moui_font_t *font)
{
    moui_widget_init(&ti->base, &textinput_vtable);
    ti->font = font;
    ti->text[0] = '\0';
    ti->cursor = 0;
    ti->len = 0;
    ti->editing = false;
    ti->char_select = false;
    ti->current_char = 'A';
    ti->charset = NULL;
}

void moui_textinput_set_text(moui_widget_textinput_t *ti, const char *text)
{
    strncpy(ti->text, text, MOUI_TEXTINPUT_MAX_LEN);
    ti->text[MOUI_TEXTINPUT_MAX_LEN] = '\0';
    ti->len = (uint8_t)strlen(ti->text);
    ti->cursor = ti->len;
}
