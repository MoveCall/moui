/*
 * Moui Get Started — SSD1306 I2C Example
 *
 * Wiring (ESP32-S3):
 *   SDA → GPIO 8
 *   SCL → GPIO 9
 *
 * Shows a label, a progress bar and basic encoder navigation.
 * Adapt the GPIO pins and I2C address to your board.
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "moui.h"
#include "drivers/moui_drv_ssd1306.h"

#define TAG "moui_demo"

/* ── I2C config ── */
#define I2C_SDA_PIN       8
#define I2C_SCL_PIN       9
#define SSD1306_ADDR      0x3C
#define OLED_W            128
#define OLED_H            64

static i2c_master_bus_handle_t i2c_bus;
static i2c_master_dev_handle_t i2c_dev;

static void i2c_init(void)
{
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &i2c_bus));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SSD1306_ADDR,
        .scl_speed_hz = 400000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus, &dev_cfg, &i2c_dev));
}

/* ── SSD1306 I2C transport ── */

static void ssd1306_write_cmd(uint8_t cmd, void *user)
{
    (void)user;
    uint8_t buf[2] = {0x00, cmd};
    i2c_master_transmit(i2c_dev, buf, 2, 100);
}

static void ssd1306_write_data(const uint8_t *data, uint32_t len, void *user)
{
    (void)user;
    uint8_t *buf = malloc(len + 1);
    buf[0] = 0x40;
    memcpy(buf + 1, data, len);
    i2c_master_transmit(i2c_dev, buf, len + 1, 100);
    free(buf);
}

/* ── Moui HAL ── */

static moui_drv_ssd1306_t oled;
static moui_draw_ctx_t ctx;
static moui_input_queue_t input;
static moui_screen_mgr_t mgr;

static void hal_flush(const moui_hal_t *hal, const uint8_t *fb, uint32_t len)
{
    (void)hal;
    moui_backend_t *be = moui_drv_ssd1306_backend(&oled);
    be->flush(be);
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

static moui_hal_t hal = {
    .display_flush = hal_flush,
    .get_time_ms = hal_get_time,
    .delay_ms = hal_delay,
    .log = NULL,
};

/* ── UI ── */

static moui_screen_t home_screen;
static moui_widget_label_t title;
static moui_widget_label_t info;
static moui_widget_progress_t progress;

static uint8_t progress_val = 0;

static void home_tick(moui_screen_t *s, uint32_t now_ms, float dt)
{
    (void)s; (void)dt;
    progress_val = (now_ms / 50) % 101;
    progress.value = progress_val;
    moui_screen_mgr_mark_dirty(&mgr);
}

static void setup_ui(void)
{
    moui_screen_init(&home_screen);
    home_screen.on_tick = home_tick;

    moui_label_init(&title, "Moui on ESP32", &moui_font_ascii_6x8);
    title.base.bounds.x = 4;
    title.base.bounds.y = 4;

    moui_label_init(&info, "SSD1306 I2C Demo", &moui_font_ascii_6x8);
    info.base.bounds.x = 4;
    info.base.bounds.y = 20;

    moui_progress_init(&progress, &moui_font_ascii_6x8, 100);
    progress.base.bounds.x = 4;
    progress.base.bounds.y = 40;
    progress.base.bounds.w = 120;
    progress.base.bounds.h = 10;
    progress.value = 0;

    moui_screen_add_widget(&home_screen, &title.base);
    moui_screen_add_widget(&home_screen, &info.base);
    moui_screen_add_widget(&home_screen, &progress.base);
}

/* ── Main ── */

void app_main(void)
{
    ESP_LOGI(TAG, "Moui get_started example");

    /* Display descriptor */
    static const moui_disp_desc_t disp = {
        .width = OLED_W,
        .height = OLED_H,
        .pixel_format = MOUI_PIXEL_FORMAT_MONO_VMSB,
    };
    moui_hal_set_display(&disp);

    /* I2C + OLED init */
    i2c_init();
    moui_drv_ssd1306_init(&oled, &(moui_ssd1306_cfg_t){
        .write_cmd = ssd1306_write_cmd,
        .write_data = ssd1306_write_data,
        .user = NULL,
        .width = OLED_W,
        .height = OLED_H,
    });

    /* Draw context + screen manager */
    moui_backend_t *be = moui_drv_ssd1306_backend(&oled);
    moui_draw_ctx_init_be(&ctx, be);
    moui_input_queue_init(&input);
    moui_screen_mgr_init_be(&mgr, be, &hal);

    /* UI setup */
    setup_ui();
    moui_screen_push(&mgr, &home_screen);

    /* Main loop */
    uint32_t last_ms = 0;
    while (1) {
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        float dt = (now - last_ms) / 1000.0f;
        if (dt < 0.001f) dt = 0.001f;
        last_ms = now;

        moui_screen_mgr_tick(&mgr, now, dt);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
