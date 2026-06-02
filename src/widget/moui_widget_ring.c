#include "moui_widget_ring.h"
#include "../core/moui_theme.h"
#include "../core/moui_dither.h"
#include <stdio.h>
#include <math.h>

#define PI_F 3.14159f

static void ring_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_ring_t *r = (moui_widget_ring_t *)w;
    int cx = w->bounds.x + w->bounds.w / 2;
    int cy = w->bounds.y + w->bounds.h / 2;
    int radius = (w->bounds.w < w->bounds.h ? w->bounds.w : w->bounds.h) / 2 - 2;
    int thick = r->thickness > 0 ? r->thickness : 4;
    moui_color_t fg = moui_theme_fg();

    int pct = r->max_val > 0 ? (int)((float)r->value * 100 / r->max_val) : 0;
    if (pct > 100) pct = 100;
    float end_angle = pct * 360.0f / 100.0f;

    /* Draw ring pixel by pixel for precise fill control */
    for (int py = cy - radius - 1; py <= cy + radius + 1; py++) {
        for (int px = cx - radius - 1; px <= cx + radius + 1; px++) {
            int dx = px - cx, dy = py - cy;
            float dist = sqrtf((float)(dx * dx + dy * dy));

            /* Check if within ring band */
            if (dist < radius - thick || dist > radius) continue;

            /* Calculate angle (0° = top, clockwise) */
            float angle = atan2f((float)dx, (float)(-dy)) * 180.0f / PI_F;
            if (angle < 0) angle += 360.0f;

            if (angle <= end_angle) {
                /* Foreground: solid fill */
                moui_draw_pixel(ctx, px, py, fg);
            } else {
                /* Background: dithered (25% density) */
                if (((px + py) & 3) == 0)
                    moui_draw_pixel(ctx, px, py, fg);
            }
        }
    }

    /* Center text — percentage */
    if (r->font) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%%", pct);
        int tw = moui_font_measure_str(r->font, buf);
        moui_font_draw_str(ctx, r->font, cx - tw / 2,
                          cy - r->font->line_height / 2, buf, fg);
    }

    /* Label below ring */
    if (r->label && r->label[0] && r->font) {
        int tw = moui_font_measure_str(r->font, r->label);
        moui_font_draw_str(ctx, r->font, cx - tw / 2,
                          w->bounds.y + w->bounds.h - r->font->line_height - 1,
                          r->label, fg);
    }
}

static const moui_widget_vtable_t ring_vtable = { .draw = ring_draw };

void moui_ring_init(moui_widget_ring_t *r, const moui_font_t *font, int32_t max_val)
{
    moui_widget_init(&r->base, &ring_vtable);
    r->font = font;
    r->value = 0;
    r->max_val = max_val;
    r->thickness = 4;
    r->label = NULL;
    r->base.enabled = 0;
}
