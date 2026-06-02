#ifndef TST_BOARD_H
#define TST_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui.h"

/* Board power / SPI init. Call once before tst_display_init. */
int tst_board_init(void);

/* SSD1677 800x480 -> portrait 480x800 backend. NULL on failure. */
moui_backend_t *tst_display_init(void);

/* Force next flush to do a clean full refresh. */
void tst_display_force_full(void);

/* Partial-refresh mode (fast key navigation). */
void tst_display_set_partial(bool force_partial);

/* 7-key input: wire presses into a moui input queue (0 = OK). */
int tst_keys_init(moui_input_queue_t *queue);

/* Poll keys once (call from the main loop every ~10 ms). */
void tst_keys_poll(void);

#ifdef __cplusplus
}
#endif
#endif
