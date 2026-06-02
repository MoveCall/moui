#include "sim_display.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static void recreate_texture(sim_display_t *disp)
{
    if (disp->texture) SDL_DestroyTexture(disp->texture);

    uint16_t dw = moui_disp_w(), dh = moui_disp_h();
    int w, h;
    if (disp->rotation == 0 || disp->rotation == 180) {
        w = dw; h = dh;
    } else {
        w = dh; h = dw;
    }

    SDL_SetWindowSize(disp->window, w, h);
    disp->texture = SDL_CreateTexture(disp->renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
}

int sim_display_init(sim_display_t *disp)
{
    memset(disp, 0, sizeof(*disp));
    disp->rotation = 0;

    uint16_t dw = moui_disp_w(), dh = moui_disp_h();

    disp->window = SDL_CreateWindow(
        "Moui Simulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        dw, dh, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!disp->window) return -1;

    disp->renderer = SDL_CreateRenderer(disp->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!disp->renderer) return -1;

    disp->texture = SDL_CreateTexture(disp->renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, dw, dh);
    if (!disp->texture) return -1;

    disp->pixel_count = (uint32_t)dw * dh;
    disp->pixels = (uint32_t *)calloc(disp->pixel_count, sizeof(uint32_t));
    if (!disp->pixels) return -1;

    return 0;
}

void sim_display_rotate(sim_display_t *disp)
{
    disp->rotation = (disp->rotation + 90) % 360;
    recreate_texture(disp);

    const char *titles[] = {
        "Moui Simulator (0\xC2\xB0)",
        "Moui Simulator (90\xC2\xB0)",
        "Moui Simulator (180\xC2\xB0)",
        "Moui Simulator (270\xC2\xB0)",
    };
    SDL_SetWindowTitle(disp->window, titles[disp->rotation / 90]);
}

void sim_display_render(sim_display_t *disp, const uint8_t *wire)
{
    uint16_t dw = moui_disp_w(), dh = moui_disp_h();

    int win_w;
    if (disp->rotation == 0 || disp->rotation == 180) {
        win_w = dw;
    } else {
        win_w = dh;
    }

    uint32_t *out = disp->pixels;

    if (moui_disp->pixel_format == MOUI_PIXEL_FORMAT_ST7305_4x2) {
        uint16_t stride = dw / 4;
        uint16_t packed_rows = dh / 2;
        for (int r = 0; r < packed_rows; r++) {
            for (int c = 0; c < stride; c++) {
                uint8_t byte = wire[r * stride + c];
                for (int px = 0; px < 4; px++) {
                    int src_x = c * 4 + px;
                    bool even_on = (byte >> (7 - px * 2)) & 1;
                    bool odd_on  = (byte >> (6 - px * 2)) & 1;
                    uint32_t ce = 0xFF000000 | (even_on ? SIM_FG_COLOR : SIM_BG_COLOR);
                    uint32_t co = 0xFF000000 | (odd_on  ? SIM_FG_COLOR : SIM_BG_COLOR);
                    int dx, dy;
                    switch (disp->rotation) {
                    case 0:
                        out[(r * 2) * win_w + src_x] = ce;
                        out[(r * 2 + 1) * win_w + src_x] = co;
                        break;
                    case 90:
                        dx = (dh - 1) - r * 2; dy = src_x;
                        out[dy * win_w + dx] = ce;
                        dx = (dh - 1) - (r * 2 + 1);
                        out[dy * win_w + dx] = co;
                        break;
                    case 180:
                        dx = (dw - 1) - src_x;
                        out[((dh - 1) - r * 2) * win_w + dx] = ce;
                        out[((dh - 1) - (r * 2 + 1)) * win_w + dx] = co;
                        break;
                    case 270:
                        dx = r * 2; dy = (dw - 1) - src_x;
                        out[dy * win_w + dx] = ce;
                        dx = r * 2 + 1;
                        out[dy * win_w + dx] = co;
                        break;
                    }
                }
            }
        }
    } else if (moui_disp->pixel_format == MOUI_PIXEL_FORMAT_GRAY2_HMSB) {
        static const uint32_t gray_lut[4] = {
            0xFFFFFFFF,  // 0 = WHITE
            0xFFAAAAAA,  // 1 = LGRAY
            0xFF555555,  // 2 = DGRAY
            0xFF000000,  // 3 = BLACK
        };
        for (int y = 0; y < dh; y++) {
            for (int x = 0; x < dw; x++) {
                int pi = y * dw + x;
                int bi = pi >> 2;
                int shift = 6 - ((pi & 3) * 2);
                uint8_t val = (wire[bi] >> shift) & 0x03;
                uint32_t color = gray_lut[val];
                int dx, dy;
                switch (disp->rotation) {
                case 0:   out[y * win_w + x] = color; break;
                case 90:  dx = (dh - 1) - y; dy = x; out[dy * win_w + dx] = color; break;
                case 180: out[((dh - 1) - y) * win_w + ((dw - 1) - x)] = color; break;
                case 270: dx = y; dy = (dw - 1) - x; out[dy * win_w + dx] = color; break;
                }
            }
        }
    } else {
        for (int y = 0; y < dh; y++) {
            for (int x = 0; x < dw; x++) {
                int bi = (y * dw + x) >> 3;
                uint8_t bm = 0x80 >> ((y * dw + x) & 7);
                bool on = (wire[bi] & bm) != 0;
                uint32_t color = 0xFF000000 | (on ? SIM_FG_COLOR : SIM_BG_COLOR);
                int dx, dy;
                switch (disp->rotation) {
                case 0:   out[y * win_w + x] = color; break;
                case 90:  dx = (dh - 1) - y; dy = x; out[dy * win_w + dx] = color; break;
                case 180: out[((dh - 1) - y) * win_w + ((dw - 1) - x)] = color; break;
                case 270: dx = y; dy = (dw - 1) - x; out[dy * win_w + dx] = color; break;
                }
            }
        }
    }

    SDL_UpdateTexture(disp->texture, NULL, out, win_w * sizeof(uint32_t));
    SDL_RenderClear(disp->renderer);
    SDL_RenderCopy(disp->renderer, disp->texture, NULL, NULL);
    SDL_RenderPresent(disp->renderer);
}

void sim_display_destroy(sim_display_t *disp)
{
    if (disp->texture)  SDL_DestroyTexture(disp->texture);
    if (disp->renderer) SDL_DestroyRenderer(disp->renderer);
    if (disp->window)   SDL_DestroyWindow(disp->window);
    free(disp->pixels);
    disp->pixels = NULL;
}

void sim_set_cwd_to_exe(const char *argv0)
{
    char buf[1024];
    strncpy(buf, argv0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    char *last_sep = strrchr(buf, '/');
    if (!last_sep) last_sep = strrchr(buf, '\\');
    if (last_sep) {
        *last_sep = '\0';
        chdir(buf);
    }
}
