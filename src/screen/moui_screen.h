#ifndef MOUI_SCREEN_H
#define MOUI_SCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../moui_conf.h"
#include "../widget/moui_widget.h"
#include "../input/moui_focus.h"
#include "../input/moui_input.h"
#include "../anim/moui_anim.h"
#include "../core/moui_fb.h"
#include "../backend/moui_backend.h"
#include "../hal/moui_hal.h"
#include "moui_popup.h"

typedef enum {
    MOUI_TRANS_NONE,
    MOUI_TRANS_FADE,
    MOUI_TRANS_SLIDE_LEFT,
    MOUI_TRANS_SLIDE_RIGHT,
    MOUI_TRANS_SLIDE_UP,
    MOUI_TRANS_SLIDE_DOWN,
    MOUI_TRANS_WIPE,
} moui_trans_type_t;

typedef struct moui_screen moui_screen_t;

struct moui_screen {
    moui_widget_t       *widgets[MOUI_SCREEN_MAX_WIDGETS];
    uint8_t             widget_count;
    moui_focus_group_t   focus;

    void (*on_enter)(moui_screen_t *s);
    void (*on_leave)(moui_screen_t *s);
    void (*on_tick)(moui_screen_t *s, uint32_t now_ms, float dt);
    bool (*on_event)(moui_screen_t *s, const moui_input_event_t *ev);

    moui_trans_type_t    enter_trans;
    void               *user_data;
};

typedef enum {
    MOUI_REFRESH_FULL,
    MOUI_REFRESH_SMART,
} moui_refresh_mode_t;

typedef struct {
    moui_screen_t       *stack[MOUI_SCREEN_STACK_SIZE];
    uint8_t             depth;
    moui_anim_mgr_t      anim;
    moui_input_queue_t   input;
    moui_fb_t           *fb;
    moui_backend_t      *be;
    const moui_hal_t    *hal;
    moui_draw_ctx_t      ctx;

    struct {
        bool            active;
        moui_trans_type_t type;
        float           progress;
        uint32_t        start_ms;
        uint32_t        duration_ms;
    } trans;

    moui_popup_t         popup;
    moui_widget_t       *pointer_capture;
    moui_refresh_mode_t  refresh_mode;
    volatile bool       needs_redraw;
    moui_rect_t          dirty_rect;
} moui_screen_mgr_t;

void moui_screen_init(moui_screen_t *s);
void moui_screen_add_widget(moui_screen_t *s, moui_widget_t *w);

void moui_screen_mgr_init(moui_screen_mgr_t *mgr, moui_fb_t *fb, const moui_hal_t *hal);
void moui_screen_mgr_init_be(moui_screen_mgr_t *mgr, moui_backend_t *be, const moui_hal_t *hal);
void moui_screen_push(moui_screen_mgr_t *mgr, moui_screen_t *s);
void moui_screen_push_with(moui_screen_mgr_t *mgr, moui_screen_t *s, void *params);
void moui_screen_replace(moui_screen_mgr_t *mgr, moui_screen_t *s);
void moui_screen_pop(moui_screen_mgr_t *mgr);
moui_screen_t *moui_screen_active(moui_screen_mgr_t *mgr);
void moui_screen_mgr_tick(moui_screen_mgr_t *mgr, uint32_t now_ms, float dt);

moui_input_queue_t *moui_screen_mgr_input(moui_screen_mgr_t *mgr);
void moui_screen_mgr_set_refresh(moui_screen_mgr_t *mgr, moui_refresh_mode_t mode);
void moui_screen_mgr_mark_dirty(moui_screen_mgr_t *mgr);
void moui_screen_mgr_mark_dirty_rect(moui_screen_mgr_t *mgr, const moui_rect_t *r);
void moui_screen_mgr_render_once(moui_screen_mgr_t *mgr, uint32_t now_ms);

typedef void (*moui_canvas_draw_fn)(moui_draw_ctx_t *ctx, void *user_data);
moui_screen_t *moui_screen_canvas_create(moui_canvas_draw_fn draw_fn, void *user_data);
void moui_screen_canvas_reset(void);


#ifdef __cplusplus
}
#endif
#endif
