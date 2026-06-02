#include "sim_input.h"

void sim_input_handle_event(const SDL_Event *sdl_ev, moui_input_queue_t *q)
{
    moui_input_event_t ev = { .type = MOUI_EV_NONE, .timestamp_ms = SDL_GetTicks() };

    if (sdl_ev->type == SDL_KEYDOWN && sdl_ev->key.repeat == 0) {
        switch (sdl_ev->key.keysym.sym) {
        case SDLK_UP:     ev.type = MOUI_EV_ENCODER_CCW;   break;
        case SDLK_DOWN:   ev.type = MOUI_EV_ENCODER_CW;    break;
        case SDLK_RETURN:
        case SDLK_SPACE:  ev.type = MOUI_EV_ENCODER_PRESS; break;
        case SDLK_ESCAPE:
        case SDLK_BACKSPACE: ev.type = MOUI_EV_ENCODER_BACK; break;
        default: return;
        }
    } else if (sdl_ev->type == SDL_MOUSEWHEEL) {
        if (sdl_ev->wheel.y > 0)
            ev.type = MOUI_EV_ENCODER_CCW;
        else if (sdl_ev->wheel.y < 0)
            ev.type = MOUI_EV_ENCODER_CW;
        else
            return;
    } else if (sdl_ev->type == SDL_MOUSEBUTTONDOWN && sdl_ev->button.button == SDL_BUTTON_LEFT) {
        ev.type = MOUI_EV_POINTER_DOWN;
        ev.x = (int16_t)sdl_ev->button.x;
        ev.y = (int16_t)sdl_ev->button.y;
    } else if (sdl_ev->type == SDL_MOUSEBUTTONUP && sdl_ev->button.button == SDL_BUTTON_LEFT) {
        ev.type = MOUI_EV_POINTER_UP;
        ev.x = (int16_t)sdl_ev->button.x;
        ev.y = (int16_t)sdl_ev->button.y;
    } else if (sdl_ev->type == SDL_MOUSEMOTION && (sdl_ev->motion.state & SDL_BUTTON_LMASK)) {
        ev.type = MOUI_EV_POINTER_MOVE;
        ev.x = (int16_t)sdl_ev->motion.x;
        ev.y = (int16_t)sdl_ev->motion.y;
    } else {
        return;
    }

    if (ev.type != MOUI_EV_NONE) {
        moui_input_queue_push(q, &ev);
    }
}
