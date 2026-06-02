/*
 * 7-key input for the OnePage C61 (3 side GPIO keys + 4 front ADC-ladder
 * keys), self-contained polling driver — no external button library.
 *
 *   side keys: WAKE=GPIO2  PREV=GPIO6  NEXT=GPIO9 (active-low)
 *   front ADC ladder: GPIO4 = ADC1_CH2
 *
 * Key mapping:
 *   WAKE=BACK  PREV=UP  NEXT=DOWN  BACK=BACK  LEFT=LEFT  RIGHT=RIGHT  ENTER=ENTER
 */

#include "tst_board.h"

#include <stdint.h>
#include <string.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "input/moui_input.h"
#include "input/moui_indev.h"

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define GPIO_WAKE 2
#define GPIO_PREV 6
#define GPIO_NEXT 9
#define ADC_UNIT  ADC_UNIT_1
#define ADC_CH    ADC_CHANNEL_2   // GPIO4

#define POLL_MS      8             // poll period
#define DEBOUNCE_MS  24            // 3 samples
#define NAV_DEBOUNCE 400           // min ms between two events for the same key (matched to 0.4s physical EPD partial refresh)

typedef enum {
    KEY_WAKE = 0, KEY_PREV, KEY_NEXT, KEY_BACK, KEY_LEFT, KEY_RIGHT, KEY_ENTER,
    KEY_COUNT,
} tst_key_t;

/* Front ADC-ladder thresholds (mV), centered around user physical measurements:
 * ENTER: 2mV (band 0..300mV)
 * RIGHT: 1339mV (band 1100..1600mV)
 * LEFT:  1989mV (band 1700..2200mV)
 * BACK:  2637mV (band 2300..2800mV)
 * REST:  ~3100mV (dead zone >2800mV)
 */
typedef struct { tst_key_t key; uint16_t min_mv, max_mv; } adc_key_t;
static const adc_key_t s_adc_keys[] = {
    { KEY_ENTER,    0,  300 },
    { KEY_RIGHT, 1100, 1600 },
    { KEY_LEFT,  1700, 2200 },
    { KEY_BACK,  2300, 2800 },
};

static const moui_key_t s_key_map[KEY_COUNT] = {
    [KEY_WAKE]  = MOUI_KEY_BACK,
    [KEY_PREV]  = MOUI_KEY_UP,
    [KEY_NEXT]  = MOUI_KEY_DOWN,
    [KEY_BACK]  = MOUI_KEY_BACK,
    [KEY_LEFT]  = MOUI_KEY_LEFT,
    [KEY_RIGHT] = MOUI_KEY_RIGHT,
    [KEY_ENTER] = MOUI_KEY_ENTER,
};

static const char *s_key_names[KEY_COUNT] = {
    "WAKE", "PREV", "NEXT", "BACK", "LEFT", "RIGHT", "ENTER",
};

static moui_input_queue_t *s_queue;
static adc_oneshot_unit_handle_t s_adc;
static adc_cali_handle_t s_cali;
static bool s_cali_ok = false;

static bool  s_prev[KEY_COUNT];      // debounced previous state (pressed?)
static int   s_stable[KEY_COUNT];    // consecutive stable samples
static int64_t s_last_us[KEY_COUNT];
static int64_t s_start_us;

static moui_event_type_t key_to_ev(moui_key_t k)
{
    switch (k) {
    case MOUI_KEY_UP:
    case MOUI_KEY_LEFT:  return MOUI_EV_ENCODER_CCW;
    case MOUI_KEY_DOWN:
    case MOUI_KEY_RIGHT: return MOUI_EV_ENCODER_CW;
    case MOUI_KEY_ENTER: return MOUI_EV_ENCODER_PRESS;
    case MOUI_KEY_BACK:
    case MOUI_KEY_HOME:  return MOUI_EV_ENCODER_BACK;
    default:             return MOUI_EV_NONE;
    }
}

static int read_adc_mv(void)
{
    int raw = 0;
    if (adc_oneshot_read(s_adc, ADC_CH, &raw) != ESP_OK) return -1;
    int mv = (raw * 3300) / 4095;
    if (s_cali_ok) {
        adc_cali_raw_to_voltage(s_cali, raw, &mv);
    }
    return mv;
}

static void fire(tst_key_t key)
{
    if (!s_queue) return;
    moui_event_type_t t = key_to_ev(s_key_map[key]);
    if (t == MOUI_EV_NONE) return;

    /* Re-verify ENTER key to filter transient ADC glitches (matching official bsp_onepage_c61) */
    if (key == KEY_ENTER) {
        int mv = read_adc_mv();
        if (mv < 0 || mv > 400) {
            ESP_LOGW("tst_keys", "ENTER rejected (adc=%dmV, node not pressed)", mv);
            return;
        }
    }

    int64_t now = esp_timer_get_time();
    if (now - s_last_us[key] < NAV_DEBOUNCE * 1000) return;
    s_last_us[key] = now;

    ESP_LOGI("tst_keys", "nav: %s", s_key_names[key]);
    moui_input_event_t e = { .type = t };
    moui_input_queue_push(s_queue, &e);
}

static tst_key_t read_adc_key(void)
{
    int mv = read_adc_mv();
    if (mv < 0) return KEY_COUNT;

    static int s_last_logged_mv = -1;
    if (mv < 2850 && (mv < s_last_logged_mv - 50 || mv > s_last_logged_mv + 50)) {
        s_last_logged_mv = mv;
        ESP_LOGI("adc_debug", "Front key pressed: ADC voltage = %d mV", mv);
    } else if (mv >= 2850) {
        s_last_logged_mv = mv;
    }

    for (size_t i = 0; i < sizeof(s_adc_keys) / sizeof(s_adc_keys[0]); i++) {
        if (mv >= (int)s_adc_keys[i].min_mv && mv <= (int)s_adc_keys[i].max_mv)
            return s_adc_keys[i].key;
    }
    return KEY_COUNT;
}

static void tst_keys_poll_internal(void)
{
    /* 3 GPIO side keys (active-low) */
    for (size_t i = 0; i < 3; i++) {
        static const int gpios[3] = { GPIO_WAKE, GPIO_PREV, GPIO_NEXT };
        static const tst_key_t keys[3] = { KEY_WAKE, KEY_PREV, KEY_NEXT };
        bool pressed = (gpio_get_level(gpios[i]) == 0);
        bool raw = pressed;
        if (raw != s_prev[keys[i]]) {
            s_stable[keys[i]]++;
            if (s_stable[keys[i]] >= 2) {
                s_stable[keys[i]] = 0;
                s_prev[keys[i]] = raw;
                if (raw) fire(keys[i]);
            }
        } else {
            s_stable[keys[i]] = 0;
        }
    }

    /* Front ADC ladder keys (2 stable samples = 16ms response) */
    tst_key_t k = read_adc_key();
    for (size_t i = 0; i < sizeof(s_adc_keys) / sizeof(s_adc_keys[0]); i++) {
        tst_key_t ck = s_adc_keys[i].key;
        bool pressed = (k == ck);
        if (pressed != s_prev[ck]) {
            s_stable[ck]++;
            if (s_stable[ck] >= 2) {
                s_stable[ck] = 0;
                s_prev[ck] = pressed;
                if (pressed) fire(ck);
            }
        } else {
            s_stable[ck] = 0;
        }
    }
}

int tst_keys_init(moui_input_queue_t *queue)
{
    s_queue = queue;
    s_start_us = esp_timer_get_time();
    memset(s_prev, 0, sizeof(s_prev));
    memset(s_stable, 0, sizeof(s_stable));

    for (size_t i = 0; i < 3; i++) {
        static const int gpios[3] = { GPIO_WAKE, GPIO_PREV, GPIO_NEXT };
        gpio_config_t g = {
            .pin_bit_mask = 1ULL << gpios[i],
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
        };
        gpio_config(&g);
    }

    adc_oneshot_unit_init_cfg_t ucfg = { .unit_id = ADC_UNIT };
    if (adc_oneshot_new_unit(&ucfg, &s_adc) != ESP_OK) {
        ESP_LOGE("tst_keys", "ADC init failed");
        return -1;
    }
    adc_oneshot_chan_cfg_t ccfg = { .atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_DEFAULT };
    if (adc_oneshot_config_channel(s_adc, ADC_CH, &ccfg) != ESP_OK) {
        ESP_LOGE("tst_keys", "ADC channel failed");
        return -1;
    }

    adc_cali_curve_fitting_config_t cal = {
        .unit_id = ADC_UNIT, .chan = ADC_CH,
        .atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    s_cali_ok = (adc_cali_create_scheme_curve_fitting(&cal, &s_cali) == ESP_OK);
    if (s_cali_ok) {
        ESP_LOGI("tst_keys", "ADC curve-fitting calibration enabled");
    }

    ESP_LOGI("tst_keys", "7 keys ready (polled)");
    return 0;
}

void tst_keys_poll(void)
{
    if (esp_timer_get_time() - s_start_us < 2500000) return;  // ADC settle grace
    tst_keys_poll_internal();
}
