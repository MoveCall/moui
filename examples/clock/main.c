/*
 * Moui Example: Digital Clock
 *
 * Minimal example showing a large-digit clock face
 * using canvas screen + number display.
 *
 * Build: link with moui_framework + moui_fonts
 */

#include "moui.h"

static moui_screen_mgr_t mgr;
static moui_fb_t fb;

static void clock_draw(moui_draw_ctx_t *ctx, void *data)
{
    (void)data;
    bool fg = moui_theme_fg();

    /* Time display (simulated) */
    moui_font_draw_str(ctx, &moui_font_inter_16, 30, 160, "12:34", fg);

    /* Date */
    moui_font_draw_str(ctx, &moui_font_ascii_6x8, 40, 200, "2026-05-27", fg);

    /* Battery icon area */
    moui_draw_rect(ctx, &(moui_rect_t){140, 4, 14, 8}, fg);
    moui_draw_fill_rect(ctx, &(moui_rect_t){154, 6, 2, 4}, fg);
    moui_draw_fill_rect(ctx, &(moui_rect_t){141, 5, 10, 6}, fg);

    /* Decorative frame */
    moui_draw_rounded_rect(ctx, &(moui_rect_t){4, 140, 160, 80}, 6, fg);
}

/*
 * Usage with screen manager:
 *
 *   moui_screen_t *clock_screen = moui_screen_canvas_create(clock_draw, NULL);
 *   moui_screen_push(&mgr, clock_screen);
 *
 * Or for E-Paper single-frame:
 *
 *   moui_screen_mgr_render_once(&mgr, millis());
 */
