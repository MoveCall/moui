#ifndef SIM_DISPLAY_H
#define SIM_DISPLAY_H

#include <SDL2/SDL.h>
#include "../src/hal/moui_hal_types.h"

#define SIM_BG_COLOR 0xE8E0D0
#define SIM_FG_COLOR 0x1A1A1A

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    SDL_Texture  *texture;
    uint32_t     *pixels;
    uint32_t      pixel_count;
    int           rotation;
} sim_display_t;

int  sim_display_init(sim_display_t *disp);
void sim_display_render(sim_display_t *disp, const uint8_t *wire);
void sim_display_rotate(sim_display_t *disp);
void sim_display_destroy(sim_display_t *disp);
void sim_set_cwd_to_exe(const char *argv0);

#endif
