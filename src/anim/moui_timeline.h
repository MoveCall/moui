#ifndef MOUI_TIMELINE_H
#define MOUI_TIMELINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../widget/moui_widget.h"
#include "moui_ease.h"

struct moui_screen_mgr_t;

#define MOUI_TIMELINE_MAX_ITEMS 16

typedef enum {
    MOUI_TL_TARGET_X = 0,
    MOUI_TL_TARGET_Y,
    MOUI_TL_TARGET_W,
    MOUI_TL_TARGET_H,
} moui_tl_target_t;

typedef struct {
    moui_widget_t   *widget;
    moui_tl_target_t target;
    int16_t          start_val;
    int16_t          end_val;
    uint32_t         start_ms;
    uint32_t         duration_ms;
    moui_ease_fn_t   ease_fn;
    bool             started;
    bool             completed;
} moui_tl_entry_t;

typedef struct {
    moui_tl_entry_t entries[MOUI_TIMELINE_MAX_ITEMS];
    uint8_t         count;
    uint32_t        begin_timestamp_ms;
    bool            running;
    bool            loop;
} moui_timeline_t;

void moui_timeline_init(moui_timeline_t *tl);
bool moui_timeline_add(moui_timeline_t *tl, moui_widget_t *w, moui_tl_target_t target,
                       int16_t start_val, int16_t end_val, uint32_t start_ms, uint32_t duration_ms, moui_ease_fn_t ease_fn);
void moui_timeline_start(moui_timeline_t *tl, uint32_t now_ms);
bool moui_timeline_update(moui_timeline_t *tl, uint32_t now_ms, struct moui_screen_mgr_t *mgr);

#ifdef __cplusplus
}
#endif
#endif
