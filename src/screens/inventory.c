#include "inventory.h"
#include <SDL2/SDL.h>

void inventory_init(InventoryScreen *s) {
    s->selected = 0;
}

InventoryResult inventory_handle_key(InventoryScreen *s, int scancode) {
    switch (scancode) {
        case SDL_SCANCODE_UP:
            s->selected--;
            if (s->selected < 0) s->selected = 0;
            break;
        case SDL_SCANCODE_DOWN:
            s->selected++;
            break;
        case SDL_SCANCODE_ESCAPE:
        case SDL_SCANCODE_I:
            return INVENTORY_CLOSED;
        default:
            break;
    }
    return INVENTORY_NONE;
}