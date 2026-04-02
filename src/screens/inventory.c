#include "inventory.h"
#include <SDL2/SDL.h>

void inventory_init(InventoryScreen *s) {
    s->selected = 0;
}

InventoryResult inventory_handle_key(InventoryScreen *s, int scancode,
                                      int item_count) {
    switch (scancode) {
        case SDL_SCANCODE_UP:
            s->selected--;
            if (s->selected < 0) s->selected = 0;
            break;
        case SDL_SCANCODE_DOWN:
            s->selected++;
            if (s->selected >= item_count) s->selected = item_count - 1;
            break;
        case SDL_SCANCODE_U:
            if (item_count > 0) return INVENTORY_USE;
            break;
        case SDL_SCANCODE_E:
            if (item_count > 0) return INVENTORY_EQUIP;
            break;
        case SDL_SCANCODE_ESCAPE:
        case SDL_SCANCODE_I:
            return INVENTORY_CLOSED;
        case SDL_SCANCODE_D:
            if (item_count > 0) return INVENTORY_DROP;
            break;
        default:
            break;
    }
    return INVENTORY_NONE;
}