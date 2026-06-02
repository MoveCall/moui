#ifndef MOUI_BACKEND_PAGE_H
#define MOUI_BACKEND_PAGE_H

#include "moui_backend.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*moui_hw_page_fn)(uint16_t page_y, uint16_t page_h,
                               const uint8_t *data, uint32_t len, void *user);

typedef struct {
    moui_backend_t  base;
    uint8_t       *page_buf;
    uint32_t       page_bytes;
    uint16_t       page_h;
    uint16_t       cur_page_y;
    uint16_t       total_pages;
    moui_hw_page_fn hw_page;
    void          *hw_user;
} moui_backend_page_t;

int  moui_backend_page_init(moui_backend_page_t *pb, int w, int h, int page_h,
                           moui_hw_page_fn hw_page, void *hw_user);
void moui_backend_page_deinit(moui_backend_page_t *pb);

bool moui_backend_page_first(moui_backend_page_t *pb);
bool moui_backend_page_next(moui_backend_page_t *pb);

#ifdef __cplusplus
}
#endif

#endif
