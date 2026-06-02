/*
 * OnePage C61 board adapter for the widget_test example.
 *
 * Self-contained: SPI bridge for the SSD1677 E-Paper (800x480 -> portrait
 * 480x800) plus the shared 7-key input. Deliberately depends only on the moui
 * framework and IDF drivers — no BSP — so it can live inside the moui repo.
 *
 * Pin map (matches the Moink OnePage C61):
 *   MOSI=GPIO23  SCLK=GPIO22  EPD_CS=GPIO25  DC=GPIO8
 *   RST/PWR_EN=GPIO27 (high = powered)  BUSY=GPIO29
 *   side keys: WAKE=GPIO2  PREV=GPIO6  NEXT=GPIO9 (active-low)
 *   front ADC ladder: GPIO4 = ADC1_CH2  (BACK/LEFT/RIGHT/ENTER)
 */

#include "tst_board.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "drivers/moui_drv_ssd1677.h"

#define PIN_MOSI    GPIO_NUM_23
#define PIN_SCLK    GPIO_NUM_22
#define PIN_CS      GPIO_NUM_25
#define PIN_DC      GPIO_NUM_8
#define PIN_RST     GPIO_NUM_27   // EPD_RST + peripheral power-enable (high = active)
#define PIN_BUSY    GPIO_NUM_29
#define SPI_HOST_ID SPI2_HOST
#define SPI_FREQ_HZ (10 * 1000 * 1000)
#define SPI_CHUNK   16384         // keep well under C61 SPI_MS_DATA_BITLEN

#define EPD_W 800
#define EPD_H 480

static const char *TAG = "tst_board";
static spi_device_handle_t s_spi;

static esp_err_t spi_bus_setup(void)
{
    gpio_config_t out_cfg = {
        .pin_bit_mask = (1ULL << PIN_DC) | (1ULL << PIN_RST),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&out_cfg);

    gpio_set_level(PIN_RST, 1);          // power the peripheral rail on
    vTaskDelay(pdMS_TO_TICKS(10));

    gpio_config_t in_cfg = { .pin_bit_mask = (1ULL << PIN_BUSY), .mode = GPIO_MODE_INPUT };
    gpio_config(&in_cfg);

    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = GPIO_NUM_24,
        .sclk_io_num = PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = EPD_W * EPD_H / 8,
    };
    esp_err_t ret = spi_bus_initialize(SPI_HOST_ID, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) return ret;

    spi_device_interface_config_t devcfg = {
        .mode = 0,
        .clock_speed_hz = SPI_FREQ_HZ,
        .spics_io_num = PIN_CS,
        .queue_size = 1,
    };
    return spi_bus_add_device(SPI_HOST_ID, &devcfg, &s_spi);
}

static void bridge_write_cmd(uint8_t cmd, void *user)
{
    (void)user;
    gpio_set_level(PIN_DC, 0);
    spi_transaction_t t = { .length = 8, .tx_buffer = &cmd };
    spi_device_polling_transmit(s_spi, &t);
}

static void bridge_write_data(const uint8_t *data, uint32_t len, void *user)
{
    (void)user;
    gpio_set_level(PIN_DC, 1);
    while (len > 0) {
        uint32_t chunk = (len > SPI_CHUNK) ? SPI_CHUNK : len;
        spi_transaction_t t = { .length = chunk * 8, .tx_buffer = data };
        spi_device_polling_transmit(s_spi, &t);
        data += chunk;
        len -= chunk;
    }
}

static void bridge_write_data_byte(uint8_t d, void *user)
{
    (void)user;
    gpio_set_level(PIN_DC, 1);
    spi_transaction_t t = { .length = 8, .tx_buffer = &d };
    spi_device_polling_transmit(s_spi, &t);
}

static void bridge_wait_busy(void *user)
{
    (void)user;
    uint32_t start = xTaskGetTickCount() * portTICK_PERIOD_MS;
    while (gpio_get_level(PIN_BUSY) == 1) {
        vTaskDelay(pdMS_TO_TICKS(1));
        if ((xTaskGetTickCount() * portTICK_PERIOD_MS - start) > 5000) {
            ESP_LOGW(TAG, "EPD busy timeout");
            break;
        }
    }
}

static void bridge_hw_reset(void *user)
{
    (void)user;
    gpio_set_level(PIN_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(20));
}

int tst_board_init(void)
{
    esp_err_t ret = spi_bus_setup();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI init failed: %s", esp_err_to_name(ret));
        return -1;
    }
    return 0;
}

static moui_drv_ssd1677_t s_epd;

moui_backend_t *tst_display_init(void)
{
    moui_ssd1677_cfg_t cfg = {
        .write_cmd        = bridge_write_cmd,
        .write_data       = bridge_write_data,
        .write_data_byte  = bridge_write_data_byte,
        .wait_busy        = bridge_wait_busy,
        .hw_reset         = bridge_hw_reset,
        .user             = NULL,
        .width            = EPD_W,
        .height           = EPD_H,
        .gate_scan_dir    = 0x02,   // SM interlaced scan (GxEPD2-compatible)
        .use_internal_lut = true,   // OTP waveform
        .use_otp_voltages = true,   // skip 3.7" voltage regs
        .anti_ghosting    = false,  /* Disable 2-pass flashing anti-ghosting matching official BSP */
        .fast_temp        = 0x5A,   /* 0x5A = 0xDF sequence, maintains charge pump voltage matching official BSP */
        .mirror_y         = true,   // panel gates reversed
    };
    if (moui_drv_ssd1677_init(&s_epd, &cfg) != 0) {
        ESP_LOGE(TAG, "SSD1677 init failed");
        return NULL;
    }
    moui_drv_ssd1677_set_partial(&s_epd, true);   /* Force partial refresh mode for fast, non-flashing key navigation */
    moui_backend_fb_set_rotation(&s_epd.fb_be, MOUI_ROTATION_270);  // -> portrait 480x800
    return moui_drv_ssd1677_backend(&s_epd);
}

void tst_display_force_full(void)
{
    s_epd.initial_refresh = true;
}

void tst_display_set_partial(bool force_partial)
{
    moui_drv_ssd1677_set_partial(&s_epd, force_partial);
}
