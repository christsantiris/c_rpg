#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/systems/save_load.h"
#include <stdio.h>
#include <string.h>

#define DRAGONSPINE_TEST_SLOT 9984

void test_dragonspine(void) {
    static GameState g;
    static GameState loaded;
    memset(&g, 0, sizeof(g));
    game_init(&g);
    ASSERT("Dragonspine gate is hidden before the Goblin King falls",
        g.map.tiles[TOWN_DRAGON_GATE_Y][TOWN_W - 1] != TILE_TOWN_EXIT);

    g.defeated_bosses |= 1 << LOCATION_MOUNTAINS;
    game_enter_mountains(&g);
    game_return_to_town(&g);
    ASSERT("Goblin King victory opens a separate east-side High Pass gate",
        g.map.tiles[TOWN_DRAGON_GATE_Y][TOWN_W - 1] == TILE_TOWN_EXIT &&
        g.map.tiles[12][TOWN_W - 1] == TILE_TOWN_EXIT &&
        g.map.tiles[TOWN_DRAGON_GATE_Y][TOWN_W - 2] == TILE_TOWN_PATH);
    g.player.x = TOWN_W - 2;
    g.player.y = TOWN_DRAGON_GATE_Y;
    Action east = {ACTION_MOVE, TOWN_W - 1, TOWN_DRAGON_GATE_Y};
    action_resolve_player(&g, east);
    ASSERT("new gate leads to an enemy-free High Pass",
        g.location == LOCATION_HIGH_PASS && g.enemy_count == 0 &&
        g.map.stairs_down_x == HIGH_PASS_W - 1);

    g.player.x = HIGH_PASS_W - 2;
    g.player.y = g.map.stairs_down_y;
    Action pass_exit = {ACTION_MOVE, HIGH_PASS_W - 1, g.map.stairs_down_y};
    action_resolve_player(&g, pass_exit);
    ASSERT("High Pass leads to Dragonspine stage one",
        g.location == LOCATION_DRAGONSPINE && g.level == 1 && g.enemy_count > 0);
    ASSERT("Dragonspine uses distinct cliff terrain",
        g.map.tiles[g.map.stairs_up_y][g.map.stairs_up_x] == TILE_DRAGON_ENTRANCE);
    ASSERT("Dragonspine trails open at the west and east map edges",
        g.map.stairs_up_x == 0 &&
        g.map.stairs_down_x == SWAMP_MAP_W - 1 &&
        g.map.tiles[g.map.stairs_down_y][SWAMP_MAP_W - 1] == TILE_DRAGON_EXIT &&
        map_is_walkable(&g.map, 1, g.map.stairs_up_y) &&
        map_is_walkable(&g.map, SWAMP_MAP_W - 2, g.map.stairs_down_y));

    for (int level = 2; level <= DRAGONSPINE_DEPTH; level++) {
        game_descend(&g);
    }
    int dragon_found = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        dragon_found |= g.enemies[i].active && g.enemies[i].type == ENEMY_RED_DRAGON;
    }
    ASSERT("the fifth stage holds the Red Dragon boss",
        g.level == DRAGONSPINE_DEPTH && dragon_found);
    Item reward = boss_equipment_reward(ENEMY_RED_DRAGON);
    ASSERT("Red Dragon has a unique equipment drop",
        strcmp(reward.name, "Dragon Scale Mantle") == 0);
    g.player.x = g.map.stairs_down_x - 1;
    g.player.y = g.map.stairs_down_y;
    Action summit_exit = {ACTION_MOVE, g.map.stairs_down_x, g.map.stairs_down_y};
    action_resolve_player(&g, summit_exit);
    ASSERT("the living dragon alone blocks the summit exit",
        g.location == LOCATION_DRAGONSPINE);
    for (int i = 0; i < g.enemy_count; i++) {
        Enemy *enemy = &g.enemies[i];
        if (enemy->type == ENEMY_RED_DRAGON) {
            enemy->hp = 1;
            g.player.x = enemy->x - 1;
            g.player.y = enemy->y;
            action_resolve_player(&g, (Action){ACTION_MOVE, enemy->x, enemy->y});
            break;
        }
    }
    int mantle_dropped = 0;
    for (int i = 0; i < g.floor_item_count; i++) {
        mantle_dropped += g.floor_items[i].active &&
            strcmp(g.floor_items[i].item.name, "Dragon Scale Mantle") == 0;
    }
    ASSERT("defeating the dragon drops its mantle and records victory",
        mantle_dropped == 1 &&
        (g.defeated_bosses & (1 << LOCATION_DRAGONSPINE)));
    g.player.x = g.map.stairs_down_x - 1;
    g.player.y = g.map.stairs_down_y;
    action_resolve_player(&g, summit_exit);
    ASSERT("summit exit returns to town after the dragon falls",
        g.location == LOCATION_TOWN);

    game_enter_high_pass(&g, 1);
    game_enter_dragonspine(&g);
    game_descend(&g);
    int saved_level = g.level;
    game_open_town_portal(&g);
    ASSERT("Dragonspine portal returns to the High Pass gate in town",
        g.location == LOCATION_TOWN && g.portal_active &&
        g.map.tiles[TOWN_DRAGON_GATE_Y + 1][41] == TILE_PORTAL);
    int saved = save_game(&g, DRAGONSPINE_TEST_SLOT);
    int restored = saved && load_game(&loaded, DRAGONSPINE_TEST_SLOT);
    ASSERT("Dragonspine expedition and portal survive save/load",
        restored && loaded.portal_active &&
        loaded.portal_location == LOCATION_DRAGONSPINE &&
        loaded.map.tiles[TOWN_DRAGON_GATE_Y][TOWN_W - 1] == TILE_TOWN_EXIT &&
        loaded.max_dragonspine_level_reached == saved_level &&
        loaded.dragonspine_cache[saved_level - 1].valid);
    if (restored) {
        game_use_town_portal(&loaded);
        ASSERT("portal resumes the saved Dragonspine stage",
            loaded.location == LOCATION_DRAGONSPINE && loaded.level == saved_level);
    }
    remove("saves/savegame_9984.json");

    memset(&g, 0, sizeof(g));
    game_init(&g);
    g.defeated_bosses |= 1 << LOCATION_MOUNTAINS;
    game_enter_mountains(&g);
    game_return_to_town(&g);
    ASSERT("Ilya appears beside the unlocked one-tile High Pass road",
        g.map.tiles[TOWN_DRAGON_NPC_Y][TOWN_DRAGON_NPC_X] ==
            TILE_NPC_DRAGON_SEEKER &&
        g.map.tiles[TOWN_DRAGON_NPC_Y][TOWN_DRAGON_NPC_X + 1] ==
            TILE_TOWN_PATH &&
        !map_is_walkable(&g.map, TOWN_DRAGON_NPC_X, TOWN_DRAGON_NPC_Y));
    game_enter_dragonspine(&g);
    game_open_town_portal(&g);
    game_talk_to_dragon_seeker(&g);
    ASSERT("Ilya assigns the Dragonspine treasure quest once",
        g.dragon_treasure_quest_state == 1 && !g.portal_active &&
        g.map.tiles[TOWN_DRAGON_GATE_Y + 1][41] == TILE_TOWN_FLOOR);
    game_enter_dragonspine(&g);
    for (int level = 2; level <= DRAGONSPINE_DEPTH; level++) {
        game_descend(&g);
    }
    Room *lair = &g.map.rooms[g.map.room_count - 1];
    int treasure_x;
    int treasure_y;
    map_room_center(lair, &treasure_x, &treasure_y);
    treasure_x += 2;
    treasure_y += 1;
    ASSERT("golden dragon seal is marked in the summit hoard",
        g.map.tiles[treasure_y][treasure_x] == TILE_DRAGON_TREASURE);
    g.player.x = treasure_x;
    g.player.y = treasure_y;
    ASSERT("standing on the seal enables interaction",
        game_has_regional_interaction(&g));
    action_resolve_player(&g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("collecting the seal records quest progress without an inventory item",
        g.dragon_treasure_quest_state == 2 &&
        g.map.tiles[treasure_y][treasure_x] == TILE_DRAGON_HOARD);
    saved = save_game(&g, DRAGONSPINE_TEST_SLOT);
    restored = saved && load_game(&loaded, DRAGONSPINE_TEST_SLOT);
    ASSERT("collected dragon treasure stays collected after saving",
        restored && loaded.dragon_treasure_quest_state == 2 &&
        loaded.map.tiles[treasure_y][treasure_x] == TILE_DRAGON_HOARD);
    if (restored) {
        game_return_to_town(&loaded);
        while (loaded.inventory_count < MAX_INVENTORY) {
            loaded.inventory[loaded.inventory_count++] = item_make_health_potion();
        }
        game_talk_to_dragon_seeker(&loaded);
        ASSERT("Ilya waits to award the potion until the pack has space",
            loaded.dragon_treasure_quest_state == 2 &&
            loaded.inventory_count == MAX_INVENTORY);
        game_remove_inventory_item(&loaded, MAX_INVENTORY - 1);
        game_talk_to_dragon_seeker(&loaded);
        ASSERT("turn-in awards exactly one Potion of Strength",
            loaded.dragon_treasure_quest_state == 3 &&
            loaded.inventory_count == MAX_INVENTORY &&
            loaded.inventory[MAX_INVENTORY - 1].type == ITEM_POTION_STRENGTH);
        game_talk_to_dragon_seeker(&loaded);
        ASSERT("Ilya cannot award the potion twice",
            loaded.inventory_count == MAX_INVENTORY);
        ASSERT("starter weapon can be equipped before testing the base bonus",
            game_equip_main_hand(&loaded, 0));
        int attack = loaded.player.attack;
        action_resolve_player(&loaded,
            (Action){ACTION_USE_ITEM, MAX_INVENTORY - 1, 0});
        ASSERT("Potion of Strength permanently adds one base attack",
            loaded.player.attack == attack + 1 &&
            loaded.inventory_count == MAX_INVENTORY - 1);
        if (loaded.equipped_main_hand >= 0) {
            int equipped = loaded.equipped_main_hand;
            int weapon_bonus = loaded.inventory[equipped].attack_bonus;
            game_unequip_main_hand(&loaded);
            ASSERT("strength bonus remains after removing a weapon",
                loaded.player.attack == attack + 1 - weapon_bonus);
            game_equip_main_hand(&loaded, equipped);
            ASSERT("strength bonus remains after re-equipping a weapon",
                loaded.player.attack == attack + 1);
        }
        saved = save_game(&loaded, DRAGONSPINE_TEST_SLOT);
        restored = saved && load_game(&g, DRAGONSPINE_TEST_SLOT);
        ASSERT("quest completion and attack gain survive save/load",
            restored && g.dragon_treasure_quest_state == 3 &&
            g.player.attack == attack + 1);
    }
    remove("saves/savegame_9984.json");
}
