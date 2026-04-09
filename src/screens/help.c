#include "help.h"
#include <SDL2/SDL.h>

HelpResult help_handle_key(int scancode) {
    switch (scancode) {
        case SDL_SCANCODE_ESCAPE:
        case SDL_SCANCODE_SLASH:
            return HELP_CLOSED;
        default:
            break;
    }
    return HELP_NONE;
}