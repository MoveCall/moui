#ifndef MOUI_WIDGET_H
#define MOUI_WIDGET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../core/moui_draw.h"
#include "../core/moui_style.h"
#include "../input/moui_input.h"

typedef struct moui_widget moui_widget_t;

typedef enum {
    MOUI_STATE_DEFAULT  = 0x00,
    MOUI_STATE_FOCUSED  = 0x01,
    MOUI_STATE_PRESSED  = 0x02,
    MOUI_STATE_CHECKED  = 0x04,
    MOUI_STATE_DISABLED = 0x08,
} moui_widget_state_t;

typedef struct {
    void        (*draw)(moui_widget_t *w, moui_draw_ctx_t *ctx);
    bool        (*on_event)(moui_widget_t *w, const moui_input_event_t *ev);
    void        (*on_focus)(moui_widget_t *w, bool gained);
    moui_point_t (*measure)(const moui_widget_t *w);
} moui_widget_vtable_t;

struct moui_widget {
    const moui_widget_vtable_t *vtable;

    moui_rect_t  bounds;
    int16_t     w_hint, h_hint;

    uint8_t visible  : 1;
    uint8_t enabled  : 1;
    uint8_t dirty    : 1;
    uint8_t _pad     : 5;

    uint8_t state;

    moui_widget_t *focus_next;
    moui_widget_t *focus_prev;
    moui_widget_t *parent;

    float anim_val[4];
    float anim_tgt[4];

    const moui_style_t *style;
    uint16_t           event_id;

    void *user_data;
};

static inline void moui_widget_set_state(moui_widget_t *w, uint8_t flag)   { w->state |= flag; }
static inline void moui_widget_clear_state(moui_widget_t *w, uint8_t flag) { w->state &= ~flag; }
static inline bool moui_widget_has_state(const moui_widget_t *w, uint8_t flag) { return (w->state & flag) != 0; }

void moui_widget_init(moui_widget_t *w, const moui_widget_vtable_t *vtable);
void moui_widget_draw(moui_widget_t *w, moui_draw_ctx_t *ctx);
bool moui_widget_send_event(moui_widget_t *w, const moui_input_event_t *ev);


#ifdef __cplusplus
}
#endif
#endif
