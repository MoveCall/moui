#ifndef MOUI_INPUT_H
#define MOUI_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../hal/moui_hal_types.h"
#include "../moui_conf.h"

typedef enum {
    MOUI_EV_NONE = 0,
    MOUI_EV_ENCODER_CW,
    MOUI_EV_ENCODER_CCW,
    MOUI_EV_ENCODER_PRESS,
    MOUI_EV_ENCODER_LONG,
    MOUI_EV_ENCODER_BACK,

    MOUI_EV_POINTER_DOWN = 16,
    MOUI_EV_POINTER_UP,
    MOUI_EV_POINTER_MOVE,
    MOUI_EV_GESTURE_TAP,
    MOUI_EV_GESTURE_LONG,
    MOUI_EV_GESTURE_SWIPE_UP,
    MOUI_EV_GESTURE_SWIPE_DOWN,
    MOUI_EV_GESTURE_SWIPE_LEFT,
    MOUI_EV_GESTURE_SWIPE_RIGHT,

    MOUI_EV_USER_BASE = 64,
} moui_event_type_t;

typedef struct {
    moui_event_type_t type;
    uint32_t         timestamp_ms;
    int16_t          x, y;
} moui_input_event_t;

typedef struct {
    moui_input_event_t buf[MOUI_INPUT_QUEUE_SIZE];
    volatile uint8_t  head;
    volatile uint8_t  tail;
    MOUI_LOCK_T       lock;
} moui_input_queue_t;

void moui_input_queue_init(moui_input_queue_t *q);
bool moui_input_queue_push(moui_input_queue_t *q, const moui_input_event_t *ev);
bool moui_input_queue_pop(moui_input_queue_t *q, moui_input_event_t *ev);
bool moui_input_queue_empty(moui_input_queue_t *q);


#ifdef __cplusplus
}
#endif
#endif
