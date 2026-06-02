#ifndef ESP_ENCODER_H
#define ESP_ENCODER_H

#include "driver/gpio.h"
#include "esp_cpu.h"
#include "input/moui_input.h"

typedef struct {
    gpio_num_t pin_a;
    gpio_num_t pin_b;
    gpio_num_t pin_btn;
    uint32_t   long_press_ms;
} esp_encoder_config_t;

typedef struct {
    esp_encoder_config_t cfg;
    moui_input_queue_t   *queue;
    volatile int8_t      last_state;
    volatile uint32_t    btn_press_time;
    volatile bool        btn_pressed;
    volatile uint32_t    last_rot_us;
    volatile uint32_t    last_btn_us;
} esp_encoder_t;

esp_err_t esp_encoder_init(esp_encoder_t *enc, const esp_encoder_config_t *cfg,
                           moui_input_queue_t *queue);

#endif
