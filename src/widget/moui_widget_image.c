#include "moui_widget_image.h"
#include "../core/moui_theme.h"

static void image_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_image_t *img = (moui_widget_image_t *)w;
    if (!img->data) return;

    int scale = img->scale > 0 ? img->scale : 1;
    int sw = img->img_w * scale;
    int sh = img->img_h * scale;

    int dx = w->bounds.x;
    if (img->align == MOUI_IMG_ALIGN_CENTER)
        dx = w->bounds.x + (w->bounds.w - sw) / 2;
    else if (img->align == MOUI_IMG_ALIGN_RIGHT)
        dx = w->bounds.x + w->bounds.w - sw;

    int dy = w->bounds.y + (w->bounds.h - sh) / 2;
    moui_color_t on = img->invert ? MOUI_WHITE : MOUI_BLACK;

    if (img->invert)
        moui_draw_fill_rect(ctx, &w->bounds, moui_theme_fg());

    if (scale == 1) {
        moui_draw_bitmap(ctx, dx, dy, img->img_w, img->img_h, img->data, on);
    } else {
        int bytes_per_row = (img->img_w + 7) / 8;
        for (int row = 0; row < img->img_h; row++) {
            for (int col = 0; col < img->img_w; col++) {
                int byte_idx = row * bytes_per_row + (col >> 3);
                if (img->data[byte_idx] & (0x80 >> (col & 7))) {
                    moui_draw_fill_rect(ctx, &(moui_rect_t){
                        dx + col * scale, dy + row * scale, scale, scale
                    }, on);
                }
            }
        }
    }
}

static moui_point_t image_measure(const moui_widget_t *w)
{
    const moui_widget_image_t *img = (const moui_widget_image_t *)w;
    int s = img->scale > 0 ? img->scale : 1;
    return (moui_point_t){ (int16_t)(img->img_w * s), (int16_t)(img->img_h * s) };
}

static const moui_widget_vtable_t image_vtable = {
    .draw    = image_draw,
    .measure = image_measure,
};

void moui_image_init(moui_widget_image_t *img, const uint8_t *data,
                    uint16_t w, uint16_t h)
{
    moui_widget_init(&img->base, &image_vtable);
    img->data   = data;
    img->img_w  = w;
    img->img_h  = h;
    img->align  = MOUI_IMG_ALIGN_CENTER;
    img->scale  = 1;
    img->invert = false;
}
