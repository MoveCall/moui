#ifndef MOUI_CONF_H
#define MOUI_CONF_H

/*
 * Moui Configuration
 *
 * ESP-IDF:  values come from Kconfig (menuconfig).
 * Desktop:  defaults below; override with -D flags or a moui_conf_user.h.
 */

#ifdef ESP_PLATFORM
#include "sdkconfig.h"
#endif

/* ── Tuning ── */

#ifndef MOUI_SCREEN_MAX_WIDGETS
#ifdef CONFIG_MOUI_SCREEN_MAX_WIDGETS
#define MOUI_SCREEN_MAX_WIDGETS    CONFIG_MOUI_SCREEN_MAX_WIDGETS
#else
#define MOUI_SCREEN_MAX_WIDGETS    16
#endif
#endif

#ifndef MOUI_SCREEN_STACK_SIZE
#ifdef CONFIG_MOUI_SCREEN_STACK_SIZE
#define MOUI_SCREEN_STACK_SIZE     CONFIG_MOUI_SCREEN_STACK_SIZE
#else
#define MOUI_SCREEN_STACK_SIZE     8
#endif
#endif

#ifndef MOUI_ANIM_POOL_SIZE
#ifdef CONFIG_MOUI_ANIM_POOL_SIZE
#define MOUI_ANIM_POOL_SIZE        CONFIG_MOUI_ANIM_POOL_SIZE
#else
#define MOUI_ANIM_POOL_SIZE        16
#endif
#endif

#ifndef MOUI_INPUT_QUEUE_SIZE
#ifdef CONFIG_MOUI_INPUT_QUEUE_SIZE
#define MOUI_INPUT_QUEUE_SIZE      CONFIG_MOUI_INPUT_QUEUE_SIZE
#else
#define MOUI_INPUT_QUEUE_SIZE      8
#endif
#endif

/*
 * Input queue lock: the queue is SPSC but may be pushed from ISR (encoder/
 * button GPIO) and from the main loop / button task concurrently. On ESP-IDF
 * use a FreeRTOS portMUX so push/pop stay ISR-safe; on desktop no-op.
 */
#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#define MOUI_LOCK_T             portMUX_TYPE
#define MOUI_LOCK_INIT(l)       do { (l) = (portMUX_TYPE)portMUX_INITIALIZER_UNLOCKED; } while (0)
#define MOUI_LOCK_ENTER(l)      portENTER_CRITICAL(&(l))
#define MOUI_LOCK_EXIT(l)       portEXIT_CRITICAL(&(l))
#else
#define MOUI_LOCK_T             int
#define MOUI_LOCK_INIT(l)       do { (l) = 0; } while (0)
#define MOUI_LOCK_ENTER(l)      (void)(l)
#define MOUI_LOCK_EXIT(l)       (void)(l)
#endif

#ifndef MOUI_EVT_MAX_HANDLERS
#ifdef CONFIG_MOUI_EVT_MAX_HANDLERS
#define MOUI_EVT_MAX_HANDLERS      CONFIG_MOUI_EVT_MAX_HANDLERS
#else
#define MOUI_EVT_MAX_HANDLERS      16
#endif
#endif

#ifndef MOUI_CLIP_STACK_DEPTH
#ifdef CONFIG_MOUI_CLIP_STACK_DEPTH
#define MOUI_CLIP_STACK_DEPTH      CONFIG_MOUI_CLIP_STACK_DEPTH
#else
#define MOUI_CLIP_STACK_DEPTH      8
#endif
#endif

#ifndef MOUI_CONTAINER_MAX_CHILDREN
#ifdef CONFIG_MOUI_CONTAINER_MAX_CHILDREN
#define MOUI_CONTAINER_MAX_CHILDREN CONFIG_MOUI_CONTAINER_MAX_CHILDREN
#else
#define MOUI_CONTAINER_MAX_CHILDREN 16
#endif
#endif

/* ── Widget limits ── */

#ifndef MOUI_SCROLL_MAX_CHILDREN
#define MOUI_SCROLL_MAX_CHILDREN   16
#endif
#ifndef MOUI_TABLE_MAX_COLS
#define MOUI_TABLE_MAX_COLS        6
#endif
#ifndef MOUI_TABLE_MAX_ROWS
#define MOUI_TABLE_MAX_ROWS        16
#endif
#ifndef MOUI_RADIO_MAX_OPTIONS
#define MOUI_RADIO_MAX_OPTIONS     8
#endif
#ifndef MOUI_TAB_MAX_PAGES
#define MOUI_TAB_MAX_PAGES         6
#endif
#ifndef MOUI_DROPDOWN_MAX_OPTIONS
#define MOUI_DROPDOWN_MAX_OPTIONS  12
#endif
#ifndef MOUI_TEXTINPUT_MAX_LEN
#define MOUI_TEXTINPUT_MAX_LEN     32
#endif
#ifndef MOUI_BTNMATRIX_MAX_BTNS
#define MOUI_BTNMATRIX_MAX_BTNS    48
#endif

/* ── QR Code ── */

#ifndef MOUI_QR_MAX_VERSION
#ifdef CONFIG_MOUI_QR_MAX_VERSION
#define MOUI_QR_MAX_VERSION        CONFIG_MOUI_QR_MAX_VERSION
#else
#define MOUI_QR_MAX_VERSION        6
#endif
#endif

/* ── Debug ── */

#ifndef MOUI_DEBUG_BOUNDS
#ifdef CONFIG_MOUI_DEBUG_BOUNDS
#define MOUI_DEBUG_BOUNDS          1
#else
#define MOUI_DEBUG_BOUNDS          0
#endif
#endif

#ifndef MOUI_LOG_ENABLE
#ifdef CONFIG_MOUI_LOG_ENABLE
#define MOUI_LOG_ENABLE            1
#else
#define MOUI_LOG_ENABLE            0
#endif
#endif

#endif
