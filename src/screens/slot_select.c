#include "slot_select.h"
#include <SDL2/SDL.h>

void slot_select_init(SlotSelect *s) {
    s->selected = 0;
    s->confirming_save = 0;
    s->overwriting_save = 0;
}

void slot_select_begin_save(SlotSelect *s, int occupied) {
    s->confirming_save = 1;
    s->overwriting_save = occupied;
}

SlotResult slot_select_handle_key(SlotSelect *s, int scancode) {
    if (s->confirming_save) {
        if (scancode == SDL_SCANCODE_Y) {
            s->confirming_save = 0;
            return SLOT_SAVE_CONFIRMED;
        }
        if (scancode == SDL_SCANCODE_N || scancode == SDL_SCANCODE_ESCAPE) {
            s->confirming_save = 0;
        }
        return SLOT_NONE;
    }
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
