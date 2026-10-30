#include "inventory.h"
#include "../game/game.h"
#include "../renderer/renderer.h"
#include <SDL2/SDL.h>

void inventory_init(InventoryScreen *s) {
    *s = (InventoryScreen){0};
}

int inventory_selected_index(const InventoryScreen *s) {
    return s->selected + (s->equipment_tab ? MAX_INVENTORY : 0);
}

int inventory_visible_rows(int tiles_y) {
    int rows = ((tiles_y - 9) * TILE_SIZE - 142) / 36;
    return rows > 0 ? rows : 1;
}

int inventory_list_start(int selected, int count, int visible_rows) {
    int start = selected - visible_rows + 1;
    int max_start = count > visible_rows ? count - visible_rows : 0;
    if (start > max_start) {
        start = max_start;
    }
    return start > 0 ? start : 0;
}

void inventory_clamp_selection(InventoryScreen *s, int bag_count) {
    int count = s->equipment_tab ? EQUIPMENT_SLOT_COUNT : bag_count;
    if (s->selected >= count) {
        s->selected = count - 1;
    }
    if (s->selected < 0) {
        s->selected = 0;
    }
}

InventoryResult inventory_handle_key(InventoryScreen *s, int scancode, int item_count, int close_key) {
    int count = s->equipment_tab ? EQUIPMENT_SLOT_COUNT : item_count;
    s->accessory_slot = 0;
    switch (scancode) {
        case SDL_SCANCODE_TAB:
            s->equipment_tab = !s->equipment_tab;
            s->selected = 0;
            break;
        case SDL_SCANCODE_UP:
            s->selected--;
            break;
        case SDL_SCANCODE_DOWN:
            s->selected++;
            break;
        case SDL_SCANCODE_U:
            if (!s->equipment_tab && count > 0) {
                return INVENTORY_USE;
            }
            break;
        case SDL_SCANCODE_Q:
            if (!s->equipment_tab && count > 0) {
                return INVENTORY_SET_QUICK_POTION;
            }
            break;
        case SDL_SCANCODE_1:
        case SDL_SCANCODE_2:
            s->accessory_slot = scancode == SDL_SCANCODE_1 ? 1 : 2;
            if (!s->equipment_tab && count > 0) {
                return INVENTORY_EQUIP;
            }
            break;
        case SDL_SCANCODE_E:
        case SDL_SCANCODE_RETURN:
        case SDL_SCANCODE_KP_ENTER:
            if (count > 0) {
                return s->equipment_tab ? INVENTORY_UNEQUIP : INVENTORY_EQUIP;
            }
            break;
        case SDL_SCANCODE_O:
            if (!s->equipment_tab && count > 0) {
                return INVENTORY_EQUIP_OFF_HAND;
            }
            break;
        case SDL_SCANCODE_X:
            if (s->equipment_tab) {
                return INVENTORY_UNEQUIP;
            }
            break;
        case SDL_SCANCODE_ESCAPE:
            return INVENTORY_CLOSED;
        case SDL_SCANCODE_D:
            if (!s->equipment_tab && count > 0) {
                return INVENTORY_DROP;
            }
            break;
        default:
            if (scancode == close_key) {
                return INVENTORY_CLOSED;
            }
            break;
    }
    inventory_clamp_selection(s, item_count);
    return INVENTORY_NONE;
}
