/*
 * Moui Example: Sensor Dashboard
 *
 * Shows how to build a multi-sensor monitoring panel
 * with charts, sparklines, and status indicators.
 *
 * Build: link with moui_framework + moui_fonts
 */

#include "moui.h"
#include <math.h>

static moui_screen_t          scr;
static moui_widget_label_t    title;
static moui_widget_chart_t    temp_chart;
static moui_widget_sparkline_t humi_spark;
static moui_widget_number_t   temp_num;
static moui_widget_ring_t     cpu_ring;
static moui_widget_dots_t     page_dots;
static moui_widget_label_t    lbl_temp, lbl_humi, lbl_cpu;

static float sim_t = 0;

static void tick(moui_screen_t *s, uint32_t now_ms, float dt)
{
    (void)s; (void)dt; (void)now_ms;
    sim_t += 0.05f;

    float temp = 23.0f + sinf(sim_t * 0.5f) * 4.0f;
    float humi = 55.0f + cosf(sim_t * 0.3f) * 10.0f;
    int cpu = (int)(50 + sinf(sim_t * 1.2f) * 40);

    temp_num.value = (int32_t)(temp * 10);
    moui_chart_push(&temp_chart, (int16_t)(temp * 10));
    moui_sparkline_push(&humi_spark, (int16_t)humi);
    cpu_ring.value = cpu;
}

void setup_sensor_panel(moui_screen_mgr_t *mgr)
{
    moui_screen_init(&scr);
    scr.on_tick = tick;
    int y = 0;

    /* Title bar */
    moui_label_init(&title, "Sensor Panel", &moui_font_ascii_6x8);
    title.inverted = true;
    title.base.bounds = (moui_rect_t){0, y, MOUI_DISP_W, 14};
    moui_screen_add_widget(&scr, &title.base);
    y += 16;

    /* Temperature number + chart */
    moui_label_init(&lbl_temp, "Temp", &moui_font_ascii_6x8);
    lbl_temp.base.bounds = (moui_rect_t){4, y, 40, 10};
    moui_screen_add_widget(&scr, &lbl_temp.base);

    moui_number_init(&temp_num, &moui_font_inter_16, 3);
    temp_num.unit = "C";
    temp_num.leading_zero = false;
    temp_num.base.bounds = (moui_rect_t){50, y, 80, 20};
    moui_screen_add_widget(&scr, &temp_num.base);
    y += 22;

    moui_chart_init(&temp_chart, 190, 270);
    temp_chart.base.bounds = (moui_rect_t){4, y, 160, 50};
    temp_chart.show_frame = true;
    moui_screen_add_widget(&scr, &temp_chart.base);
    y += 54;

    /* Humidity sparkline */
    moui_label_init(&lbl_humi, "Humi", &moui_font_ascii_6x8);
    lbl_humi.base.bounds = (moui_rect_t){4, y, 40, 10};
    moui_screen_add_widget(&scr, &lbl_humi.base);

    moui_sparkline_init(&humi_spark, 30, 80);
    humi_spark.base.bounds = (moui_rect_t){50, y, 110, 20};
    moui_screen_add_widget(&scr, &humi_spark.base);
    y += 24;

    /* CPU ring */
    moui_label_init(&lbl_cpu, "CPU", &moui_font_ascii_6x8);
    lbl_cpu.base.bounds = (moui_rect_t){4, y, 30, 10};
    moui_screen_add_widget(&scr, &lbl_cpu.base);

    moui_ring_init(&cpu_ring, &moui_font_ascii_6x8, 100);
    cpu_ring.base.bounds = (moui_rect_t){40, y, 60, 60};
    moui_screen_add_widget(&scr, &cpu_ring.base);

    /* Page dots */
    moui_dots_init(&page_dots, 3);
    page_dots.active = 1;
    page_dots.base.bounds = (moui_rect_t){4, 370, 160, 10};
    moui_screen_add_widget(&scr, &page_dots.base);

    moui_screen_push(mgr, &scr);
}

/*
 * Usage:
 *   setup_sensor_panel(&mgr);
 *   // then run mgr tick loop
 */
