#ifndef ESP_ST7305_H
#define ESP_ST7305_H

#include <stdint.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"

typedef struct {
    gpio_num_t pin_dc;
    gpio_num_t pin_rst;
    gpio_num_t pin_cs;
    gpio_num_t pin_sclk;
    gpio_num_t pin_mosi;
    spi_host_device_t spi_host;
    int spi_freq_hz;
} esp_st7305_config_t;

typedef struct {
    esp_st7305_config_t cfg;
    spi_device_handle_t spi_dev;
} esp_st7305_t;

esp_err_t esp_st7305_init(esp_st7305_t *dev, const esp_st7305_config_t *cfg);
esp_err_t esp_st7305_flush(esp_st7305_t *dev, const uint8_t *wire_fb, int len);
void esp_st7305_sleep(esp_st7305_t *dev, bool enable);
void esp_st7305_display_on(esp_st7305_t *dev, bool enable);
void esp_st7305_set_hpm(esp_st7305_t *dev);
void esp_st7305_set_lpm(esp_st7305_t *dev);

#endif
