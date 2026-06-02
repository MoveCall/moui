#include "moui_qrcode.h"
#include <string.h>

/*
 * Minimal but correct QR Code encoder (Version 1-6, ECC Level L, Byte mode).
 *
 * Reference: ISO/IEC 18004:2015
 */

/* ── GF(256) arithmetic, primitive poly 0x11D ── */

static uint8_t gf_exp_tab[256];
static uint8_t gf_log_tab[256];
static bool    gf_ready = false;

static void gf_init(void)
{
    if (gf_ready) return;
    int v = 1;
    for (int i = 0; i < 255; i++) {
        gf_exp_tab[i] = (uint8_t)v;
        gf_log_tab[v] = (uint8_t)i;
        v <<= 1;
        if (v >= 256) v ^= 0x11D;
    }
    gf_exp_tab[255] = gf_exp_tab[0];
    gf_ready = true;
}

static uint8_t gf_mul(uint8_t a, uint8_t b)
{
    if (a == 0 || b == 0) return 0;
    return gf_exp_tab[(gf_log_tab[a] + gf_log_tab[b]) % 255];
}

/* ── Version tables (ECC Level L, single block) ── */

/* Total codewords, data codewords, EC codewords per version (Level L) */
static const uint8_t ver_data[]  = {0, 19, 34, 55,  80, 108, 136};
static const uint8_t ver_ec[]    = {0,  7, 10, 15,  20,  26,  36};
/* Byte mode capacity (Level L) per version */
static const uint8_t ver_cap[]   = {0, 17, 32, 53,  78, 106, 134};

/* Alignment pattern positions */
static const uint8_t align_center[][7] = {
    {0},          /* v0 unused */
    {0},          /* v1: none */
    {6, 18},      /* v2 */
    {6, 22},      /* v3 */
    {6, 26},      /* v4 */
    {6, 30},      /* v5 */
    {6, 34},      /* v6 */
};
static const uint8_t align_count[] = {0, 0, 2, 2, 2, 2, 2};

/* ── Bit buffer helper ── */

static void bits_append(uint8_t *buf, int *bit_pos, uint32_t val, int nbits)
{
    for (int i = nbits - 1; i >= 0; i--) {
        int p = *bit_pos;
        if ((val >> i) & 1)
            buf[p >> 3] |= (0x80 >> (p & 7));
        (*bit_pos)++;
    }
}

/* ── Module get/set with bounds checking ── */

static void qr_set(moui_qr_t *qr, int x, int y, bool v)
{
    if (x < 0 || x >= qr->size || y < 0 || y >= qr->size) return;
    int idx = y * qr->size + x;
    if (v) qr->modules[idx >> 3] |= (0x80 >> (idx & 7));
    else   qr->modules[idx >> 3] &= ~(0x80 >> (idx & 7));
}

static bool qr_get(const moui_qr_t *qr, int x, int y)
{
    if (x < 0 || x >= qr->size || y < 0 || y >= qr->size) return false;
    int idx = y * qr->size + x;
    return (qr->modules[idx >> 3] >> (7 - (idx & 7))) & 1;
}

/* ── Reserved-module tracking ── */

static uint8_t reserved[MOUI_QR_BUF_SIZE];

static void res_set(int size, int x, int y)
{
    if (x < 0 || x >= size || y < 0 || y >= size) return;
    int idx = y * size + x;
    reserved[idx >> 3] |= (0x80 >> (idx & 7));
}

static bool res_get(int size, int x, int y)
{
    if (x < 0 || x >= size || y < 0 || y >= size) return true;
    int idx = y * size + x;
    return (reserved[idx >> 3] >> (7 - (idx & 7))) & 1;
}

/* ── Fixed pattern placement ── */

static void place_finder(moui_qr_t *qr, int ox, int oy)
{
    /* 7×7 finder + 1-module white separator */
    for (int dy = -1; dy <= 7; dy++) {
        for (int dx = -1; dx <= 7; dx++) {
            int px = ox + dx, py = oy + dy;
            if (px < 0 || px >= qr->size || py < 0 || py >= qr->size) continue;
            bool on = false;
            if (dx >= 0 && dx <= 6 && dy >= 0 && dy <= 6) {
                on = (dy == 0 || dy == 6 || dx == 0 || dx == 6 ||
                     (dx >= 2 && dx <= 4 && dy >= 2 && dy <= 4));
            }
            qr_set(qr, px, py, on);
            res_set(qr->size, px, py);
        }
    }
}

static bool overlaps_finder(int cx, int cy, int size)
{
    /* Check if alignment pattern at (cx,cy) overlaps any finder+separator */
    if (cx - 2 <= 8 && cy - 2 <= 8) return true;                       /* top-left */
    if (cx + 2 >= size - 8 && cy - 2 <= 8) return true;                /* top-right */
    if (cx - 2 <= 8 && cy + 2 >= size - 8) return true;                /* bottom-left */
    return false;
}

static void place_alignment(moui_qr_t *qr, int cx, int cy)
{
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            bool on = (dy == -2 || dy == 2 || dx == -2 || dx == 2 || (dx == 0 && dy == 0));
            qr_set(qr, cx + dx, cy + dy, on);
            res_set(qr->size, cx + dx, cy + dy);
        }
    }
}

static void place_timing(moui_qr_t *qr)
{
    for (int i = 8; i < qr->size - 8; i++) {
        bool on = (i & 1) == 0;
        qr_set(qr, i, 6, on);
        qr_set(qr, 6, i, on);
        res_set(qr->size, i, 6);
        res_set(qr->size, 6, i);
    }
}

static void reserve_format_area(moui_qr_t *qr)
{
    /* Around top-left finder */
    for (int i = 0; i < 9; i++) {
        res_set(qr->size, 8, i);
        res_set(qr->size, i, 8);
    }
    /* Bottom-left */
    for (int i = 0; i < 8; i++) {
        res_set(qr->size, 8, qr->size - 1 - i);
    }
    /* Top-right */
    for (int i = 0; i < 8; i++) {
        res_set(qr->size, qr->size - 1 - i, 8);
    }
    /* Dark module (always set) */
    qr_set(qr, 8, 4 * qr->version + 9, true);
    res_set(qr->size, 8, 4 * qr->version + 9);
}

/* ── Format information ── */

static uint32_t format_bch(uint32_t data5)
{
    /* BCH(15,5) with generator 10100110111 (0x537) */
    uint32_t d = data5 << 10;
    uint32_t g = 0x537;
    for (int i = 4; i >= 0; i--) {
        if (d & (1 << (i + 10)))
            d ^= g << i;
    }
    return (data5 << 10) | d;
}

static void place_format_bits(moui_qr_t *qr, int mask)
{
    /* ECC Level L = 01, mask pattern 0-7 */
    uint32_t data5 = (0x01 << 3) | (uint32_t)mask;  /* 01 xxx */
    uint32_t bits15 = format_bch(data5) ^ 0x5412;    /* XOR mask per spec */

    /* Top-left copy: ISO 18004 Table 9 */
    /* Bits 0-5 go down column 8: rows 0,1,2,3,4,5 */
    for (int i = 0; i <= 5; i++)
        qr_set(qr, 8, i, (bits15 >> i) & 1);
    /* Bit 6: column 8, row 7 (skip timing at row 6) */
    qr_set(qr, 8, 7, (bits15 >> 6) & 1);
    /* Bit 7: column 8, row 8 */
    qr_set(qr, 8, 8, (bits15 >> 7) & 1);
    /* Bit 8: column 7, row 8 */
    qr_set(qr, 7, 8, (bits15 >> 8) & 1);
    /* Bits 9-14 go left along row 8: columns 5,4,3,2,1,0 (skip timing at col 6) */
    for (int i = 9; i <= 14; i++)
        qr_set(qr, 14 - i, 8, (bits15 >> i) & 1);

    /* Top-right copy: bits 0-7 along row 8, columns size-1 down to size-8 */
    for (int i = 0; i < 8; i++)
        qr_set(qr, qr->size - 1 - i, 8, (bits15 >> i) & 1);

    /* Bottom-left copy: bits 8-14 down column 8, rows size-7 to size-1 */
    for (int i = 8; i < 15; i++)
        qr_set(qr, 8, qr->size - 15 + i, (bits15 >> i) & 1);
}

/* ── Reed-Solomon error correction ── */

static void rs_encode(const uint8_t *data, int data_len, uint8_t *ecc, int ecc_len)
{
    gf_init();

    /* Build generator polynomial: g(x) = (x - α^0)(x - α^1)...(x - α^(n-1))
     * Coefficients stored low-to-high initially, then reversed to high-to-low (monic). */
    uint8_t gen[40];
    memset(gen, 0, sizeof(gen));
    gen[0] = 1;
    for (int i = 0; i < ecc_len; i++) {
        for (int j = i + 1; j >= 1; j--) {
            gen[j] = gen[j - 1] ^ gf_mul(gen[j], gf_exp_tab[i]);
        }
        gen[0] = gf_mul(gen[0], gf_exp_tab[i]);
    }
    /* Reverse so gen[0] = 1 (leading coefficient, monic polynomial) */
    for (int i = 0; i < (ecc_len + 1) / 2; i++) {
        uint8_t tmp = gen[i];
        gen[i] = gen[ecc_len - i];
        gen[ecc_len - i] = tmp;
    }

    /* Polynomial long division */
    memset(ecc, 0, ecc_len);
    for (int i = 0; i < data_len; i++) {
        uint8_t coef = data[i] ^ ecc[0];
        memmove(ecc, ecc + 1, ecc_len - 1);
        ecc[ecc_len - 1] = 0;
        if (coef == 0) continue;
        for (int j = 0; j < ecc_len; j++) {
            ecc[j] ^= gf_mul(gen[j + 1], coef);
        }
    }
}

/* ── Masking ── */

static bool apply_mask(int mask, int x, int y)
{
    switch (mask) {
    case 0: return ((x + y) & 1) == 0;
    case 1: return (y & 1) == 0;
    case 2: return (x % 3) == 0;
    case 3: return ((x + y) % 3) == 0;
    case 4: return (((y / 2) + (x / 3)) & 1) == 0;
    case 5: return ((x * y) % 2 + (x * y) % 3) == 0;
    case 6: return (((x * y) % 2 + (x * y) % 3) & 1) == 0;
    case 7: return (((x + y) % 2 + (x * y) % 3) & 1) == 0;
    }
    return false;
}

static int penalty_score(const moui_qr_t *qr)
{
    int s = qr->size;
    int score = 0;

    /* Rule 1: runs of same color ≥ 5 */
    for (int y = 0; y < s; y++) {
        int run = 1;
        for (int x = 1; x < s; x++) {
            if (qr_get(qr, x, y) == qr_get(qr, x - 1, y)) {
                run++;
                if (run == 5) score += 3;
                else if (run > 5) score++;
            } else run = 1;
        }
    }
    for (int x = 0; x < s; x++) {
        int run = 1;
        for (int y = 1; y < s; y++) {
            if (qr_get(qr, x, y) == qr_get(qr, x, y - 1)) {
                run++;
                if (run == 5) score += 3;
                else if (run > 5) score++;
            } else run = 1;
        }
    }

    /* Rule 2: 2×2 same-color blocks */
    for (int y = 0; y < s - 1; y++) {
        for (int x = 0; x < s - 1; x++) {
            bool c = qr_get(qr, x, y);
            if (c == qr_get(qr, x+1, y) && c == qr_get(qr, x, y+1) && c == qr_get(qr, x+1, y+1))
                score += 3;
        }
    }

    /* Rule 4: proportion of dark modules */
    int dark = 0;
    for (int y = 0; y < s; y++)
        for (int x = 0; x < s; x++)
            if (qr_get(qr, x, y)) dark++;
    int pct = dark * 100 / (s * s);
    int prev5 = (pct / 5) * 5;
    int next5 = prev5 + 5;
    int d1 = prev5 - 50; if (d1 < 0) d1 = -d1;
    int d2 = next5 - 50; if (d2 < 0) d2 = -d2;
    score += (d1 < d2 ? d1 : d2) / 5 * 10;

    return score;
}

/* ── Main encode ── */

bool moui_qr_encode(moui_qr_t *qr, const char *text)
{
    gf_init();
    int len = (int)strlen(text);
    if (len > 134 || len == 0) return false;

    /* Version tables (ver_cap etc.) are defined up to version 6 only. Clamp
     * MOUI_QR_MAX_VERSION at 6 so a larger Kconfig value can't cause OOB reads. */
    int max_v = MOUI_QR_MAX_VERSION > 6 ? 6 : MOUI_QR_MAX_VERSION;

    /* Pick smallest version */
    int version = 0;
    for (int v = 1; v <= max_v; v++) {
        if (len <= ver_cap[v]) { version = v; break; }
    }
    if (version == 0) return false;

    qr->version = (uint8_t)version;
    qr->size = (uint8_t)(17 + version * 4);

    memset(qr->modules, 0, sizeof(qr->modules));
    memset(reserved, 0, sizeof(reserved));

    /* 1. Place finder patterns */
    place_finder(qr, 0, 0);
    place_finder(qr, qr->size - 7, 0);
    place_finder(qr, 0, qr->size - 7);

    /* 2. Timing patterns */
    place_timing(qr);

    /* 3. Alignment patterns (version ≥ 2) */
    if (version >= 2) {
        int n = align_count[version];
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                int cx = align_center[version][i];
                int cy = align_center[version][j];
                if (!overlaps_finder(cx, cy, qr->size))
                    place_alignment(qr, cx, cy);
            }
        }
    }

    /* 4. Reserve format info areas */
    reserve_format_area(qr);

    /* 5. Encode data: byte mode */
    int data_cw = ver_data[version];
    uint8_t codewords[180];
    memset(codewords, 0, sizeof(codewords));

    int bp = 0;
    /* Mode indicator: 0100 */
    bits_append(codewords, &bp, 0x4, 4);
    /* Character count: 8 bits for V1-9 */
    bits_append(codewords, &bp, (uint32_t)len, 8);
    /* Data bytes */
    for (int i = 0; i < len; i++)
        bits_append(codewords, &bp, (uint8_t)text[i], 8);
    /* Terminator: up to 4 zero bits */
    int term = data_cw * 8 - bp;
    if (term > 4) term = 4;
    bits_append(codewords, &bp, 0, term);
    /* Pad to byte boundary */
    if (bp & 7) bits_append(codewords, &bp, 0, 8 - (bp & 7));
    /* Pad codewords */
    int used = bp / 8;
    for (int i = used; i < data_cw; i++)
        codewords[i] = (i - used) % 2 == 0 ? 0xEC : 0x11;

    /* 6. Reed-Solomon EC */
    int ec_cw = ver_ec[version];
    uint8_t ecc[40];
    rs_encode(codewords, data_cw, ecc, ec_cw);

    /* 7. Final message = data + EC */
    uint8_t final_msg[220];
    memcpy(final_msg, codewords, data_cw);
    memcpy(final_msg + data_cw, ecc, ec_cw);
    int total_bits = (data_cw + ec_cw) * 8;

    /* 8. Place data modules in zigzag pattern */
    int data_bit = 0;
    for (int right = qr->size - 1; right >= 1; right -= 2) {
        if (right == 6) right = 5;  /* Skip vertical timing column */
        bool upward = ((qr->size - 1 - right) / 2) % 2 == 0;
        for (int vert = 0; vert < qr->size; vert++) {
            int y = upward ? (qr->size - 1 - vert) : vert;
            for (int c = 0; c < 2; c++) {
                int x = right - c;
                if (x < 0 || x >= qr->size) continue;
                if (res_get(qr->size, x, y)) continue;

                bool val = false;
                if (data_bit < total_bits) {
                    val = (final_msg[data_bit >> 3] >> (7 - (data_bit & 7))) & 1;
                    data_bit++;
                }
                qr_set(qr, x, y, val);
            }
        }
    }

    /* 9. Select best mask */
    uint8_t unmasked[MOUI_QR_BUF_SIZE];
    memcpy(unmasked, qr->modules, sizeof(unmasked));

    uint8_t best_modules[MOUI_QR_BUF_SIZE];
    int best_score = 0x7FFFFFFF;
    int best_mask = 0;
    (void)best_mask;

    for (int mask = 0; mask < 8; mask++) {
        /* Start from unmasked data */
        memcpy(qr->modules, unmasked, sizeof(qr->modules));

        /* Apply mask to data modules only */
        for (int y = 0; y < qr->size; y++) {
            for (int x = 0; x < qr->size; x++) {
                if (res_get(qr->size, x, y)) continue;
                if (apply_mask(mask, x, y)) {
                    int idx = y * qr->size + x;
                    qr->modules[idx >> 3] ^= (0x80 >> (idx & 7));
                }
            }
        }

        /* Write format info for this mask */
        place_format_bits(qr, mask);

        int s = penalty_score(qr);
        if (s < best_score) {
            best_score = s;
            best_mask = mask;
            memcpy(best_modules, qr->modules, sizeof(best_modules));
        }
    }

    memcpy(qr->modules, best_modules, sizeof(qr->modules));

    return true;
}

/* ── Draw QR code ── */

void moui_qr_draw(moui_draw_ctx_t *ctx, const moui_qr_t *qr, int x, int y, int scale)
{
    if (!qr) return;
    /* Valid QR sizes are 21, 25, 29, ... 17+4*version. Anything else means
     * the struct was never produced by moui_qr_encode. */
    int size = qr->size;
    if (size < 21 || size > MOUI_QR_MAX_MODULES || ((size - 17) % 4) != 0) return;
    if (scale < 1) scale = 1;
    /* Quiet zone (4 modules) */
    int quiet = 4 * scale;
    int total = size * scale + quiet * 2;
    moui_draw_fill_rect(ctx, &(moui_rect_t){x - quiet, y - quiet, total, total}, MOUI_WHITE);

    for (int qy = 0; qy < size; qy++) {
        for (int qx = 0; qx < size; qx++) {
            if (qr_get(qr, qx, qy)) {
                if (scale == 1) {
                    moui_draw_pixel(ctx, x + qx, y + qy, MOUI_BLACK);
                } else {
                    moui_draw_fill_rect(ctx, &(moui_rect_t){
                        x + qx * scale, y + qy * scale, scale, scale
                    }, MOUI_BLACK);
                }
            }
        }
    }
}
