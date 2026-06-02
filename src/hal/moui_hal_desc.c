#include "moui_hal_types.h"
#include "moui_hal.h"

const moui_disp_desc_t *moui_disp = NULL;

void moui_hal_set_display(const moui_disp_desc_t *desc)
{
    moui_disp = desc;
}

const moui_disp_desc_t moui_disp_st7305_2p9 = {
    .width        = 168,
    .height       = 384,
    .pixel_format = MOUI_PIXEL_FORMAT_ST7305_4x2,
};

const moui_disp_desc_t moui_disp_ssd1306_128x64 = {
    .width        = 128,
    .height       = 64,
    .pixel_format = MOUI_PIXEL_FORMAT_MONO_VMSB,
};

const moui_disp_desc_t moui_disp_st7305_2p13 = {
    .width        = 122,
    .height       = 250,
    .pixel_format = MOUI_PIXEL_FORMAT_ST7305_4x2,
};

const moui_disp_desc_t moui_disp_st7305_4p2 = {
    .width        = 300,
    .height       = 400,
    .pixel_format = MOUI_PIXEL_FORMAT_ST7305_4x2,
};

const moui_disp_desc_t moui_disp_st7306_4p2 = {
    .width        = 300,
    .height       = 400,
    .pixel_format = MOUI_PIXEL_FORMAT_ST7305_4x2,
};

const moui_disp_desc_t moui_disp_gdew0154 = {
    .width        = 200,
    .height       = 200,
    .pixel_format = MOUI_PIXEL_FORMAT_MONO_HMSB,
};

const moui_disp_desc_t moui_disp_ssd1677_3p7 = {
    .width        = 280,
    .height       = 480,
    .pixel_format = MOUI_PIXEL_FORMAT_MONO_HMSB,
};

const moui_disp_desc_t moui_disp_ssd1677_4p26 = {
    .width        = 480,
    .height       = 800,
    .pixel_format = MOUI_PIXEL_FORMAT_MONO_HMSB,
};

const moui_disp_desc_t moui_disp_ssd1677_4p26_4g = {
    .width        = 480,
    .height       = 800,
    .pixel_format = MOUI_PIXEL_FORMAT_GRAY2_HMSB,
};
