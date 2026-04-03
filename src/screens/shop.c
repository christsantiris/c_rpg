#include "shop.h"
#include <SDL2/SDL.h>

void shop_init(ShopScreen *s, ShopType type) {
    s->selected   = 0;
    s->type       = type;
    s->item_count = 0;

    if (type == SHOP_TYPE_ALCHEMIST) {
        s->items[s->item_count++] = item_make_health_potion();
        s->items[s->item_count++] = item_make_mana_potion();
        s->items[s->item_count++] = item_make_scroll_magic_arrow();
        s->items[s->item_count++] = item_make_scroll_fireball();
        s->items[s->item_count++] = item_make_scroll_heal();
    }
    if (type == SHOP_TYPE_BLACKSMITH) {
        s->items[s->item_count++] = item_make_rusty_sword();
        s->items[s->item_count++] = item_make_short_sword();
        s->items[s->item_count++] = item_make_long_sword();
        s->items[s->item_count++] = item_make_battle_axe();
        s->items[s->item_count++] = item_make_staff();
        s->items[s->item_count++] = item_make_bow();
        s->items[s->item_count++] = item_make_leather_armor();
        s->items[s->item_count++] = item_make_chain_mail();
    }
}

ShopResult shop_handle_key(ShopScreen *s, int scancode) {
    switch (scancode) {
        case SDL_SCANCODE_UP:
            s->selected--;
            if (s->selected < 0) s->selected = 0;
            break;
        case SDL_SCANCODE_DOWN:
            s->selected++;
            if (s->selected >= s->item_count)
                s->selected = s->item_count - 1;
            break;
        case SDL_SCANCODE_RETURN:
            if (s->item_count > 0) return SHOP_BUY;
            break;
        case SDL_SCANCODE_ESCAPE:
            return SHOP_CLOSED;
        default:
            break;
    }
    return SHOP_NONE;
}