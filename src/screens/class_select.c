#include "class_select.h"
#include <SDL2/SDL.h>

void class_select_init(ClassSelectScreen *s) {
    s->selected = 0;
}

ClassSelectResult class_select_handle_key(ClassSelectScreen *s, int scancode) {
    switch (scancode) {
        case SDL_SCANCODE_UP:
            s->selected--;
            if (s->selected < 0) {
                s->selected = 2;
            }
            break;
        case SDL_SCANCODE_DOWN:
            s->selected++;
            if (s->selected > 2) {
                s->selected = 0;
            }
            break;
        case SDL_SCANCODE_RETURN:
            return CLASS_SELECT_CONFIRMED;
        case SDL_SCANCODE_ESCAPE:
            return CLASS_SELECT_CANCELLED;
        default:
            break;
    }
    return CLASS_SELECT_NONE;
}