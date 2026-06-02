#ifndef MOUI_DRV_CST836U_H
#define MOUI_DRV_CST836U_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../input/moui_indev.h"
#include <stdint.h>
#include <stdbool.h>

/*
 * CST836U capacitive touch driver (I2C, compatible with CST816S register set)
 *
 * Usage:
 *   1. Configure I2C bus externally (this driver only performs transactions).
 *   2. Fill moui_cst836u_cfg_t with platform callbacks.
 *   3. Call moui_drv_cst836u_init(&drv, &cfg).
 *   4. Init indev: moui_indev_init(&indev, MOUI_INDEV_POINTER, moui_drv_cst836u_read, queue).
 *   5. Bind driver to indev: indev.priv = &drv;
 *   6. On GPIO interrupt (falling edge on INT pin), call moui_drv_cst836u_isr_notify(&drv).
 */

#define MOUI_CST836U_I2C_ADDR  0x15

typedef struct {
    int (*i2c_read)(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len, void *user);
    void (*gpio_set_rst)(bool level, void *user);
    void (*delay_ms)(uint32_t ms, void *user);
    void *user;
    uint16_t screen_w;
    uint16_t screen_h;
    uint8_t  rotation;
} moui_cst836u_cfg_t;

typedef struct {
    moui_cst836u_cfg_t cfg;
    volatile bool      irq_pending;
    bool               last_pressed;
    int16_t            last_x;
    int16_t            last_y;
} moui_drv_cst836u_t;

void moui_drv_cst836u_init(moui_drv_cst836u_t *drv, const moui_cst836u_cfg_t *cfg);
void moui_drv_cst836u_isr_notify(moui_drv_cst836u_t *drv);
void moui_drv_cst836u_read(moui_indev_t *indev, moui_indev_data_t *data);

#ifdef __cplusplus
}
#endif

#endif
