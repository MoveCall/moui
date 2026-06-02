#ifndef MOUI_ICONS_H
#define MOUI_ICONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "moui_draw.h"

#define MOUI_ICON_W 16
#define MOUI_ICON_H 16

typedef enum {
    MOUI_ICON_MENU,
    MOUI_ICON_CLOSE,
    MOUI_ICON_BACK,
    MOUI_ICON_FORWARD,
    MOUI_ICON_REFRESH,
    MOUI_ICON_MORE_VERT,
    MOUI_ICON_MORE_HORIZ,
    MOUI_ICON_EXPAND_MORE,
    MOUI_ICON_EXPAND_LESS,
    MOUI_ICON_FULLSCREEN,
    MOUI_ICON_ARROW_UP,
    MOUI_ICON_ARROW_DOWN,
    MOUI_ICON_ARROW_LEFT,
    MOUI_ICON_ARROW_RIGHT,
    MOUI_ICON_CHECK,
    MOUI_ICON_ADD,
    MOUI_ICON_REMOVE,
    MOUI_ICON_DELETE,
    MOUI_ICON_EDIT,
    MOUI_ICON_SAVE,
    MOUI_ICON_COPY,
    MOUI_ICON_SHARE,
    MOUI_ICON_SEARCH,
    MOUI_ICON_FILTER,
    MOUI_ICON_SORT,
    MOUI_ICON_DONE_ALL,
    MOUI_ICON_WIFI,
    MOUI_ICON_WIFI_OFF,
    MOUI_ICON_BLUETOOTH,
    MOUI_ICON_SIGNAL,
    MOUI_ICON_AIRPLANE,
    MOUI_ICON_BATTERY_FULL,
    MOUI_ICON_BATTERY_LOW,
    MOUI_ICON_BATTERY_CHARGE,
    MOUI_ICON_FLASH_ON,
    MOUI_ICON_FLASH_OFF,
    MOUI_ICON_GPS,
    MOUI_ICON_NFC,
    MOUI_ICON_PLAY,
    MOUI_ICON_PAUSE,
    MOUI_ICON_STOP,
    MOUI_ICON_SKIP_NEXT,
    MOUI_ICON_SKIP_PREV,
    MOUI_ICON_VOLUME_UP,
    MOUI_ICON_VOLUME_OFF,
    MOUI_ICON_MUSIC,
    MOUI_ICON_SETTINGS,
    MOUI_ICON_HOME,
    MOUI_ICON_POWER,
    MOUI_ICON_USB,
    MOUI_ICON_SD_CARD,
    MOUI_ICON_MEMORY,
    MOUI_ICON_SPEED,
    MOUI_ICON_BRIGHTNESS,
    MOUI_ICON_DARK_MODE,
    MOUI_ICON_RESTART,
    MOUI_ICON_NOTIFICATION,
    MOUI_ICON_WARNING,
    MOUI_ICON_ERROR,
    MOUI_ICON_INFO,
    MOUI_ICON_HELP,
    MOUI_ICON_LOCK,
    MOUI_ICON_UNLOCK,
    MOUI_ICON_VERIFIED,
    MOUI_ICON_CLOUD,
    MOUI_ICON_TEMP,
    MOUI_ICON_WATER_DROP,
    MOUI_ICON_AIR,
    MOUI_ICON_SUNNY,
    MOUI_ICON_NIGHT,
    MOUI_ICON_TIMER,
    MOUI_ICON_ALARM,
    MOUI_ICON_SCHEDULE,
    MOUI_ICON_UPDATE,
    MOUI_ICON_MESSAGE,
    MOUI_ICON_EMAIL,
    MOUI_ICON_PHONE,
    MOUI_ICON_PERSON,
    MOUI_ICON_FOLDER,
    MOUI_ICON_FILE,
    MOUI_ICON_DOWNLOAD,
    MOUI_ICON_UPLOAD,
    MOUI_ICON_BOOK_OPEN,
    MOUI_ICON_BOOKMARK,
    MOUI_ICON_LIBRARY,
    MOUI_ICON_CHEVRON_UP,
    MOUI_ICON_CHEVRON_DOWN,
    MOUI_ICON_MOON,
    MOUI_ICON_SUN,
    MOUI_ICON_POWER_OFF,
    MOUI_ICON_PLUG,
    MOUI_ICON_TEXT_SIZE,
    MOUI_ICON_BOOK,
    MOUI_ICON_BRIGHTNESS_LOW,
    MOUI_ICON_FONT_DOWNLOAD,
    MOUI_ICON_SWAP_HORIZ,
    MOUI_ICON_LIST_ALT,
    MOUI_ICON_FACE_SMILE,
    MOUI_ICON_THUMBS_UP,
    MOUI_ICON_HEART,
    MOUI_ICON_ROCKET,
    MOUI_ICON_STAR,
    MOUI_ICON_CPU,
    MOUI_ICON_SHIELD,
    MOUI_ICON_EYE,
    MOUI_ICON_EYE_OFF,
    MOUI_ICON_KEYBOARD,
    MOUI_ICON_BT_CONNECTED,
    MOUI_ICON_BT_SEARCH,
    MOUI_ICON_ROUTER,
    MOUI_ICON_CLOUD_DL,
    MOUI_ICON_CLOUD_UL,
    MOUI_ICON_SHUTDOWN,
    MOUI_ICON_BATTERY_CHG_FULL,
    MOUI_ICON_VOLUME_DOWN,
    MOUI_ICON_MIC,
    MOUI_ICON_MIC_OFF,
    MOUI_ICON_CAMERA,
    MOUI_ICON_IMAGE,
    MOUI_ICON_VIDEO,
    MOUI_ICON_LIGHTBULB,
    MOUI_ICON_FAN,
    MOUI_ICON_THERMOMETER,
    MOUI_ICON_HUMIDITY,
    MOUI_ICON_COMPASS,
    MOUI_ICON_PIN,
    MOUI_ICON_QRCODE,
    MOUI_ICON_COUNT
} moui_icon_id_t;

extern const uint8_t moui_icon_menu[];
extern const uint8_t moui_icon_close[];
extern const uint8_t moui_icon_back[];
extern const uint8_t moui_icon_forward[];
extern const uint8_t moui_icon_refresh[];
extern const uint8_t moui_icon_more_vert[];
extern const uint8_t moui_icon_more_horiz[];
extern const uint8_t moui_icon_expand_more[];
extern const uint8_t moui_icon_expand_less[];
extern const uint8_t moui_icon_fullscreen[];
extern const uint8_t moui_icon_arrow_up[];
extern const uint8_t moui_icon_arrow_down[];
extern const uint8_t moui_icon_arrow_left[];
extern const uint8_t moui_icon_arrow_right[];
extern const uint8_t moui_icon_check[];
extern const uint8_t moui_icon_add[];
extern const uint8_t moui_icon_remove[];
extern const uint8_t moui_icon_delete[];
extern const uint8_t moui_icon_edit[];
extern const uint8_t moui_icon_save[];
extern const uint8_t moui_icon_copy[];
extern const uint8_t moui_icon_share[];
extern const uint8_t moui_icon_search[];
extern const uint8_t moui_icon_filter[];
extern const uint8_t moui_icon_sort[];
extern const uint8_t moui_icon_done_all[];
extern const uint8_t moui_icon_wifi[];
extern const uint8_t moui_icon_wifi_off[];
extern const uint8_t moui_icon_bluetooth[];
extern const uint8_t moui_icon_signal[];
extern const uint8_t moui_icon_airplane[];
extern const uint8_t moui_icon_battery_full[];
extern const uint8_t moui_icon_battery_low[];
extern const uint8_t moui_icon_battery_charge[];
extern const uint8_t moui_icon_flash_on[];
extern const uint8_t moui_icon_flash_off[];
extern const uint8_t moui_icon_gps[];
extern const uint8_t moui_icon_nfc[];
extern const uint8_t moui_icon_play[];
extern const uint8_t moui_icon_pause[];
extern const uint8_t moui_icon_stop[];
extern const uint8_t moui_icon_skip_next[];
extern const uint8_t moui_icon_skip_prev[];
extern const uint8_t moui_icon_volume_up[];
extern const uint8_t moui_icon_volume_off[];
extern const uint8_t moui_icon_music[];
extern const uint8_t moui_icon_settings[];
extern const uint8_t moui_icon_home[];
extern const uint8_t moui_icon_power[];
extern const uint8_t moui_icon_usb[];
extern const uint8_t moui_icon_sd_card[];
extern const uint8_t moui_icon_memory[];
extern const uint8_t moui_icon_speed[];
extern const uint8_t moui_icon_brightness[];
extern const uint8_t moui_icon_dark_mode[];
extern const uint8_t moui_icon_restart[];
extern const uint8_t moui_icon_notification[];
extern const uint8_t moui_icon_warning[];
extern const uint8_t moui_icon_error[];
extern const uint8_t moui_icon_info[];
extern const uint8_t moui_icon_help[];
extern const uint8_t moui_icon_lock[];
extern const uint8_t moui_icon_unlock[];
extern const uint8_t moui_icon_verified[];
extern const uint8_t moui_icon_cloud[];
extern const uint8_t moui_icon_temp[];
extern const uint8_t moui_icon_water_drop[];
extern const uint8_t moui_icon_air[];
extern const uint8_t moui_icon_sunny[];
extern const uint8_t moui_icon_night[];
extern const uint8_t moui_icon_timer[];
extern const uint8_t moui_icon_alarm[];
extern const uint8_t moui_icon_schedule[];
extern const uint8_t moui_icon_update[];
extern const uint8_t moui_icon_message[];
extern const uint8_t moui_icon_email[];
extern const uint8_t moui_icon_phone[];
extern const uint8_t moui_icon_person[];
extern const uint8_t moui_icon_folder[];
extern const uint8_t moui_icon_file[];
extern const uint8_t moui_icon_download[];
extern const uint8_t moui_icon_upload[];

extern const uint8_t moui_icon_book_open[];
extern const uint8_t moui_icon_bookmark[];
extern const uint8_t moui_icon_library[];
extern const uint8_t moui_icon_chevron_up[];
extern const uint8_t moui_icon_chevron_down[];
extern const uint8_t moui_icon_moon[];
extern const uint8_t moui_icon_sun[];
extern const uint8_t moui_icon_power_off[];
extern const uint8_t moui_icon_plug[];
extern const uint8_t moui_icon_text_size[];
extern const uint8_t moui_icon_book[];
extern const uint8_t moui_icon_brightness_low[];
extern const uint8_t moui_icon_font_download[];
extern const uint8_t moui_icon_swap_horiz[];
extern const uint8_t moui_icon_list_alt[];
extern const uint8_t moui_icon_face_smile[];
extern const uint8_t moui_icon_thumbs_up[];
extern const uint8_t moui_icon_heart[];
extern const uint8_t moui_icon_rocket[];
extern const uint8_t moui_icon_star[];
extern const uint8_t moui_icon_cpu[];
extern const uint8_t moui_icon_shield[];
extern const uint8_t moui_icon_eye[];
extern const uint8_t moui_icon_eye_off[];
extern const uint8_t moui_icon_keyboard[];
extern const uint8_t moui_icon_bt_connected[];
extern const uint8_t moui_icon_bt_search[];
extern const uint8_t moui_icon_router[];
extern const uint8_t moui_icon_cloud_dl[];
extern const uint8_t moui_icon_cloud_ul[];
extern const uint8_t moui_icon_shutdown[];
extern const uint8_t moui_icon_battery_chg_full[];
extern const uint8_t moui_icon_volume_down[];
extern const uint8_t moui_icon_mic[];
extern const uint8_t moui_icon_mic_off[];
extern const uint8_t moui_icon_camera[];
extern const uint8_t moui_icon_image[];
extern const uint8_t moui_icon_video[];
extern const uint8_t moui_icon_lightbulb[];
extern const uint8_t moui_icon_fan[];
extern const uint8_t moui_icon_thermometer[];
extern const uint8_t moui_icon_humidity[];
extern const uint8_t moui_icon_compass[];
extern const uint8_t moui_icon_pin[];
extern const uint8_t moui_icon_qrcode[];

void moui_draw_icon(moui_draw_ctx_t *ctx, int x, int y, moui_icon_id_t id, moui_color_t color);
void moui_draw_icon_scaled(moui_draw_ctx_t *ctx, int x, int y, moui_icon_id_t id, uint8_t scale, moui_color_t color);
void moui_draw_icon_in_rect(moui_draw_ctx_t *ctx, const moui_rect_t *rect, moui_icon_id_t id, moui_color_t color);
const char *moui_icon_get_name(moui_icon_id_t id);

#ifdef __cplusplus
}
#endif
#endif
