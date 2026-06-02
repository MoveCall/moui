#ifndef MOUI_POPUP_H
#define MOUI_POPUP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../core/moui_draw.h"
#include "../font/moui_font.h"
#include "../input/moui_input.h"
#include <stdbool.h>

#define MOUI_POPUP_MENU_MAX_ITEMS 12

typedef enum {
    MOUI_POPUP_NONE,
    MOUI_POPUP_TOAST,
    MOUI_POPUP_CONFIRM,
    MOUI_POPUP_ALERT,
    MOUI_POPUP_MENU,
} moui_popup_type_t;

typedef struct moui_popup moui_popup_t;

struct moui_popup {
    moui_popup_type_t type;
    const char      *title;
    const char      *message;
    const moui_font_t *font;
    bool             visible;
    bool             result;
    uint8_t          selected;
    uint32_t         show_time;
    uint32_t         duration_ms;

    const char     **items;
    uint8_t          item_count;
    uint8_t          scroll_top;

    void (*on_close)(moui_popup_t *p, bool confirmed);
    void (*on_menu_select)(moui_popup_t *p, int index);
};

void moui_popup_init(moui_popup_t *p, const moui_font_t *font);

void moui_popup_show_toast(moui_popup_t *p, const char *msg, uint32_t duration_ms);
void moui_popup_show_alert(moui_popup_t *p, const char *title, const char *msg);
void moui_popup_show_confirm(moui_popup_t *p, const char *title, const char *msg,
                            void (*on_close)(moui_popup_t *, bool));
void moui_popup_show_menu(moui_popup_t *p, const char *title,
                         const char **items, uint8_t count,
                         void (*on_select)(moui_popup_t *, int));

bool moui_popup_handle_event(moui_popup_t *p, const moui_input_event_t *ev);
void moui_popup_draw(moui_popup_t *p, moui_draw_ctx_t *ctx, uint32_t now_ms);
void moui_popup_hide(moui_popup_t *p);

static inline bool moui_popup_active(const moui_popup_t *p) { return p->visible; }


#ifdef __cplusplus
}
#endif
#endif
