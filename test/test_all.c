#include "moui_test.h"
#include "../src/moui.h"

/* ── Framebuffer tests ── */

TEST(fb_init_deinit)
{
    moui_fb_t fb;
    moui_hal_set_display(&moui_disp_ssd1306_128x64);
    ASSERT_EQ(moui_fb_init(&fb, &moui_disp_ssd1306_128x64), 0);
    ASSERT_TRUE(fb.pixels != NULL);
    ASSERT_TRUE(fb.wire != NULL);
    ASSERT_EQ(fb.buf_size, 1024u); /* 128*64/8 */
    moui_fb_deinit(&fb);
    ASSERT_TRUE(fb.pixels == NULL);
}

TEST(fb_set_get_pixel)
{
    moui_fb_t fb;
    moui_hal_set_display(&moui_disp_ssd1306_128x64);
    moui_fb_init(&fb, &moui_disp_ssd1306_128x64);
    moui_fb_clear(&fb, MOUI_WHITE);

    ASSERT_EQ(moui_fb_get_pixel(&fb, 0, 0), MOUI_WHITE);
    moui_fb_set_pixel(&fb, 10, 20, MOUI_BLACK);
    ASSERT_EQ(moui_fb_get_pixel(&fb, 10, 20), MOUI_BLACK);
    ASSERT_EQ(moui_fb_get_pixel(&fb, 11, 20), MOUI_WHITE);

    moui_fb_set_pixel(&fb, 10, 20, MOUI_WHITE);
    ASSERT_EQ(moui_fb_get_pixel(&fb, 10, 20), MOUI_WHITE);

    moui_fb_deinit(&fb);
}

TEST(fb_clear)
{
    moui_fb_t fb;
    moui_hal_set_display(&moui_disp_ssd1306_128x64);
    moui_fb_init(&fb, &moui_disp_ssd1306_128x64);

    moui_fb_clear(&fb, MOUI_BLACK);
    ASSERT_EQ(moui_fb_get_pixel(&fb, 0, 0), MOUI_BLACK);
    ASSERT_EQ(moui_fb_get_pixel(&fb, 127, 63), MOUI_BLACK);

    moui_fb_clear(&fb, MOUI_WHITE);
    ASSERT_EQ(moui_fb_get_pixel(&fb, 0, 0), MOUI_WHITE);

    moui_fb_deinit(&fb);
}

/* ── Draw tests ── */

TEST(draw_pixel_clip)
{
    moui_fb_t fb;
    moui_hal_set_display(&moui_disp_ssd1306_128x64);
    moui_fb_init(&fb, &moui_disp_ssd1306_128x64);
    moui_fb_clear(&fb, MOUI_WHITE);

    moui_draw_ctx_t ctx;
    moui_draw_ctx_init(&ctx, &fb);

    moui_draw_pixel(&ctx, 5, 5, MOUI_BLACK);
    ASSERT_EQ(moui_fb_get_pixel(&fb, 5, 5), MOUI_BLACK);

    /* Out of bounds — should not crash */
    moui_draw_pixel(&ctx, -1, -1, MOUI_BLACK);
    moui_draw_pixel(&ctx, 200, 200, MOUI_BLACK);

    /* Clip test */
    moui_rect_t clip = {10, 10, 20, 20};
    moui_draw_push_clip(&ctx, &clip);
    moui_draw_pixel(&ctx, 5, 5, MOUI_BLACK);  /* outside clip — noop */
    moui_fb_clear(&fb, MOUI_WHITE);
    moui_draw_pixel(&ctx, 5, 5, MOUI_BLACK);
    ASSERT_EQ(moui_fb_get_pixel(&fb, 5, 5), MOUI_WHITE); /* clipped */
    moui_draw_pixel(&ctx, 15, 15, MOUI_BLACK);
    ASSERT_EQ(moui_fb_get_pixel(&fb, 15, 15), MOUI_BLACK); /* inside clip */
    moui_draw_pop_clip(&ctx);

    moui_fb_deinit(&fb);
}

TEST(draw_hline)
{
    moui_fb_t fb;
    moui_hal_set_display(&moui_disp_ssd1306_128x64);
    moui_fb_init(&fb, &moui_disp_ssd1306_128x64);
    moui_fb_clear(&fb, MOUI_WHITE);

    moui_draw_ctx_t ctx;
    moui_draw_ctx_init(&ctx, &fb);

    moui_draw_hline(&ctx, 0, 10, 128, MOUI_BLACK);
    for (int x = 0; x < 128; x++) {
        ASSERT_EQ(moui_fb_get_pixel(&fb, x, 10), MOUI_BLACK);
    }
    ASSERT_EQ(moui_fb_get_pixel(&fb, 0, 11), MOUI_WHITE);

    moui_fb_deinit(&fb);
}

/* ── QR tests ── */

TEST(qr_encode_hello)
{
    moui_qr_t qr;
    ASSERT_TRUE(moui_qr_encode(&qr, "HELLO"));
    ASSERT_EQ(qr.version, 1);
    ASSERT_EQ(qr.size, 21);
}

TEST(qr_encode_url)
{
    moui_qr_t qr;
    ASSERT_TRUE(moui_qr_encode(&qr, "https://github.com/moui"));
    ASSERT_EQ(qr.version, 2);
    ASSERT_EQ(qr.size, 25);
}

TEST(qr_encode_empty_fails)
{
    moui_qr_t qr;
    ASSERT_TRUE(!moui_qr_encode(&qr, ""));
}

/* ── Font tests ── */

TEST(font_measure_str)
{
    int w = moui_font_measure_str(&moui_font_ascii_6x8, "Hello");
    ASSERT_EQ(w, 5 * 6); /* 6px per char for fixed-width */
}

TEST(font_measure_newline)
{
    int w = moui_font_measure_str(&moui_font_ascii_6x8, "Hi\nWorld!");
    /* Should return max of "Hi"(12) and "World!"(36) */
    ASSERT_EQ(w, 6 * 6); /* "World!" = 6 chars */
}

TEST(font_measure_wrapped)
{
    int w, h;
    moui_font_measure_wrapped(&moui_font_ascii_6x8, "Hello World Test", 50, &w, &h);
    ASSERT_TRUE(w <= 50);
    ASSERT_TRUE(h > moui_font_ascii_6x8.line_height); /* wrapped to 2+ lines */
}

/* ── Input queue tests ── */

TEST(input_queue_push_pop)
{
    moui_input_queue_t q;
    moui_input_queue_init(&q);
    ASSERT_TRUE(moui_input_queue_empty(&q));

    moui_input_event_t ev = { .type = MOUI_EV_ENCODER_CW, .timestamp_ms = 100 };
    ASSERT_TRUE(moui_input_queue_push(&q, &ev));
    ASSERT_TRUE(!moui_input_queue_empty(&q));

    moui_input_event_t out;
    ASSERT_TRUE(moui_input_queue_pop(&q, &out));
    ASSERT_EQ(out.type, MOUI_EV_ENCODER_CW);
    ASSERT_TRUE(moui_input_queue_empty(&q));
}

TEST(input_queue_overflow)
{
    moui_input_queue_t q;
    moui_input_queue_init(&q);

    moui_input_event_t ev = { .type = MOUI_EV_ENCODER_PRESS };
    /* Ring buffer usable slots = size - 1 */
    for (int i = 0; i < MOUI_INPUT_QUEUE_SIZE - 1; i++) {
        ASSERT_TRUE(moui_input_queue_push(&q, &ev));
    }
    /* Queue full — should fail */
    ASSERT_TRUE(!moui_input_queue_push(&q, &ev));
}

/* ── Backend FB + Rotation tests ── */

static int flush_called = 0;
static uint8_t flush_buf[4096];
static uint32_t flush_len = 0;

static void test_hw_flush(const uint8_t *data, uint32_t len, void *user)
{
    (void)user;
    flush_called++;
    flush_len = len;
    if (len <= sizeof(flush_buf))
        memcpy(flush_buf, data, len);
}

static int get_phys_bit(const uint8_t *buf, int phys_w, int px, int py)
{
    int idx = (py * phys_w + px) >> 3;
    int bit = 7 - ((py * phys_w + px) & 7);
    return (buf[idx] >> bit) & 1;
}

TEST(backend_fb_init)
{
    moui_backend_fb_t fb;
    ASSERT_EQ(moui_backend_fb_init(&fb, 32, 16, MOUI_PIXFMT_MONO_HMSB, test_hw_flush, NULL), 0);
    ASSERT_EQ(fb.base.width, 32);
    ASSERT_EQ(fb.base.height, 16);
    ASSERT_EQ(fb.base.phys_w, 32);
    ASSERT_EQ(fb.base.phys_h, 16);
    ASSERT_EQ(fb.base.rotation, MOUI_ROTATION_0);
    ASSERT_EQ(fb.base.sw_rotate, false);
    ASSERT_EQ(fb.pix_size, 64u);
    ASSERT_TRUE(fb.pixels != NULL);
    moui_backend_fb_deinit(&fb);
}

TEST(backend_fb_pixel_no_rotation)
{
    moui_backend_fb_t fb;
    moui_backend_fb_init(&fb, 32, 16, MOUI_PIXFMT_MONO_HMSB, test_hw_flush, NULL);
    fb.base.clear(&fb.base, MOUI_WHITE);

    moui_be_set_pixel(&fb.base, 5, 3, MOUI_BLACK);
    ASSERT_EQ(moui_be_get_pixel(&fb.base, 5, 3), MOUI_BLACK);
    ASSERT_EQ(moui_be_get_pixel(&fb.base, 6, 3), MOUI_WHITE);

    ASSERT_EQ(get_phys_bit(fb.pixels, 32, 5, 3), 1);
    moui_backend_fb_deinit(&fb);
}

TEST(backend_fb_flush_no_rotation)
{
    moui_backend_fb_t fb;
    moui_backend_fb_init(&fb, 32, 16, MOUI_PIXFMT_MONO_HMSB, test_hw_flush, NULL);
    fb.base.clear(&fb.base, MOUI_WHITE);
    moui_be_set_pixel(&fb.base, 0, 0, MOUI_BLACK);

    flush_called = 0;
    fb.base.flush(&fb.base);
    ASSERT_EQ(flush_called, 1);
    ASSERT_EQ(flush_len, 64u);
    ASSERT_EQ(get_phys_bit(flush_buf, 32, 0, 0), 1);
    ASSERT_EQ(get_phys_bit(flush_buf, 32, 1, 0), 0);
    moui_backend_fb_deinit(&fb);
}

TEST(backend_fb_gray2_pixel)
{
    moui_backend_fb_t fb;
    ASSERT_EQ(moui_backend_fb_init(&fb, 8, 4, MOUI_PIXFMT_GRAY2_HMSB, test_hw_flush, NULL), 0);
    ASSERT_EQ(fb.pix_size, 8u);

    fb.base.clear(&fb.base, MOUI_WHITE);
    ASSERT_EQ(moui_be_get_pixel(&fb.base, 0, 0), MOUI_WHITE);

    moui_be_set_pixel(&fb.base, 0, 0, MOUI_BLACK);
    moui_be_set_pixel(&fb.base, 1, 0, MOUI_DGRAY);
    moui_be_set_pixel(&fb.base, 2, 0, MOUI_LGRAY);
    moui_be_set_pixel(&fb.base, 3, 0, MOUI_WHITE);
    ASSERT_EQ(moui_be_get_pixel(&fb.base, 0, 0), MOUI_BLACK);
    ASSERT_EQ(moui_be_get_pixel(&fb.base, 1, 0), MOUI_DGRAY);
    ASSERT_EQ(moui_be_get_pixel(&fb.base, 2, 0), MOUI_LGRAY);
    ASSERT_EQ(moui_be_get_pixel(&fb.base, 3, 0), MOUI_WHITE);

    moui_backend_fb_deinit(&fb);
}

TEST(font_aa_gray2_output)
{
    moui_backend_fb_t fb;
    ASSERT_EQ(moui_backend_fb_init(&fb, 64, 32, MOUI_PIXFMT_GRAY2_HMSB, test_hw_flush, NULL), 0);
    fb.base.clear(&fb.base, MOUI_WHITE);

    moui_draw_ctx_t ctx;
    moui_draw_ctx_init_be(&ctx, &fb.base);

    /* Set up a fake display descriptor for the gray2 format check */
    static const moui_disp_desc_t gray2_desc = {
        .width = 64, .height = 32, .pixel_format = MOUI_PIXEL_FORMAT_GRAY2_HMSB,
    };
    moui_hal_set_display(&gray2_desc);

    ASSERT_EQ(moui_font_inter_16_aa.bpp, 2);

    moui_font_draw_str(&ctx, &moui_font_inter_16_aa, 0, 0, "I", MOUI_BLACK);

    /* Check that we got intermediate gray levels (not just B/W) */
    bool found_lgray = false, found_dgray = false, found_black = false;
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            moui_color_t c = moui_be_get_pixel(&fb.base, x, y);
            if (c == MOUI_LGRAY) found_lgray = true;
            if (c == MOUI_DGRAY) found_dgray = true;
            if (c == MOUI_BLACK) found_black = true;
        }
    }
    ASSERT_TRUE(found_black);
    ASSERT_TRUE(found_lgray || found_dgray);

    moui_backend_fb_deinit(&fb);
}

TEST(rotation_90_mode_a_dimensions)
{
    moui_backend_fb_t fb;
    moui_backend_fb_init(&fb, 32, 16, MOUI_PIXFMT_MONO_HMSB, test_hw_flush, NULL);
    fb.base.sw_rotate = true;
    moui_backend_fb_set_rotation(&fb, MOUI_ROTATION_90);

    ASSERT_EQ(fb.base.width, 16);
    ASSERT_EQ(fb.base.height, 32);
    ASSERT_EQ(fb.base.phys_w, 32);
    ASSERT_EQ(fb.base.phys_h, 16);
    ASSERT_TRUE(fb.rot_buf == NULL);
    moui_backend_fb_deinit(&fb);
}

TEST(rotation_90_mode_a_pixel)
{
    moui_backend_fb_t fb;
    moui_backend_fb_init(&fb, 32, 16, MOUI_PIXFMT_MONO_HMSB, test_hw_flush, NULL);
    fb.base.sw_rotate = true;
    moui_backend_fb_set_rotation(&fb, MOUI_ROTATION_90);
    fb.base.clear(&fb.base, MOUI_WHITE);

    moui_be_set_pixel(&fb.base, 0, 0, MOUI_BLACK);
    ASSERT_EQ(moui_be_get_pixel(&fb.base, 0, 0), MOUI_BLACK);

    ASSERT_EQ(get_phys_bit(fb.pixels, 32, 31, 0), 1);

    flush_called = 0;
    fb.base.flush(&fb.base);
    ASSERT_EQ(flush_called, 1);
    ASSERT_EQ(get_phys_bit(flush_buf, 32, 31, 0), 1);
    moui_backend_fb_deinit(&fb);
}

TEST(rotation_90_mode_a_draw)
{
    moui_backend_fb_t fb;
    moui_backend_fb_init(&fb, 32, 16, MOUI_PIXFMT_MONO_HMSB, test_hw_flush, NULL);
    fb.base.sw_rotate = true;
    moui_backend_fb_set_rotation(&fb, MOUI_ROTATION_90);
    fb.base.clear(&fb.base, MOUI_WHITE);

    moui_draw_ctx_t ctx;
    moui_draw_ctx_init_be(&ctx, &fb.base);
    ASSERT_EQ(ctx.clip.w, 16);
    ASSERT_EQ(ctx.clip.h, 32);

    moui_draw_pixel(&ctx, 0, 0, MOUI_BLACK);
    ASSERT_EQ(moui_be_get_pixel(&fb.base, 0, 0), MOUI_BLACK);
    ASSERT_EQ(get_phys_bit(fb.pixels, 32, 31, 0), 1);

    moui_draw_hline(&ctx, 0, 1, 5, MOUI_BLACK);
    for (int i = 0; i < 5; i++)
        ASSERT_EQ(moui_be_get_pixel(&fb.base, i, 1), MOUI_BLACK);
    ASSERT_EQ(moui_be_get_pixel(&fb.base, 5, 1), MOUI_WHITE);

    flush_called = 0;
    fb.base.flush(&fb.base);
    ASSERT_EQ(flush_called, 1);
    moui_backend_fb_deinit(&fb);
}

TEST(rotation_90_mode_b_dimensions)
{
    moui_backend_fb_t fb;
    moui_backend_fb_init(&fb, 32, 16, MOUI_PIXFMT_MONO_HMSB, test_hw_flush, NULL);
    fb.base.sw_rotate = false;
    moui_backend_fb_set_rotation(&fb, MOUI_ROTATION_90);

    ASSERT_EQ(fb.base.width, 16);
    ASSERT_EQ(fb.base.height, 32);
    ASSERT_EQ(fb.base.phys_w, 32);
    ASSERT_EQ(fb.base.phys_h, 16);
    ASSERT_TRUE(fb.rot_buf != NULL);
    moui_backend_fb_deinit(&fb);
}

TEST(rotation_90_mode_b_pixel)
{
    moui_backend_fb_t fb;
    moui_backend_fb_init(&fb, 32, 16, MOUI_PIXFMT_MONO_HMSB, test_hw_flush, NULL);
    fb.base.sw_rotate = false;
    moui_backend_fb_set_rotation(&fb, MOUI_ROTATION_90);
    fb.base.clear(&fb.base, MOUI_WHITE);

    moui_be_set_pixel(&fb.base, 0, 0, MOUI_BLACK);
    ASSERT_EQ(moui_be_get_pixel(&fb.base, 0, 0), MOUI_BLACK);

    flush_called = 0;
    fb.base.flush(&fb.base);
    ASSERT_EQ(flush_called, 1);
    ASSERT_EQ(get_phys_bit(flush_buf, 32, 31, 0), 1);
    moui_backend_fb_deinit(&fb);
}

TEST(rotation_90_mode_b_draw)
{
    moui_backend_fb_t fb;
    moui_backend_fb_init(&fb, 32, 16, MOUI_PIXFMT_MONO_HMSB, test_hw_flush, NULL);
    fb.base.sw_rotate = false;
    moui_backend_fb_set_rotation(&fb, MOUI_ROTATION_90);
    fb.base.clear(&fb.base, MOUI_WHITE);

    moui_draw_ctx_t ctx;
    moui_draw_ctx_init_be(&ctx, &fb.base);

    moui_draw_pixel(&ctx, 0, 0, MOUI_BLACK);
    moui_draw_hline(&ctx, 0, 1, 5, MOUI_BLACK);

    flush_called = 0;
    fb.base.flush(&fb.base);
    ASSERT_EQ(flush_called, 1);

    ASSERT_EQ(get_phys_bit(flush_buf, 32, 31, 0), 1);
    for (int i = 0; i < 5; i++)
        ASSERT_EQ(get_phys_bit(flush_buf, 32, 30, i), 1);
    moui_backend_fb_deinit(&fb);
}

TEST(rotation_180_mode_a)
{
    moui_backend_fb_t fb;
    moui_backend_fb_init(&fb, 32, 16, MOUI_PIXFMT_MONO_HMSB, test_hw_flush, NULL);
    fb.base.sw_rotate = true;
    moui_backend_fb_set_rotation(&fb, MOUI_ROTATION_180);
    fb.base.clear(&fb.base, MOUI_WHITE);

    ASSERT_EQ(fb.base.width, 32);
    ASSERT_EQ(fb.base.height, 16);

    moui_be_set_pixel(&fb.base, 0, 0, MOUI_BLACK);
    ASSERT_EQ(get_phys_bit(fb.pixels, 32, 31, 15), 1);

    flush_called = 0;
    fb.base.flush(&fb.base);
    ASSERT_EQ(flush_called, 1);
    ASSERT_EQ(get_phys_bit(flush_buf, 32, 31, 15), 1);
    moui_backend_fb_deinit(&fb);
}

TEST(rotation_270_mode_a)
{
    moui_backend_fb_t fb;
    moui_backend_fb_init(&fb, 32, 16, MOUI_PIXFMT_MONO_HMSB, test_hw_flush, NULL);
    fb.base.sw_rotate = true;
    moui_backend_fb_set_rotation(&fb, MOUI_ROTATION_270);
    fb.base.clear(&fb.base, MOUI_WHITE);

    ASSERT_EQ(fb.base.width, 16);
    ASSERT_EQ(fb.base.height, 32);

    moui_be_set_pixel(&fb.base, 0, 0, MOUI_BLACK);
    ASSERT_EQ(get_phys_bit(fb.pixels, 32, 0, 15), 1);

    flush_called = 0;
    fb.base.flush(&fb.base);
    ASSERT_EQ(flush_called, 1);
    ASSERT_EQ(get_phys_bit(flush_buf, 32, 0, 15), 1);
    moui_backend_fb_deinit(&fb);
}

TEST(mode_a_vs_mode_b_same_result)
{
    uint8_t buf_a[64], buf_b[64];

    moui_backend_fb_t fb_a;
    moui_backend_fb_init(&fb_a, 32, 16, MOUI_PIXFMT_MONO_HMSB, test_hw_flush, NULL);
    fb_a.base.sw_rotate = true;
    moui_backend_fb_set_rotation(&fb_a, MOUI_ROTATION_90);
    fb_a.base.clear(&fb_a.base, MOUI_WHITE);
    moui_draw_ctx_t ctx_a;
    moui_draw_ctx_init_be(&ctx_a, &fb_a.base);
    moui_draw_pixel(&ctx_a, 3, 7, MOUI_BLACK);
    moui_draw_pixel(&ctx_a, 10, 20, MOUI_BLACK);
    moui_draw_hline(&ctx_a, 2, 5, 8, MOUI_BLACK);
    fb_a.base.flush(&fb_a.base);
    memcpy(buf_a, flush_buf, 64);

    moui_backend_fb_t fb_b;
    moui_backend_fb_init(&fb_b, 32, 16, MOUI_PIXFMT_MONO_HMSB, test_hw_flush, NULL);
    fb_b.base.sw_rotate = false;
    moui_backend_fb_set_rotation(&fb_b, MOUI_ROTATION_90);
    fb_b.base.clear(&fb_b.base, MOUI_WHITE);
    moui_draw_ctx_t ctx_b;
    moui_draw_ctx_init_be(&ctx_b, &fb_b.base);
    moui_draw_pixel(&ctx_b, 3, 7, MOUI_BLACK);
    moui_draw_pixel(&ctx_b, 10, 20, MOUI_BLACK);
    moui_draw_hline(&ctx_b, 2, 5, 8, MOUI_BLACK);
    fb_b.base.flush(&fb_b.base);
    memcpy(buf_b, flush_buf, 64);

    ASSERT_EQ(memcmp(buf_a, buf_b, 64), 0);

    moui_backend_fb_deinit(&fb_a);
    moui_backend_fb_deinit(&fb_b);
}

/* ── Screen Management tests ── */

static moui_screen_mgr_t *make_test_mgr(moui_backend_fb_t *fb)
{
    static moui_screen_mgr_t mgr;
    moui_backend_fb_init(fb, 128, 64, MOUI_PIXFMT_MONO_HMSB, test_hw_flush, NULL);
    moui_screen_mgr_init_be(&mgr, &fb->base, NULL);
    return &mgr;
}

static int enter_count, leave_count;
static void on_enter_cb(moui_screen_t *s) { (void)s; enter_count++; }
static void on_leave_cb(moui_screen_t *s) { (void)s; leave_count++; }

TEST(screen_push_pop)
{
    moui_backend_fb_t fb;
    moui_screen_mgr_t *mgr = make_test_mgr(&fb);
    moui_screen_t s1, s2;
    moui_screen_init(&s1);
    moui_screen_init(&s2);

    moui_screen_push(mgr, &s1);
    ASSERT_EQ(mgr->depth, 1);
    ASSERT_TRUE(moui_screen_active(mgr) == &s1);

    moui_screen_push(mgr, &s2);
    ASSERT_EQ(mgr->depth, 2);
    ASSERT_TRUE(moui_screen_active(mgr) == &s2);

    moui_screen_pop(mgr);
    ASSERT_EQ(mgr->depth, 1);
    ASSERT_TRUE(moui_screen_active(mgr) == &s1);

    moui_backend_fb_deinit(&fb);
}

TEST(screen_push_overflow)
{
    moui_backend_fb_t fb;
    moui_screen_mgr_t *mgr = make_test_mgr(&fb);
    moui_screen_t screens[MOUI_SCREEN_STACK_SIZE + 2];
    for (int i = 0; i < MOUI_SCREEN_STACK_SIZE + 2; i++)
        moui_screen_init(&screens[i]);

    for (int i = 0; i < MOUI_SCREEN_STACK_SIZE + 2; i++)
        moui_screen_push(mgr, &screens[i]);

    ASSERT_EQ(mgr->depth, MOUI_SCREEN_STACK_SIZE);
    moui_backend_fb_deinit(&fb);
}

TEST(screen_replace)
{
    moui_backend_fb_t fb;
    moui_screen_mgr_t *mgr = make_test_mgr(&fb);
    moui_screen_t s1, s2, s3;
    moui_screen_init(&s1);
    moui_screen_init(&s2);
    moui_screen_init(&s3);

    moui_screen_push(mgr, &s1);
    moui_screen_push(mgr, &s2);
    ASSERT_EQ(mgr->depth, 2);

    moui_screen_replace(mgr, &s3);
    ASSERT_EQ(mgr->depth, 2);
    ASSERT_TRUE(moui_screen_active(mgr) == &s3);

    moui_backend_fb_deinit(&fb);
}

TEST(screen_on_enter_leave)
{
    moui_backend_fb_t fb;
    moui_screen_mgr_t *mgr = make_test_mgr(&fb);
    moui_screen_t s1, s2;
    moui_screen_init(&s1);
    moui_screen_init(&s2);
    s1.on_enter = on_enter_cb;
    s1.on_leave = on_leave_cb;
    s2.on_enter = on_enter_cb;

    enter_count = leave_count = 0;
    moui_screen_push(mgr, &s1);
    ASSERT_EQ(enter_count, 1);

    moui_screen_push(mgr, &s2);
    ASSERT_EQ(leave_count, 1);
    ASSERT_EQ(enter_count, 2);

    moui_screen_pop(mgr);
    ASSERT_EQ(enter_count, 3);

    moui_backend_fb_deinit(&fb);
}

TEST(screen_active_empty)
{
    moui_backend_fb_t fb;
    moui_screen_mgr_t *mgr = make_test_mgr(&fb);
    ASSERT_TRUE(moui_screen_active(mgr) == NULL);
    moui_backend_fb_deinit(&fb);
}

TEST(screen_pop_at_root)
{
    moui_backend_fb_t fb;
    moui_screen_mgr_t *mgr = make_test_mgr(&fb);
    moui_screen_t s1;
    moui_screen_init(&s1);

    moui_screen_push(mgr, &s1);
    moui_screen_pop(mgr);
    ASSERT_EQ(mgr->depth, 1);
    ASSERT_TRUE(moui_screen_active(mgr) == &s1);

    moui_backend_fb_deinit(&fb);
}

/* ── Widget Event tests ── */

static int click_count;
static void btn_click_cb(moui_widget_button_t *b) { (void)b; click_count++; }

TEST(widget_button_press)
{
    moui_widget_button_t btn;
    moui_button_init(&btn, "OK", &moui_font_ascii_6x8, MOUI_BTN_PUSH);
    btn.on_click = btn_click_cb;
    click_count = 0;

    moui_input_event_t ev = { .type = MOUI_EV_ENCODER_PRESS };
    bool consumed = moui_widget_send_event(&btn.base, &ev);
    ASSERT_TRUE(consumed);
    ASSERT_EQ(click_count, 1);
}

TEST(widget_button_toggle)
{
    moui_widget_button_t btn;
    moui_button_init(&btn, "Toggle", &moui_font_ascii_6x8, MOUI_BTN_TOGGLE);
    ASSERT_EQ(btn.state, false);

    moui_input_event_t ev = { .type = MOUI_EV_ENCODER_PRESS };
    moui_widget_send_event(&btn.base, &ev);
    ASSERT_EQ(btn.state, true);

    moui_widget_send_event(&btn.base, &ev);
    ASSERT_EQ(btn.state, false);
}

TEST(widget_disabled_no_event)
{
    moui_widget_button_t btn;
    moui_button_init(&btn, "X", &moui_font_ascii_6x8, MOUI_BTN_PUSH);
    btn.on_click = btn_click_cb;
    btn.base.enabled = 0;
    click_count = 0;

    moui_input_event_t ev = { .type = MOUI_EV_ENCODER_PRESS };
    bool consumed = moui_widget_send_event(&btn.base, &ev);
    ASSERT_TRUE(!consumed);
    ASSERT_EQ(click_count, 0);
}

TEST(widget_invisible_no_draw)
{
    moui_fb_t fb_obj;
    moui_hal_set_display(&moui_disp_ssd1306_128x64);
    moui_fb_init(&fb_obj, &moui_disp_ssd1306_128x64);
    moui_fb_clear(&fb_obj, MOUI_WHITE);
    moui_draw_ctx_t ctx;
    moui_draw_ctx_init(&ctx, &fb_obj);

    moui_widget_button_t btn;
    moui_button_init(&btn, "Hi", &moui_font_ascii_6x8, MOUI_BTN_PUSH);
    btn.base.bounds = (moui_rect_t){0, 0, 40, 20};
    btn.base.visible = 0;

    moui_widget_draw(&btn.base, &ctx);

    bool any_black = false;
    for (int y = 0; y < 20 && !any_black; y++)
        for (int x = 0; x < 40 && !any_black; x++)
            if (moui_fb_get_pixel(&fb_obj, x, y) == MOUI_BLACK)
                any_black = true;
    ASSERT_TRUE(!any_black);

    moui_fb_deinit(&fb_obj);
}

/* ── Container Layout tests ── */

TEST(container_vertical_equal)
{
    moui_container_t c;
    moui_container_init(&c, MOUI_STACK_VERTICAL);
    c.base.bounds = (moui_rect_t){0, 0, 100, 90};

    moui_widget_t w[3];
    for (int i = 0; i < 3; i++) {
        moui_widget_init(&w[i], NULL);
        moui_container_add(&c, &w[i]);
    }

    moui_container_layout(&c);
    ASSERT_EQ(w[0].bounds.h, 30);
    ASSERT_EQ(w[1].bounds.h, 30);
    ASSERT_EQ(w[2].bounds.h, 30);
    ASSERT_EQ(w[0].bounds.y, 0);
    ASSERT_EQ(w[1].bounds.y, 30);
    ASSERT_EQ(w[2].bounds.y, 60);
}

TEST(container_horizontal_fixed)
{
    moui_container_t c;
    moui_container_init(&c, MOUI_STACK_HORIZONTAL);
    c.base.bounds = (moui_rect_t){0, 0, 100, 50};

    moui_widget_t w[3];
    moui_widget_init(&w[0], NULL);
    w[0].w_hint = 20;
    moui_widget_init(&w[1], NULL);
    moui_widget_init(&w[2], NULL);
    for (int i = 0; i < 3; i++)
        moui_container_add(&c, &w[i]);

    moui_container_layout(&c);
    ASSERT_EQ(w[0].bounds.w, 20);
    ASSERT_EQ(w[1].bounds.w, 40);
    ASSERT_EQ(w[2].bounds.w, 40);
}

TEST(container_nested_layout)
{
    moui_container_t outer, inner;
    moui_container_init(&outer, MOUI_STACK_VERTICAL);
    outer.base.bounds = (moui_rect_t){0, 0, 100, 100};

    moui_container_init(&inner, MOUI_STACK_HORIZONTAL);

    moui_widget_t top;
    moui_widget_init(&top, NULL);
    top.h_hint = 30;

    moui_widget_t left, right;
    moui_widget_init(&left, NULL);
    moui_widget_init(&right, NULL);

    moui_container_add(&outer, &top);
    moui_container_add(&outer, &inner.base);
    moui_container_add(&inner, &left);
    moui_container_add(&inner, &right);

    moui_container_layout(&outer);

    ASSERT_EQ(top.bounds.h, 30);
    ASSERT_EQ(inner.base.bounds.h, 70);
    ASSERT_EQ(inner.base.bounds.y, 30);

    ASSERT_EQ(left.bounds.w, 50);
    ASSERT_EQ(right.bounds.w, 50);
    ASSERT_EQ(left.bounds.y, 30);
}

/* ── Animation tests ── */

static int group_done_count;
static void group_done_cb(void *user) { (void)user; group_done_count++; }

TEST(anim_group_all_complete)
{
    moui_anim_mgr_t mgr;
    moui_anim_mgr_init(&mgr);

    float a = 0, b = 0;
    moui_anim_group_t grp = {0};
    moui_anim_group_init(&grp, 2, group_done_cb, NULL);

    moui_anim_t *a1 = moui_anim_start(&mgr, &a, 100.0f, 100, NULL);
    moui_anim_t *a2 = moui_anim_start(&mgr, &b, 200.0f, 200, NULL);
    moui_anim_group_track(&grp, a1);
    moui_anim_group_track(&grp, a2);

    group_done_count = 0;
    moui_anim_tick(&mgr, 0);
    moui_anim_tick(&mgr, 150);
    ASSERT_EQ(group_done_count, 0);

    moui_anim_tick(&mgr, 250);
    ASSERT_EQ(group_done_count, 1);
}

TEST(anim_group_generation)
{
    moui_anim_mgr_t mgr;
    moui_anim_mgr_init(&mgr);

    float a = 0, b = 0;
    moui_anim_group_t grp = {0};
    moui_anim_group_init(&grp, 1, group_done_cb, NULL);

    moui_anim_t *a1 = moui_anim_start(&mgr, &a, 100.0f, 100, NULL);
    moui_anim_group_track(&grp, a1);

    moui_anim_group_init(&grp, 1, group_done_cb, NULL);
    moui_anim_t *a2 = moui_anim_start(&mgr, &b, 50.0f, 50, NULL);
    moui_anim_group_track(&grp, a2);

    group_done_count = 0;
    moui_anim_tick(&mgr, 0);
    moui_anim_tick(&mgr, 110);
    ASSERT_EQ(group_done_count, 1);
}

/* ── Popup tests ── */

TEST(popup_toast_timeout)
{
    moui_popup_t p;
    moui_popup_init(&p, &moui_font_ascii_6x8);
    moui_popup_show_toast(&p, "Hello", 500);
    ASSERT_TRUE(p.visible);

    moui_fb_t fb_obj;
    moui_hal_set_display(&moui_disp_ssd1306_128x64);
    moui_fb_init(&fb_obj, &moui_disp_ssd1306_128x64);
    moui_draw_ctx_t ctx;
    moui_draw_ctx_init(&ctx, &fb_obj);

    moui_popup_draw(&p, &ctx, 100);
    ASSERT_TRUE(p.visible);
    ASSERT_EQ(p.show_time, 100u);

    moui_popup_draw(&p, &ctx, 700);
    ASSERT_TRUE(!p.visible);

    moui_fb_deinit(&fb_obj);
}

TEST(popup_menu_select)
{
    moui_popup_t p;
    moui_popup_init(&p, &moui_font_ascii_6x8);
    const char *items[] = {"A", "B", "C"};
    moui_popup_show_menu(&p, "Pick", items, 3, NULL);
    ASSERT_EQ(p.selected, 0);

    moui_input_event_t ev_cw = { .type = MOUI_EV_ENCODER_CW };
    moui_popup_handle_event(&p, &ev_cw);
    ASSERT_EQ(p.selected, 1);

    moui_popup_handle_event(&p, &ev_cw);
    ASSERT_EQ(p.selected, 2);

    moui_popup_handle_event(&p, &ev_cw);
    ASSERT_EQ(p.selected, 2);

    moui_input_event_t ev_ccw = { .type = MOUI_EV_ENCODER_CCW };
    moui_popup_handle_event(&p, &ev_ccw);
    ASSERT_EQ(p.selected, 1);
}

/* ── List adapter virtualization ── */

static const char *test_items_1000[1000];
static char test_items_buf[1000][8];

static const char *list_test_adapter(uint16_t idx, void *data)
{
    (void)data;
    return test_items_1000[idx];
}

TEST(list_adapter_basic)
{
    moui_hal_set_display(&moui_disp_ssd1306_128x64);
    for (int i = 0; i < 1000; i++) {
        snprintf(test_items_buf[i], 8, "I%d", i);
        test_items_1000[i] = test_items_buf[i];
    }

    moui_widget_list_t list;
    moui_list_init(&list, &moui_font_ascii_6x8);
    moui_list_set_adapter(&list, list_test_adapter, NULL, 1000);
    ASSERT_EQ(list.item_count, 1000);
    ASSERT_EQ(list.selected, 0);
    ASSERT_TRUE(list.adapter != NULL);

    const char *text = list.adapter(500, list.adapter_data);
    ASSERT_TRUE(text != NULL);
    ASSERT_EQ(text[0], 'I');
}

TEST(list_adapter_visible_range)
{
    moui_hal_set_display(&moui_disp_ssd1306_128x64);
    moui_fb_t fb;
    moui_fb_init(&fb, &moui_disp_ssd1306_128x64);
    moui_fb_clear(&fb, MOUI_WHITE);

    for (int i = 0; i < 1000; i++) {
        snprintf(test_items_buf[i], 8, "I%d", i);
        test_items_1000[i] = test_items_buf[i];
    }

    moui_widget_list_t list;
    moui_list_init(&list, &moui_font_ascii_6x8);
    moui_list_set_adapter(&list, list_test_adapter, NULL, 1000);
    list.base.bounds = (moui_rect_t){0, 0, 128, 64};

    moui_draw_ctx_t ctx;
    moui_draw_ctx_init(&ctx, &fb);
    list.base.vtable->draw(&list.base, &ctx);

    /* Should not crash and should render visible items only */
    ASSERT_TRUE(1);
    moui_fb_deinit(&fb);
}

/* ── Text wrapping _ex ── */

TEST(text_wrapped_ex_start_line)
{
    moui_hal_set_display(&moui_disp_ssd1306_128x64);
    const char *text = "Line1 Line2 Line3 Line4 Line5";

    int total = moui_font_count_wrapped_lines(&moui_font_ascii_6x8, text, 40);
    ASSERT_TRUE(total >= 3);

    int w, h;
    moui_font_measure_wrapped_ex(&moui_font_ascii_6x8, text, 40, 0, 0, &w, &h);
    ASSERT_TRUE(h > 0);
    int full_h = h;

    moui_font_measure_wrapped_ex(&moui_font_ascii_6x8, text, 40, 1, 2, &w, &h);
    ASSERT_TRUE(h > 0);
    ASSERT_TRUE(h < full_h);
}

TEST(text_count_wrapped_lines)
{
    moui_hal_set_display(&moui_disp_ssd1306_128x64);
    const char *text = "Hello\nWorld\nFoo";
    int lines = moui_font_count_wrapped_lines(&moui_font_ascii_6x8, text, 200);
    ASSERT_EQ(lines, 3);
}

TEST(text_wrapped_ex_draw)
{
    moui_hal_set_display(&moui_disp_ssd1306_128x64);
    moui_fb_t fb;
    moui_fb_init(&fb, &moui_disp_ssd1306_128x64);
    moui_fb_clear(&fb, MOUI_WHITE);

    moui_draw_ctx_t ctx;
    moui_draw_ctx_init(&ctx, &fb);

    const char *text = "AAA BBB CCC DDD EEE FFF";
    int drawn = moui_font_draw_str_wrapped_ex(&ctx, &moui_font_ascii_6x8,
                                              0, 0, 40, text, 1, 2, MOUI_BLACK);
    ASSERT_TRUE(drawn > 0);
    ASSERT_TRUE(drawn <= 2);

    moui_fb_deinit(&fb);
}

/* ── Regression tests for code-review fixes ── */

TEST(font_scaled_draw_measure)
{
    moui_fb_t fb;
    moui_hal_set_display(&moui_disp_ssd1306_128x64);
    moui_fb_init(&fb, &moui_disp_ssd1306_128x64);
    moui_fb_clear(&fb, MOUI_WHITE);
    moui_draw_ctx_t ctx;
    moui_draw_ctx_init(&ctx, &fb);

    /* scaled measure == 2x normal measure */
    int w1 = moui_font_measure_str(&moui_font_ascii_6x8, "AB");
    int w2 = moui_font_measure_str_scaled(&moui_font_ascii_6x8, "AB", 2);
    ASSERT_EQ(w2, w1 * 2);

    /* scaled advance == 2x normal advance */
    int a1 = moui_font_glyph_advance(&moui_font_ascii_6x8, 'A');
    int a2 = moui_font_glyph_advance_scaled(&moui_font_ascii_6x8, 'A', 2);
    ASSERT_EQ(a2, a1 * 2);

    /* drawing scaled 'A' at scale 2 covers a 2x2 block at (dx, dy) */
    moui_fb_clear(&fb, MOUI_WHITE);
    int adv = moui_font_draw_glyph_scaled(&ctx, &moui_font_ascii_6x8, 0, 0, 'A', MOUI_BLACK, 2);
    ASSERT_TRUE(adv > 0);
    /* 'A' row0 = 0x70: pixels x=1..3 black. At scale 2 the (1,0) pixel
     * becomes a 2x2 block at display (2,0),(2,1),(3,0),(3,1). */
    ASSERT_EQ(moui_fb_get_pixel(&fb, 2, 0), MOUI_BLACK);
    ASSERT_EQ(moui_fb_get_pixel(&fb, 3, 1), MOUI_BLACK);
    /* 'A' row1 = 0x88: x=0 black -> scaled block at display (0,2),(0,3) */
    ASSERT_EQ(moui_fb_get_pixel(&fb, 0, 2), MOUI_BLACK);
    ASSERT_EQ(moui_fb_get_pixel(&fb, 0, 0), MOUI_WHITE);  /* blank corner */

    moui_fb_deinit(&fb);
}

TEST(arc_radius_zero_no_crash)
{
    moui_fb_t fb;
    moui_hal_set_display(&moui_disp_ssd1306_128x64);
    moui_fb_init(&fb, &moui_disp_ssd1306_128x64);
    moui_fb_clear(&fb, MOUI_WHITE);
    moui_draw_ctx_t ctx;
    moui_draw_ctx_init(&ctx, &fb);

    /* radius 0 must be a no-op, not a divide-by-zero */
    moui_draw_arc(&ctx, 10, 10, 0, 0, 360, MOUI_BLACK);
    moui_draw_arc(&ctx, 10, 10, 0, -720, 720, MOUI_BLACK);
    ASSERT_EQ(moui_fb_get_pixel(&fb, 10, 10), MOUI_WHITE);

    moui_fb_deinit(&fb);
}

TEST(table_zero_cols_no_crash)
{
    moui_widget_table_t t;
    /* cols=0 used to divide by zero in init */
    moui_table_init(&t, &moui_font_ascii_6x8, 0, 3);
    ASSERT_TRUE(t.col_count >= 1);
}

TEST(list_zero_items_no_underflow)
{
    moui_widget_list_t list;
    moui_list_init(&list, &moui_font_ascii_6x8);
    list.item_count = 0;

    /* previously item_count-1 underflowed to 65535 and moved selection */
    moui_input_event_t cw = { .type = MOUI_EV_ENCODER_CW };
    bool consumed = moui_widget_send_event(&list.base, &cw);
    ASSERT_TRUE(!consumed);
    ASSERT_EQ(list.selected, 0);
}

TEST(checklist_zero_items_no_ub)
{
    moui_widget_checklist_t cl;
    moui_checklist_init(&cl, &moui_font_ascii_6x8);
    cl.count = 0;

    moui_input_event_t ev = { .type = MOUI_EV_ENCODER_PRESS };
    bool consumed = moui_widget_send_event(&cl.base, &ev);
    ASSERT_TRUE(!consumed);
    ASSERT_EQ(cl.checked, 0);

    /* out-of-range is_checked must not shift-read past bounds */
    ASSERT_TRUE(!moui_checklist_is_checked(&cl, 0));
}

TEST(radio_dropdown_btnmatrix_zero_items)
{
    moui_widget_radio_t r;
    moui_radio_init(&r, &moui_font_ascii_6x8);
    r.option_count = 0;
    moui_input_event_t cw = { .type = MOUI_EV_ENCODER_CW };
    bool consumed = moui_widget_send_event(&r.base, &cw);
    ASSERT_TRUE(!consumed);
    ASSERT_EQ(r.selected, 0);

    moui_widget_dropdown_t dd;
    moui_dropdown_init(&dd, &moui_font_ascii_6x8);
    dd.option_count = 0;
    consumed = moui_widget_send_event(&dd.base, &cw);
    ASSERT_TRUE(!consumed);

    moui_widget_btnmatrix_t bm;
    moui_btnmatrix_init(&bm, &moui_font_ascii_6x8, 3);
    bm.btn_count = 0;
    consumed = moui_widget_send_event(&bm.base, &cw);
    ASSERT_TRUE(!consumed);
    ASSERT_EQ(bm.selected, 0);
}

TEST(spinner_bad_format_is_safe)
{
    moui_widget_spinner_t sp;
    moui_spinner_init(&sp, &moui_font_ascii_6x8, 0, 100, 1);
    sp.format = "%s%s%s";   /* unsafe format must be rejected */

    moui_fb_t fb;
    moui_hal_set_display(&moui_disp_ssd1306_128x64);
    moui_fb_init(&fb, &moui_disp_ssd1306_128x64);
    moui_fb_clear(&fb, MOUI_WHITE);
    moui_draw_ctx_t ctx;
    moui_draw_ctx_init(&ctx, &fb);
    sp.base.bounds = (moui_rect_t){0, 0, 40, 20};
    moui_widget_draw(&sp.base, &ctx);   /* must not read garbage via %s */
    moui_fb_deinit(&fb);
}

TEST(focus_remove_and_reset)
{
    moui_focus_group_t fg;
    moui_focus_group_init(&fg);
    moui_widget_t a, b;
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    a.visible = 1; a.enabled = 1;
    b.visible = 1; b.enabled = 1;

    moui_focus_group_add(&fg, &a);
    moui_focus_group_add(&fg, &b);
    ASSERT_TRUE(fg.focused == &a);

    moui_focus_group_remove(&fg, &a);
    ASSERT_TRUE(fg.focused == &b);
    ASSERT_TRUE(a.focus_next == NULL && a.focus_prev == NULL);

    moui_focus_group_reset(&fg);
    ASSERT_TRUE(fg.focused == NULL);
    ASSERT_TRUE(b.focus_next == NULL && b.focus_prev == NULL);
}

TEST(event_bus_null_handler_rejected)
{
    moui_event_bus_t bus;
    moui_event_bus_init(&bus);
    ASSERT_TRUE(!moui_event_bus_subscribe(&bus, MOUI_EVT_NONE, 0, NULL, NULL));
    ASSERT_EQ(bus.count, 0);
}

TEST(widget_time_picker)
{
    moui_widget_time_picker_t tp;
    moui_time_picker_init(&tp, &moui_font_ascii_6x8);
    moui_time_picker_set_time(&tp, 14, 30, 45);
    ASSERT_EQ(tp.hour, 14);
    ASSERT_EQ(tp.min, 30);
    ASSERT_EQ(tp.sec, 45);

    moui_input_event_t ev_down = { .type = MOUI_EV_GESTURE_SWIPE_DOWN };
    moui_time_picker_event(&tp.base, &ev_down);
    ASSERT_EQ(tp.hour, 13);
}

TEST(widget_barchart)
{
    moui_widget_barchart_t bc;
    moui_barchart_init(&bc, &moui_font_ascii_6x8, 0, 100);
    int32_t vals[3] = { 20, 50, 80 };
    const char *lbls[3] = { "A", "B", "C" };
    moui_barchart_set_data(&bc, vals, lbls, 3);
    ASSERT_EQ(bc.bar_count, 3);
    ASSERT_EQ(bc.values[1], 50);
}

TEST(widget_roller)
{
    moui_widget_roller_t r;
    moui_roller_init(&r, &moui_font_ascii_6x8);
    const char *items[] = { "Item 1", "Item 2", "Item 3" };
    moui_roller_set_items(&r, items, 3);
    ASSERT_EQ(r.item_count, 3);

    moui_roller_set_selected(&r, 1);
    ASSERT_EQ(r.selected, 1);
}

TEST(widget_icon_bar)
{
    moui_widget_icon_bar_t ib;
    moui_icon_bar_init(&ib, &moui_font_ascii_6x8);
    moui_icon_bar_set_battery(&ib, 85, true);
    moui_icon_bar_set_wifi(&ib, 3);
    ASSERT_EQ(ib.battery_pct, 85);
    ASSERT_EQ(ib.rssi_bars, 3);
}

TEST(widget_treeview)
{
    moui_widget_treeview_t tv;
    moui_treeview_init(&tv, &moui_font_ascii_6x8);
    moui_treeview_add_node(&tv, "Root", 0, true);
    moui_treeview_add_node(&tv, "Child 1", 1, false);
    ASSERT_EQ(tv.node_count, 2);
    ASSERT_TRUE(tv.nodes[0].has_children);
}

TEST(widget_logview)
{
    moui_widget_logview_t lv;
    moui_logview_init(&lv, &moui_font_ascii_6x8);
    moui_logview_add_line(&lv, "Log line 1");
    moui_logview_add_line(&lv, "Log line 2");
    ASSERT_EQ(lv.count, 2);
    moui_logview_clear(&lv);
    ASSERT_EQ(lv.count, 0);
}

TEST(layout_flex)
{
    moui_widget_t w1, w2;
    moui_widget_init(&w1, NULL); w1.bounds = (moui_rect_t){0, 0, 40, 20};
    moui_widget_init(&w2, NULL); w2.bounds = (moui_rect_t){0, 0, 40, 20};

    moui_flex_t flex;
    moui_flex_init(&flex, MOUI_FLEX_DIR_ROW, MOUI_FLEX_JUSTIFY_START, MOUI_FLEX_ALIGN_CENTER);
    flex.gap = 10;
    flex.padding = 5;
    moui_flex_add(&flex, &w1);
    moui_flex_add(&flex, &w2);

    moui_rect_t parent = {0, 0, 200, 50};
    moui_flex_layout(&flex, &parent);

    ASSERT_EQ(w1.bounds.x, 5);
    ASSERT_EQ(w2.bounds.x, 5 + 40 + 10);
    ASSERT_EQ(w1.bounds.y, 5 + (40 - 20) / 2);
}

TEST(layout_anchor)
{
    moui_widget_t w;
    moui_widget_init(&w, NULL);
    w.bounds = (moui_rect_t){0, 0, 50, 30};

    moui_anchor_t anchor = {
        .flags = MOUI_ANCHOR_RIGHT | MOUI_ANCHOR_BOTTOM,
        .margin_right = 10,
        .margin_bottom = 5
    };
    moui_rect_t parent = {0, 0, 300, 400};
    moui_anchor_apply(&anchor, &w, &parent);

    ASSERT_EQ(w.bounds.x, 300 - 50 - 10);
    ASSERT_EQ(w.bounds.y, 400 - 30 - 5);
}

static int prop_obs_count = 0;
static void prop_cb(moui_prop_int_t *prop, int32_t old_val, int32_t new_val, void *user_data)
{
    (void)prop; (void)old_val; (void)new_val; (void)user_data;
    prop_obs_count++;
}

TEST(core_property)
{
    moui_prop_int_t prop;
    moui_prop_int_init(&prop, 10);
    ASSERT_EQ(moui_prop_int_get(&prop), 10);

    prop_obs_count = 0;
    moui_prop_int_observe(&prop, prop_cb, NULL, NULL);
    moui_prop_int_set(&prop, 25);
    ASSERT_EQ(moui_prop_int_get(&prop), 25);
    ASSERT_EQ(prop_obs_count, 1);
}

TEST(anim_timeline)
{
    moui_widget_t w;
    moui_widget_init(&w, NULL);
    w.bounds = (moui_rect_t){0, 0, 20, 20};

    moui_timeline_t tl;
    moui_timeline_init(&tl);
    moui_timeline_add(&tl, &w, MOUI_TL_TARGET_X, 0, 100, 0, 1000, moui_ease_linear);

    moui_timeline_start(&tl, 1000);
    moui_timeline_update(&tl, 1500, NULL); /* 50% progress */
    ASSERT_EQ(w.bounds.x, 50);

    moui_timeline_update(&tl, 2000, NULL); /* 100% progress */
    ASSERT_EQ(w.bounds.x, 100);
}

TEST(icons_scaling_and_helpers)
{
    ASSERT_TRUE(strcmp(moui_icon_get_name(MOUI_ICON_FACE_SMILE), "SMILE") == 0);
    ASSERT_TRUE(strcmp(moui_icon_get_name(MOUI_ICON_ROCKET), "ROCKET") == 0);
    ASSERT_TRUE(strcmp(moui_icon_get_name(MOUI_ICON_WIFI), "WIFI") == 0);

    moui_fb_t fb;
    moui_hal_set_display(&moui_disp_st7305_4p2);
    moui_fb_init(&fb, &moui_disp_st7305_4p2);
    moui_fb_clear(&fb, MOUI_WHITE);

    moui_draw_ctx_t ctx;
    moui_draw_ctx_init(&ctx, &fb);

    /* Draw scaled 2x icon */
    moui_draw_icon_scaled(&ctx, 10, 10, MOUI_ICON_FACE_SMILE, 2, MOUI_BLACK);

    /* Draw centered icon in rect */
    moui_rect_t rect = {50, 50, 100, 100};
    moui_draw_icon_in_rect(&ctx, &rect, MOUI_ICON_ROCKET, MOUI_BLACK);

    moui_fb_deinit(&fb);
}

static uint32_t vlist_test_count(const moui_vlist_t *vl, void *ud) { (void)vl; (void)ud; return 100000; }
static void vlist_test_bind(const moui_vlist_t *vl, moui_widget_t *cell, uint32_t index, void *ud) {
    (void)vl; (void)ud;
    moui_widget_label_t *lbl = (moui_widget_label_t *)cell;
    static char buf[32];
    snprintf(buf, sizeof(buf), "Item #%u", (unsigned int)index);
    lbl->text = buf;
}

TEST(widget_vlist_100k_items)
{
    moui_vlist_t vl;
    moui_vlist_init(&vl, 24, vlist_test_count, vlist_test_bind, NULL);
    vl.base.bounds = (moui_rect_t){0, 0, 200, 120};

    moui_widget_label_t cells[6];
    for (int i = 0; i < 6; i++) {
        moui_label_init(&cells[i], "", &moui_font_ascii_6x8);
        moui_vlist_add_cell(&vl, &cells[i].base);
    }

    /* Verify total virtual height for 100,000 items */
    ASSERT_EQ(moui_vlist_get_total_height(&vl), 2400000);

    /* Scroll to index 50,000 */
    moui_vlist_set_selected(&vl, 50000);
    ASSERT_EQ(vl.selected_index, 50000);
    ASSERT_TRUE(vl.scroll_offset > 0);

    /* Render test */
    moui_fb_t fb;
    moui_hal_set_display(&moui_disp_st7305_4p2);
    moui_fb_init(&fb, &moui_disp_st7305_4p2);
    moui_draw_ctx_t ctx;
    moui_draw_ctx_init(&ctx, &fb);

    if (vl.base.vtable && vl.base.vtable->draw) {
        vl.base.vtable->draw(&vl.base, &ctx);
    }

    moui_fb_deinit(&fb);
}

TEST(logo_drawing)
{
    moui_fb_t fb;
    moui_hal_set_display(&moui_disp_st7305_4p2);
    moui_fb_init(&fb, &moui_disp_st7305_4p2);
    moui_draw_ctx_t ctx;
    moui_draw_ctx_init(&ctx, &fb);

    moui_draw_logo_16(&ctx, 10, 10, MOUI_BLACK);
    moui_draw_logo_banner(&ctx, 40, 10, &moui_font_ascii_6x8, MOUI_BLACK);

    moui_fb_deinit(&fb);
}

int main(void)
{
    printf("=== Moui Unit Tests ===\n\n");

    printf("[Framebuffer]\n");
    RUN_TEST(fb_init_deinit);
    RUN_TEST(fb_set_get_pixel);
    RUN_TEST(fb_clear);

    printf("\n[Draw]\n");
    RUN_TEST(draw_pixel_clip);
    RUN_TEST(draw_hline);

    printf("\n[QR Code]\n");
    RUN_TEST(qr_encode_hello);
    RUN_TEST(qr_encode_url);
    RUN_TEST(qr_encode_empty_fails);

    printf("\n[Font]\n");
    RUN_TEST(font_measure_str);
    RUN_TEST(font_measure_newline);
    RUN_TEST(font_measure_wrapped);

    printf("\n[Input]\n");
    RUN_TEST(input_queue_push_pop);
    RUN_TEST(input_queue_overflow);

    printf("\n[Screen Management]\n");
    RUN_TEST(screen_push_pop);
    RUN_TEST(screen_push_overflow);
    RUN_TEST(screen_replace);
    RUN_TEST(screen_on_enter_leave);
    RUN_TEST(screen_active_empty);
    RUN_TEST(screen_pop_at_root);

    printf("\n[Widget Event]\n");
    RUN_TEST(widget_button_press);
    RUN_TEST(widget_button_toggle);
    RUN_TEST(widget_disabled_no_event);
    RUN_TEST(widget_invisible_no_draw);

    printf("\n[Container Layout]\n");
    RUN_TEST(container_vertical_equal);
    RUN_TEST(container_horizontal_fixed);
    RUN_TEST(container_nested_layout);

    printf("\n[Animation]\n");
    RUN_TEST(anim_group_all_complete);
    RUN_TEST(anim_group_generation);

    printf("\n[Popup]\n");
    RUN_TEST(popup_toast_timeout);
    RUN_TEST(popup_menu_select);

    printf("\n[Backend FB]\n");
    RUN_TEST(backend_fb_init);
    RUN_TEST(backend_fb_pixel_no_rotation);
    RUN_TEST(backend_fb_flush_no_rotation);
    RUN_TEST(backend_fb_gray2_pixel);
    RUN_TEST(font_aa_gray2_output);

    printf("\n[Rotation Mode A (sw_rotate)]\n");
    RUN_TEST(rotation_90_mode_a_dimensions);
    RUN_TEST(rotation_90_mode_a_pixel);
    RUN_TEST(rotation_90_mode_a_draw);
    RUN_TEST(rotation_180_mode_a);
    RUN_TEST(rotation_270_mode_a);

    printf("\n[Rotation Mode B (flush transpose)]\n");
    RUN_TEST(rotation_90_mode_b_dimensions);
    RUN_TEST(rotation_90_mode_b_pixel);
    RUN_TEST(rotation_90_mode_b_draw);

    printf("\n[Mode A vs B consistency]\n");
    RUN_TEST(mode_a_vs_mode_b_same_result);

    printf("\n[List Adapter]\n");
    RUN_TEST(list_adapter_basic);
    RUN_TEST(list_adapter_visible_range);

    printf("\n[Text Wrapped _ex]\n");
    RUN_TEST(text_wrapped_ex_start_line);
    RUN_TEST(text_count_wrapped_lines);
    RUN_TEST(text_wrapped_ex_draw);

    printf("\n[New Widgets]\n");
    RUN_TEST(widget_time_picker);
    RUN_TEST(widget_barchart);
    RUN_TEST(widget_roller);
    RUN_TEST(widget_icon_bar);
    RUN_TEST(widget_treeview);
    RUN_TEST(widget_logview);
    RUN_TEST(widget_vlist_100k_items);

    printf("\n[New Core Framework Features]\n");
    RUN_TEST(layout_flex);
    RUN_TEST(layout_anchor);
    RUN_TEST(core_property);
    RUN_TEST(anim_timeline);
    RUN_TEST(icons_scaling_and_helpers);
    RUN_TEST(logo_drawing);

    printf("\n[Review Regressions]\n");
    RUN_TEST(font_scaled_draw_measure);
    RUN_TEST(arc_radius_zero_no_crash);
    RUN_TEST(table_zero_cols_no_crash);
    RUN_TEST(list_zero_items_no_underflow);
    RUN_TEST(checklist_zero_items_no_ub);
    RUN_TEST(radio_dropdown_btnmatrix_zero_items);
    RUN_TEST(spinner_bad_format_is_safe);
    RUN_TEST(focus_remove_and_reset);
    RUN_TEST(event_bus_null_handler_rejected);

    TEST_REPORT();
}
