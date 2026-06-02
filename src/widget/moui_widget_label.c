#include "moui_widget_label.h"
#include "../core/moui_theme.h"

static void label_draw_wrap(moui_widget_label_t *l, moui_draw_ctx_t *ctx)
{
    moui_rect_t b = l->base.bounds;
    int max_w = b.w - (l->inverted ? 4 : 0);
    int x = b.x + (l->inverted ? 2 : 0);
    int y = b.y + 2;
    moui_color_t on = l->inverted ? moui_theme_bg() : moui_theme_fg();
    moui_font_draw_str_wrapped(ctx, l->font, x, y, max_w, l->text, on);
}

static void label_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_label_t *l = (moui_widget_label_t *)w;
    if (!l->text || !l->font) return;

    if (l->inverted)
        moui_draw_fill_rect(ctx, &w->bounds, moui_theme_fg());

    if (l->wrap) {
        label_draw_wrap(l, ctx);
    } else {
        moui_color_t on = l->inverted ? moui_theme_bg() : moui_theme_fg();
        int xoff = l->inverted ? 2 : 0;
        moui_font_draw_str(ctx, l->font, w->bounds.x + xoff, w->bounds.y + 2, l->text, on);
    }
}

static moui_point_t label_measure(const moui_widget_t *w)
{
    const moui_widget_label_t *l = (const moui_widget_label_t *)w;
    if (!l->text || !l->font) return (moui_point_t){0, 0};
    int tw = moui_font_measure_str(l->font, l->text);
    return (moui_point_t){ (int16_t)tw, (int16_t)l->font->line_height };
}

static const moui_widget_vtable_t label_vtable = {
    .draw    = label_draw,
    .measure = label_measure,
};

void moui_label_init(moui_widget_label_t *l, const char *text, const moui_font_t *font)
{
    moui_widget_init(&l->base, &label_vtable);
    l->text     = text;
    l->font     = font;
    l->inverted = false;
    l->wrap     = false;
}
