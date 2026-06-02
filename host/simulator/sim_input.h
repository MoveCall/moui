#ifndef SIM_INPUT_H
#define SIM_INPUT_H

#include <SDL2/SDL.h>
#include "../src/input/moui_input.h"

void sim_input_handle_event(const SDL_Event *sdl_ev, moui_input_queue_t *q);

#endif
