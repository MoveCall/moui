# Moui Examples

## get_started/
Complete ESP-IDF project: SSD1306 I2C OLED + Label + Progress bar.
Ready to build: `idf.py set-target esp32s3 && idf.py build`
Ideal for: first-time setup, verifying your hardware wiring.

## clock/
Digital clock face using `moui_screen_canvas_create()`.
Shows: canvas screen, number display, Inter font, decorative frames.
Ideal for: E-Paper watch face, bedside clock, status display.

## sensor_panel/
Multi-sensor monitoring dashboard with live data.
Shows: Chart, Sparkline, Ring progress, Number display, Page dots.
Ideal for: IoT sensor hub, weather station, system monitor.

## menu/
Multi-level settings menu with various input widgets.
Shows: List navigation, Slider, Switch, Stepper, Checklist, Screen stack, Popup toast.
Ideal for: Device configuration, control panel, preference editor.

## esp32s3_port/
HAL reference implementation for ESP32-S3 (ST7305 RLCD + rotary encoder).
Copy these files into your project and adapt to your hardware.

## How to Use

The `get_started/` example is a complete ESP-IDF project — just build and flash.

For `clock/`, `sensor_panel/`, and `menu/`, each is a single `.c` file with a `setup_xxx()` function.
Integrate into your project:

```c
#include "moui.h"

// Your HAL + backend init...

moui_screen_mgr_t mgr;
moui_backend_t *be = moui_drv_ssd1306_backend(&display);
moui_screen_mgr_init_be(&mgr, be, &hal);

// Call the example setup
setup_menu_example(&mgr);  // or sensor_panel, etc.

// Run the tick loop
while (1) {
    moui_screen_mgr_tick(&mgr, get_millis(), 16.0f);
    delay(20);
}
```
