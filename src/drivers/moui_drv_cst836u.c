#include "moui_drv_cst836u.h"
#include <string.h>

#define REG_GESTURE_ID  0x01
#define REG_FINGER_NUM  0x02
#define REG_XH          0x03
#define REG_XL          0x04
#define REG_YH          0x05
#define REG_YL          0x06

static inline int16_t clamp_i(int16_t v, int16_t lo, int16_t hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void moui_drv_cst836u_init(moui_drv_cst836u_t *drv, const moui_cst836u_cfg_t *cfg)
{
    memset(drv, 0, sizeof(*drv));
    drv->cfg = *cfg;
    if (drv->cfg.screen_w <= 0) drv->cfg.screen_w = 1;
    if (drv->cfg.screen_h <= 0) drv->cfg.screen_h = 1;

    if (cfg->gpio_set_rst) {
        cfg->gpio_set_rst(false, cfg->user);
        if (cfg->delay_ms) cfg->delay_ms(5, cfg->user);
        cfg->gpio_set_rst(true, cfg->user);
        if (cfg->delay_ms) cfg->delay_ms(50, cfg->user);
    }
}

void moui_drv_cst836u_isr_notify(moui_drv_cst836u_t *drv)
{
    drv->irq_pending = true;
}

void moui_drv_cst836u_read(moui_indev_t *indev, moui_indev_data_t *data)
{
    moui_drv_cst836u_t *drv = (moui_drv_cst836u_t *)indev->priv;

    data->data.pointer.pressed = false;
    data->data.pointer.x = drv->last_x;
    data->data.pointer.y = drv->last_y;

    if (!drv->irq_pending) {
        data->data.pointer.pressed = drv->last_pressed;
        return;
    }
    drv->irq_pending = false;

    uint8_t buf[6];
    int ret = drv->cfg.i2c_read(MOUI_CST836U_I2C_ADDR, REG_GESTURE_ID, buf, 6, drv->cfg.user);
    if (ret != 0) {
        data->data.pointer.pressed = false;
        drv->last_pressed = false;
        return;
    }

    uint8_t fingers = buf[1];
    uint8_t event = (buf[2] >> 6) & 0x03;
    int16_t raw_x = ((buf[2] & 0x0F) << 8) | buf[3];
    int16_t raw_y = ((buf[4] & 0x0F) << 8) | buf[5];

    int16_t x = raw_x;
    int16_t y = raw_y;
    switch (drv->cfg.rotation) {
    case 1:
        x = raw_y;
        y = drv->cfg.screen_w - 1 - raw_x;
        break;
    case 2:
        x = drv->cfg.screen_w - 1 - raw_x;
        y = drv->cfg.screen_h - 1 - raw_y;
        break;
    case 3:
        x = drv->cfg.screen_h - 1 - raw_y;
        y = raw_x;
        break;
    default:
        break;
    }

    bool pressed = (fingers > 0) && (event != 1);
    /* Clamp to the logical screen so rotated/oversized coordinates never
     * escape the panel (12-bit touch resolution vs. small screens). */
    data->data.pointer.x = clamp_i(x, 0, (int16_t)(drv->cfg.screen_w - 1));
    data->data.pointer.y = clamp_i(y, 0, (int16_t)(drv->cfg.screen_h - 1));
    data->data.pointer.pressed = pressed;

    drv->last_pressed = pressed;
    drv->last_x = data->data.pointer.x;
    drv->last_y = data->data.pointer.y;
}
