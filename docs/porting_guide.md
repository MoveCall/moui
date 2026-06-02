# Moui Porting Guide

Porting Moui to a new display or MCU platform takes only 3 steps.

## Option 1: Built-in Driver (fastest)

If your display controller is in the built-in list (SSD1306/SSD1309/SH1106/ST7565/ST7567/ST7920/UC1701/UC8151/SSD1677), just implement two functions:

```c
#include "moui.h"

// 1. Implement low-level transport (I2C or SPI)
void my_write_cmd(uint8_t cmd, void *user) {
    // your I2C/SPI command send code
}

void my_write_data(const uint8_t *data, uint32_t len, void *user) {
    // your I2C/SPI data send code
}

// 2. Initialize the driver
moui_drv_ssd1306_t display;
moui_drv_ssd1306_init(&display, &(moui_ssd1306_cfg_t){
    .write_cmd = my_write_cmd,
    .write_data = my_write_data,
    .user = &your_handle,
    .width = 128, .height = 64,
});

// 3. Create draw context and start using
moui_draw_ctx_t ctx;
moui_draw_ctx_init_be(&ctx, moui_drv_ssd1306_backend(&display));
```

### SSD1677 E-Paper (panel presets)

For SSD1677-based E-Paper panels, use a panel preset macro to get the correct configuration in one line:

```c
#include "drivers/moui_drv_ssd1677.h"

// 1. Pick a panel preset
moui_ssd1677_cfg_t cfg = MOUI_SSD1677_PANEL_EPD0426A02;

// 2. Fill in platform callbacks
cfg.write_cmd       = my_spi_cmd;
cfg.write_data      = my_spi_data;
cfg.write_data_byte = my_spi_byte;
cfg.wait_busy       = my_wait_busy;
cfg.hw_reset        = my_hw_reset;
cfg.user            = &my_spi_handle;

// 3. Initialize driver
moui_drv_ssd1677_t epd;
moui_drv_ssd1677_init(&epd, &cfg);

// 4. Use as backend
moui_draw_ctx_t ctx;
moui_draw_ctx_init_be(&ctx, moui_drv_ssd1677_backend(&epd));
```

Available presets:

| Macro | Panel | Resolution | Notes |
|-------|-------|------------|-------|
| `MOUI_SSD1677_PANEL_ED037TC1` | ED037TC1 3.7" | 280×480 | Software LUT, default |
| `MOUI_SSD1677_PANEL_EPD0426A02` | Osptek EPD0426A02 4.26" | 800×480 | OTP waveforms, fast refresh, Y mirror |
| `MOUI_SSD1677_PANEL_EPD0426A02_4G` | Osptek EPD0426A02 4.26" | 800×480 | 4-level grayscale (4G) mode |

You can override any field after the preset assignment:

```c
moui_ssd1677_cfg_t cfg = MOUI_SSD1677_PANEL_EPD0426A02;
cfg.fast_temp = 0x6A;       // try a different temperature value
cfg.anti_ghosting = true;   // enable anti-ghosting for partial refresh
```

### 4-Level Grayscale (4G) Mode

Use the `_4G` preset for 4-level grayscale output (white / light grey / dark grey / black):

```c
moui_ssd1677_cfg_t cfg = MOUI_SSD1677_PANEL_EPD0426A02_4G;
cfg.write_cmd       = my_write_cmd;
cfg.write_data      = my_write_data;
cfg.write_data_byte = my_write_data_byte;
cfg.wait_busy       = my_wait_busy;
cfg.hw_reset        = my_hw_reset;
cfg.user            = &my_spi_ctx;

moui_drv_ssd1677_t drv;
moui_drv_ssd1677_init(&drv, &cfg);
```

Drawing with 4 gray levels:

```c
moui_draw_pixel(ctx, x, y, MOUI_WHITE);   // 0 - white
moui_draw_pixel(ctx, x, y, MOUI_LGRAY);   // 1 - light grey
moui_draw_pixel(ctx, x, y, MOUI_DGRAY);   // 2 - dark grey
moui_draw_pixel(ctx, x, y, MOUI_BLACK);   // 3 - black
```

To supply a panel-vendor LUT (recommended for best quality):

```c
cfg.gray_lut = my_vendor_lut;   // 112-byte LUT (105 waveform + 5 voltage + 2 reserved)
cfg.gray_lut_len = 112;
```

Notes:
- 4G mode is full-refresh only (no partial update), ~4 seconds per refresh
- B/W mode (`MOUI_SSD1677_PANEL_EPD0426A02`) is unaffected — use it for text/UI, switch to 4G for images
- Framebuffer is 2bpp (`MOUI_PIXFMT_GRAY2_HMSB`), auto-selected when `grayscale = true`

## Option 2: Custom Backend (any display)

For displays not in the built-in list, implement the `moui_backend_t` interface:

```c
#include "backend/moui_backend.h"

// Implement 4 functions
void my_set_pixel(moui_backend_t *be, int x, int y, moui_color_t c) {
    // write to your framebuffer
}

moui_color_t my_get_pixel(moui_backend_t *be, int x, int y) {
    // read from your framebuffer
}

void my_clear(moui_backend_t *be, moui_color_t c) {
    // clear screen
}

void my_flush(moui_backend_t *be) {
    // send framebuffer to display
}

// Assemble the backend
moui_backend_t my_backend = {
    .set_pixel = my_set_pixel,
    .get_pixel = my_get_pixel,
    .clear = my_clear,
    .flush = my_flush,
    .width = 128,
    .height = 64,
};

// Use it
moui_draw_ctx_t ctx;
moui_draw_ctx_init_be(&ctx, &my_backend);
```

## Option 3: Page Buffer Mode (low RAM)

Suitable for 8-bit MCUs with < 2 KB RAM:

```c
#include "backend/moui_backend_page.h"

void my_send_page(uint16_t page_y, uint16_t page_h,
                  const uint8_t *data, uint32_t len, void *user) {
    // send one page of data to the display
}

moui_backend_page_t pb;
moui_backend_page_init(&pb, 128, 64, 8, my_send_page, NULL);
// each page needs only 128 bytes of RAM (128 x 8 / 8)

// Render loop
moui_draw_ctx_t ctx;
moui_draw_ctx_init_be(&ctx, &pb.base);

moui_backend_page_first(&pb);
do {
    // draw everything (framework auto-clips to current page)
    my_draw_screen(&ctx);
} while (moui_backend_page_next(&pb));
```

## Display Rotation

Two rotation modes available — choose based on your RAM budget:

```c
moui_backend_fb_t fb;
moui_backend_fb_init(&fb, 480, 800, MOUI_PIXFMT_MONO_HMSB, hw_flush, NULL);

// Mode A: zero extra RAM (coordinate mapping per pixel)
fb.base.sw_rotate = true;
moui_backend_fb_set_rotation(&fb, MOUI_ROTATION_90);
// UI sees 800x480, coordinates transformed on every pixel write

// Mode B: extra buffer (flush-time transpose)
fb.base.sw_rotate = false;
moui_backend_fb_set_rotation(&fb, MOUI_ROTATION_90);
// UI draws to logical buffer, rotated at flush time
// Extra RAM = physical framebuffer size (e.g. 48 KB for 480x800)
```

| Mode | Extra RAM | CPU cost | Best for |
|------|-----------|----------|----------|
| A (`sw_rotate=true`) | 0 | Per-pixel switch | Small RAM (ESP32-C3), small displays |
| B (`sw_rotate=false`) | +1 framebuffer | Per-flush traverse | Large RAM (PSRAM), large displays + DMA |

**Note:** Mode A is verified via unit tests but has a known hardware incompatibility with the SSD1677 E-Paper driver. Use Mode B for SSD1677.

## E-Paper Scenario (deep sleep)

```c
// wake up -> render one frame -> sleep
moui_be_wakeup(&backend);
moui_screen_mgr_render_once(&mgr, now_ms);
moui_be_sleep(&backend);
enter_deep_sleep();
```

## HAL Time Function

The framework requires a millisecond clock, provided via `moui_hal_t`:

```c
static const moui_hal_t my_hal = {
    .get_time_ms = my_get_millis,   // required
    .delay_ms = my_delay,           // optional
    .log = my_printf,               // optional (for debug)
    .display_flush = NULL,          // legacy path only
};
```

## Pixel Formats

```c
// 1bpp — OLED, LCD (colors >= MOUI_DGRAY threshold to black)
moui_backend_fb_init(&fb, w, h, MOUI_PIXFMT_MONO_HMSB, hw_flush, NULL);

// 2bpp — E-Paper grayscale (native 4-level gray)
moui_backend_fb_init(&fb, w, h, MOUI_PIXFMT_GRAY2_HMSB, hw_flush, NULL);
```

1bpp buffer size = `(w*h+7)/8`. 2bpp buffer size = `(w*h+3)/4`.
Draw API is the same — `moui_draw_pixel(ctx, x, y, MOUI_DGRAY)` works on both;
1bpp backends automatically threshold to black/white.

## Font Fallback

```c
// Auto CJK/Latin mixed rendering
moui_font_set_fallback(&moui_font_inter_16, &moui_font_cn_16);
// Now moui_font_draw_str with moui_font_inter_16 auto-falls back to cn_16 for CJK characters
```

## Build Integration

### CMake

```cmake
add_subdirectory(path/to/moui/src)
target_link_libraries(your_app PRIVATE moui_framework)
```

### ESP-IDF

Set `EXTRA_COMPONENT_DIRS` to point to `src/` in your project `CMakeLists.txt`.

### Bare-metal Makefile

```makefile
MOUI_SRCS = $(wildcard src/core/*.c) $(wildcard src/widget/*.c) \
           src/font/moui_font.c src/input/*.c src/anim/*.c \
           src/screen/*.c src/layout/*.c src/backend/*.c
CFLAGS += -Isrc/
```

## Configuration

Override constants in `moui_conf.h` via compiler flags:

```bash
# Reduce RAM usage (for small displays)
-DMOUI_SCREEN_MAX_WIDGETS=8 -DMOUI_ANIM_POOL_SIZE=8 -DMOUI_INPUT_QUEUE_SIZE=4
```
