#ifndef CASTLE_ROUTE_CHECKS_H
#define CASTLE_ROUTE_CHECKS_H

#include "game/game.h"

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
