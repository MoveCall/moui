#include "esp_encoder.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include <string.h>

#define ROT_DEBOUNCE_US   10000
#define BTN_DEBOUNCE_US   50000

static IRAM_ATTR int8_t read_encoder_state(esp_encoder_t *enc)
{
    int a = gpio_get_level(enc->cfg.pin_a);
    int b = gpio_get_level(enc->cfg.pin_b);
    return (int8_t)((a << 1) | b);
}

static IRAM_ATTR uint32_t get_time_us(void)
{
    return (uint32_t)esp_cpu_get_cycle_count() / (CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ);
}

static void IRAM_ATTR encoder_isr(void *arg)
{
    esp_encoder_t *enc = (esp_encoder_t *)arg;

    uint32_t now_us = get_time_us();
    if ((now_us - enc->last_rot_us) < ROT_DEBOUNCE_US) return;

    int8_t state = read_encoder_state(enc);
    int8_t last = enc->last_state;

    if (state == last) return;

    int8_t dir = 0;
    if ((last == 0 && state == 1) || (last == 1 && state == 3) ||
        (last == 3 && state == 2) || (last == 2 && state == 0)) {
        dir = 1;
    } else if ((last == 0 && state == 2) || (last == 2 && state == 3) ||
               (last == 3 && state == 1) || (last == 1 && state == 0)) {
        dir = -1;
    }

    enc->last_state = state;

    if (dir != 0) {
        enc->last_rot_us = now_us;
        moui_input_event_t ev = {
            .type = (dir > 0) ? MOUI_EV_ENCODER_CW : MOUI_EV_ENCODER_CCW,
            .timestamp_ms = now_us / 1000,
        };
        moui_input_queue_push(enc->queue, &ev);
    }
}

static void IRAM_ATTR button_isr(void *arg)
{
    esp_encoder_t *enc = (esp_encoder_t *)arg;
    uint32_t now_us = get_time_us();

    if ((now_us - enc->last_btn_us) < BTN_DEBOUNCE_US) return;
    enc->last_btn_us = now_us;

    uint32_t now_ms = now_us / 1000;
    int level = gpio_get_level(enc->cfg.pin_btn);

    if (level == 0 && !enc->btn_pressed) {
        enc->btn_pressed = true;
        enc->btn_press_time = now_ms;
    } else if (level == 1 && enc->btn_pressed) {
        enc->btn_pressed = false;
        uint32_t duration = now_ms - enc->btn_press_time;
        moui_input_event_t ev = { .timestamp_ms = now_ms };

        if (duration > enc->cfg.long_press_ms) {
            ev.type = MOUI_EV_ENCODER_BACK;
        } else if (duration > 50) {
            ev.type = MOUI_EV_ENCODER_PRESS;
        } else {
            return;
        }

        moui_input_queue_push(enc->queue, &ev);
    }
}
esp_err_t esp_encoder_init(esp_encoder_t *enc, const esp_encoder_config_t *cfg,
                           moui_input_queue_t *queue)
{
    memcpy(&enc->cfg, cfg, sizeof(*cfg));
    enc->queue = queue;
    enc->btn_pressed = false;
    enc->btn_press_time = 0;
    enc->last_rot_us = 0;
    enc->last_btn_us = 0;

    if (cfg->long_press_ms == 0) enc->cfg.long_press_ms = 500;

    gpio_config_t io_cfg = {
        .pin_bit_mask = (1ULL << cfg->pin_a) | (1ULL << cfg->pin_b),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_cfg));

    gpio_config_t btn_cfg = {
        .pin_bit_mask = (1ULL << cfg->pin_btn),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&btn_cfg));

    enc->last_state = read_encoder_state(enc);

    esp_err_t ret = gpio_install_isr_service(0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) return ret;

    gpio_isr_handler_add(cfg->pin_a, encoder_isr, enc);
    gpio_isr_handler_add(cfg->pin_b, encoder_isr, enc);
    gpio_isr_handler_add(cfg->pin_btn, button_isr, enc);

    return ESP_OK;
}
