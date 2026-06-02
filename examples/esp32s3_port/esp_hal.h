#ifndef ESP_HAL_H
#define ESP_HAL_H

#include "hal/moui_hal.h"
#include "esp_st7305.h"
#include "esp_encoder.h"

typedef struct {
    moui_hal_t         hal;
    esp_st7305_t      display;
    esp_encoder_t     encoder;
    moui_input_queue_t input;
} esp_hal_ctx_t;

esp_err_t esp_hal_init(esp_hal_ctx_t *ctx,
                       const esp_st7305_config_t *disp_cfg,
                       const esp_encoder_config_t *enc_cfg);

const moui_hal_t *esp_hal_get(esp_hal_ctx_t *ctx);
moui_input_queue_t *esp_hal_input(esp_hal_ctx_t *ctx);

#endif
