idf_component_register(
    SRCS "main.c" "widget_test.c" "tst_board.c" "tst_keys.c"
    INCLUDE_DIRS "."
    REQUIRES src esp_timer esp_driver_spi esp_adc esp_driver_gpio
)
