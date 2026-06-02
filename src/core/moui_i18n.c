#include "moui_i18n.h"
#include <stddef.h>

static moui_lang_t s_lang = MOUI_LANG_EN;

static const char *s_strings_en[MOUI_STR_MAX] = {
    [MOUI_STR_OK]              = "OK",
    [MOUI_STR_CANCEL]          = "Cancel",
    [MOUI_STR_YES]             = "Yes",
    [MOUI_STR_NO]              = "No",
    [MOUI_STR_BACK]            = "Back",
    [MOUI_STR_SAVE]            = "Save",
    [MOUI_STR_SETTINGS]        = "Settings",
    [MOUI_STR_ABOUT]           = "About",
    [MOUI_STR_DASHBOARD]       = "Dashboard",
    [MOUI_STR_BRIGHTNESS]      = "Brightness",
    [MOUI_STR_CONTRAST]        = "Contrast",
    [MOUI_STR_REFRESH_RATE]    = "Refresh Rate",
    [MOUI_STR_WIFI]            = "WiFi",
    [MOUI_STR_BLUETOOTH]       = "Bluetooth",
    [MOUI_STR_LED]             = "LED",
    [MOUI_STR_AUTO_SLEEP]      = "Auto Sleep",
    [MOUI_STR_INVERT_DISPLAY]  = "Invert Display",
    [MOUI_STR_TEMPERATURE]     = "Temperature",
    [MOUI_STR_HUMIDITY]        = "Humidity",
    [MOUI_STR_BATTERY]         = "Battery",
    [MOUI_STR_UPTIME]          = "Uptime",
    [MOUI_STR_LANGUAGE]        = "Language",
    [MOUI_STR_CONFIRM_EXIT]    = "Return to menu?",
    [MOUI_STR_SAVED]           = "Settings saved!",
    [MOUI_STR_VERSION]         = "Version",
};

static const char *s_strings_zh[MOUI_STR_MAX] = {
    [MOUI_STR_OK]              = "\xe7\xa1\xae\xe5\xae\x9a",
    [MOUI_STR_CANCEL]          = "\xe5\x8f\x96\xe6\xb6\x88",
    [MOUI_STR_YES]             = "\xe6\x98\xaf",
    [MOUI_STR_NO]              = "\xe5\x90\xa6",
    [MOUI_STR_BACK]            = "\xe8\xbf\x94\xe5\x9b\x9e",
    [MOUI_STR_SAVE]            = "\xe4\xbf\x9d\xe5\xad\x98",
    [MOUI_STR_SETTINGS]        = "\xe8\xae\xbe\xe7\xbd\xae",
    [MOUI_STR_ABOUT]           = "\xe5\x85\xb3\xe4\xba\x8e",
    [MOUI_STR_DASHBOARD]       = "\xe4\xbb\xaa\xe8\xa1\xa8\xe7\x9b\x98",
    [MOUI_STR_BRIGHTNESS]      = "\xe4\xba\xae\xe5\xba\xa6",
    [MOUI_STR_CONTRAST]        = "\xe5\xaf\xb9\xe6\xaf\x94\xe5\xba\xa6",
    [MOUI_STR_REFRESH_RATE]    = "\xe5\x88\xb7\xe6\x96\xb0\xe7\x8e\x87",
    [MOUI_STR_WIFI]            = "WiFi",
    [MOUI_STR_BLUETOOTH]       = "\xe8\x93\x9d\xe7\x89\x99",
    [MOUI_STR_LED]             = "LED",
    [MOUI_STR_AUTO_SLEEP]      = "\xe8\x87\xaa\xe5\x8a\xa8\xe4\xbc\x91\xe7\x9c\xa0",
    [MOUI_STR_INVERT_DISPLAY]  = "\xe5\x8f\x8d\xe8\x89\xb2\xe6\x98\xbe\xe7\xa4\xba",
    [MOUI_STR_TEMPERATURE]     = "\xe6\xb8\xa9\xe5\xba\xa6",
    [MOUI_STR_HUMIDITY]        = "\xe6\xb9\xbf\xe5\xba\xa6",
    [MOUI_STR_BATTERY]         = "\xe7\x94\xb5\xe6\xb1\xa0",
    [MOUI_STR_UPTIME]          = "\xe8\xbf\x90\xe8\xa1\x8c\xe6\x97\xb6\xe9\x97\xb4",
    [MOUI_STR_LANGUAGE]        = "\xe8\xaf\xad\xe8\xa8\x80",
    [MOUI_STR_CONFIRM_EXIT]    = "\xe8\xbf\x94\xe5\x9b\x9e\xe4\xb8\xbb\xe8\x8f\x9c\xe5\x8d\x95\xef\xbc\x9f",
    [MOUI_STR_SAVED]           = "\xe8\xae\xbe\xe7\xbd\xae\xe5\xb7\xb2\xe4\xbf\x9d\xe5\xad\x98",
    [MOUI_STR_VERSION]         = "\xe7\x89\x88\xe6\x9c\xac",
};

static const char **s_string_tables[MOUI_LANG_MAX] = {
    [MOUI_LANG_EN] = s_strings_en,
    [MOUI_LANG_ZH] = s_strings_zh,
};

void moui_i18n_set_lang(moui_lang_t lang)
{
    if (lang < MOUI_LANG_MAX) s_lang = lang;
}

moui_lang_t moui_i18n_get_lang(void)
{
    return s_lang;
}

const char *moui_i18n_get(moui_str_id_t id)
{
    if (id >= MOUI_STR_MAX) return "?";
    const char *str = s_string_tables[s_lang][id];
    if (!str) str = s_strings_en[id];
    if (!str) return "?";
    return str;
}
