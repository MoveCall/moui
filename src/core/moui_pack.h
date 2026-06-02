#ifndef MOUI_PACK_H
#define MOUI_PACK_H

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void moui_pack_st7305_4x2(uint8_t *wire, const uint8_t *pixels, int w, int h)
{
    if ((w & 7) == 0) {
        int stride_bytes = w >> 3;
        int out_stride = w >> 2;
        int packed_rows = h >> 1;
        for (int r = 0; r < packed_rows; r++) {
            const uint8_t *row_e = &pixels[(r * 2) * stride_bytes];
            const uint8_t *row_o = &pixels[(r * 2 + 1) * stride_bytes];
            uint8_t *out = &wire[r * out_stride];
            for (int c = 0; c < out_stride; c++) {
                int byte_idx = c >> 1;
                uint8_t be = row_e[byte_idx];
                uint8_t bo = row_o[byte_idx];
                uint8_t byte = 0;
                if (c & 1) {
                    if (be & 0x08) byte |= 0x80;
                    if (bo & 0x08) byte |= 0x40;
                    if (be & 0x04) byte |= 0x20;
                    if (bo & 0x04) byte |= 0x10;
                    if (be & 0x02) byte |= 0x08;
                    if (bo & 0x02) byte |= 0x04;
                    if (be & 0x01) byte |= 0x02;
                    if (bo & 0x01) byte |= 0x01;
                } else {
                    if (be & 0x80) byte |= 0x80;
                    if (bo & 0x80) byte |= 0x40;
                    if (be & 0x40) byte |= 0x20;
                    if (bo & 0x40) byte |= 0x10;
                    if (be & 0x20) byte |= 0x08;
                    if (bo & 0x20) byte |= 0x04;
                    if (be & 0x10) byte |= 0x02;
                    if (bo & 0x10) byte |= 0x01;
                }
                out[c] = byte;
            }
        }
        return;
    }

    int stride = w / 4;
    int packed_rows = h / 2;
    for (int r = 0; r < packed_rows; r++) {
        for (int c = 0; c < stride; c++) {
            uint8_t byte = 0;
            for (int px = 0; px < 4; px++) {
                int x = c * 4 + px;
                int y_even = r * 2, y_odd = r * 2 + 1;
                int bi_e = (y_even * w + x) >> 3;
                uint8_t bm_e = 0x80 >> ((y_even * w + x) & 7);
                int bi_o = (y_odd * w + x) >> 3;
                uint8_t bm_o = 0x80 >> ((y_odd * w + x) & 7);
                if (pixels[bi_e] & bm_e) byte |= (1 << (7 - px * 2));
                if (pixels[bi_o] & bm_o) byte |= (1 << (6 - px * 2));
            }
            wire[r * stride + c] = byte;
        }
    }
}

static inline void moui_pack_mono_vmsb(uint8_t *wire, const uint8_t *pixels, int w, int h)
{
    if ((w & 7) == 0) {
        int stride_bytes = w >> 3;
        int pages = (h + 7) >> 3;
        for (int page = 0; page < pages; page++) {
            int y_base = page << 3;
            int valid_bits = (h - y_base < 8) ? (h - y_base) : 8;
            const uint8_t *page_ptrs[8];
            for (int b = 0; b < valid_bits; b++) {
                page_ptrs[b] = &pixels[(y_base + b) * stride_bytes];
            }
            uint8_t *out = &wire[page * w];

            for (int col_byte = 0; col_byte < stride_bytes; col_byte++) {
                uint8_t r0 = valid_bits > 0 ? page_ptrs[0][col_byte] : 0;
                uint8_t r1 = valid_bits > 1 ? page_ptrs[1][col_byte] : 0;
                uint8_t r2 = valid_bits > 2 ? page_ptrs[2][col_byte] : 0;
                uint8_t r3 = valid_bits > 3 ? page_ptrs[3][col_byte] : 0;
                uint8_t r4 = valid_bits > 4 ? page_ptrs[4][col_byte] : 0;
                uint8_t r5 = valid_bits > 5 ? page_ptrs[5][col_byte] : 0;
                uint8_t r6 = valid_bits > 6 ? page_ptrs[6][col_byte] : 0;
                uint8_t r7 = valid_bits > 7 ? page_ptrs[7][col_byte] : 0;

                int x_base = col_byte << 3;
                out[x_base + 0] = ((r0 >> 7) & 1) | (((r1 >> 7) & 1) << 1) | (((r2 >> 7) & 1) << 2) | (((r3 >> 7) & 1) << 3) |
                                  (((r4 >> 7) & 1) << 4) | (((r5 >> 7) & 1) << 5) | (((r6 >> 7) & 1) << 6) | (((r7 >> 7) & 1) << 7);
                out[x_base + 1] = ((r0 >> 6) & 1) | (((r1 >> 6) & 1) << 1) | (((r2 >> 6) & 1) << 2) | (((r3 >> 6) & 1) << 3) |
                                  (((r4 >> 6) & 1) << 4) | (((r5 >> 6) & 1) << 5) | (((r6 >> 6) & 1) << 6) | (((r7 >> 6) & 1) << 7);
                out[x_base + 2] = ((r0 >> 5) & 1) | (((r1 >> 5) & 1) << 1) | (((r2 >> 5) & 1) << 2) | (((r3 >> 5) & 1) << 3) |
                                  (((r4 >> 5) & 1) << 4) | (((r5 >> 5) & 1) << 5) | (((r6 >> 5) & 1) << 6) | (((r7 >> 5) & 1) << 7);
                out[x_base + 3] = ((r0 >> 4) & 1) | (((r1 >> 4) & 1) << 1) | (((r2 >> 4) & 1) << 2) | (((r3 >> 4) & 1) << 3) |
                                  (((r4 >> 4) & 1) << 4) | (((r5 >> 4) & 1) << 5) | (((r6 >> 4) & 1) << 6) | (((r7 >> 4) & 1) << 7);
                out[x_base + 4] = ((r0 >> 3) & 1) | (((r1 >> 3) & 1) << 1) | (((r2 >> 3) & 1) << 2) | (((r3 >> 3) & 1) << 3) |
                                  (((r4 >> 3) & 1) << 4) | (((r5 >> 3) & 1) << 5) | (((r6 >> 3) & 1) << 6) | (((r7 >> 3) & 1) << 7);
                out[x_base + 5] = ((r0 >> 2) & 1) | (((r1 >> 2) & 1) << 1) | (((r2 >> 2) & 1) << 2) | (((r3 >> 2) & 1) << 3) |
                                  (((r4 >> 2) & 1) << 4) | (((r5 >> 2) & 1) << 5) | (((r6 >> 2) & 1) << 6) | (((r7 >> 2) & 1) << 7);
                out[x_base + 6] = ((r0 >> 1) & 1) | (((r1 >> 1) & 1) << 1) | (((r2 >> 1) & 1) << 2) | (((r3 >> 1) & 1) << 3) |
                                  (((r4 >> 1) & 1) << 4) | (((r5 >> 1) & 1) << 5) | (((r6 >> 1) & 1) << 6) | (((r7 >> 1) & 1) << 7);
                out[x_base + 7] = ((r0 >> 0) & 1) | (((r1 >> 0) & 1) << 1) | (((r2 >> 0) & 1) << 2) | (((r3 >> 0) & 1) << 3) |
                                  (((r4 >> 0) & 1) << 4) | (((r5 >> 0) & 1) << 5) | (((r6 >> 0) & 1) << 6) | (((r7 >> 0) & 1) << 7);
            }
        }
        return;
    }

    int pages = (h + 7) / 8;
    for (int page = 0; page < pages; page++) {
        for (int x = 0; x < w; x++) {
            uint8_t byte = 0;
            for (int bit = 0; bit < 8; bit++) {
                int y = page * 8 + bit;
                if (y >= h) break;
                int bi = (y * w + x) >> 3;
                uint8_t bm = 0x80 >> ((y * w + x) & 7);
                if (pixels[bi] & bm) byte |= (1 << bit);
            }
            wire[page * w + x] = byte;
        }
    }
}

static inline void moui_pack_mono_hmsb(uint8_t *wire, const uint8_t *pixels, int w, int h)
{
    uint32_t size = ((uint32_t)w * h + 7) / 8;
    memcpy(wire, pixels, size);
}

#ifdef __cplusplus
}
#endif

#endif
