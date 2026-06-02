#ifndef MOUI_QRCODE_H
#define MOUI_QRCODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui_draw.h"
#include "../moui_conf.h"
#include <stdint.h>

#define MOUI_QR_MAX_MODULES (17 + MOUI_QR_MAX_VERSION * 4)
#define MOUI_QR_BUF_SIZE    ((MOUI_QR_MAX_MODULES * MOUI_QR_MAX_MODULES + 7) / 8)

typedef struct {
    uint8_t  modules[MOUI_QR_BUF_SIZE];
    uint8_t  size;
    uint8_t  version;
} moui_qr_t;

bool moui_qr_encode(moui_qr_t *qr, const char *text);

void moui_qr_draw(moui_draw_ctx_t *ctx, const moui_qr_t *qr, int x, int y, int scale);


#ifdef __cplusplus
}
#endif
#endif
