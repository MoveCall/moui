#include "esp_st7305.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/moui_hal_types.h"
#include <string.h>

static const char *TAG = "st7305";

static void write_cmd(esp_st7305_t *dev, uint8_t cmd)
{
    gpio_set_level(dev->cfg.pin_dc, 0);
    gpio_set_level(dev->cfg.pin_cs, 0);
    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_TXDATA,
        .length = 8,
        .tx_data = { cmd },
    };
    spi_device_polling_transmit(dev->spi_dev, &t);
    gpio_set_level(dev->cfg.pin_cs, 1);
}

static void write_data(esp_st7305_t *dev, uint8_t data)
{
    gpio_set_level(dev->cfg.pin_dc, 1);
    gpio_set_level(dev->cfg.pin_cs, 0);
    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_TXDATA,
        .length = 8,
        .tx_data = { data },
    };
    spi_device_polling_transmit(dev->spi_dev, &t);
    gpio_set_level(dev->cfg.pin_cs, 1);
}

static void init_sequence(esp_st7305_t *dev)
{
    gpio_set_level(dev->cfg.pin_rst, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(dev->cfg.pin_rst, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(dev->cfg.pin_rst, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    write_cmd(dev, 0xD6); write_data(dev, 0x13); write_data(dev, 0x02);
    write_cmd(dev, 0xD1); write_data(dev, 0x01);

    write_cmd(dev, 0xC0); write_data(dev, 0x12); write_data(dev, 0x0A);

    write_cmd(dev, 0xC1);
    write_data(dev, 115); write_data(dev, 0x3E);
    write_data(dev, 0x3C); write_data(dev, 0x3C);

    write_cmd(dev, 0xC2);
    write_data(dev, 0); write_data(dev, 0x21);
    write_data(dev, 0x23); write_data(dev, 0x23);

    write_cmd(dev, 0xC4);
    write_data(dev, 50); write_data(dev, 0x5C);
    write_data(dev, 0x5A); write_data(dev, 0x5A);

    write_cmd(dev, 0xC5);
    write_data(dev, 50); write_data(dev, 0x35);
    write_data(dev, 0x37); write_data(dev, 0x37);

    write_cmd(dev, 0xD8); write_data(dev, 0x80); write_data(dev, 0xE9);
    write_cmd(dev, 0xB2); write_data(dev, 0x12);

    write_cmd(dev, 0xB3);
    write_data(dev, 0xE5); write_data(dev, 0xF6); write_data(dev, 0x17);
    write_data(dev, 0x77); write_data(dev, 0x77); write_data(dev, 0x77);
    write_data(dev, 0x77); write_data(dev, 0x77); write_data(dev, 0x77);
    write_data(dev, 0x71);

    write_cmd(dev, 0xB4);
    write_data(dev, 0x05); write_data(dev, 0x46);
    write_data(dev, 0x77); write_data(dev, 0x77); write_data(dev, 0x77);
    write_data(dev, 0x77); write_data(dev, 0x76); write_data(dev, 0x45);

    write_cmd(dev, 0x62);
    write_data(dev, 0x32); write_data(dev, 0x03); write_data(dev, 0x1F);

    write_cmd(dev, 0xB7); write_data(dev, 0x13);
    write_cmd(dev, 0xB0); write_data(dev, 0x60);

    write_cmd(dev, 0x11);
    vTaskDelay(pdMS_TO_TICKS(120));

    write_cmd(dev, 0xC9); write_data(dev, 0x00);
    write_cmd(dev, 0x36); write_data(dev, 0x48);
    write_cmd(dev, 0x3A); write_data(dev, 0x11);
    write_cmd(dev, 0xB9); write_data(dev, 0x20);
    write_cmd(dev, 0xB8); write_data(dev, 0x29);

    write_cmd(dev, 0x2A); write_data(dev, 0x17); write_data(dev, 0x24);
    write_cmd(dev, 0x2B); write_data(dev, 0x00); write_data(dev, 0xBF);

    write_cmd(dev, 0x35); write_data(dev, 0x00);
    write_cmd(dev, 0xD0); write_data(dev, 0xFF);
    write_cmd(dev, 0x38);
    write_cmd(dev, 0x29);
    write_cmd(dev, 0x20);
    write_cmd(dev, 0xBB); write_data(dev, 0x4F);

    ESP_LOGI(TAG, "ST7305 init done (168x384, HPM, 51Hz)");
}

esp_err_t esp_st7305_init(esp_st7305_t *dev, const esp_st7305_config_t *cfg)
{
    memcpy(&dev->cfg, cfg, sizeof(*cfg));

    gpio_config_t io_cfg = {
        .pin_bit_mask = (1ULL << cfg->pin_dc) | (1ULL << cfg->pin_rst) | (1ULL << cfg->pin_cs),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_cfg);

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = cfg->pin_mosi,
        .miso_io_num = -1,
        .sclk_io_num = cfg->pin_sclk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = MOUI_FB_BYTES,
    };
    esp_err_t ret = spi_bus_initialize(cfg->spi_host, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) return ret;

    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = cfg->spi_freq_hz,
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 1,
    };
    ret = spi_bus_add_device(cfg->spi_host, &dev_cfg, &dev->spi_dev);
    if (ret != ESP_OK) return ret;

    init_sequence(dev);
    return ESP_OK;
}

esp_err_t esp_st7305_flush(esp_st7305_t *dev, const uint8_t *wire_fb, int len)
{
    write_cmd(dev, 0x2A); write_data(dev, 0x17); write_data(dev, 0x24);
    write_cmd(dev, 0x2B); write_data(dev, 0x00); write_data(dev, 0xBF);
    write_cmd(dev, 0x2C);

    gpio_set_level(dev->cfg.pin_dc, 1);
    gpio_set_level(dev->cfg.pin_cs, 0);

    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = wire_fb,
    };
    esp_err_t ret = spi_device_polling_transmit(dev->spi_dev, &t);

    gpio_set_level(dev->cfg.pin_cs, 1);
    return ret;
}

void esp_st7305_display_on(esp_st7305_t *dev, bool enable)
{
    write_cmd(dev, enable ? 0x29 : 0x28);
}

void esp_st7305_sleep(esp_st7305_t *dev, bool enable)
{
    if (enable) {
        write_cmd(dev, 0x38);
        vTaskDelay(pdMS_TO_TICKS(300));
        write_cmd(dev, 0x10);
        vTaskDelay(pdMS_TO_TICKS(100));
    } else {
        write_cmd(dev, 0x11);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void esp_st7305_set_hpm(esp_st7305_t *dev)
{
    write_cmd(dev, 0x38);
    vTaskDelay(pdMS_TO_TICKS(300));

    write_cmd(dev, 0xC1);
    write_data(dev, 115); write_data(dev, 0x3E);
    write_data(dev, 0x3C); write_data(dev, 0x3C);
    write_cmd(dev, 0xC2);
    write_data(dev, 0); write_data(dev, 0x21);
    write_data(dev, 0x23); write_data(dev, 0x23);
    write_cmd(dev, 0xC4);
    write_data(dev, 50); write_data(dev, 0x5C);
    write_data(dev, 0x5A); write_data(dev, 0x5A);
    write_cmd(dev, 0xC5);
    write_data(dev, 50); write_data(dev, 0x35);
    write_data(dev, 0x37); write_data(dev, 0x37);
    write_cmd(dev, 0xC9); write_data(dev, 0x00);
    vTaskDelay(pdMS_TO_TICKS(20));
}

void esp_st7305_set_lpm(esp_st7305_t *dev)
{
    write_cmd(dev, 0xC1);
    write_data(dev, 115); write_data(dev, 0x3E);
    write_data(dev, 0x3C); write_data(dev, 0x3C);
    write_cmd(dev, 0xC2);
    write_data(dev, 0); write_data(dev, 0x21);
    write_data(dev, 0x23); write_data(dev, 0x23);
    write_cmd(dev, 0xC4);
    write_data(dev, 50); write_data(dev, 0x5C);
    write_data(dev, 0x5A); write_data(dev, 0x5A);
    write_cmd(dev, 0xC5);
    write_data(dev, 50); write_data(dev, 0x35);
    write_data(dev, 0x37); write_data(dev, 0x37);
    write_cmd(dev, 0xC9); write_data(dev, 0x00);
    vTaskDelay(pdMS_TO_TICKS(20));

    write_cmd(dev, 0x39);
    vTaskDelay(pdMS_TO_TICKS(100));
}
