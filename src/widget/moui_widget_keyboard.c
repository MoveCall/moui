#include "moui_widget_keyboard.h"
#include "../core/moui_theme.h"
#include <string.h>

static const char *kb_lower[] = {
    "1","2","3","4","5","6","7","8","9","0",
    "q","w","e","r","t","y","u","i","o","p",
    "a","s","d","f","g","h","j","k","l",".",
    "\x18","z","x","c","v","b","n","m","<","OK"
};

static const char *kb_upper[] = {
    "!","@","#","$","%","^","&","*","(",")","Q","W","E","R","T","Y","U","I","O","P",
    "A","S","D","F","G","H","J","K","L",".",
    "\x19","Z","X","C","V","B","N","M","<","OK"
};

static void kb_on_click(moui_widget_btnmatrix_t *bm, uint8_t idx)
{
    moui_widget_keyboard_t *kb = (moui_widget_keyboard_t *)bm->base.user_data;
    const char **keys = kb->shift ? kb_upper : kb_lower;
    const char *key = keys[idx];

    if (strcmp(key, "\x18") == 0 || strcmp(key, "\x19") == 0) {
        kb->shift = !kb->shift;
        moui_btnmatrix_set_labels(&kb->matrix, kb->shift ? kb_upper : kb_lower, 40);
    } else if (strcmp(key, "<") == 0) {
        if (kb->out_len > 0) {
            kb->output[--kb->out_len] = '\0';
        }
    } else if (strcmp(key, "OK") == 0) {
        if (kb->on_done) kb->on_done(kb, kb->output);
    } else {
        if (kb->out_len < sizeof(kb->output) - 1) {
            kb->output[kb->out_len++] = key[0];
            kb->output[kb->out_len] = '\0';
            if (kb->on_key) kb->on_key(kb, key[0]);
        }
    }
}

static void kb_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_keyboard_t *kb = (moui_widget_keyboard_t *)w;
    moui_color_t fg = moui_theme_fg();

    /* Show current text at top */
    int ty = w->bounds.y + 2;
    moui_draw_rounded_rect(ctx, &(moui_rect_t){w->bounds.x, ty, w->bounds.w, 14}, 2, fg);
    moui_font_draw_str(ctx, kb->matrix.font, w->bounds.x + 4, ty + 2, kb->output, fg);

    /* Draw matrix below */
    kb->matrix.base.bounds = (moui_rect_t){
        w->bounds.x, w->bounds.y + 18,
        w->bounds.w, w->bounds.h - 18
    };
    moui_widget_draw(&kb->matrix.base, ctx);
}

static bool kb_on_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_widget_keyboard_t *kb = (moui_widget_keyboard_t *)w;
    return moui_widget_send_event(&kb->matrix.base, ev);
}

static void kb_on_focus(moui_widget_t *w, bool gained)
{
    moui_widget_keyboard_t *kb = (moui_widget_keyboard_t *)w;
    if (gained) kb->matrix.capturing = true;
    else kb->matrix.capturing = false;
}

static const moui_widget_vtable_t kb_vtable = {
    .draw = kb_draw, .on_event = kb_on_event, .on_focus = kb_on_focus,
};

void moui_keyboard_init(moui_widget_keyboard_t *kb, const moui_font_t *font)
{
    moui_widget_init(&kb->base, &kb_vtable);
    kb->shift = false;
    kb->output[0] = '\0';
    kb->out_len = 0;

    moui_btnmatrix_init(&kb->matrix, font, 10);
    moui_btnmatrix_set_labels(&kb->matrix, kb_lower, 40);
    kb->matrix.on_click = kb_on_click;
    kb->matrix.base.user_data = kb;
}
