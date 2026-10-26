#ifndef NORTHROAD_CHECKS_H
#define NORTHROAD_CHECKS_H
#include "test_utils.h"
#include <string.h>
#include "game/crownroad_north.h"

static inline void walk_crownroad_north(GameState *g, int north) {
    if (g->location != LOCATION_CROWNROAD_NORTH) {
        return;
    }
    int previous[CROWN_NORTH_H][CROWN_NORTH_W];
    unsigned char occupied[CROWN_NORTH_H][CROWN_NORTH_W] = {{0}};
    int queue[CROWN_NORTH_W * CROWN_NORTH_H];
    memset(previous, -1, sizeof(previous));
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active) {
            occupied[g->enemies[i].y][g->enemies[i].x] = 1;
        }
    }
    int target = (north ? 0 : CROWN_NORTH_H - 1) * CROWN_NORTH_W + CROWN_NORTH_X;
    int head = 0;
    int tail = 1;
    queue[0] = target;
    previous[target / CROWN_NORTH_W][CROWN_NORTH_X] = target;
    const int steps[4][2] = {{0,1},{1,0},{0,-1},{-1,0}};
    while (head < tail) {
        int cell = queue[head++];
        for (int i = 0; i < 4; i++) {
            int x = cell % CROWN_NORTH_W + steps[i][0];
            int y = cell / CROWN_NORTH_W + steps[i][1];
            if (x >= 0 && y >= 0 && x < CROWN_NORTH_W && y < CROWN_NORTH_H && !occupied[y][x] &&
                previous[y][x] < 0 && map_is_walkable(&g->map, x, y)) {
                previous[y][x] = cell;
                queue[tail++] = y * CROWN_NORTH_W + x;
            }
        }
    }
    int route = previous[g->player.y][g->player.x] >= 0;
    for (int i = 0; route && i < CROWN_NORTH_W * CROWN_NORTH_H && g->location == LOCATION_CROWNROAD_NORTH; i++) {
        int cell = previous[g->player.y][g->player.x];
        action_resolve_player(g, (Action){ACTION_MOVE, cell % CROWN_NORTH_W, cell / CROWN_NORTH_W});
    }
    ASSERT("the northern road can be crossed on foot around living patrols", route && g->location != LOCATION_CROWNROAD_NORTH);
}
#endif
