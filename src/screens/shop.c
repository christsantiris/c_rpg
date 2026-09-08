#include "shop.h"
#include <SDL2/SDL.h>

static int defeated_boss_count(int defeated_bosses) {
    int count = 0;
    while (defeated_bosses) {
        count += defeated_bosses & 1;
        defeated_bosses >>= 1;
    }
    return count;
}

void shop_init(ShopScreen *s, ShopType type, int defeated_bosses) {
    s->selected = 0;
    s->type = type;
    s->item_count = 0;
    s->mode = 0;
    s->stock_tier = 0;

    if (type == SHOP_TYPE_ALCHEMIST) {
        s->items[s->item_count++] = item_make_health_potion();
        s->items[s->item_count++] = item_make_mana_potion();
        s->items[s->item_count++] = item_make_scroll_magic_arrow();
        s->items[s->item_count++] = item_make_scroll_fireball();
        s->items[s->item_count++] = item_make_scroll_heal();
    }
    if (type == SHOP_TYPE_BLACKSMITH) {
        int boss_count = defeated_boss_count(defeated_bosses);
        s->stock_tier = boss_count + 1;
        if (s->stock_tier > 4) {
            s->stock_tier = 4;
        }

        s->items[s->item_count++] = item_make_rusty_sword();
        s->items[s->item_count++] = item_make_short_sword();
        s->items[s->item_count++] = item_make_staff();
        s->items[s->item_count++] = item_make_bow();
        s->items[s->item_count++] = item_make_dagger();
        s->items[s->item_count++] = item_make_leather_armor();
        s->items[s->item_count++] = item_make_chain_mail();
        s->items[s->item_count++] = item_make_apprentice_robes();

        if (s->stock_tier >= 2) {
            s->items[s->item_count++] = item_make_long_sword();
            s->items[s->item_count++] = item_make_battle_axe();
            s->items[s->item_count++] = item_make_greatsword();
            s->items[s->item_count++] = item_make_longbow();
            s->items[s->item_count++] = item_make_runed_staff();
            s->items[s->item_count++] = item_make_scale_mail();
            s->items[s->item_count++] = item_make_studded_leather();
            s->items[s->item_count++] = item_make_runed_robes();
        }
        if (s->stock_tier >= 3) {
            s->items[s->item_count++] = item_make_magic_long_sword();
            s->items[s->item_count++] = item_make_magic_battle_axe();
            s->items[s->item_count++] = item_make_magic_dagger();
            s->items[s->item_count++] = item_make_plate_armor();
            s->items[s->item_count++] = item_make_ranger_cloak();
            s->items[s->item_count++] = item_make_enchanter_robes();
        }
        if (s->stock_tier >= 4) {
            s->items[s->item_count++] = item_make_magic_greatsword();
            s->items[s->item_count++] = item_make_magic_staff();
            s->items[s->item_count++] = item_make_magic_longbow();
            s->items[s->item_count++] = item_make_magic_plate();
            s->items[s->item_count++] = item_make_shadow_armor();
            s->items[s->item_count++] = item_make_archmage_robes();
        }
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
            if (s->type == SHOP_TYPE_ALCHEMIST || s->mode == 0) {
                if (s->selected >= s->item_count)
                    s->selected = s->item_count - 1;
            }
            break;
        case SDL_SCANCODE_TAB:
            s->mode = s->mode == 0 ? 1 : 0;
            s->selected = 0;
            break;
        case SDL_SCANCODE_RETURN:
            if (s->mode == 0) return SHOP_BUY;
            else              return SHOP_SELL;
        case SDL_SCANCODE_ESCAPE:
            return SHOP_CLOSED;
        default:
            break;
    }
    return SHOP_NONE;
}
