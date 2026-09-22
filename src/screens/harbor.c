#include "harbor.h"
#include <SDL2/SDL.h>

void harbor_init(HarborScreen *s) {
    s->selected = 0;
}

HarborResult harbor_activate(const HarborScreen *s, int has_map) {
    if (s->selected == 1) {
        return HARBOR_CLOSED;
    }
    return has_map ? HARBOR_BOARD : HARBOR_MAP_REQUIRED;
}

HarborResult harbor_handle_key(HarborScreen *s, int scancode, int has_map) {
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
            return harbor_activate(s, has_map);
        case SDL_SCANCODE_ESCAPE:
            return HARBOR_CLOSED;
        default:
            break;
    }
    return HARBOR_NONE;
}
