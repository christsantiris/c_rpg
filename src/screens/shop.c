#include "shop.h"
#include "../game/game.h"
#include <SDL2/SDL.h>

int shop_buy_price(const Item *item) {
    return item->value * 2;
}

int shop_purchase_price(const GameState *g, const Item *item) {
    int price = shop_buy_price(item);
    if (item->type == ITEM_ARROWS) {
        int amount = MAX_ARROWS - g->player.arrows;
        if (amount < ARROW_BUNDLE_SIZE) {
            price = (price * amount + ARROW_BUNDLE_SIZE - 1) / ARROW_BUNDLE_SIZE;
        }
    }
    return price;
}

int shop_purchase(GameState *g, const Item *item) {
    if ((item->type == ITEM_SCROLL || item->type == ITEM_SPELL_TOME) &&
        !item_class_allowed(item, g->player.player_class)) {
        push_message(g, "Your class cannot use that spell item.");
        return 0;
    }
    if (item->type == ITEM_ARROWS && g->player.arrows >= MAX_ARROWS) {
        push_message(g, "Your quiver is full: 100 arrows.");
        return 0;
    }
    int price = shop_purchase_price(g, item);
    if (g->gold < price) {
        push_message(g, "Not enough gold!");
        return 0;
    }
    if (item->type == ITEM_ARROWS) {
        int amount = MAX_ARROWS - g->player.arrows;
        if (amount > ARROW_BUNDLE_SIZE) {
            amount = ARROW_BUNDLE_SIZE;
        }
        g->player.arrows += amount;
        g->gold -= price;
        char message[MAX_MESSAGE_LEN];
        snprintf(message, sizeof(message), "Bought %d arrows. Quiver: %d/%d.", amount, g->player.arrows, MAX_ARROWS);
        push_message(g, message);
        return 1;
    }
    if (g->inventory_count >= MAX_INVENTORY) {
        push_message(g, "Inventory full!");
        return 0;
    }
    g->gold -= price;
    g->inventory[g->inventory_count++] = *item;
    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message), "Bought %s", item->name);
    push_message(g, message);
    return 1;
}

int shop_sell_price(const Item *item) {
    return item->value / 4;
}

int shop_accepts_item(ShopType type, const Item *item) {
    if (type == SHOP_TYPE_BLACKSMITH) {
        return item->type == ITEM_WEAPON || item->type == ITEM_ARMOR ||
            item->type == ITEM_SHIELD || item->type == ITEM_ARROWS;
    }
    if (type == SHOP_TYPE_ALCHEMIST) {
        return item->type == ITEM_POTION_HEALTH || item->type == ITEM_POTION_MANA ||
            item->type == ITEM_SCROLL || item->type == ITEM_SPELL_TOME;
    }
    if (type == SHOP_TYPE_HEALER) {
        return item->type == ITEM_POTION_HEALTH;
    }
    if (type == SHOP_TYPE_WITCH) {
        return item->type == ITEM_POTION_MANA;
    }
    if (type == SHOP_TYPE_APOTHECARY) {
        return item->type == ITEM_POTION_HEALTH || item->type == ITEM_POTION_MANA ||
            item->type == ITEM_POTION_STRENGTH ||
            item->type == ITEM_POTION_INTELLIGENCE;
    }
    return 0;
}

static int defeated_boss_count(int defeated_bosses) {
    defeated_bosses &= ~(1 << LOCATION_LABYRINTH);
    int count = 0;
    while (defeated_bosses) {
        count += defeated_bosses & 1;
        defeated_bosses >>= 1;
    }
    return count;
}

void shop_init(ShopScreen *s, ShopType type, int defeated_bosses, PlayerClass player_class) {
    s->selected = 0;
    s->type = type;
    s->item_count = 0;
    s->mode = 0;
    s->stock_tier = 0;

    if (type == SHOP_TYPE_HEALER) {
        s->items[s->item_count++] = item_make_health_potion();
    } else if (type == SHOP_TYPE_WITCH) {
        s->items[s->item_count++] = item_make_mana_potion();
    } else if (type == SHOP_TYPE_APOTHECARY) {
        s->items[s->item_count++] = item_make_health_potion();
        s->items[s->item_count++] = item_make_mana_potion();
        s->items[s->item_count++] = item_make_strength_potion();
        s->items[s->item_count++] = item_make_intelligence_potion();
    }

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
        s->items[s->item_count++] = item_make_arrows();
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
    int count = 0;
    for (int i = 0; i < s->item_count; i++) {
        Item *item = &s->items[i];
        if ((item->type == ITEM_SCROLL || item->type == ITEM_SPELL_TOME) &&
            !item_class_allowed(item, player_class)) {
            continue;
        }
        s->items[count++] = *item;
    }
    s->item_count = count;
}

ShopResult shop_handle_key(ShopScreen *s, int scancode) {
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
        case SDL_SCANCODE_KP_ENTER:
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
