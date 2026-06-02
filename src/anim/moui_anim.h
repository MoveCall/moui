#ifndef MOUI_ANIM_H
#define MOUI_ANIM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_ease.h"
#include "../moui_conf.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct moui_anim moui_anim_t;

struct moui_anim {
    float          *target_prop;
    float           from_val;
    float           to_val;
    uint32_t        duration_ms;
    uint32_t        start_ms;
    moui_ease_fn_t   ease_fn;
    bool            active;
    uint8_t         group_gen;

    void (*on_complete)(moui_anim_t *anim, void *user);
    void *user_data;
};

typedef struct {
    moui_anim_t pool[MOUI_ANIM_POOL_SIZE];
} moui_anim_mgr_t;

void moui_anim_mgr_init(moui_anim_mgr_t *mgr);

moui_anim_t *moui_anim_start(moui_anim_mgr_t *mgr,
                            float *prop,
                            float to_val,
                            uint32_t duration_ms,
                            moui_ease_fn_t ease_fn);

void moui_anim_cancel(moui_anim_mgr_t *mgr, float *prop);

void moui_anim_tick(moui_anim_mgr_t *mgr, uint32_t now_ms);

bool moui_anim_any_active(const moui_anim_mgr_t *mgr);

/* Sequence: chain animations so each starts when the previous completes */
typedef struct {
    float          *props[8];
    float           to_vals[8];
    uint32_t        durations[8];
    moui_ease_fn_t   eases[8];
    uint8_t         count;
    uint8_t         current;
    moui_anim_mgr_t *mgr;
    void          (*on_done)(void *user);
    void           *user_data;
} moui_anim_seq_t;

void moui_anim_seq_init(moui_anim_seq_t *seq, moui_anim_mgr_t *mgr);
void moui_anim_seq_add(moui_anim_seq_t *seq, float *prop, float to,
                      uint32_t duration_ms, moui_ease_fn_t ease);
void moui_anim_seq_start(moui_anim_seq_t *seq);

/* Parallel group: track N animations, callback when all complete */
typedef struct {
    uint8_t total;
    uint8_t completed;
    uint8_t generation;
    void  (*on_done)(void *user);
    void   *user_data;
} moui_anim_group_t;

void moui_anim_group_init(moui_anim_group_t *grp, uint8_t count,
                         void (*on_done)(void *), void *user);
void moui_anim_group_track(moui_anim_group_t *grp, moui_anim_t *anim);


#ifdef __cplusplus
}
#endif
#endif
