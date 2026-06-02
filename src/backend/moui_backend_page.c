#include "moui_backend_page.h"
#include <string.h>
#include <stdlib.h>

static void page_set_pixel(moui_backend_t *be, int x, int y, moui_color_t c)
{
    moui_backend_page_t *pb = (moui_backend_page_t *)be;
    if (x < 0 || x >= be->width) return;
    int ly = y - pb->cur_page_y;
    if (ly < 0 || ly >= pb->page_h) return;
    int idx = (ly * be->width + x) >> 3;
    uint8_t mask = 0x80 >> ((ly * be->width + x) & 7);
    if (c) pb->page_buf[idx] |= mask;
    else   pb->page_buf[idx] &= ~mask;
}

static moui_color_t page_get_pixel(moui_backend_t *be, int x, int y)
{
    moui_backend_page_t *pb = (moui_backend_page_t *)be;
    if (x < 0 || x >= be->width) return MOUI_WHITE;
    int ly = y - pb->cur_page_y;
    if (ly < 0 || ly >= pb->page_h) return MOUI_WHITE;
    int idx = (ly * be->width + x) >> 3;
    uint8_t mask = 0x80 >> ((ly * be->width + x) & 7);
    return (pb->page_buf[idx] & mask) ? MOUI_BLACK : MOUI_WHITE;
}

static void page_clear(moui_backend_t *be, moui_color_t c)
{
    moui_backend_page_t *pb = (moui_backend_page_t *)be;
    memset(pb->page_buf, c ? 0xFF : 0x00, pb->page_bytes);
}

static void page_flush(moui_backend_t *be)
{
    moui_backend_page_t *pb = (moui_backend_page_t *)be;
    if (pb->hw_page)
        pb->hw_page(pb->cur_page_y, pb->page_h, pb->page_buf, pb->page_bytes, pb->hw_user);
}

int moui_backend_page_init(moui_backend_page_t *pb, int w, int h, int page_h,
                          moui_hw_page_fn hw_page, void *hw_user)
{
    memset(pb, 0, sizeof(*pb));
    pb->base.width     = w;
    pb->base.height    = h;
    pb->base.phys_w    = w;
    pb->base.phys_h    = h;
    pb->base.set_pixel = page_set_pixel;
    pb->base.get_pixel = page_get_pixel;
    pb->base.clear     = page_clear;
    pb->base.flush     = page_flush;

    pb->page_h      = (uint16_t)(page_h > 0 ? page_h : 8);
    pb->total_pages = (uint16_t)((h + pb->page_h - 1) / pb->page_h);
    pb->page_bytes  = ((uint32_t)w * pb->page_h + 7) / 8;

    pb->page_buf = (uint8_t *)calloc(1, pb->page_bytes);
    if (!pb->page_buf) return -1;

    pb->hw_page = hw_page;
    pb->hw_user = hw_user;
    return 0;
}

void moui_backend_page_deinit(moui_backend_page_t *pb)
{
    free(pb->page_buf);
    pb->page_buf = NULL;
}

bool moui_backend_page_first(moui_backend_page_t *pb)
{
    pb->cur_page_y = 0;
    page_clear(&pb->base, MOUI_WHITE);
    return true;
}

bool moui_backend_page_next(moui_backend_page_t *pb)
{
    page_flush(&pb->base);
    pb->cur_page_y += pb->page_h;
    if (pb->cur_page_y >= pb->base.height) return false;
    page_clear(&pb->base, MOUI_WHITE);
    return true;
}
