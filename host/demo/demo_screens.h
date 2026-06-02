#ifndef DEMO_SCREENS_H
#define DEMO_SCREENS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "moui.h"

void demo_init(moui_screen_mgr_t *mgr);
void demo_setup_all_screens(void);
moui_screen_t *demo_get_main_screen(void);

#ifdef __cplusplus
}
#endif

#endif
