#ifndef CASTLE_ROUTE_CHECKS_H
#define CASTLE_ROUTE_CHECKS_H

#include "game/game.h"
#include <string.h>

// Archived six-room geometry: migration fixtures must not depend on V2 generators.
static inline void castle_legacy_layout(Map *m, int level) {
    memset(m, 0, sizeof(*m));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = TILE_CASTLE_WALL;
        }
    }
    m->room_count = 6;
    for (int i = 0; i < 6; i++) {
        Room *r = &m->rooms[i];
        *r = (Room){4 + (i % 2) * 30, 4 + (i / 2) * 20, 22, 14};
        for (int y = r->y; y < r->y + r->h; y++) {
            for (int x = r->x; x < r->x + r->w; x++) {
                m->tiles[y][x] = TILE_CASTLE_FLOOR;
            }
        }
    }
    for (int i = 0; i < 6; i++) {
        int cx;
        int cy;
        map_room_center(&m->rooms[i], &cx, &cy);
        for (int vertical = 0; vertical < 2; vertical++) {
            if ((!vertical && i % 2) || (vertical && i >= 4)) {
                continue;
            }
            int length = vertical ? 20 : 30;
            for (int step = 0; step <= length; step++) {
                for (int oy = -1; oy <= 1; oy++) {
                    for (int ox = -1; ox <= 1; ox++) {
                        m->tiles[cy + (vertical ? step : 0) + oy][cx + (vertical ? 0 : step) + ox] = TILE_CASTLE_CARPET;
                    }
                }
            }
        }
    }
    m->stairs_up_x = 15;
    m->stairs_up_y = 11;
    m->stairs_down_x = 45;
    m->stairs_down_y = 55;
    m->tiles[11][15] = TILE_STAIRS_DOWN;
    m->tiles[55][45] = TILE_STAIRS_UP;
    m->tiles[13][24] = TILE_CASTLE_LEVER;
    for (int y = 10; y <= 12; y++) {
        m->tiles[y][29] = TILE_CASTLE_GATE;
    }
    if (level >= 2) {
        m->burial_trap_count = 1;
        m->burial_traps[0] = (BurialTrap){17, 32, 0, 0, 0};
        m->tiles[32][17] = TILE_CASTLE_TRAP_HIDDEN;
    }
}

static inline int castle_walk_to(GameState *g, int tx, int ty) {
    int previous[CASTLE_W * CASTLE_H];
    int queue[CASTLE_W * CASTLE_H];
    for (int i = 0; i < CASTLE_W * CASTLE_H; i++) {
        previous[i] = -1;
    }
    int start = g->player.y * CASTLE_W + g->player.x;
    int target = ty * CASTLE_W + tx;
    int head = 0;
    int tail = 1;
    queue[0] = start;
    previous[start] = start;
    const int steps[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    while (head < tail && previous[target] < 0) {
        int cell = queue[head++];
        for (int side = 0; side < 4; side++) {
            int x = cell % CASTLE_W + steps[side][0];
            int y = cell / CASTLE_W + steps[side][1];
            if (x < 0 || y < 0 || x >= CASTLE_W || y >= CASTLE_H || !map_is_walkable(&g->map, x, y) ||
                g->map.tiles[y][x] == TILE_CASTLE_TRAP_HIDDEN || g->map.tiles[y][x] == TILE_CASTLE_TRAP_OPEN) {
                continue;
            }
            int next = y * CASTLE_W + x;
            if (previous[next] < 0) {
                previous[next] = cell;
                queue[tail++] = next;
            }
        }
    }
    if (previous[target] < 0) {
        return 0;
    }
    int count = 0;
    for (int cell = target; cell != start; cell = previous[cell]) {
        queue[count++] = cell;
    }
    for (int i = count - 1; i >= 0; i--) {
        int x = queue[i] % CASTLE_W;
        int y = queue[i] / CASTLE_W;
        action_resolve_player(g, (Action){ACTION_MOVE, x, y});
        if (g->player.x != x || g->player.y != y) {
            return 0;
        }
    }
    return 1;
}

#endif
