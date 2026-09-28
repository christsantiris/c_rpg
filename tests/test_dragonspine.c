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
}
