#ifndef MOUI_WIDGET_IMAGE_H
#define MOUI_WIDGET_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_widget.h"

typedef enum {
    MOUI_IMG_ALIGN_LEFT,
    MOUI_IMG_ALIGN_CENTER,
    MOUI_IMG_ALIGN_RIGHT,
} moui_img_align_t;

typedef struct {
    moui_widget_t      base;
    const uint8_t    *data;
    uint16_t          img_w;
    uint16_t          img_h;
    moui_img_align_t   align;
    uint8_t           scale;
    bool              invert;
} moui_widget_image_t;

void moui_image_init(moui_widget_image_t *img, const uint8_t *data,
                    uint16_t w, uint16_t h);

#ifdef __cplusplus
}
#endif

#endif
