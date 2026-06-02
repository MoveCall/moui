#ifndef MOUI_I18N_H
#define MOUI_I18N_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    MOUI_LANG_EN = 0,
    MOUI_LANG_ZH,
    MOUI_LANG_MAX,
} moui_lang_t;

typedef enum {
    MOUI_STR_OK = 0,
    MOUI_STR_CANCEL,
    MOUI_STR_YES,
    MOUI_STR_NO,
    MOUI_STR_BACK,
    MOUI_STR_SAVE,
    MOUI_STR_SETTINGS,
    MOUI_STR_ABOUT,
    MOUI_STR_DASHBOARD,
    MOUI_STR_BRIGHTNESS,
    MOUI_STR_CONTRAST,
    MOUI_STR_REFRESH_RATE,
    MOUI_STR_WIFI,
    MOUI_STR_BLUETOOTH,
    MOUI_STR_LED,
    MOUI_STR_AUTO_SLEEP,
    MOUI_STR_INVERT_DISPLAY,
    MOUI_STR_TEMPERATURE,
    MOUI_STR_HUMIDITY,
    MOUI_STR_BATTERY,
    MOUI_STR_UPTIME,
    MOUI_STR_LANGUAGE,
    MOUI_STR_CONFIRM_EXIT,
    MOUI_STR_SAVED,
    MOUI_STR_VERSION,
    MOUI_STR_MAX,
} moui_str_id_t;

void moui_i18n_set_lang(moui_lang_t lang);
moui_lang_t moui_i18n_get_lang(void);
const char *moui_i18n_get(moui_str_id_t id);

#define _S(id) moui_i18n_get(id)


#ifdef __cplusplus
}
#endif
#endif
