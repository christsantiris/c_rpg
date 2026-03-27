#include "test_utils.h"
#include "../src/game/map.h"
#include <stdlib.h>
#include <time.h>

void test_map_tiles(void) {
    printf("Map tile tests:\n");

    srand((unsigned)time(NULL));

    Map m;
    map_generate(&m, 1);

    // Border tiles are walls
    ASSERT("top-left corner is wall",     m.tiles[0][0]             == TILE_WALL);
    ASSERT("top-right corner is wall",    m.tiles[0][MAP_W-1]       == TILE_WALL);
    ASSERT("bottom-left corner is wall",  m.tiles[MAP_H-1][0]       == TILE_WALL);
    ASSERT("bottom-right corner is wall", m.tiles[MAP_H-1][MAP_W-1] == TILE_WALL);

    // Walkability
    ASSERT("border tile is not walkable",
        map_is_walkable(&m, 0, 0) == 0);
    ASSERT("out of bounds is not walkable",
        map_is_walkable(&m, -1, -1) == 0);
    ASSERT("out of bounds right is not walkable",
        map_is_walkable(&m, MAP_W, MAP_H) == 0);

    // Stairs are walkable
    ASSERT("stairs up is walkable",
        map_is_walkable(&m, m.stairs_up_x, m.stairs_up_y) == 1);
    ASSERT("stairs down is walkable",
        map_is_walkable(&m, m.stairs_down_x, m.stairs_down_y) == 1);
}