#include "landing.h"
#include <SDL2/SDL.h>

void landing_init(LandingScreen *s) {
    s->selected = 0;
}

LandingResult landing_handle_key(LandingScreen *s, int scancode) {
    switch (scancode) {
        case SDL_SCANCODE_UP:
            s->selected--;
            if (s->selected < 0) s->selected = 2;
            break;
        case SDL_SCANCODE_DOWN:
            s->selected++;
            if (s->selected > 2) s->selected = 0;
            break;
        case SDL_SCANCODE_RETURN:
            switch (s->selected) {
                case 0: return LANDING_NEW_GAME;
                case 1: return LANDING_LOAD_GAME;
                case 2: return LANDING_QUIT;
            }
            break;
        case SDL_SCANCODE_ESCAPE:
            return LANDING_QUIT;
        default:
            break;
    }
    return LANDING_NONE;
}