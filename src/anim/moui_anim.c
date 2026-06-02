#include "moui_anim.h"
#include <string.h>

void moui_anim_mgr_init(moui_anim_mgr_t *mgr)
{
    memset(mgr, 0, sizeof(*mgr));
}

moui_anim_t *moui_anim_start(moui_anim_mgr_t *mgr,
                            float *prop,
                            float to_val,
                            uint32_t duration_ms,
                            moui_ease_fn_t ease_fn)
{
    for (int i = 0; i < MOUI_ANIM_POOL_SIZE; i++) {
        if (mgr->pool[i].active && mgr->pool[i].target_prop == prop) {
            moui_anim_t *a = &mgr->pool[i];
            a->from_val    = *prop;
            a->to_val      = to_val;
            a->duration_ms = duration_ms;
            a->start_ms    = UINT32_MAX;
            a->ease_fn     = ease_fn;
            a->on_complete = NULL;
            a->user_data   = NULL;
            return a;
        }
    }

    for (int i = 0; i < MOUI_ANIM_POOL_SIZE; i++) {
        if (!mgr->pool[i].active) {
            moui_anim_t *a = &mgr->pool[i];
            a->target_prop = prop;
            a->from_val    = *prop;
            a->to_val      = to_val;
            a->duration_ms = duration_ms;
            a->start_ms    = UINT32_MAX;
            a->ease_fn     = ease_fn;
            a->active      = true;
            a->on_complete = NULL;
            a->user_data   = NULL;
            return a;
        }
    }
    return NULL;
}

void moui_anim_cancel(moui_anim_mgr_t *mgr, float *prop)
{
    for (int i = 0; i < MOUI_ANIM_POOL_SIZE; i++) {
        moui_anim_t *a = &mgr->pool[i];
        if (a->active && a->target_prop == prop) {
            a->active = false;
            /* Keep group/sequence completion accounting intact: report the
             * cancelled member as done so on_done still fires. */
            if (a->on_complete)
                a->on_complete(a, a->user_data);
            a->on_complete = NULL;
        }
    }
}

void moui_anim_tick(moui_anim_mgr_t *mgr, uint32_t now_ms)
{
    for (int i = 0; i < MOUI_ANIM_POOL_SIZE; i++) {
        moui_anim_t *a = &mgr->pool[i];
        if (!a->active) continue;

        if (a->start_ms == UINT32_MAX) {
            a->start_ms = now_ms;
            a->from_val = *a->target_prop;
        }

        uint32_t elapsed = now_ms - a->start_ms;
        if (elapsed >= a->duration_ms) {
            *a->target_prop = a->to_val;
            a->active = false;
            if (a->on_complete)
                a->on_complete(a, a->user_data);
        } else {
            float t = (float)elapsed / (float)a->duration_ms;
            float eased = a->ease_fn ? a->ease_fn(t) : t;
            *a->target_prop = a->from_val + (a->to_val - a->from_val) * eased;
        }
    }
}

bool moui_anim_any_active(const moui_anim_mgr_t *mgr)
{
    for (int i = 0; i < MOUI_ANIM_POOL_SIZE; i++) {
        if (mgr->pool[i].active) return true;
    }
    return false;
}

/* ── Sequence ── */

static void seq_step_complete(moui_anim_t *a, void *user)
{
    (void)a;
    moui_anim_seq_t *seq = (moui_anim_seq_t *)user;
    seq->current++;
    /* If the pool is exhausted the sequence cannot proceed; skip the stuck
     * step and try the next one so on_done still fires. */
    int guard = 8;
    while (seq->current < seq->count) {
        int i = seq->current;
        moui_anim_t *next = moui_anim_start(seq->mgr, seq->props[i],
            seq->to_vals[i], seq->durations[i], seq->eases[i]);
        if (next) {
            next->on_complete = seq_step_complete;
            next->user_data = seq;
            return;
        }
        seq->current++;
        if (--guard <= 0) break;
    }
    if (seq->current >= seq->count) {
        if (seq->on_done) seq->on_done(seq->user_data);
    }
}

void moui_anim_seq_init(moui_anim_seq_t *seq, moui_anim_mgr_t *mgr)
{
    memset(seq, 0, sizeof(*seq));
    seq->mgr = mgr;
}

void moui_anim_seq_add(moui_anim_seq_t *seq, float *prop, float to,
                      uint32_t duration_ms, moui_ease_fn_t ease)
{
    if (seq->count >= 8) return;
    int i = seq->count++;
    seq->props[i] = prop;
    seq->to_vals[i] = to;
    seq->durations[i] = duration_ms;
    seq->eases[i] = ease;
}

void moui_anim_seq_start(moui_anim_seq_t *seq)
{
    if (seq->count == 0) return;
    seq->current = 0;
    moui_anim_t *a = moui_anim_start(seq->mgr, seq->props[0],
        seq->to_vals[0], seq->durations[0], seq->eases[0]);
    if (a) {
        a->on_complete = seq_step_complete;
        a->user_data = seq;
    }
}

/* ── Parallel group ── */

static void group_member_complete(moui_anim_t *a, void *user)
{
    moui_anim_group_t *grp = (moui_anim_group_t *)user;
    if (a->group_gen != grp->generation) return;
    grp->completed++;
    if (grp->completed >= grp->total) {
        if (grp->on_done) grp->on_done(grp->user_data);
    }
}

void moui_anim_group_init(moui_anim_group_t *grp, uint8_t count,
                         void (*on_done)(void *), void *user)
{
    grp->generation++;
    grp->total = count;
    grp->completed = 0;
    grp->on_done = on_done;
    grp->user_data = user;
}

void moui_anim_group_track(moui_anim_group_t *grp, moui_anim_t *anim)
{
    if (!anim) return;
    anim->on_complete = group_member_complete;
    anim->user_data = grp;
    anim->group_gen = grp->generation;
}
