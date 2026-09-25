#include "shop.h"
#include <SDL2/SDL.h>

int shop_buy_price(const Item *item) {
    return item->value * 2;
}

int shop_sell_price(const Item *item) {
    return item->value / 4;
}

int shop_accepts_item(ShopType type, const Item *item) {
    if (type == SHOP_TYPE_BLACKSMITH) {
        return item->type == ITEM_WEAPON || item->type == ITEM_ARMOR ||
            item->type == ITEM_SHIELD;
    }
    if (type == SHOP_TYPE_ALCHEMIST) {
        return item->type == ITEM_POTION_HEALTH || item->type == ITEM_POTION_MANA ||
            item->type == ITEM_SCROLL || item->type == ITEM_SPELL_TOME;
    }
    return 0;
}

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
        int boss_count = defeated_boss_count(defeated_bosses);
        s->stock_tier = boss_count + 1;
        s->items[s->item_count++] = item_make_health_potion();
        s->items[s->item_count++] = item_make_mana_potion();
        s->items[s->item_count++] = item_make_scroll_magic_arrow();
        s->items[s->item_count++] = item_make_scroll_fireball();
        s->items[s->item_count++] = item_make_scroll_heal();
        if (s->stock_tier >= 2) {
            s->items[s->item_count++] = item_make_magic_arrow_tome();
            s->items[s->item_count++] = item_make_scroll_frost_bolt();
        }
        if (s->stock_tier >= 3) {
            s->items[s->item_count++] = item_make_fireball_tome();
            s->items[s->item_count++] = item_make_scroll_teleport();
        }
        if (s->stock_tier >= 4) {
            s->items[s->item_count++] = item_make_heal_tome();
        }
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
        s->items[s->item_count++] = item_make_buckler();

        if (s->stock_tier >= 2) {
            s->items[s->item_count++] = item_make_long_sword();
            s->items[s->item_count++] = item_make_battle_axe();
            s->items[s->item_count++] = item_make_greatsword();
            s->items[s->item_count++] = item_make_longbow();
            s->items[s->item_count++] = item_make_runed_staff();
            s->items[s->item_count++] = item_make_scale_mail();
            s->items[s->item_count++] = item_make_studded_leather();
            s->items[s->item_count++] = item_make_runed_robes();
            s->items[s->item_count++] = item_make_kite_shield();
        }
        if (s->stock_tier >= 3) {
            s->items[s->item_count++] = item_make_magic_long_sword();
            s->items[s->item_count++] = item_make_magic_battle_axe();
            s->items[s->item_count++] = item_make_magic_dagger();
            s->items[s->item_count++] = item_make_magic_greatsword();
            s->items[s->item_count++] = item_make_magic_staff();
            s->items[s->item_count++] = item_make_magic_longbow();
            s->items[s->item_count++] = item_make_plate_armor();
            s->items[s->item_count++] = item_make_ranger_cloak();
            s->items[s->item_count++] = item_make_enchanter_robes();
            s->items[s->item_count++] = item_make_tower_shield();
            s->items[s->item_count++] = item_make_magic_shield();
        }
        if (s->stock_tier >= 4) {
            s->items[s->item_count++] = item_make_magic_plate();
            s->items[s->item_count++] = item_make_shadow_armor();
            s->items[s->item_count++] = item_make_archmage_robes();
        }
    }
}

ShopResult shop_handle_key(ShopScreen *s, int scancode) {
    if (s->type == SHOP_TYPE_HEALER || s->type == SHOP_TYPE_WITCH) {
        int last_option = 2;
        switch (scancode) {
            case SDL_SCANCODE_UP:
            case SDL_SCANCODE_W:
                s->selected--;
                if (s->selected < 0) {
                    s->selected = 0;
                }
                break;
            case SDL_SCANCODE_DOWN:
            case SDL_SCANCODE_S:
                s->selected++;
                if (s->selected > last_option) {
                    s->selected = last_option;
                }
                break;
            case SDL_SCANCODE_RETURN:
            case SDL_SCANCODE_KP_ENTER:
                if (s->selected == last_option) {
                    return SHOP_CLOSED;
                }
                if (s->selected == 1) {
                    return s->type == SHOP_TYPE_WITCH
                        ? SHOP_EMERGENCY_MANA : SHOP_EMERGENCY_HEAL;
                }
                return s->type == SHOP_TYPE_WITCH
                    ? SHOP_RESTORE_MANA : SHOP_HEAL;
            case SDL_SCANCODE_ESCAPE:
                return SHOP_CLOSED;
            default:
                break;
        }
        return SHOP_NONE;
    }
    switch (scancode) {
        case SDL_SCANCODE_UP:
            s->selected--;
            if (s->selected < 0) {
                s->selected = 0;
            }
            break;
        case SDL_SCANCODE_DOWN:
            s->selected++;
            if (s->mode == 0) {
                if (s->selected >= s->item_count) {
                    s->selected = s->item_count - 1;
                }
            }
            break;
        case SDL_SCANCODE_TAB:
            s->mode = s->mode == 0 ? 1 : 0;
            s->selected = 0;
            break;
        case SDL_SCANCODE_RETURN:
            if (s->mode == 0) {
                return SHOP_BUY;
            } else {
                return SHOP_SELL;
            }
        case SDL_SCANCODE_ESCAPE:
            return SHOP_CLOSED;
        default:
            break;
    }
    return SHOP_NONE;
}
