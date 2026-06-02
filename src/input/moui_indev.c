#include "moui_indev.h"
#include <string.h>
#include <stdlib.h>

#define POINTER_MOVE_THRESHOLD   4
#define GESTURE_SWIPE_THRESHOLD  30
#define GESTURE_TAP_MAX_MS       300
#define GESTURE_LONG_MIN_MS      500
#define ENCODER_LONG_PRESS_MS    500
#define ENCODER_DEBOUNCE_MS      30

void moui_indev_init(moui_indev_t *dev, moui_indev_type_t type,
                    void (*read_cb)(moui_indev_t *, moui_indev_data_t *),
                    moui_input_queue_t *queue)
{
    memset(dev, 0, sizeof(*dev));
    dev->type  = type;
    dev->read  = read_cb;
    dev->queue = queue;
    dev->repeat_delay_ms  = 400;
    dev->repeat_period_ms = 100;
}

static moui_event_type_t key_to_event(moui_key_t key)
{
    switch (key) {
    case MOUI_KEY_UP:    return MOUI_EV_ENCODER_CCW;
    case MOUI_KEY_DOWN:  return MOUI_EV_ENCODER_CW;
    case MOUI_KEY_LEFT:  return MOUI_EV_ENCODER_CCW;
    case MOUI_KEY_RIGHT: return MOUI_EV_ENCODER_CW;
    case MOUI_KEY_ENTER: return MOUI_EV_ENCODER_PRESS;
    case MOUI_KEY_BACK:  return MOUI_EV_ENCODER_BACK;
    case MOUI_KEY_HOME:  return MOUI_EV_ENCODER_BACK;
    case MOUI_KEY_A:     return MOUI_EV_ENCODER_PRESS;
    case MOUI_KEY_B:     return MOUI_EV_ENCODER_BACK;
    default:            return MOUI_EV_NONE;
    }
}

static void push_event(moui_indev_t *dev, moui_event_type_t type, uint32_t now_ms)
{
    if (type == MOUI_EV_NONE || !dev->queue) return;
    moui_input_event_t ev = { .type = type, .timestamp_ms = now_ms };
    moui_input_queue_push(dev->queue, &ev);
}

static void poll_encoder(moui_indev_t *dev, uint32_t now_ms)
{
    moui_indev_data_t data = {0};
    dev->read(dev, &data);

    if (data.data.encoder.diff > 0) {
        for (int i = 0; i < data.data.encoder.diff; i++)
            push_event(dev, MOUI_EV_ENCODER_CW, now_ms);
    } else if (data.data.encoder.diff < 0) {
        for (int i = 0; i < -data.data.encoder.diff; i++)
            push_event(dev, MOUI_EV_ENCODER_CCW, now_ms);
    }

    bool pressed = data.data.encoder.pressed;
    bool was_pressed = dev->last.data.encoder.pressed;

    if (pressed && !was_pressed) {
        dev->last_key_time = now_ms;
    } else if (!pressed && was_pressed) {
        uint32_t dur = now_ms - dev->last_key_time;
        if (dur > ENCODER_LONG_PRESS_MS)
            push_event(dev, MOUI_EV_ENCODER_LONG, now_ms);
        else if (dur > ENCODER_DEBOUNCE_MS)
            push_event(dev, MOUI_EV_ENCODER_PRESS, now_ms);
    }

    dev->last = data;
}

static void poll_keypad(moui_indev_t *dev, uint32_t now_ms)
{
    moui_indev_data_t data = {0};
    dev->read(dev, &data);

    moui_key_t key = data.data.keypad.key;
    bool pressed = data.data.keypad.pressed;
    moui_key_t last_key = dev->last.data.keypad.key;
    bool was_pressed = dev->last.data.keypad.pressed;

    if (pressed && !was_pressed) {
        push_event(dev, key_to_event(key), now_ms);
        dev->last_key_time = now_ms;
    } else if (pressed && was_pressed && key == last_key) {
        uint32_t held = now_ms - dev->last_key_time;
        if (held > dev->repeat_delay_ms) {
            uint32_t since_repeat = (held - dev->repeat_delay_ms);
            uint32_t last_held = (now_ms - 20) > dev->last_key_time ?
                                 ((now_ms - 20) - dev->last_key_time) : 0;
            uint32_t prev_since = last_held > dev->repeat_delay_ms ?
                                  (last_held - dev->repeat_delay_ms) : 0;
            if ((since_repeat / dev->repeat_period_ms) > (prev_since / dev->repeat_period_ms)) {
                push_event(dev, key_to_event(key), now_ms);
            }
        }
    }

    dev->last = data;
}

static void push_pointer_event(moui_indev_t *dev, moui_event_type_t type,
                               int16_t x, int16_t y, uint32_t now_ms)
{
    if (type == MOUI_EV_NONE || !dev->queue) return;
    moui_input_event_t ev = { .type = type, .timestamp_ms = now_ms, .x = x, .y = y };
    moui_input_queue_push(dev->queue, &ev);
}

static void poll_pointer(moui_indev_t *dev, uint32_t now_ms)
{
    moui_indev_data_t data = {0};
    dev->read(dev, &data);

    bool pressed = data.data.pointer.pressed;
    bool was_pressed = dev->last.data.pointer.pressed;
    int16_t x = data.data.pointer.x;
    int16_t y = data.data.pointer.y;

    if (pressed && !was_pressed) {
        dev->pointer_start_x = x;
        dev->pointer_start_y = y;
        dev->last_key_time = now_ms;
        push_pointer_event(dev, MOUI_EV_POINTER_DOWN, x, y, now_ms);
    } else if (pressed && was_pressed) {
        int16_t dx = x - dev->last.data.pointer.x;
        int16_t dy = y - dev->last.data.pointer.y;
        if (abs(dx) + abs(dy) > POINTER_MOVE_THRESHOLD)
            push_pointer_event(dev, MOUI_EV_POINTER_MOVE, x, y, now_ms);
    } else if (!pressed && was_pressed) {
        push_pointer_event(dev, MOUI_EV_POINTER_UP, x, y, now_ms);

        uint32_t dur = now_ms - dev->last_key_time;
        int16_t dx = x - dev->pointer_start_x;
        int16_t dy = y - dev->pointer_start_y;
        int16_t dist = abs(dx) + abs(dy);

        if (dist > GESTURE_SWIPE_THRESHOLD) {
            if (abs(dx) > abs(dy))
                push_pointer_event(dev, dx > 0 ? MOUI_EV_GESTURE_SWIPE_RIGHT : MOUI_EV_GESTURE_SWIPE_LEFT,
                                   x, y, now_ms);
            else
                push_pointer_event(dev, dy > 0 ? MOUI_EV_GESTURE_SWIPE_DOWN : MOUI_EV_GESTURE_SWIPE_UP,
                                   x, y, now_ms);
        } else if (dur > GESTURE_LONG_MIN_MS) {
            push_pointer_event(dev, MOUI_EV_GESTURE_LONG, x, y, now_ms);
        } else if (dur > ENCODER_DEBOUNCE_MS) {
            push_pointer_event(dev, MOUI_EV_GESTURE_TAP, x, y, now_ms);
        }
    }

    dev->last = data;
}

void moui_indev_poll(moui_indev_t *dev, uint32_t now_ms)
{
    if (!dev->read) return;

    switch (dev->type) {
    case MOUI_INDEV_ENCODER: poll_encoder(dev, now_ms); break;
    case MOUI_INDEV_KEYPAD:  poll_keypad(dev, now_ms);  break;
    case MOUI_INDEV_POINTER: poll_pointer(dev, now_ms); break;
    }
}
