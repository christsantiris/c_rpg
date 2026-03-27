#include "map.h"

void map_init(Map *m) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (x == 0 || x == MAP_W - 1 || y == 0 || y == MAP_H - 1)
                m->tiles[y][x] = TILE_WALL;
            else
                m->tiles[y][x] = TILE_FLOOR;
        }
    }
}

int map_is_walkable(const Map *m, int x, int y) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return 0;
    return m->tiles[y][x] == TILE_FLOOR;
}