/*
 * widget_test — moui full-component coverage test on the OnePage C61
 * (SSD1677 800x480 E-Paper, portrait 480x800, 7-key input).
 *
 *   idf.py set-target esp32c61 && idf.py build && idf.py flash monitor
 */

#include "moui.h"
#include "tst_board.h"
#include "widget_test.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "widget_test";

static moui_screen_mgr_t s_mgr;
static moui_hal_t        s_hal;

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

void app_main(void)
{
    ESP_LOGI(TAG, "boot");

    if (tst_board_init() != 0) {
        ESP_LOGE(TAG, "board init failed");
        return;
    }

    moui_backend_t *be = tst_display_init();
    if (!be) {
        ESP_LOGE(TAG, "display init failed");
        return;
    }

    s_hal.get_time_ms   = hal_get_time;
    s_hal.delay_ms      = hal_delay;
    s_hal.log           = hal_log;
    s_hal.priv          = NULL;

    moui_screen_mgr_init_be(&s_mgr, be, &s_hal);

    if (tst_keys_init(moui_screen_mgr_input(&s_mgr)) != 0) {
        ESP_LOGE(TAG, "keys init failed");
        return;
    }

    widget_test_build(&s_mgr);

    ESP_LOGI(TAG, "start (%dx%d)", (int)be->width, (int)be->height);

    uint32_t last_ms = 0;
    for (;;) {
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        tst_keys_poll();
        moui_screen_mgr_tick(&s_mgr, now, 16.0f);
        if (now - last_ms > 5000) {
            last_ms = now;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
