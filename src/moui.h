#ifndef MOUI_H
#define MOUI_H

#include "moui_conf.h"
#include "hal/moui_hal.h"
#include "hal/moui_hal_types.h"
#include "moui_log.h"
#include "core/moui_color.h"
#include "core/moui_fb.h"
#include "core/moui_draw.h"
#include "backend/moui_backend.h"
#include "backend/moui_backend_fb.h"
#include "backend/moui_backend_page.h"
#include "core/moui_theme.h"
#include "core/moui_style.h"
#include "core/moui_event_bus.h"
#include "core/moui_debug.h"
#include "font/moui_font.h"
#include "input/moui_input.h"
#include "input/moui_focus.h"
#include "input/moui_indev.h"
#include "widget/moui_widget.h"
#include "layout/moui_layout.h"
#include "screen/moui_screen.h"

/* ── Conditional includes ── */

#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_DITHER)
#include "core/moui_dither.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_PATTERN)
#include "core/moui_pattern.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_QRCODE)
#include "core/moui_qrcode.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_ICONS)
#include "core/moui_icons.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_I18N)
#include "core/moui_i18n.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_ANIM)
#include "anim/moui_ease.h"
#include "anim/moui_anim.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_GRID)
#include "layout/moui_grid.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_POPUP)
#include "screen/moui_popup.h"
#endif

/* Widgets */
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_LABEL)
#include "widget/moui_widget_label.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_BUTTON)
#include "widget/moui_widget_button.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_LIST)
#include "widget/moui_widget_list.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_SLIDER)
#include "widget/moui_widget_slider.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_SWITCH)
#include "widget/moui_widget_switch.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_PROGRESS)
#include "widget/moui_widget_progress.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_RADIO)
#include "widget/moui_widget_radio.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_CHART)
#include "widget/moui_widget_chart.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_SCROLL)
#include "widget/moui_widget_scroll.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_STATUSBAR)
#include "widget/moui_widget_statusbar.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_SPINNER)
#include "widget/moui_widget_spinner.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_TAB)
#include "widget/moui_widget_tab.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_DROPDOWN)
#include "widget/moui_widget_dropdown.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_TEXTINPUT)
#include "widget/moui_widget_textinput.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_GAUGE)
#include "widget/moui_widget_gauge.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_TABLE)
#include "widget/moui_widget_table.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_BTNMATRIX)
#include "widget/moui_widget_btnmatrix.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_CALENDAR)
#include "widget/moui_widget_calendar.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_IMAGE)
#include "widget/moui_widget_image.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_RING)
#include "widget/moui_widget_ring.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_KEYBOARD)
#include "widget/moui_widget_keyboard.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_EXTRA)
#include "widget/moui_widget_extra.h"
#endif
#if !defined(ESP_PLATFORM) || defined(CONFIG_MOUI_USE_MISC)
#include "widget/moui_widget_misc.h"
#endif

#include "widget/moui_widget_time_picker.h"
#include "widget/moui_widget_barchart.h"
#include "widget/moui_widget_roller.h"
#include "widget/moui_widget_icon_bar.h"
#include "widget/moui_widget_treeview.h"
#include "widget/moui_widget_logview.h"
#include "widget/moui_widget_vlist.h"
#include "layout/moui_layout_flex.h"
#include "layout/moui_anchor.h"
#include "core/moui_property.h"
#include "core/moui_logo.h"
#include "anim/moui_timeline.h"

#endif
