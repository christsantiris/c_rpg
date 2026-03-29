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

LandingResult landing_handle_click(LandingScreen *s, int mouse_x, int mouse_y,
                                   int screen_w, int screen_h) {
    int item_x     = screen_w / 2 - 80;
    int item_x_end = screen_w / 2 + 80;
    int base_y     = screen_h / 2 - 20;

    for (int i = 0; i < 3; i++) {
        int item_y     = base_y + i * 36;
        int item_y_end = item_y + 24;

        if (mouse_x >= item_x && mouse_x <= item_x_end &&
            mouse_y >= item_y && mouse_y <= item_y_end) {
            s->selected = i;
            switch (i) {
                case 0: return LANDING_NEW_GAME;
                case 1: return LANDING_LOAD_GAME;
                case 2: return LANDING_QUIT;
            }
        }
    }
    return LANDING_NONE;
}