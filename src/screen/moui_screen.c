#include "moui_screen.h"
#include "../anim/moui_ease.h"
#include "../font/moui_font.h"
#include "../core/moui_theme.h"
#include "../core/moui_event_bus.h"
#include <string.h>

void moui_screen_init(moui_screen_t *s)
{
    memset(s, 0, sizeof(*s));
    moui_focus_group_init(&s->focus);
    s->enter_trans = MOUI_TRANS_NONE;
}

void moui_screen_add_widget(moui_screen_t *s, moui_widget_t *w)
{
    if (!w || s->widget_count >= MOUI_SCREEN_MAX_WIDGETS) return;
    s->widgets[s->widget_count++] = w;
    if (w->vtable && w->vtable->on_event)
        moui_focus_group_add(&s->focus, w);
}

void moui_screen_mgr_init(moui_screen_mgr_t *mgr, moui_fb_t *fb, const moui_hal_t *hal)
{
    memset(mgr, 0, sizeof(*mgr));
    mgr->fb  = fb;
    mgr->hal = hal;
    moui_anim_mgr_init(&mgr->anim);
    moui_input_queue_init(&mgr->input);
    moui_draw_ctx_init(&mgr->ctx, fb);
    moui_popup_init(&mgr->popup, &moui_font_ascii_6x8);
    moui_theme_set_default();
    moui_event_bus_init(&moui_bus);
}

static moui_disp_desc_t s_be_disp_desc;

void moui_screen_mgr_init_be(moui_screen_mgr_t *mgr, moui_backend_t *be, const moui_hal_t *hal)
{
    memset(mgr, 0, sizeof(*mgr));
    mgr->be  = be;
    mgr->hal = hal;

    if (!moui_disp) {
        s_be_disp_desc.width = (uint16_t)be->width;
        s_be_disp_desc.height = (uint16_t)be->height;
        s_be_disp_desc.pixel_format = MOUI_PIXEL_FORMAT_MONO_HMSB;
        moui_hal_set_display(&s_be_disp_desc);
    }

    moui_anim_mgr_init(&mgr->anim);
    moui_input_queue_init(&mgr->input);
    moui_draw_ctx_init_be(&mgr->ctx, be);
    moui_popup_init(&mgr->popup, &moui_font_ascii_6x8);
    moui_theme_set_default();
    moui_event_bus_init(&moui_bus);
}

moui_screen_t *moui_screen_active(moui_screen_mgr_t *mgr)
{
    if (mgr->depth == 0) return NULL;
    return mgr->stack[mgr->depth - 1];
}

void moui_screen_push(moui_screen_mgr_t *mgr, moui_screen_t *s)
{
    if (!s || mgr->depth >= MOUI_SCREEN_STACK_SIZE) return;

    moui_screen_t *old = moui_screen_active(mgr);
    if (old && old->on_leave) old->on_leave(old);

    mgr->stack[mgr->depth++] = s;

    if (s->enter_trans != MOUI_TRANS_NONE) {
        mgr->trans.active      = true;
        mgr->trans.type        = s->enter_trans;
        mgr->trans.progress    = 0.0f;
        mgr->trans.start_ms    = UINT32_MAX;
        mgr->trans.duration_ms = 300;
    }

    if (s->on_enter) s->on_enter(s);
    moui_focus_activate_first(&s->focus);
    moui_screen_mgr_mark_dirty(mgr);
}

void moui_screen_push_with(moui_screen_mgr_t *mgr, moui_screen_t *s, void *params)
{
    s->user_data = params;
    moui_screen_push(mgr, s);
}

void moui_screen_replace(moui_screen_mgr_t *mgr, moui_screen_t *s)
{
    if (!s) return;
    if (mgr->depth == 0) {
        moui_screen_push(mgr, s);
        return;
    }

    moui_screen_t *old = mgr->stack[mgr->depth - 1];
    if (old && old->on_leave) old->on_leave(old);

    mgr->stack[mgr->depth - 1] = s;
    mgr->pointer_capture = NULL;

    if (s->enter_trans != MOUI_TRANS_NONE) {
        mgr->trans.active      = true;
        mgr->trans.type        = s->enter_trans;
        mgr->trans.progress    = 0.0f;
        mgr->trans.start_ms    = UINT32_MAX;
        mgr->trans.duration_ms = 300;
    }

    moui_focus_activate_first(&s->focus);
    if (s->on_enter) s->on_enter(s);
    moui_screen_mgr_mark_dirty(mgr);
}

void moui_screen_pop(moui_screen_mgr_t *mgr)
{
    if (mgr->depth <= 1) return;

    moui_screen_t *old = mgr->stack[--mgr->depth];
    if (old->on_leave) old->on_leave(old);
    mgr->pointer_capture = NULL;
    moui_popup_hide(&mgr->popup);

    moui_screen_t *now = moui_screen_active(mgr);
    if (now) {
        moui_focus_activate_first(&now->focus);
        if (now->on_enter) now->on_enter(now);
    }
    moui_screen_mgr_mark_dirty(mgr);
}

moui_input_queue_t *moui_screen_mgr_input(moui_screen_mgr_t *mgr)
{
    return &mgr->input;
}

void moui_screen_mgr_set_refresh(moui_screen_mgr_t *mgr, moui_refresh_mode_t mode)
{
    mgr->refresh_mode = mode;
}

void moui_screen_mgr_mark_dirty(moui_screen_mgr_t *mgr)
{
    mgr->needs_redraw = true;
    int w = mgr->be ? mgr->be->width : MOUI_DISP_W;
    int h = mgr->be ? mgr->be->height : MOUI_DISP_H;
    mgr->dirty_rect = (moui_rect_t){0, 0, (int16_t)w, (int16_t)h};
}

void moui_screen_mgr_mark_dirty_rect(moui_screen_mgr_t *mgr, const moui_rect_t *r)
{
    if (!mgr || !r) return;
    int disp_w = mgr->be ? mgr->be->width : MOUI_DISP_W;
    int disp_h = mgr->be ? mgr->be->height : MOUI_DISP_H;

    if (!mgr->needs_redraw) {
        int16_t x0 = r->x < 0 ? 0 : r->x;
        int16_t y0 = r->y < 0 ? 0 : r->y;
        int16_t x1 = r->x + r->w > disp_w ? disp_w : r->x + r->w;
        int16_t y1 = r->y + r->h > disp_h ? disp_h : r->y + r->h;
        if (x1 <= x0 || y1 <= y0) return;
        mgr->dirty_rect = (moui_rect_t){x0, y0, x1 - x0, y1 - y0};
        mgr->needs_redraw = true;
    } else {
        int16_t x0 = mgr->dirty_rect.x < r->x ? mgr->dirty_rect.x : r->x;
        int16_t y0 = mgr->dirty_rect.y < r->y ? mgr->dirty_rect.y : r->y;
        int16_t x1_a = mgr->dirty_rect.x + mgr->dirty_rect.w;
        int16_t x1_b = r->x + r->w;
        int16_t y1_a = mgr->dirty_rect.y + mgr->dirty_rect.h;
        int16_t y1_b = r->y + r->h;
        int16_t x1 = x1_a > x1_b ? x1_a : x1_b;
        int16_t y1 = y1_a > y1_b ? y1_a : y1_b;
        if (x0 < 0) x0 = 0;
        if (y0 < 0) y0 = 0;
        if (x1 > disp_w) x1 = disp_w;
        if (y1 > disp_h) y1 = disp_h;
        if (x1 > x0 && y1 > y0)
            mgr->dirty_rect = (moui_rect_t){x0, y0, x1 - x0, y1 - y0};
        else
            mgr->dirty_rect = (moui_rect_t){0, 0, (int16_t)disp_w, (int16_t)disp_h};
    }
}

static void step_widget_anims(moui_widget_t *w, float factor)
{
    for (int j = 0; j < 4; j++) {
        w->anim_val[j] = moui_ease_asymptotic(w->anim_val[j], w->anim_tgt[j], factor);
    }
}

void moui_screen_mgr_tick(moui_screen_mgr_t *mgr, uint32_t now_ms, float dt)
{
    moui_screen_t *scr = moui_screen_active(mgr);
    if (!scr) return;

    bool had_input = false;
    moui_input_event_t ev;
    while (moui_input_queue_pop(&mgr->input, &ev)) {
        had_input = true;
        if (moui_popup_handle_event(&mgr->popup, &ev))
            continue;

        scr = moui_screen_active(mgr);
        if (!scr) break;

        bool consumed = false;

        if (ev.type >= MOUI_EV_POINTER_DOWN && ev.type <= MOUI_EV_POINTER_MOVE) {
            moui_widget_t *target = NULL;

            if (ev.type == MOUI_EV_POINTER_DOWN) {
                for (int i = scr->widget_count - 1; i >= 0; i--) {
                    moui_widget_t *w = scr->widgets[i];
                    if (w->visible && w->enabled &&
                        moui_rect_contains(&w->bounds, ev.x, ev.y)) {
                        target = w;
                        break;
                    }
                }
                mgr->pointer_capture = target;
                if (target && target->vtable && target->vtable->on_event)
                    moui_focus_set(&scr->focus, target);
            } else {
                target = mgr->pointer_capture;
                if (ev.type == MOUI_EV_POINTER_UP)
                    mgr->pointer_capture = NULL;
            }

            if (target && target->vtable && target->vtable->on_event)
                consumed = target->vtable->on_event(target, &ev);
            if (!consumed && scr->on_event)
                consumed = scr->on_event(scr, &ev);
        } else if (ev.type >= MOUI_EV_GESTURE_TAP && ev.type <= MOUI_EV_GESTURE_SWIPE_RIGHT) {
            if (ev.type == MOUI_EV_GESTURE_TAP || ev.type == MOUI_EV_GESTURE_LONG) {
                moui_widget_t *hit = NULL;
                for (int i = scr->widget_count - 1; i >= 0; i--) {
                    moui_widget_t *w = scr->widgets[i];
                    if (w->visible && w->enabled &&
                        moui_rect_contains(&w->bounds, ev.x, ev.y)) {
                        hit = w;
                        break;
                    }
                }
                if (hit && hit->vtable && hit->vtable->on_event) {
                    moui_focus_set(&scr->focus, hit);
                    consumed = hit->vtable->on_event(hit, &ev);
                }
            } else {
                consumed = moui_focus_dispatch(&scr->focus, &ev);
            }
            if (!consumed && scr->on_event)
                consumed = scr->on_event(scr, &ev);
            if (!consumed && ev.type == MOUI_EV_GESTURE_SWIPE_RIGHT && mgr->depth > 1) {
                moui_screen_pop(mgr);
                continue;
            }
        } else {
            consumed = moui_focus_dispatch(&scr->focus, &ev);

            if (!consumed && scr->on_event)
                consumed = scr->on_event(scr, &ev);

            if (!consumed && (ev.type == MOUI_EV_ENCODER_BACK || ev.type == MOUI_EV_ENCODER_LONG) && mgr->depth > 1) {
                moui_screen_pop(mgr);
                continue;
            }
        }

        scr = moui_screen_active(mgr);
        if (!scr) break;
    }

    scr = moui_screen_active(mgr);
    if (!scr) return;

    if (scr->on_tick) {
        scr->on_tick(scr, now_ms, dt);
    }

    moui_anim_tick(&mgr->anim, now_ms);

    float factor = dt / 80.0f;
    if (factor > 1.0f) factor = 1.0f;
    bool anim_moving = false;
    for (int i = 0; i < scr->widget_count; i++) {
        moui_widget_t *w = scr->widgets[i];
        for (int j = 0; j < 4; j++) {
            if (w->anim_val[j] != w->anim_tgt[j]) anim_moving = true;
        }
        step_widget_anims(w, factor);
    }

    if (had_input) moui_screen_mgr_mark_dirty(mgr);
    if (anim_moving) moui_screen_mgr_mark_dirty(mgr);
    if (moui_anim_any_active(&mgr->anim)) moui_screen_mgr_mark_dirty(mgr);

    if (mgr->trans.active) {
        if (mgr->trans.start_ms == UINT32_MAX) mgr->trans.start_ms = now_ms;
        uint32_t elapsed = now_ms - mgr->trans.start_ms;
        if (elapsed >= mgr->trans.duration_ms) {
            mgr->trans.active = false;
            mgr->trans.progress = 1.0f;
        } else {
            mgr->trans.progress = moui_ease_out_cubic(
                (float)elapsed / (float)mgr->trans.duration_ms);
        }
        moui_screen_mgr_mark_dirty(mgr);
    }

    if (mgr->refresh_mode == MOUI_REFRESH_SMART && !mgr->needs_redraw)
        return;
    mgr->needs_redraw = false;
    moui_rect_t dirty = mgr->dirty_rect;
    mgr->dirty_rect = (moui_rect_t){0, 0, 0, 0};

    int disp_w = mgr->be ? mgr->be->width : MOUI_DISP_W;
    int disp_h = mgr->be ? mgr->be->height : MOUI_DISP_H;

    bool use_dirty_clip = !mgr->trans.active
                          && dirty.w > 0 && dirty.h > 0
                          && (dirty.w < disp_w || dirty.h < disp_h);

    if (use_dirty_clip) {
        moui_draw_ctx_t *ctx = &mgr->ctx;
        moui_draw_push_clip(ctx, &dirty);
        moui_draw_fill_rect(ctx, &dirty, moui_theme_bg());
    } else {
        if (mgr->be)
            moui_be_clear(mgr->be, moui_theme_bg());
        else
            moui_fb_clear(mgr->fb, moui_theme_bg());
    }

    int offset_x = 0, offset_y = 0;

    bool fade_active = false;
    float fade_progress = 0.0f;

    if (mgr->trans.active) {
        float p = mgr->trans.progress;
        switch (mgr->trans.type) {
        case MOUI_TRANS_SLIDE_LEFT:
            offset_x = (int)((1.0f - p) * disp_w);
            break;
        case MOUI_TRANS_SLIDE_RIGHT:
            offset_x = -(int)((1.0f - p) * disp_w);
            break;
        case MOUI_TRANS_SLIDE_UP:
            offset_y = (int)((1.0f - p) * disp_h);
            break;
        case MOUI_TRANS_SLIDE_DOWN:
            offset_y = -(int)((1.0f - p) * disp_h);
            break;
        case MOUI_TRANS_FADE:
            fade_active = true;
            fade_progress = p;
            break;
        case MOUI_TRANS_WIPE:
            fade_active = true;
            fade_progress = p;
            break;
        default:
            break;
        }
    }

    moui_draw_ctx_t *ctx = &mgr->ctx;

    if (offset_x != 0 || offset_y != 0) {
        moui_rect_t offset_clip = { (int16_t)offset_x, (int16_t)offset_y,
                                   (int16_t)disp_w, (int16_t)disp_h };
        moui_draw_push_clip(ctx, &offset_clip);
    }

    for (int i = 0; i < scr->widget_count; i++) {
        moui_widget_t *w = scr->widgets[i];
        if (use_dirty_clip && !moui_rect_intersects(&w->bounds, &dirty))
            continue;
        if (offset_x != 0 || offset_y != 0) {
            moui_rect_t saved = w->bounds;
            w->bounds.x += offset_x;
            w->bounds.y += offset_y;
            moui_widget_draw(w, ctx);
            w->bounds = saved;
        } else {
            moui_widget_draw(w, ctx);
        }
    }

    if (offset_x != 0 || offset_y != 0) {
        moui_draw_pop_clip(ctx);
    }

    if (fade_active && mgr->trans.type == MOUI_TRANS_FADE) {
        for (int y = 0; y < disp_h; y++) {
            for (int x = 0; x < disp_w; x++) {
                if (((x + y) & 1) && fade_progress < 0.5f)
                    moui_draw_pixel(ctx, x, y, MOUI_WHITE);
                else if (((x ^ y) & 3) == 0 && fade_progress < 0.25f)
                    moui_draw_pixel(ctx, x, y, MOUI_WHITE);
            }
        }
    }

    if (fade_active && mgr->trans.type == MOUI_TRANS_WIPE && fade_progress > 0.01f) {
        int reveal_x = (int)(fade_progress * disp_w);
        moui_draw_fill_rect(ctx, &(moui_rect_t){(int16_t)reveal_x, 0,
            (int16_t)(disp_w - reveal_x), (int16_t)disp_h}, MOUI_WHITE);
    }

    moui_popup_draw(&mgr->popup, ctx, now_ms);

    if (use_dirty_clip)
        moui_draw_pop_clip(ctx);

    if (mgr->be) {
        moui_be_flush(mgr->be);
    } else {
        moui_fb_pack_wire(mgr->fb);
        mgr->hal->display_flush(mgr->hal, mgr->fb->wire, mgr->fb->wire_size);
    }
}

void moui_screen_mgr_render_once(moui_screen_mgr_t *mgr, uint32_t now_ms)
{
    moui_refresh_mode_t saved = mgr->refresh_mode;
    mgr->refresh_mode = MOUI_REFRESH_FULL;
    moui_screen_mgr_mark_dirty(mgr);
    moui_screen_mgr_tick(mgr, now_ms, 0.0f);
    mgr->refresh_mode = saved;
}

/* ── Canvas Screen (single draw function as a full screen) ── */

typedef struct {
    moui_screen_t       screen;
    moui_widget_t       canvas_widget;
    moui_canvas_draw_fn draw_fn;
    void              *user_data;
} moui_canvas_screen_t;

static moui_canvas_screen_t s_canvas_pool[4];
static uint8_t s_canvas_used = 0;

static void canvas_widget_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_canvas_screen_t *cs = (moui_canvas_screen_t *)w->user_data;
    if (cs && cs->draw_fn)
        cs->draw_fn(ctx, cs->user_data);
}

static const moui_widget_vtable_t canvas_widget_vtable = { .draw = canvas_widget_draw };

moui_screen_t *moui_screen_canvas_create(moui_canvas_draw_fn draw_fn, void *user_data)
{
    if (s_canvas_used >= 4) return NULL;
    moui_canvas_screen_t *cs = &s_canvas_pool[s_canvas_used++];

    moui_screen_init(&cs->screen);
    cs->draw_fn = draw_fn;
    cs->user_data = user_data;

    moui_widget_init(&cs->canvas_widget, &canvas_widget_vtable);
    cs->canvas_widget.bounds = (moui_rect_t){0, 0, MOUI_DISP_W, MOUI_DISP_H};
    cs->canvas_widget.user_data = cs;
    moui_screen_add_widget(&cs->screen, &cs->canvas_widget);

    return &cs->screen;
}

void moui_screen_canvas_reset(void)
{
    s_canvas_used = 0;
    memset(s_canvas_pool, 0, sizeof(s_canvas_pool));
}
