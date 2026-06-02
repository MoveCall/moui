#ifndef MOUI_INDEV_H
#define MOUI_INDEV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_input.h"

typedef enum {
    MOUI_INDEV_ENCODER,
    MOUI_INDEV_KEYPAD,
    MOUI_INDEV_POINTER,
} moui_indev_type_t;

typedef enum {
    MOUI_KEY_NONE = 0,
    MOUI_KEY_UP,
    MOUI_KEY_DOWN,
    MOUI_KEY_LEFT,
    MOUI_KEY_RIGHT,
    MOUI_KEY_ENTER,
    MOUI_KEY_BACK,
    MOUI_KEY_HOME,
    MOUI_KEY_A,
    MOUI_KEY_B,
} moui_key_t;

typedef struct {
    moui_indev_type_t type;
    union {
        struct {
            int8_t  diff;
            bool    pressed;
        } encoder;
        struct {
            moui_key_t key;
            bool      pressed;
        } keypad;
        struct {
            int16_t x, y;
            bool    pressed;
        } pointer;
    } data;
} moui_indev_data_t;

typedef struct moui_indev moui_indev_t;

struct moui_indev {
    moui_indev_type_t type;

    void (*read)(moui_indev_t *dev, moui_indev_data_t *data);

    moui_input_queue_t *queue;
    void              *priv;

    moui_indev_data_t   last;
    uint32_t           last_key_time;
    int16_t            pointer_start_x;
    int16_t            pointer_start_y;
    uint16_t           repeat_delay_ms;
    uint16_t           repeat_period_ms;
};

void moui_indev_init(moui_indev_t *dev, moui_indev_type_t type,
                    void (*read_cb)(moui_indev_t *, moui_indev_data_t *),
                    moui_input_queue_t *queue);

void moui_indev_poll(moui_indev_t *dev, uint32_t now_ms);


#ifdef __cplusplus
}
#endif
#endif
