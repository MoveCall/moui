# Moui API Reference

## Color System (moui_color.h)

```c
typedef uint8_t moui_color_t;

#define MOUI_WHITE  0   // white
#define MOUI_LGRAY  1   // light gray (2bpp displays only)
#define MOUI_DGRAY  2   // dark gray  (2bpp displays only)
#define MOUI_BLACK  3   // black
```

1bpp displays (OLED/LCD) threshold: colors >= MOUI_DGRAY render as black, < MOUI_DGRAY as white.
2bpp displays (E-Paper grayscale) render all 4 levels natively.

## Drawing (moui_draw.h)

```c
void moui_draw_pixel(ctx, x, y, color);
void moui_draw_hline(ctx, x, y, len, color);
void moui_draw_vline(ctx, x, y, len, color);
void moui_draw_line(ctx, x0, y0, x1, y1, color);
void moui_draw_rect(ctx, &rect, color);
void moui_draw_fill_rect(ctx, &rect, color);
void moui_draw_circle(ctx, cx, cy, r, color);
void moui_draw_fill_circle(ctx, cx, cy, r, color);
void moui_draw_arc(ctx, cx, cy, r, start_deg, end_deg, color);
void moui_draw_rounded_rect(ctx, &rect, radius, color);
void moui_draw_fill_rounded_rect(ctx, &rect, radius, color);
void moui_draw_bitmap(ctx, x, y, w, h, data, color);
void moui_draw_bitmap_rle(ctx, x, y, w, h, rle_data, rle_len, color);
void moui_draw_push_clip(ctx, &rect);
void moui_draw_pop_clip(ctx);
void moui_draw_set_xor(ctx, bool);
```

## Mono-Specific (moui_dither.h / moui_pattern.h / moui_qrcode.h)

```c
void moui_draw_fill_rect_dither(ctx, &rect, MOUI_DITHER_50);
void moui_draw_fill_circle_dither(ctx, cx, cy, r, MOUI_DITHER_CHECKER);
void moui_draw_fill_rect_pattern(ctx, &rect, MOUI_PAT_CROSS_HATCH, spacing);

moui_qr_t qr;
moui_qr_encode(&qr, "text");
moui_qr_draw(ctx, &qr, x, y, scale);
```

## Font (moui_font.h)

```c
int  moui_font_draw_str(ctx, font, x, y, "text", color);
int  moui_font_draw_str_aa(ctx, font, x, y, "text", color);   // anti-aliased (uses gray levels on 2bpp)
int  moui_font_draw_str_wrapped(ctx, font, x, y, max_w, "text", color);
int  moui_font_measure_str(font, "text");
void moui_font_measure_wrapped(font, text, max_w, &w, &h);
int  moui_font_glyph_advance(font, codepoint);
int  moui_font_draw_glyph(ctx, font, x, y, codepoint, color);

// Fallback chain — auto CJK/Latin mixed rendering
moui_font_set_fallback(&moui_font_inter_16, &moui_font_cn_16);
```

Built-in fonts: `moui_font_ascii_6x8` / `moui_font_inter_12` / `moui_font_cn_12` / `moui_font_cn_16` / `moui_font_cn_24` / `moui_font_cn_32` / `moui_font_inter_16` / `moui_font_inter_24` / `moui_font_inter_32` / `moui_font_inter_48`

## Widgets (30 types)

```c
moui_widget_xxx_t widget;
moui_xxx_init(&widget, font, ...);
widget.base.bounds = (moui_rect_t){x, y, w, h};
moui_screen_add_widget(&screen, &widget.base);  // auto-adds to focus group if widget has on_event

moui_widget_has_state(&w.base, MOUI_STATE_FOCUSED);
moui_widget_set_state(&w.base, MOUI_STATE_CHECKED);
```

| Widget | Init Function / API |
|--------|---------------------|
| Label | `moui_label_init(l, text, font)` — `.wrap=true` enables word wrap |
| Button | `moui_button_init(b, label, font, MOUI_BTN_PUSH/TOGGLE/CHECKBOX)` |
| List | `moui_list_init(l, font)` + `moui_list_set_adapter(l, adapter_fn, user_data, count)` |
| VirtualList | `moui_vlist_init(vl, item_h, count_fn, bind_fn, ud)` + `moui_vlist_add_cell(vl, cell)` — cell recycling, 100k+ items <1KB RAM |
| Slider | `moui_slider_init(s, font, min, max, step)` |
| Chart | `moui_chart_init(c, y_min, y_max)` + `moui_chart_push(c, value)` |
| Progress | `moui_progress_init(p, font, max)` |
| Gauge | `moui_gauge_init(g, font, min, max)` |
| Ring | `moui_ring_init(r, font, max)` — arc progress |
| StatusBar | `moui_statusbar_init(sb, font, title)` |
| Spinner | `moui_spinner_init(sp, font, min, max, step)` |
| Radio | `moui_radio_init(r, font)` + `moui_radio_set_options(r, opts, count)` |
| Dropdown | `moui_dropdown_init(dd, font)` + `moui_dropdown_set_options(dd, opts, count)` |
| Tab | `moui_tab_init(t, font)` + `moui_tab_add_page(t, title, page)` |
| TextInput | `moui_textinput_init(ti, font)` |
| Table | `moui_table_init(t, font, cols, rows)` + `moui_table_set_cell(t, r, c, text)` |
| BtnMatrix | `moui_btnmatrix_init(bm, font, cols)` |
| Switch | `moui_switch_init(sw)` |
| Calendar | `moui_calendar_init(cal, font, year, month, day)` |
| Image | `moui_image_init(img, data, w, h)` |
| ScrollView | `moui_scroll_init(s)` + `moui_scroll_add(s, child)` |
| Keyboard | `moui_keyboard_init(kb, font)` |
| TimePicker | `moui_time_picker_init(tp, font)` + `moui_time_picker_set_time(tp, h, m, s)` |
| BarChart | `moui_barchart_init(bc, font, min, max)` + `moui_barchart_set_data(bc, vals, lbls, count)` |
| Roller | `moui_roller_init(r, font)` + `moui_roller_set_items(r, items, count)` |
| IconBar | `moui_icon_bar_init(ib, font)` + `moui_icon_bar_set_battery(ib, pct, chg)` + `set_wifi` |
| TreeView | `moui_treeview_init(tv, font)` + `moui_treeview_add_node(tv, label, depth, has_child)` |
| LogView | `moui_logview_init(lv, font)` + `moui_logview_add_line(lv, line)` |
| Extra | `moui_dots_init` / `moui_number_init` / `moui_stepper_init` / `moui_sparkline_init` / `moui_checklist_init` |
| Misc | `moui_marquee_init` / `moui_badge_init` / `moui_divider_init` / `moui_loading_init` |

## Advanced Layout (Flexbox & Anchor Constraints)

### Flexbox Elastic Layout Engine (moui_layout_flex.h)

```c
moui_flex_t flex;
moui_flex_init(&flex, MOUI_FLEX_DIR_ROW, MOUI_FLEX_JUSTIFY_SPACE_BETWEEN, MOUI_FLEX_ALIGN_CENTER);
flex.padding = 4;
flex.gap = 8;
moui_flex_add(&flex, &btn1.base);
moui_flex_add(&flex, &btn2.base);
moui_flex_layout(&flex, &parent_bounds);  // Auto-calculates coordinates & spacing
```

- Direction: `MOUI_FLEX_DIR_ROW` / `MOUI_FLEX_DIR_COLUMN`
- Justify: `MOUI_FLEX_JUSTIFY_START` / `CENTER` / `END` / `SPACE_BETWEEN` / `SPACE_AROUND`
- Align: `MOUI_FLEX_ALIGN_START` / `CENTER` / `END` / `STRETCH`

### Anchor & Constraint System (moui_anchor.h)

```c
moui_anchor_t anchor = {
    .flags = MOUI_ANCHOR_RIGHT | MOUI_ANCHOR_BOTTOM,
    .margin_right = 10,
    .margin_bottom = 10
};
moui_anchor_apply(&anchor, &widget.base, &parent_bounds);
```

Flags: `MOUI_ANCHOR_LEFT`, `RIGHT`, `TOP`, `BOTTOM`, `CENTER_H`, `CENTER_V`, `FILL`

## Reactive Property & Data-Binding (moui_property.h)

```c
moui_prop_int_t battery_prop;
moui_prop_int_init(&battery_prop, 85);
moui_prop_int_observe(&battery_prop, on_battery_change, target_widget, user_data);

// Updating property automatically triggers observers and invalidates target widget dirty rect
moui_prop_int_set(&battery_prop, 92);
```

## Timeline Animation Engine (moui_timeline.h)

```c
moui_timeline_t tl;
moui_timeline_init(&tl);
moui_timeline_add(&tl, &widgetA.base, MOUI_TL_TARGET_X, -200, 10, 0, 300, moui_ease_out_cubic);
moui_timeline_add(&tl, &widgetB.base, MOUI_TL_TARGET_X, -200, 10, 200, 400, moui_ease_out_cubic);
moui_timeline_start(&tl, now_ms);

// Call in screen.on_tick
moui_timeline_update(&tl, now_ms, mgr);
```

## Display Rotation (moui_backend_fb.h)

```c
// Mode A: pixel-level coordinate mapping (zero extra RAM)
fb.base.sw_rotate = true;
moui_backend_fb_set_rotation(&fb, MOUI_ROTATION_90);

// Mode B: flush-time buffer transpose (extra framebuffer needed)
fb.base.sw_rotate = false;
moui_backend_fb_set_rotation(&fb, MOUI_ROTATION_90);
```

Rotation values: `MOUI_ROTATION_0` / `MOUI_ROTATION_90` / `MOUI_ROTATION_180` / `MOUI_ROTATION_270`

| Mode | Extra RAM | CPU cost | Best for |
|------|-----------|----------|----------|
| A (`sw_rotate=true`) | 0 | Per-pixel switch | Small RAM, small displays |
| B (`sw_rotate=false`) | +1 framebuffer | Per-flush traverse | Large RAM, DMA displays |

## Screen Manager (moui_screen.h)

```c
moui_screen_mgr_init_be(&mgr, backend, &hal);

moui_screen_push(&mgr, &screen);
moui_screen_push_with(&mgr, &screen, params);
moui_screen_replace(&mgr, &screen);
moui_screen_pop(&mgr);
moui_screen_mgr_tick(&mgr, now_ms, dt);
moui_screen_mgr_render_once(&mgr, now_ms);   // E-Paper single-frame mode

moui_screen_t *s = moui_screen_canvas_create(draw_fn, user_data);
```

Event lifecycle: `focus dispatch` → `screen.on_event` → `BACK auto-pop` → `on_tick`

```c
// Screen-level event handler (catches events not consumed by focused widget)
scr.on_event = my_event_handler;  // bool (*)(moui_screen_t *s, const moui_input_event_t *ev)
```

Transitions: `MOUI_TRANS_NONE/FADE/SLIDE_LEFT/RIGHT/UP/DOWN/WIPE`

## Popup (moui_popup.h)

```c
moui_popup_show_toast(&mgr->popup, "Saved!", 2000);
moui_popup_show_alert(&mgr->popup, "Error", "Connection failed");
moui_popup_show_confirm(&mgr->popup, "Delete?", "Remove this item?", on_confirm);

// Menu popup — list selection with CW/CCW navigation
const char *items[] = {"Bookmark", "Jump to", "Delete", "Cancel"};
moui_popup_show_menu(&mgr->popup, "Actions", items, 4, on_menu_select);
// on_menu_select(popup, index) — index=-1 if cancelled with BACK
```

## Animation (moui_anim.h)

```c
moui_anim_t *a = moui_anim_start(&mgr.anim, &prop, to, duration_ms, moui_ease_out_cubic);
a->on_complete = callback;

// Sequence
moui_anim_seq_t seq;
moui_anim_seq_init(&seq, &mgr.anim);
moui_anim_seq_add(&seq, &prop1, to1, 300, moui_ease_out_quad);
moui_anim_seq_add(&seq, &prop2, to2, 200, moui_ease_linear);
moui_anim_seq_start(&seq);

// Parallel group
moui_anim_group_t grp;
moui_anim_group_init(&grp, 2, on_all_done, NULL);
moui_anim_group_track(&grp, anim1);
moui_anim_group_track(&grp, anim2);
```

Easing: `linear/out_quad/in_out_quad/out_cubic/out_expo/spring/out_bounce/out_elastic/in_back/out_back`

## Theme / Style

```c
moui_theme_set_default();    // padding=4, radius=4
moui_theme_set_compact();    // padding=2, radius=2
moui_theme_set_large();      // padding=6, radius=6
moui_theme.invert_colors = true;  // global dark mode
moui_theme.default_font = &moui_font_inter_16;

moui_color_t fg = moui_theme_fg();  // MOUI_BLACK (normal) or MOUI_WHITE (inverted)
moui_color_t bg = moui_theme_bg();  // MOUI_WHITE (normal) or MOUI_BLACK (inverted)
```

## Icons (moui_icons.h)

```c
// 1:1 Normal size (16x16 px)
moui_draw_icon(ctx, x, y, MOUI_ICON_HOME, MOUI_BLACK);

// Integer Scaling (1x, 2x, 3x...)
moui_draw_icon_scaled(ctx, x, y, MOUI_ICON_FACE_SMILE, 2, MOUI_BLACK);  // 32x32 px

// Auto Centering in Rect
moui_draw_icon_in_rect(ctx, &bounds, MOUI_ICON_ROCKET, MOUI_BLACK);

// Name string
const char *name = moui_icon_get_name(MOUI_ICON_WIFI); // "WIFI"
```

108 built-in Material Mono Icons (16x16 px vector bitmaps, scalable 1x/2x/3x). Includes: menu, close, back, forward, refresh, wifi, bluetooth, battery, check, add, delete, edit, save, search, settings, home, play, pause, stop, volume, smile, thumbs_up, heart, rocket, star, cpu, shield, eye, eye_off, keyboard, notification, warning, error, lock, power, usb, timer, cloud, temp, arrows, book_open, bookmark, library, chevron_up/down, moon, sun, etc.

## Custom Events (moui_input.h)

```c
// Framework events: MOUI_EV_ENCODER_CW/CCW/PRESS/LONG/BACK (1-5)
// Application events start at MOUI_EV_USER_BASE (64):
#define MY_KEY_LEFT   (MOUI_EV_USER_BASE + 1)
#define MY_KEY_RIGHT  (MOUI_EV_USER_BASE + 2)
#define MY_KEY_SLEEP  (MOUI_EV_USER_BASE + 3)
```

## Event Bus

```c
moui_event_bus_subscribe(&moui_bus, MOUI_EVT_VALUE_CHANGED, 0, handler, ctx);
moui_event_bus_publish(&moui_bus, &(moui_event_t){.type=..., .id=..., .value=...});
```

## Debug

```c
moui_debug_ctx_t dbg;
moui_debug_init(&dbg);
moui_debug_tick(&dbg, now_ms);
moui_debug_dump(&dbg, &mgr);  // prints FPS / RAM / widget tree
```
