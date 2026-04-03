#include "test_utils.h"
#include "../src/game/map.h"
#include "../src/game/game.h"
#include "../src/game/actions.h"
#include <stdlib.h>
#include <time.h>

void test_dungeon(void) {
    printf("Dungeon generation tests:\n");

    srand((unsigned)time(NULL));

    for (int run = 0; run < 5; run++) {
        Map m;
        map_generate(&m, 1);

        ASSERT("at least min rooms generated", m.room_count >= MIN_ROOMS);
        ASSERT("at most max rooms generated",  m.room_count <= MAX_ROOMS);

        ASSERT("stairs up tile is set",
            m.tiles[m.stairs_up_y][m.stairs_up_x] == TILE_STAIRS_UP);
        ASSERT("stairs down tile is set",
            m.tiles[m.stairs_down_y][m.stairs_down_x] == TILE_STAIRS_DOWN);

        ASSERT("stairs up and down are not on same tile",
            !(m.stairs_up_x == m.stairs_down_x &&
              m.stairs_up_y == m.stairs_down_y));

        ASSERT("stairs up within bounds",
            m.stairs_up_x >= 0 && m.stairs_up_x < MAP_W &&
            m.stairs_up_y >= 0 && m.stairs_up_y < MAP_H);
        ASSERT("stairs down within bounds",
            m.stairs_down_x >= 0 && m.stairs_down_x < MAP_W &&
            m.stairs_down_y >= 0 && m.stairs_down_y < MAP_H);
    }
}

void test_stairs_locked(void) {
    printf("Stairs lock tests:\n");

    GameState g;
    game_init(&g);
    game_descend(&g);

    ASSERT("level not cleared on start", g.level_cleared == 0);

    g.player.x = g.map.stairs_down_x;
    g.player.y = g.map.stairs_down_y;
    int level_before = g.level;

    Action a = {ACTION_DESCEND, 0, 0};
    action_resolve_player(&g, a);
    ASSERT("cannot descend when level not cleared", g.level == level_before);

    for (int i = 0; i < g.enemy_count; i++)
        g.enemies[i].active = 0;
    g.level_cleared = 1;

    action_resolve_player(&g, a);
    ASSERT("can descend when level cleared", g.level == level_before + 1);
}

void test_level_cache_cleared(void) {
    printf("Level cache cleared tests:\n");

    GameState g;
    game_init(&g);

    // Set up dungeon
    g.location = LOCATION_DUNGEON;
    g.level    = 1;
    map_generate(&g.map, g.level);
    enemies_spawn(&g);
    g.player.x = g.map.stairs_up_x;
    g.player.y = g.map.stairs_up_y;

    // Kill all enemies to clear level
    for (int i = 0; i < g.enemy_count; i++)
        g.enemies[i].active = 0;
    g.level_cleared = 1;

    // Descend to level 2 — level 1 should be cached as cleared
    g.player.x = g.map.stairs_down_x;
    g.player.y = g.map.stairs_down_y;
    game_descend(&g);
    ASSERT("level is now 2",                    g.level == 2);
    ASSERT("level 1 cached as cleared",         g.level_cache[0].level_cleared == 1);
    ASSERT("level 2 not cleared",               g.level_cleared == 0);

    // Ascend back to level 1 — should restore cleared state
    g.player.x = g.map.stairs_up_x;
    g.player.y = g.map.stairs_up_y;
    game_ascend(&g);
    ASSERT("back on level 1",                   g.level == 1);
    ASSERT("level 1 restored as cleared",       g.level_cleared == 1);
}