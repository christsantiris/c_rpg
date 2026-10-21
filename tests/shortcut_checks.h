#ifndef SHORTCUT_CHECKS_H
#define SHORTCUT_CHECKS_H

#include "game/map.h"

static inline int shortcut_exit_is_recessed(const Map *map, int x, int y, TileType wall) {
    static const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int direction = 0; direction < 4; direction++) {
        int dx = offsets[direction][0];
        int dy = offsets[direction][1];
        int clear = 1;
        int bx = x + dx;
        int by = y + dy;
        if (bx < 0 || bx >= MAP_W || by < 0 || by >= MAP_H || map->tiles[by][bx] != wall) {
            continue;
        }
        for (int step = 0; step < 3; step++) {
            int px = x - dx * step;
            int py = y - dy * step;
            if (px < 1 || px >= MAP_W - 1 || py < 1 || py >= MAP_H - 1 || !map_is_walkable(map, px, py) ||
                map->tiles[py - dx][px + dy] != wall || map->tiles[py + dx][px - dy] != wall) {
                clear = 0;
                break;
            }
        }
        int px = x - dx * 3;
        int py = y - dy * 3;
        if (clear && px >= 0 && px < MAP_W && py >= 0 && py < MAP_H && map_is_walkable(map, px, py)) {
            return 1;
        }
    }
    return 0;
}

#endif
