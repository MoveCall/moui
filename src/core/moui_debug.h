#ifndef MOUI_DEBUG_H
#define MOUI_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../screen/moui_screen.h"
#include "../moui_log.h"

typedef struct {
    float    fps;
    uint32_t frame_count;
    uint32_t last_fps_time;
} moui_debug_ctx_t;

void moui_debug_init(moui_debug_ctx_t *dbg);
void moui_debug_tick(moui_debug_ctx_t *dbg, uint32_t now_ms);
void moui_debug_dump(const moui_debug_ctx_t *dbg, const moui_screen_mgr_t *mgr);

#ifdef __cplusplus
}
#endif

#endif
