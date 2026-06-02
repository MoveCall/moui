#include "moui_input.h"
#include <string.h>

#if defined(__GNUC__) || defined(__clang__)
#define MOUI_MEMORY_BARRIER()     __atomic_thread_fence(__ATOMIC_RELEASE)
#define MOUI_MEMORY_BARRIER_ACQ() __atomic_thread_fence(__ATOMIC_ACQUIRE)
#else
#define MOUI_MEMORY_BARRIER()
#define MOUI_MEMORY_BARRIER_ACQ()
#endif

void moui_input_queue_init(moui_input_queue_t *q)
{
    memset(q, 0, sizeof(*q));
    MOUI_LOCK_INIT(q->lock);
}

bool moui_input_queue_push(moui_input_queue_t *q, const moui_input_event_t *ev)
{
    MOUI_LOCK_ENTER(q->lock);
    uint8_t next = (q->head + 1) % MOUI_INPUT_QUEUE_SIZE;
    if (next == q->tail) {
        MOUI_LOCK_EXIT(q->lock);
        return false;
    }
    q->buf[q->head] = *ev;
    MOUI_MEMORY_BARRIER();
    q->head = next;
    MOUI_LOCK_EXIT(q->lock);
    return true;
}

bool moui_input_queue_pop(moui_input_queue_t *q, moui_input_event_t *ev)
{
    MOUI_LOCK_ENTER(q->lock);
    if (q->head == q->tail) {
        MOUI_LOCK_EXIT(q->lock);
        return false;
    }
    MOUI_MEMORY_BARRIER_ACQ();
    *ev = q->buf[q->tail];
    q->tail = (q->tail + 1) % MOUI_INPUT_QUEUE_SIZE;
    MOUI_LOCK_EXIT(q->lock);
    return true;
}

bool moui_input_queue_empty(moui_input_queue_t *q)
{
    bool e;
    MOUI_LOCK_ENTER(q->lock);
    e = q->head == q->tail;
    MOUI_LOCK_EXIT(q->lock);
    return e;
}
