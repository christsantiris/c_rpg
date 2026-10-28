#include "coast_exits.h"
#include <limits.h>
#include <string.h>

#define CELLS (MAP_W * MAP_H)

static const int steps[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

int coast_exit_point(const Map *map, int forward, int *x, int *y) {
    *x = forward ? map->stairs_down_x : map->stairs_up_x;
    *y = forward ? map->stairs_down_y : map->stairs_up_y;
    if (*x < 1 || *y < 1 || *x >= MAP_W - 1 || *y >= MAP_H - 1) {
        return 0;
    }
    if (*x == 1) {
        *x = 0;
    } else if (*x == MAP_W - 2) {
        *x = MAP_W - 1;
    } else if (*y == 1) {
        *y = 0;
    } else if (*y == MAP_H - 2) {
        *y = MAP_H - 1;
    } else {
        return 0;
    }
    return 1;
}

static int passable(const Map *map, int x, int y, int tide) {
    if (tide && map_is_coast_tidal_tile(map->tiles[y][x])) {
        TileType tile = map_coast_swapped_tile(map->tiles[y][x]);
        return tile == TILE_COAST_DRAINED_WATER || tile == TILE_COAST_CHANNEL_DRY;
    }
    return map_is_walkable(map, x, y);
}

static void reachable(const Map *map, int tide, unsigned char seen[CELLS]) {
    int queue[CELLS];
    int head = 0;
    int tail = 1;
    int start = map->stairs_up_y * MAP_W + map->stairs_up_x;
    memset(seen, 0, CELLS);
    seen[start] = 1;
    queue[0] = start;
    while (head < tail) {
        int cell = queue[head++];
        for (int side = 0; side < 4; side++) {
            int x = cell % MAP_W + steps[side][0];
            int y = cell / MAP_W + steps[side][1];
            if (x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && !seen[y * MAP_W + x] && passable(map, x, y, tide)) {
                seen[y * MAP_W + x] = 1;
                queue[tail++] = y * MAP_W + x;
            }
        }
    }
}

int coast_exits_valid(const Map *map) {
    int ix, iy, ex, ey;
    if (!coast_exit_point(map, 0, &ix, &iy) || !coast_exit_point(map, 1, &ex, &ey) ||
        map->tiles[iy][ix] != TILE_COAST_ENTRANCE || map->tiles[ey][ex] != TILE_COAST_EXIT) {
        return 0;
    }
    int entries = 0;
    int exits = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            entries += map->tiles[y][x] == TILE_COAST_ENTRANCE;
            exits += map->tiles[y][x] == TILE_COAST_EXIT;
        }
    }
    if (entries != 1 || exits != 1) {
        return 0;
    }
    unsigned char seen[CELLS];
    for (int tide = 0; tide < 2; tide++) {
        if (!passable(map, map->stairs_up_x, map->stairs_up_y, tide) ||
            !passable(map, map->stairs_down_x, map->stairs_down_y, tide)) {
            return 0;
        }
        reachable(map, tide, seen);
        if (!seen[iy * MAP_W + ix] || !seen[ey * MAP_W + ex]) {
            return 0;
        }
    }
    return 1;
}

static int repairable(TileType tile) {
    return tile == TILE_COAST_WALL || tile == TILE_COAST_SEA || tile == TILE_WALL ||
        map_is_coast_tidal_tile(tile);
}

static int connect(Map *map, int tx, int ty, int tide) {
    int distance[CELLS];
    int previous[CELLS];
    int queue[CELLS + 1];
    unsigned char queued[CELLS] = {0};
    for (int cell = 0; cell < CELLS; cell++) {
        distance[cell] = INT_MAX;
        previous[cell] = -1;
    }
    int start = map->stairs_up_y * MAP_W + map->stairs_up_x;
    int target = ty * MAP_W + tx;
    int head = 0;
    int tail = 1;
    queue[0] = start;
    distance[start] = 0;
    queued[start] = 1;
    // Prefer existing dry paths. Repair the fewest impassable terrain cells;
    // quest objects, chests and encounter state are never replaced.
    while (head != tail) {
        int cell = queue[head];
        head = (head + 1) % (CELLS + 1);
        queued[cell] = 0;
        for (int side = 0; side < 4; side++) {
            int x = cell % MAP_W + steps[side][0];
            int y = cell / MAP_W + steps[side][1];
            if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
                continue;
            }
            int next = y * MAP_W + x;
            int cost = passable(map, x, y, tide) ? 0 : 1;
            if ((cost && !repairable(map->tiles[y][x])) || distance[cell] + cost >= distance[next]) {
                continue;
            }
            distance[next] = distance[cell] + cost;
            previous[next] = cell;
            if (!queued[next]) {
                queue[tail] = next;
                tail = (tail + 1) % (CELLS + 1);
                queued[next] = 1;
            }
        }
    }
    if (distance[target] == INT_MAX) {
        return 0;
    }
    int changed = 0;
    for (int cell = target; cell != start; cell = previous[cell]) {
        int x = cell % MAP_W;
        int y = cell / MAP_W;
        if (!passable(map, x, y, tide)) {
            map->tiles[y][x] = TILE_COAST_FLOOR;
            changed = 1;
        }
    }
    return changed;
}

static void restore_anchor(Map *map, int level, int forward) {
    int x, y;
    TileType marker = forward ? TILE_COAST_EXIT : TILE_COAST_ENTRANCE;
    if (coast_exit_point(map, forward, &x, &y) && map->tiles[y][x] == marker) {
        return;
    }
    // Recover coordinates from a surviving boundary marker before adding a route.
    for (int row = 0; row < MAP_H; row++) {
        for (int col = 0; col < MAP_W; col++) {
            if (map->tiles[row][col] != marker ||
                (col != 0 && col != MAP_W - 1 && row != 0 && row != MAP_H - 1)) {
                continue;
            }
            x = col == 0 ? 1 : col == MAP_W - 1 ? MAP_W - 2 : col;
            y = row == 0 ? 1 : row == MAP_H - 1 ? MAP_H - 2 : row;
            if (forward) {
                map->stairs_down_x = x;
                map->stairs_down_y = y;
            } else {
                map->stairs_up_x = x;
                map->stairs_up_y = y;
            }
            return;
        }
    }
    if (coast_exit_point(map, forward, &x, &y)) {
        return;
    }
    int index = level > 0 && level <= COAST_DEPTH ? level - 1 : 0;
    const int entry_sides[COAST_DEPTH] = {0, 3, 2, 1, 2};
    const int exit_sides[COAST_DEPTH] = {1, 2, 3, 2, 1};
    int side = forward ? exit_sides[index] : entry_sides[index];
    int room = forward ? map->room_count - 1 : 0;
    if (room >= 0 && room < map->room_count) {
        map_room_center(&map->rooms[room], &x, &y);
    } else {
        x = MAP_W / 2;
        y = MAP_H / 2;
    }
    x = side == 3 ? 1 : side == 1 ? MAP_W - 2 : x < 2 ? 2 : x > MAP_W - 3 ? MAP_W - 3 : x;
    y = side == 0 ? 1 : side == 2 ? MAP_H - 2 : y < 2 ? 2 : y > MAP_H - 3 ? MAP_H - 3 : y;
    if (forward) {
        map->stairs_down_x = x;
        map->stairs_down_y = y;
    } else {
        map->stairs_up_x = x;
        map->stairs_up_y = y;
    }
}

int coast_exits_repair_map(Map *map, int level) {
    if (coast_exits_valid(map)) {
        return 0;
    }
    restore_anchor(map, level, 0);
    restore_anchor(map, level, 1);
    int ix, iy, ex, ey;
    coast_exit_point(map, 0, &ix, &iy);
    coast_exit_point(map, 1, &ex, &ey);
    if (ix == ex && iy == ey) {
        if (map->tiles[iy][ix] == TILE_COAST_EXIT) {
            map->tiles[iy][ix] = TILE_COAST_FLOOR;
        }
        map->stairs_down_x = -1;
        map->stairs_down_y = -1;
        restore_anchor(map, level, 1);
        coast_exit_point(map, 1, &ex, &ey);
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if ((map->tiles[y][x] == TILE_COAST_ENTRANCE && (x != ix || y != iy)) ||
                (map->tiles[y][x] == TILE_COAST_EXIT && (x != ex || y != ey))) {
                map->tiles[y][x] = TILE_COAST_FLOOR;
            }
        }
    }
    map->tiles[iy][ix] = TILE_COAST_ENTRANCE;
    map->tiles[ey][ex] = TILE_COAST_EXIT;
    map->tiles[map->stairs_up_y][map->stairs_up_x] = TILE_COAST_FLOOR;
    map->tiles[map->stairs_down_y][map->stairs_down_x] = TILE_COAST_FLOOR;
    unsigned char seen[CELLS];
    for (int tide = 0; tide < 2; tide++) {
        reachable(map, tide, seen);
        if (!seen[ey * MAP_W + ex]) {
            connect(map, ex, ey, tide);
        }
    }
    return 1;
}

void coast_exits_repair(GameState *g) {
    if (g->location != LOCATION_COAST) {
        return;
    }
    // Temporarily expose loot underlays, then retain the repaired terrain beneath
    // every item. Rendering and movement must still recognize covered exits.
    unsigned char covered[CELLS] = {0};
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x >= 0 && item->y >= 0 && item->x < MAP_W && item->y < MAP_H &&
            g->map.tiles[item->y][item->x] == TILE_ITEM) {
            covered[item->y * MAP_W + item->x] = 1;
            g->map.tiles[item->y][item->x] = item->underlying_tile;
        }
    }
    coast_exits_repair_map(&g->map, g->level);
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x >= 0 && item->y >= 0 && item->x < MAP_W && item->y < MAP_H) {
            item->underlying_tile = g->map.tiles[item->y][item->x];
        }
    }
    for (int cell = 0; cell < CELLS; cell++) {
        if (covered[cell]) {
            g->map.tiles[cell / MAP_W][cell % MAP_W] = TILE_ITEM;
        }
    }
}

void coast_exits_migrate(GameState *g) {
    coast_exits_repair(g);
    for (int level = 1; level <= COAST_DEPTH; level++) {
        if (g->coast_cache[level - 1].valid) {
            coast_exits_repair_map(&g->coast_cache[level - 1].map, level);
        }
    }
}
