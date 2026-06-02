#include "moui_widget_misc.h"
#include "../core/moui_theme.h"
#include <math.h>
#include "../core/moui_theme.h"
#include <stdio.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979f
#endif

/* ======== Marquee ======== */
static void marquee_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_marquee_t *m = (moui_widget_marquee_t *)w;
    if (!m->font || !m->text) return;

    moui_draw_push_clip(ctx, &w->bounds);
    moui_font_draw_str(ctx, m->font, w->bounds.x - m->offset,
                      w->bounds.y + (w->bounds.h - m->font->line_height) / 2,
                      m->text, moui_theme_fg());
    moui_draw_pop_clip(ctx);
}

static const moui_widget_vtable_t marquee_vtable = { .draw = marquee_draw };

void moui_marquee_init(moui_widget_marquee_t *m, const moui_font_t *font, const char *text)
{
    moui_widget_init(&m->base, &marquee_vtable);
    m->font = font;
    m->text = text;
    m->offset = 0;
    m->text_w = (int16_t)moui_font_measure_str(font, text);
    m->speed = 1;
    m->base.enabled = 0;
}

void moui_marquee_tick(moui_widget_marquee_t *m)
{
    if (m->text_w <= m->base.bounds.w) { m->offset = 0; return; }
    m->offset += m->speed;
    if (m->offset > m->text_w + 20)
        m->offset = -m->base.bounds.w;
}

/* ======== Badge ======== */
static void badge_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_badge_t *b = (moui_widget_badge_t *)w;
    int cx = w->bounds.x + w->bounds.w / 2;
    int cy = w->bounds.y + w->bounds.h / 2;
    int r = w->bounds.h / 2;

    moui_draw_fill_circle(ctx, cx, cy, r, moui_theme_fg());

    if (!b->dot_only && b->font && b->count > 0) {
        char buf[4];
        snprintf(buf, sizeof(buf), "%d", b->count > 99 ? 99 : (int)b->count);
        int tw = moui_font_measure_str(b->font, buf);
        moui_font_draw_str(ctx, b->font, cx - tw / 2, cy - b->font->line_height / 2, buf, moui_theme_bg());
    }
}

static const moui_widget_vtable_t badge_vtable = { .draw = badge_draw };

void moui_badge_init(moui_widget_badge_t *b, const moui_font_t *font)
{
    moui_widget_init(&b->base, &badge_vtable);
    b->font = font;
    b->count = 0;
    b->dot_only = false;
    b->base.enabled = 0;
}

/* ======== Divider ======== */
static void divider_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_divider_t *d = (moui_widget_divider_t *)w;
    if (d->vertical)
        moui_draw_vline(ctx, w->bounds.x + w->bounds.w / 2, w->bounds.y, w->bounds.h, moui_theme_fg());
    else
        moui_draw_hline(ctx, w->bounds.x, w->bounds.y + w->bounds.h / 2, w->bounds.w, moui_theme_fg());
}

static const moui_widget_vtable_t divider_vtable = { .draw = divider_draw };

void moui_divider_init(moui_widget_divider_t *d, bool vertical)
{
    moui_widget_init(&d->base, &divider_vtable);
    d->vertical = vertical;
    d->base.enabled = 0;
}

/* ======== Loading Spinner ======== */
static void loading_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_loading_t *l = (moui_widget_loading_t *)w;
    int cx = w->bounds.x + w->bounds.w / 2;
    int cy = w->bounds.y + w->bounds.h / 2;
    int r = (w->bounds.w < w->bounds.h ? w->bounds.w : w->bounds.h) / 2 - 2;

    moui_draw_circle(ctx, cx, cy, r, moui_theme_fg());

    for (int i = 0; i < 8; i++) {
        float a = l->angle + i * M_PI / 4.0f;
        int x1 = cx + (int)(cosf(a) * (r - 4));
        int y1 = cy + (int)(sinf(a) * (r - 4));
        int x2 = cx + (int)(cosf(a) * (r - 1));
        int y2 = cy + (int)(sinf(a) * (r - 1));
        if (i < 4)
            moui_draw_line(ctx, x1, y1, x2, y2, moui_theme_fg());
    }
}

static const moui_widget_vtable_t loading_vtable = { .draw = loading_draw };

void moui_loading_init(moui_widget_loading_t *l)
{
    moui_widget_init(&l->base, &loading_vtable);
    l->angle = 0;
    l->speed = 3;
    l->base.enabled = 0;
}

void moui_loading_tick(moui_widget_loading_t *l, float dt)
{
    l->angle += dt * 0.005f * l->speed;
    if (l->angle > 2 * M_PI) l->angle -= 2 * M_PI;
}
