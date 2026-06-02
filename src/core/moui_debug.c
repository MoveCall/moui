#include "moui_debug.h"
#include <string.h>

void moui_debug_init(moui_debug_ctx_t *dbg)
{
    memset(dbg, 0, sizeof(*dbg));
}

void moui_debug_tick(moui_debug_ctx_t *dbg, uint32_t now_ms)
{
    dbg->frame_count++;
    if (now_ms - dbg->last_fps_time >= 1000) {
        dbg->fps = (float)dbg->frame_count * 1000.0f / (float)(now_ms - dbg->last_fps_time);
        dbg->frame_count = 0;
        dbg->last_fps_time = now_ms;
    }
}

void moui_debug_dump(const moui_debug_ctx_t *dbg, const moui_screen_mgr_t *mgr)
{
    (void)dbg; (void)mgr;
    MOUI_LOG("=== MOUI Debug Dump ===");
    MOUI_LOG("FPS: %.1f", dbg->fps);
    MOUI_LOG("Screen stack: %d/%d", mgr->depth, MOUI_SCREEN_STACK_SIZE);

    moui_screen_t *scr = mgr->depth > 0 ? mgr->stack[mgr->depth - 1] : NULL;
    if (scr) {
        MOUI_LOG("Active screen: %d widgets", scr->widget_count);
        for (int i = 0; i < scr->widget_count; i++) {
            MOUI_LOG("  [%d] bounds=(%d,%d,%d,%d) state=0x%02X vis=%d en=%d",
                    i, scr->widgets[i]->bounds.x, scr->widgets[i]->bounds.y,
                    scr->widgets[i]->bounds.w, scr->widgets[i]->bounds.h,
                    scr->widgets[i]->state, scr->widgets[i]->visible,
                    scr->widgets[i]->enabled);
        }
    }

    int anim_active = 0;
    for (int i = 0; i < MOUI_ANIM_POOL_SIZE; i++) {
        if (mgr->anim.pool[i].active) anim_active++;
    }
    MOUI_LOG("Anim pool: %d/%d active", anim_active, MOUI_ANIM_POOL_SIZE);
    (void)anim_active;

    MOUI_LOG("Input queue: head=%d tail=%d", mgr->input.head, mgr->input.tail);
    MOUI_LOG("Refresh mode: %s", mgr->refresh_mode == MOUI_REFRESH_SMART ? "SMART" : "FULL");
    MOUI_LOG("Transition: %s", mgr->trans.active ? "active" : "idle");

    MOUI_LOG("RAM estimate:");
    MOUI_LOG("  screen_mgr: %zu B", sizeof(moui_screen_mgr_t));
    MOUI_LOG("  per screen: %zu B", sizeof(moui_screen_t));
    MOUI_LOG("  per widget: %zu B", sizeof(moui_widget_t));
    MOUI_LOG("======================");
}
