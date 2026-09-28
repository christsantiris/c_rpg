#include "harbor.h"
#include <SDL2/SDL.h>

void harbor_init(HarborScreen *s) {
    s->selected = 0;
}

HarborResult harbor_activate(const HarborScreen *s, int can_sail, int on_island) {
    if (s->selected == 1) {
        return HARBOR_CLOSED;
    }
    if (on_island) {
        return HARBOR_SAIL_TOWN;
    }
    return can_sail ? HARBOR_BOARD : HARBOR_MAP_REQUIRED;
}

HarborResult harbor_handle_key(HarborScreen *s, int scancode, int can_sail, int on_island) {
    switch (scancode) {
        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_W:
            s->selected = 0;
            break;
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
            s->selected = 1;
            break;
        case SDL_SCANCODE_RETURN:
        case SDL_SCANCODE_KP_ENTER:
            return harbor_activate(s, can_sail, on_island);
        case SDL_SCANCODE_ESCAPE:
            return HARBOR_CLOSED;
        default:
            break;
    }
    return HARBOR_NONE;
}
