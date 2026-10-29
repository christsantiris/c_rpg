#ifndef TOWN_ROAD_CHECKS_H
#define TOWN_ROAD_CHECKS_H

#include "test_utils.h"
#include "game/town_road.h"
#include "game/actions.h"

// Walk the exterior's roads and grass, avoiding every other town gate and regional exit.
static inline void walk_town_road_to(GameState *g, int x, int y) {
    int previous[TOWN_ROAD_W * TOWN_ROAD_H];
    int queue[TOWN_ROAD_W * TOWN_ROAD_H];
    for (int i = 0; i < TOWN_ROAD_W * TOWN_ROAD_H; i++) {
        previous[i] = -1;
    }
    int start = g->player.y * TOWN_ROAD_W + g->player.x;
    int target = y * TOWN_ROAD_W + x;
    int head = 0;
    int tail = 0;
    queue[tail++] = start;
    previous[start] = start;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    while (head < tail && previous[target] < 0) {
        int cell = queue[head++];
        for (int side = 0; side < 4; side++) {
            int nx = cell % TOWN_ROAD_W + dx[side];
            int ny = cell / TOWN_ROAD_W + dy[side];
            if (nx < 0 || nx >= TOWN_ROAD_W || ny < 0 || ny >= TOWN_ROAD_H) {
                continue;
            }
            int next = ny * TOWN_ROAD_W + nx;
            TileType tile = g->map.tiles[ny][nx];
            if (previous[next] < 0 && (next == target || tile == TILE_TOWN_PATH || tile == TILE_TOWN_FLOOR)) {
                previous[next] = cell;
                queue[tail++] = next;
            }
        }
    }
    ASSERT("the exterior road reaches the chosen gate without entering another destination", previous[target] >= 0);
    if (previous[target] < 0) {
        return;
    }
    int count = 0;
    for (int cell = target; cell != start; cell = previous[cell]) {
        queue[count++] = cell;
    }
    for (int i = count - 1; i >= 0 && g->location == LOCATION_TOWN_ROAD; i--) {
        action_resolve_player(g, (Action){ACTION_MOVE, queue[i] % TOWN_ROAD_W, queue[i] / TOWN_ROAD_W});
    }
}

static inline void finish_town_departure(GameState *g) {
    if (g->location == LOCATION_TOWN_ROAD) {
        TownSide side = g->player.last_dy < 0 ? TOWN_SIDE_NORTH : g->player.last_dx > 0 ? TOWN_SIDE_EAST : g->player.last_dy > 0 ? TOWN_SIDE_SOUTH : TOWN_SIDE_WEST;
        int x;
        int y;
        town_road_exit(side, &x, &y);
        walk_town_road_to(g, x, y);
    }
}

static inline void walk_into_arrived_town(GameState *g) {
    if (g->location == LOCATION_TOWN_ROAD) {
        TownSide side = g->player.y < TOWN_SPRITE_Y ? TOWN_SIDE_NORTH : g->player.y >= TOWN_SPRITE_Y + TOWN_SPRITE_H ? TOWN_SIDE_SOUTH : g->player.x < TOWN_SPRITE_X ? TOWN_SIDE_WEST : TOWN_SIDE_EAST;
        int x;
        int y;
        town_road_entrance(side, &x, &y);
        walk_town_road_to(g, x, y);
    }
}

#endif
