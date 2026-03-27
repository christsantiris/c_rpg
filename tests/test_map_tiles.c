#include "test_utils.h"
#include "../src/game/map.h"

void test_map_tiles(void) {
    printf("Map tile tests:\n");

    Map m;
    map_init(&m);

    // Border tiles are walls
    ASSERT("top-left corner is wall",    m.tiles[0][0]           == TILE_WALL);
    ASSERT("top-right corner is wall",   m.tiles[0][MAP_W-1]     == TILE_WALL);
    ASSERT("bottom-left corner is wall", m.tiles[MAP_H-1][0]     == TILE_WALL);
    ASSERT("bottom-right corner is wall",m.tiles[MAP_H-1][MAP_W-1] == TILE_WALL);

    // Interior tiles are floor
    ASSERT("center is floor", m.tiles[MAP_H/2][MAP_W/2] == TILE_FLOOR);
    ASSERT("near top-left is floor",     m.tiles[1][1]           == TILE_FLOOR);
    ASSERT("near top-right is floor",    m.tiles[1][MAP_W-2]     == TILE_FLOOR);
    ASSERT("near bottom-left is floor",  m.tiles[MAP_H-2][1]     == TILE_FLOOR);

    // Walkability
    ASSERT("border tile is not walkable", map_is_walkable(&m, 0, 0)           == 0);
    ASSERT("interior tile is walkable",   map_is_walkable(&m, MAP_W/2, MAP_H/2) == 1);
    ASSERT("out of bounds is not walkable", map_is_walkable(&m, -1, -1)       == 0);
    ASSERT("out of bounds right is not walkable", map_is_walkable(&m, MAP_W, MAP_H) == 0);
}