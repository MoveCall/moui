# ESP32-S3 HAL reference implementation
#
# These files show how to implement moui_hal_t for ESP32-S3 with:
#   - ST7305 RLCD display (SPI)
#   - Rotary encoder (GPIO ISR)
#
# To use in your project, copy these files into your main/ component
# and add them to your SRCS list. They depend on the moui component.
#
# Example main/CMakeLists.txt:
#   idf_component_register(
#       SRCS "main.c" "esp_hal.c" "esp_st7305.c" "esp_encoder.c"
#       INCLUDE_DIRS "."
#       REQUIRES driver esp_timer moui
#   )
