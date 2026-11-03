#include "enemy_tactics.h"
#include "aldric.h"
#include "eastern_arc.h"
#include <stdlib.h>

static const int dx[] = {0, 1, 0, -1};
static const int dy[] = {-1, 0, 1, 0};

int enemy_goblin_frontline(const Enemy *e) {
    return e->type == ENEMY_GOBLIN_SCOUT || e->type == ENEMY_GOBLIN_WARCHIEF || e->type == ENEMY_HOBGOBLIN_GUARD;
}

static int frontline(const Enemy *e) {
    return enemy_goblin_frontline(e) || e->type == ENEMY_ROYAL_GUARD;
}

static int royal(const Enemy *e) {
    return e->type == ENEMY_ROYAL_GUARD || e->type == ENEMY_ROAD_ARCHER;
}

static int goblin(const Enemy *e) {
    return enemy_goblin_frontline(e) || e->type == ENEMY_GOBLIN_ARCHER || e->type == ENEMY_GOBLIN_SHAMAN;
}

static int dark_elf(const Enemy *e) {
    return e->type == ENEMY_DARK_ELF_GUARD || e->type == ENEMY_DARK_ELF_ARCHER || e->type == ENEMY_DARK_ELF_RANGER;
}

static int occupied(const GameState *g, int index, int x, int y) {
    if ((x == g->player.x && y == g->player.y) || aldric_at(g, x, y)) {
        return 1;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (i != index && g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y) {
            return 1;
        }
    }
    return 0;
}

static int open_slot(const GameState *g, int x, int y) {
    return map_is_walkable(&g->map, x, y) && !map_is_travel_tile(g->map.tiles[y][x]);
}

static int firing_lane(const GameState *g, int index, int x, int y) {
    int distance = abs(x - g->player.x) + abs(y - g->player.y);
    if ((x != g->player.x && y != g->player.y) || distance < 2 || distance > 6) {
        return 0;
    }
    int sx = (g->player.x > x) - (g->player.x < x);
    int sy = (g->player.y > y) - (g->player.y < y);
    for (int step = 1; step < distance; step++) {
        int tx = x + sx * step;
        int ty = y + sy * step;
        if (!map_is_walkable(&g->map, tx, ty) || occupied(g, index, tx, ty)) {
            return 0;
        }
    }
    return 1;
}

// Route to a real position around terrain and allies; never teleport or clear a path.
static int step_to(GameState *g, int index, int x, int y) {
    Enemy *e = &g->enemies[index];
    if (!open_slot(g, x, y) || occupied(g, index, x, y)) {
        return -1;
    }
    if (e->x == x && e->y == y) {
        return 0;
    }
    static int distance[MAP_H][MAP_W];
    static int queue[MAP_H * MAP_W];
    for (int row = 0; row < MAP_H; row++) {
        for (int col = 0; col < MAP_W; col++) {
            distance[row][col] = -1;
        }
    }
    int head = 0;
    int tail = 0;
    int keep_range = !frontline(e) && e->type != ENEMY_DARK_ELF_GUARD;
    distance[y][x] = 0;
    queue[tail++] = y * MAP_W + x;
    while (head < tail && distance[e->y][e->x] < 0) {
        int cell = queue[head++];
        for (int side = 0; side < 4; side++) {
            int nx = cell % MAP_W + dx[side];
            int ny = cell / MAP_W + dy[side];
            if (!map_is_walkable(&g->map, nx, ny) || distance[ny][nx] >= 0 || occupied(g, index, nx, ny)) {
                continue;
            }
            if (keep_range && abs(nx - g->player.x) + abs(ny - g->player.y) < 3 && !(nx == e->x && ny == e->y)) {
                continue;
            }
            distance[ny][nx] = distance[cell / MAP_W][cell % MAP_W] + 1;
            queue[tail++] = ny * MAP_W + nx;
        }
    }
    int path = distance[e->y][e->x];
    // A flank can take a short detour, but must not send a squad across the map.
    int limit = abs(e->x - g->player.x) + abs(e->y - g->player.y) + 6;
    if (path < 1 || path > limit) {
        return -1;
    }
    int best_x = e->x;
    int best_y = e->y;
    int best_range = MAP_W + MAP_H;
    for (int side = 0; side < 4; side++) {
        int nx = e->x + dx[side];
        int ny = e->y + dy[side];
        if (map_is_walkable(&g->map, nx, ny) && !occupied(g, index, nx, ny) && distance[ny][nx] == path - 1) {
            int range = abs(nx - g->player.x) + abs(ny - g->player.y);
            if (range < best_range) {
                best_x = nx;
                best_y = ny;
                best_range = range;
            }
        }
    }
    if (best_range == MAP_W + MAP_H) {
        return -1;
    }
    e->x = best_x;
    e->y = best_y;
    return 1;
}

int enemy_tactics_move(GameState *g, int index) {
    Enemy *e = &g->enemies[index];
    int royal_formation = eastern_town_combat(g) && royal(e);
    int formation = (g->location == LOCATION_MOUNTAINS && goblin(e)) || royal_formation;
    int flank = g->location == LOCATION_FOREST && dark_elf(e);
    if (!formation && !flank) {
        return -1;
    }
    int anchor = -1;
    int members = 0;
    int rank = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *ally = &g->enemies[i];
        if (!ally->active || !(royal_formation ? royal(ally) : formation ? goblin(ally) : dark_elf(ally)) ||
            (royal_formation && ally->boss_aux_y != e->boss_aux_y) ||
            abs(ally->x - e->x) + abs(ally->y - e->y) > 6) {
            continue;
        }
        members++;
        if (i < index && (flank || frontline(ally) == frontline(e))) {
            rank++;
        }
        // A rear archer stabilizes a goblin line; a guard anchors the elf feint.
        if (anchor < 0 || (formation && (ally->type == ENEMY_GOBLIN_ARCHER || ally->type == ENEMY_ROAD_ARCHER) && g->enemies[anchor].type != ally->type) ||
            (flank && ally->type == ENEMY_DARK_ELF_GUARD && g->enemies[anchor].type != ENEMY_DARK_ELF_GUARD)) {
            anchor = i;
        }
    }
    if (members < 2) {
        return -1;
    }
    int ax = g->enemies[anchor].x - g->player.x;
    int ay = g->enemies[anchor].y - g->player.y;
    int facing = abs(ax) >= abs(ay) ? (ax >= 0 ? 1 : 3) : (ay >= 0 ? 2 : 0);
    if (formation) {
        int fx = dx[facing];
        int fy = dy[facing];
        int sx = -fy;
        int sy = fx;
        // Leave the center lane open for arrows, with melee on its shoulders.
        for (int side = -1; side <= 1; side++) {
            if (!open_slot(g, g->player.x + fx + sx * side, g->player.y + fy + sy * side)) {
                return -1;
            }
        }
        int front = frontline(e);
        int depth = front ? 1 : (e->type == ENEMY_GOBLIN_ARCHER || e->type == ENEMY_ROAD_ARCHER) ? 3 + rank / 2 : 4;
        if (depth > 6) {
            depth = 6;
        }
        int shoulder = front ? (rank % 2 ? -1 : 1) : e->type == ENEMY_GOBLIN_SHAMAN ? 1 : 0;
        int gx = g->player.x + fx * depth + sx * shoulder;
        int gy = g->player.y + fy * depth + sy * shoulder;
        return step_to(g, index, gx, gy);
    }
    static const int sectors[] = {0, 1, 3, 2};
    int side = (facing + sectors[rank % 4]) % 4;
    int melee = e->type == ENEMY_DARK_ELF_GUARD;
    for (int radius = melee ? 1 : 3; radius <= (melee ? 1 : 5); radius++) {
        int gx = g->player.x + dx[side] * radius;
        int gy = g->player.y + dy[side] * radius;
        if (!open_slot(g, gx, gy) || (!melee && !firing_lane(g, index, gx, gy))) {
            continue;
        }
        int moved = step_to(g, index, gx, gy);
        if (moved >= 0) {
            return moved;
        }
    }
    return -1;
}
