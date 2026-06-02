#include "moui_timeline.h"
#include "../screen/moui_screen.h"
#include <string.h>

void moui_timeline_init(moui_timeline_t *tl)
{
    memset(tl, 0, sizeof(*tl));
}

bool moui_timeline_add(moui_timeline_t *tl, moui_widget_t *w, moui_tl_target_t target,
                       int16_t start_val, int16_t end_val, uint32_t start_ms, uint32_t duration_ms, moui_ease_fn_t ease_fn)
{
    if (!tl || !w || tl->count >= MOUI_TIMELINE_MAX_ITEMS) return false;
    moui_tl_entry_t *e = &tl->entries[tl->count++];
    e->widget = w;
    e->target = target;
    e->start_val = start_val;
    e->end_val = end_val;
    e->start_ms = start_ms;
    e->duration_ms = duration_ms > 0 ? duration_ms : 1;
    e->ease_fn = ease_fn ? ease_fn : moui_ease_linear;
    e->started = false;
    e->completed = false;
    return true;
}

void moui_timeline_start(moui_timeline_t *tl, uint32_t now_ms)
{
    if (!tl) return;
    tl->begin_timestamp_ms = now_ms;
    tl->running = true;
    for (uint8_t i = 0; i < tl->count; i++) {
        tl->entries[i].started = false;
        tl->entries[i].completed = false;
    }
}

bool moui_timeline_update(moui_timeline_t *tl, uint32_t now_ms, struct moui_screen_mgr_t *mgr)
{
    if (!tl || !tl->running) return false;

    uint32_t elapsed = now_ms - tl->begin_timestamp_ms;
    bool all_done = true;

    for (uint8_t i = 0; i < tl->count; i++) {
        moui_tl_entry_t *e = &tl->entries[i];

        if (elapsed < e->start_ms) {
            all_done = false;
            continue;
        }

        uint32_t item_elapsed = elapsed - e->start_ms;
        float progress = (float)item_elapsed / (float)e->duration_ms;
        if (progress >= 1.0f) {
            progress = 1.0f;
            e->completed = true;
        } else {
            all_done = false;
        }

        float factor = e->ease_fn(progress);
        int16_t curr_val = (int16_t)(e->start_val + (e->end_val - e->start_val) * factor);

        switch (e->target) {
        case MOUI_TL_TARGET_X: e->widget->bounds.x = curr_val; break;
        case MOUI_TL_TARGET_Y: e->widget->bounds.y = curr_val; break;
        case MOUI_TL_TARGET_W: e->widget->bounds.w = curr_val; break;
        case MOUI_TL_TARGET_H: e->widget->bounds.h = curr_val; break;
        }

        if (mgr) {
            moui_screen_mgr_mark_dirty_rect((moui_screen_mgr_t *)mgr, &e->widget->bounds);
        }
    }

    if (all_done) {
        if (tl->loop) {
            moui_timeline_start(tl, now_ms);
        } else {
            tl->running = false;
        }
    }

    return tl->running;
}
