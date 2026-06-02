#include "moui_widget_tab.h"
#include "../core/moui_theme.h"
#include <string.h>

static void tab_draw(moui_widget_t *w, moui_draw_ctx_t *ctx)
{
    moui_widget_tab_t *t = (moui_widget_tab_t *)w;
    if (t->page_count == 0 || !t->font) return;

    int x = w->bounds.x;
    int y = w->bounds.y;
    int width = w->bounds.w;

    int tab_w = width / t->page_count;
    for (int i = 0; i < t->page_count; i++) {
        int tx = x + i * tab_w;
        if (i == t->active) {
            moui_draw_fill_rect(ctx, &(moui_rect_t){tx, y, tab_w, t->tab_h}, moui_theme_fg());
            moui_font_draw_str(ctx, t->font, tx + 2, y + 2, t->titles[i], moui_theme_bg());
        } else {
            moui_draw_rect(ctx, &(moui_rect_t){tx, y, tab_w, t->tab_h}, moui_theme_fg());
            moui_font_draw_str(ctx, t->font, tx + 2, y + 2, t->titles[i], moui_theme_fg());
        }
    }

    moui_draw_hline(ctx, x, y + t->tab_h, width, moui_theme_fg());

    if (t->pages[t->active]) {
        moui_widget_t *page = t->pages[t->active];
        page->bounds.x = x;
        page->bounds.y = y + t->tab_h + 1;
        page->bounds.w = width;
        page->bounds.h = w->bounds.h - t->tab_h - 1;
        moui_draw_push_clip(ctx, &page->bounds);
        moui_widget_draw(page, ctx);
        moui_draw_pop_clip(ctx);
    }
}

static bool tab_on_event(moui_widget_t *w, const moui_input_event_t *ev)
{
    moui_widget_tab_t *t = (moui_widget_tab_t *)w;

    if (!t->capturing) {
        if (ev->type == MOUI_EV_ENCODER_PRESS) {
            t->capturing = true;
            return true;
        }
        return false;
    }

    switch (ev->type) {
    case MOUI_EV_ENCODER_CW:
        if (t->page_count > 0 && t->active < t->page_count - 1) {
            t->active++;
            if (t->on_change) t->on_change(t, t->active);
        }
        return true;
    case MOUI_EV_ENCODER_CCW:
        if (t->active > 0) {
            t->active--;
            if (t->on_change) t->on_change(t, t->active);
        }
        return true;
    case MOUI_EV_ENCODER_PRESS:
    case MOUI_EV_ENCODER_BACK:
        t->capturing = false;
        return true;
    default:
        return false;
    }
}

static void tab_on_focus(moui_widget_t *w, bool gained)
{
    moui_widget_tab_t *t = (moui_widget_tab_t *)w;
    if (!gained) t->capturing = false;
}

static const moui_widget_vtable_t tab_vtable = {
    .draw     = tab_draw,
    .on_event = tab_on_event,
    .on_focus = tab_on_focus,
};

void moui_tab_init(moui_widget_tab_t *t, const moui_font_t *font)
{
    moui_widget_init(&t->base, &tab_vtable);
    t->font = font;
    t->page_count = 0;
    t->active = 0;
    t->tab_h = moui_theme.tab_h > 0 ? moui_theme.tab_h : (font ? font->line_height + 4 : 14);
    t->capturing = false;
}

void moui_tab_add_page(moui_widget_tab_t *t, const char *title, moui_widget_t *page)
{
    if (t->page_count >= MOUI_TAB_MAX_PAGES) return;
    t->titles[t->page_count] = title;
    t->pages[t->page_count] = page;
    t->page_count++;
}
