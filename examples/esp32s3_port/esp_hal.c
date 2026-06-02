#include "esp_hal.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdarg.h>
#include <stdio.h>

static const char *TAG = "moui_hal";

static void hal_flush(const moui_hal_t *hal, const uint8_t *fb, uint32_t len)
{
    esp_hal_ctx_t *ctx = (esp_hal_ctx_t *)hal->priv;
    esp_st7305_flush(&ctx->display, fb, (int)len);
}

static moui_time_ms_t hal_get_time(const moui_hal_t *hal)
{
    (void)hal;
    return (moui_time_ms_t)(esp_timer_get_time() / 1000);
}

static void hal_delay(const moui_hal_t *hal, uint32_t ms)
{
    (void)hal;
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static void hal_log(const moui_hal_t *hal, const char *fmt, ...)
{
    (void)hal;
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    ESP_LOGI(TAG, "%s", buf);
}

esp_err_t esp_hal_init(esp_hal_ctx_t *ctx,
                       const esp_st7305_config_t *disp_cfg,
                       const esp_encoder_config_t *enc_cfg)
{
    moui_input_queue_init(&ctx->input);

    esp_err_t ret = esp_st7305_init(&ctx->display, disp_cfg);
    if (ret != ESP_OK) return ret;

    ret = esp_encoder_init(&ctx->encoder, enc_cfg, &ctx->input);
    if (ret != ESP_OK) return ret;

    ctx->hal.display_flush = hal_flush;
    ctx->hal.get_time_ms   = hal_get_time;
    ctx->hal.delay_ms      = hal_delay;
    ctx->hal.log           = hal_log;
    ctx->hal.priv          = ctx;

    return ESP_OK;
}

const moui_hal_t *esp_hal_get(esp_hal_ctx_t *ctx)
{
    return &ctx->hal;
}

moui_input_queue_t *esp_hal_input(esp_hal_ctx_t *ctx)
{
    return &ctx->input;
}
