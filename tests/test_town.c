#include "test_utils.h"
#include "../src/game/map.h"
#include "../src/game/game.h"

void test_town_tiles(void) {
    printf("Town tile tests:\n");

    // Tile types are distinct
    ASSERT("TILE_TOWN_FLOOR is distinct",    TILE_TOWN_FLOOR    != TILE_WALL);
    ASSERT("TILE_TOWN_PATH is distinct",     TILE_TOWN_PATH     != TILE_TOWN_FLOOR);
    ASSERT("TILE_TOWN_EXIT is distinct",     TILE_TOWN_EXIT     != TILE_TOWN_PATH);
    ASSERT("TILE_SHOP_BLACKSMITH distinct",  TILE_SHOP_BLACKSMITH != TILE_TOWN_EXIT);
    ASSERT("TILE_SHOP_ALCHEMIST distinct",   TILE_SHOP_ALCHEMIST  != TILE_SHOP_BLACKSMITH);

    // Constants are defined
    ASSERT("TOWN_W is 40", TOWN_W == 40);
    ASSERT("TOWN_H is 25", TOWN_H == 25);
}

void test_town_map(void) {
    printf("Town map tests:\n");

    Map m;
    int spawn_x, spawn_y;
    map_generate_town(&m, &spawn_x, &spawn_y);

    // Spawn is walkable
    ASSERT("spawn tile is walkable",
        map_is_walkable(&m, spawn_x, spawn_y));

    // Exit tiles at north edge
    ASSERT("exit tile at north center",
        m.tiles[0][20] == TILE_TOWN_EXIT);

    // Shop tiles in correct positions
    ASSERT("blacksmith at (7,7)",
        m.tiles[7][7] == TILE_SHOP_BLACKSMITH);
    ASSERT("alchemist at (28,7)",
        m.tiles[7][28] == TILE_SHOP_ALCHEMIST);

    // Path tiles exist
    ASSERT("vertical path at center",
        m.tiles[10][20] == TILE_TOWN_PATH);
    ASSERT("horizontal path at mid row",
        m.tiles[12][10] == TILE_TOWN_PATH);

    // Border is wall
    ASSERT("south border is wall",
        m.tiles[TOWN_H-1][20] == TILE_WALL);
}

void test_town_spawn(void) {
    printf("Town spawn tests:\n");

    GameState g;
    game_init(&g);

    ASSERT("new game starts in town",     g.location == LOCATION_TOWN);
    ASSERT("player spawn is walkable",
        map_is_walkable(&g.map, g.player.x, g.player.y));
    ASSERT("player not on exit tile",
        g.map.tiles[g.player.y][g.player.x] != TILE_TOWN_EXIT);
    ASSERT("player not on shop tile",
        g.map.tiles[g.player.y][g.player.x] != TILE_SHOP_BLACKSMITH &&
        g.map.tiles[g.player.y][g.player.x] != TILE_SHOP_ALCHEMIST);
}

void test_return_to_town(void) {
    printf("Return to town tests:\n");

    GameState g;
    game_init(&g);

    // Set up dungeon state
    g.location = LOCATION_DUNGEON;
    g.level = 3;
    map_generate(&g.map, g.level);
    enemies_spawn(&g);
    g.level_cleared = 1;
    g.player.x = g.map.stairs_up_x;
    g.player.y = g.map.stairs_up_y;

    int enemies_before = g.enemy_count;
    ASSERT("enemies exist before return", enemies_before > 0);

    game_return_to_town(&g);

    ASSERT("location is town after return",
        g.location == LOCATION_TOWN);
    ASSERT("enemy count is zero after return",
        g.enemy_count == 0);
    ASSERT("player spawn is walkable",
        map_is_walkable(&g.map, g.player.x, g.player.y));
    ASSERT("player not on wall tile",
        g.map.tiles[g.player.y][g.player.x] != TILE_WALL);
    ASSERT("level 3 cached after return",
        g.level_cache[2].valid == 1);
    ASSERT("level 3 cleared state cached",
        g.level_cache[2].level_cleared == 1);
    ASSERT("floor items cleared",
        g.floor_item_count == 0);
}