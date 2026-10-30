#include "test_utils.h"
#include "game/game.h"
#include "game/castle.h"
#include "game/jail.h"
#include "screens/shop.h"
#include "systems/save_load.h"
#include <stdlib.h>
#include <string.h>

static ShopType accessory_shop(AccessoryFamily family) {
    if (family == ACCESSORY_FAMILY_BOOTS) {
        return SHOP_TYPE_COBBLER;
    }
    if (family == ACCESSORY_FAMILY_BELT) {
        return SHOP_TYPE_BELT_MAKER;
    }
    return family == ACCESSORY_FAMILY_RING || family == ACCESSORY_FAMILY_AMULET ? SHOP_TYPE_GOLDSMITH : SHOP_TYPE_BLACKSMITH;
}

static int shop_contains(const ShopScreen *shop, const Item *item) {
    for (int i = 0; i < shop->item_count; i++) {
        if (strcmp(shop->items[i].name, item->name) == 0) {
            return 1;
        }
    }
    return 0;
}

void test_accessory_expansion(void) {
    printf("Accessory tiers, specialties and persistence:\n");
    const ShopType shops[] = {SHOP_TYPE_BLACKSMITH, SHOP_TYPE_GOLDSMITH, SHOP_TYPE_COBBLER, SHOP_TYPE_BELT_MAKER};
    int bosses[] = {0, 1 << LOCATION_DUNGEON, (1 << LOCATION_DUNGEON) | (1 << LOCATION_FOREST)};
    int stock = 1;
    for (int cls = CLASS_WARRIOR; cls <= CLASS_ROGUE; cls++) {
        for (int progress = 0; progress < 3; progress++) {
            for (int store = 0; store < 4; store++) {
                ShopScreen shop;
                shop_init(&shop, shops[store], bosses[progress], cls);
                stock &= shop.item_count <= MAX_SHOP_ITEMS && shop.stock_tier == progress + 1;
                for (int family = ACCESSORY_FAMILY_GAUNTLETS; family <= ACCESSORY_FAMILY_BOOTS; family++) {
                    for (int tier = 1; tier <= 3; tier++) {
                        Item item = item_make_accessory(family, tier);
                        stock &= shop_contains(&shop, &item) == (accessory_shop(family) == shops[store] && tier <= progress + 1);
                        stock &= shop_accepts_item(shops[store], &item) == (accessory_shop(family) == shops[store]);
                    }
                }
            }
        }
    }
    ASSERT("all eighteen accessories unlock at the intended boss tier and sell only at their specialty shop for every class", stock);
    ShopScreen shop;
    shop_init(&shop, SHOP_TYPE_COBBLER, 1 << LOCATION_LABYRINTH, CLASS_WARRIOR);
    ASSERT("Labyrinth repeats do not unlock accessory tiers", shop.stock_tier == 1 && shop.item_count == 1);
    ASSERT("invalid accessory categories and tiers return no item", !item_make_accessory(ACCESSORY_FAMILY_NONE, 1).active && !item_make_accessory(ACCESSORY_FAMILY_BOOTS, 4).active);
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("expanded accessory fixtures allocate", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    int equipment = 1;
    for (int cls = CLASS_WARRIOR; cls <= CLASS_ROGUE; cls++) {
        for (int family = ACCESSORY_FAMILY_GAUNTLETS; family <= ACCESSORY_FAMILY_BOOTS; family++) {
            for (int tier = 1; tier <= 3; tier++) {
                g->player.player_class = cls;
                game_init(g);
                g->inventory_count = 0;
                g->gold = 10000;
                g->player.hp = 37;
                g->player.mp = 5;
                Item item = item_make_accessory(family, tier);
                int attack = g->player.attack;
                int defense = g->player.defense;
                int hp = g->player.max_hp;
                int mp = g->player.max_mp;
                equipment &= shop_purchase(g, &item) && g->gold == 10000 - shop_buy_price(&item);
                equipment &= game_equip_item(g, 0, EQUIPMENT_ACCESSORY_1) && g->inventory_count == 0;
                equipment &= g->player.attack == attack + item.attack_bonus && g->player.defense == defense + item.defense_bonus && g->player.max_hp == hp + item.max_hp_bonus && g->player.max_mp == mp + item.max_mp_bonus && g->player.hp == 37 && g->player.mp == 5;
                g->inventory[g->inventory_count++] = item;
                int duplicate = game_equip_item(g, 0, EQUIPMENT_ACCESSORY_2);
                equipment &= duplicate == (family == ACCESSORY_FAMILY_RING);
                if (duplicate) {
                    equipment &= game_unequip_item(g, EQUIPMENT_ACCESSORY_2);
                }
                equipment &= game_unequip_item(g, EQUIPMENT_ACCESSORY_1);
                equipment &= g->player.attack == attack && g->player.defense == defense && g->player.max_hp == hp && g->player.max_mp == mp && g->player.hp == 37 && g->player.mp == 5;
            }
        }
    }
    ASSERT("all accessory tiers buy, equip, enforce category limits and reverse their bonuses without healing exploits", equipment);
    int saved = 1;
    for (int family = ACCESSORY_FAMILY_GAUNTLETS; family <= ACCESSORY_FAMILY_BOOTS; family++) {
        for (int tier = 1; tier <= 3; tier++) {
            game_init(g);
            Item item = item_make_accessory(family, tier);
            g->inventory_count = 1;
            g->inventory[0] = item;
            game_restore_equipment(g, &item, EQUIPMENT_ACCESSORY_1);
            g->floor_item_count = 1;
            g->floor_items[0] = (FloorItem){.active = 1, .x = 17, .y = 17, .underlying_tile = TILE_TOWN_FLOOR, .item = item};
            g->map.tiles[17][17] = TILE_ITEM;
            g->castle_cache[0].valid = 1;
            castle_generate(&g->castle_cache[0].map, 1);
            g->castle_loot_count[0] = 1;
            g->castle_loot[0][0] = (FloorItem){.active = 1, .x = 15, .y = 11, .underlying_tile = TILE_CASTLE_FLOOR, .item = item};
            saved &= save_game(g, 99229) && load_game(loaded, 99229);
            const Item *worn = game_equipped_item(loaded, EQUIPMENT_ACCESSORY_1);
            saved &= worn && !memcmp(worn, &item, sizeof(item)) && !memcmp(&loaded->inventory[0], &item, sizeof(item)) && !memcmp(&loaded->floor_items[0].item, &item, sizeof(item)) && !memcmp(&loaded->castle_loot[0][0].item, &item, sizeof(item));
        }
    }
    ASSERT("all tiers retain category, rarity and bonuses in bags, worn slots, dropped loot and cached castle loot", saved);
    game_init(g);
    g->inventory_count = 0;
    Item boots = item_make_accessory(ACCESSORY_FAMILY_BOOTS, 3);
    Item belt = item_make_accessory(ACCESSORY_FAMILY_BELT, 3);
    game_restore_equipment(g, &boots, EQUIPMENT_ACCESSORY_1);
    game_restore_equipment(g, &belt, EQUIPMENT_ACCESSORY_2);
    int hp = g->player.max_hp;
    int defense = g->player.defense;
    game_enter_town_gate(g, LOCATION_CASTLE, INFORMANT_X, INFORMANT_Y + 1);
    ASSERT("arrest confiscates boots and belts with their bonuses", jail_talk_nearby(g) && !game_equipped_item(g, EQUIPMENT_ACCESSORY_1) && !game_equipped_item(g, EQUIPMENT_ACCESSORY_2) && g->player.max_hp == hp - 30 && g->player.defense == defense - 4);
    ASSERT("confiscated new categories survive save/load", save_game(g, 99229) && load_game(loaded, 99229) && loaded->jail_gear_count == 2 && loaded->jail_gear[0].accessory_family == ACCESSORY_FAMILY_BOOTS && loaded->jail_gear[1].accessory_family == ACCESSORY_FAMILY_BELT);
    loaded->jail_quest_state = 3;
    loaded->dialogue_active = 0;
    game_enter_town_gate(loaded, LOCATION_CASTLE, JAIL_DOOR_X, JAIL_DOOR_Y + 1);
    action_resolve_player(loaded, (Action){ACTION_MOVE, JAIL_DOOR_X, JAIL_DOOR_Y});
    for (int i = 0; i < loaded->enemy_count; i++) {
        loaded->enemies[i].active = 0;
    }
    loaded->player.x = JAIL_GEAR_X;
    loaded->player.y = JAIL_GEAR_Y + 1;
    jail_recover_gear(loaded);
    ASSERT("evidence recovery restores boots and belt to their original accessory slots once", loaded->jail_gear_state == 3 && game_equipped_item(loaded, EQUIPMENT_ACCESSORY_1) && game_equipped_item(loaded, EQUIPMENT_ACCESSORY_2) && loaded->player.max_hp == hp && loaded->player.defense == defense);
    remove("saves/savegame_99229.json");
    free(g);
    free(loaded);
}
