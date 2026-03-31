#include "slot_select.h"
#include <SDL2/SDL.h>

void slot_select_init(SlotSelect *s) {
    s->selected = 0;
}

SlotResult slot_select_handle_key(SlotSelect *s, int scancode) {
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
            return SLOT_SELECTED;
        case SDL_SCANCODE_ESCAPE:
            return SLOT_CANCELLED;
        default:
            break;
    }
    return SLOT_NONE;
}