#include "containers.h"
#include "stormwatch.h"
#include "stormbell.h"
#include "approach.h"
#include "combat_feedback.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const int wind_dx[4] = {1, 0, -1, 0};
static const int wind_dy[4] = {0, 1, 0, -1};

static int inside(int x, int y) {
    return x >= 0 && x < STORM_W && y >= 0 && y < STORM_H;
}

static StormwatchStage *current(GameState *g) {
    if (g->location != LOCATION_STORMWATCH || g->level < 1 || g->level > STORM_DEPTH) {
        return NULL;
    }
    return &g->stormwatch[g->level - 1];
}

const char *stormwatch_stage_name(int level) {
    const char *names[] = {"Windward Approach", "Broken Gatehouse", "Three Watchtowers", "Stormwall Fort", "Matriarch's Court"};
    return level >= 1 && level <= STORM_DEPTH ? names[level - 1] : "Stormwatch Headland";
}

static void land(Map *map, int cx, int cy, int rx, int ry) {
    for (int y = cy - ry; y <= cy + ry; y++) {
        for (int x = cx - rx; x <= cx + rx; x++) {
            if (x > 1 && x < STORM_W - 2 && y > 1 && y < STORM_H - 2 &&
                (x - cx) * (x - cx) * ry * ry + (y - cy) * (y - cy) * rx * rx <= rx * rx * ry * ry) {
                map->tiles[y][x] = TILE_STORM_GRASS;
            }
        }
    }
}

static void path(Map *map, int x, int y, int tx, int ty, int radius) {
    int dx = abs(tx - x);
    int dy = abs(ty - y);
    int steps = dx > dy ? dx : dy;
    for (int n = 0; n <= steps; n++) {
        land(map, x + (tx - x) * n / (steps ? steps : 1), y + (ty - y) * n / (steps ? steps : 1), radius, radius);
    }
}

static void room(Map *map, int cx, int cy, int rx, int ry) {
    for (int y = cy - ry; y <= cy + ry; y++) {
        for (int x = cx - rx; x <= cx + rx; x++) {
            map->tiles[y][x] = x == cx - rx || x == cx + rx || y == cy - ry || y == cy + ry ? TILE_STORM_WALL : TILE_STORM_STONE;
        }
    }
    map->tiles[cy][cx - rx] = TILE_STORM_STONE;
    map->tiles[cy][cx + rx] = TILE_STORM_STONE;
    map->tiles[cy - ry][cx] = TILE_STORM_STONE;
    map->tiles[cy + ry][cx] = TILE_STORM_STONE;
}

// Each stage has its own coastal silhouette and graph. The optional lookouts
// branch off the loop; fort rooms never seal the walking route around them.
void stormwatch_generate(Map *map, int level) {
    memset(map, 0, sizeof(*map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            map->tiles[y][x] = TILE_STORM_WATER;
        }
    }
    static const int nodes[STORM_DEPTH][8][2] = {
        {{10,37},{24,33},{43,34},{49,20},{32,13},{15,21},{7,10},{53,8}},
        {{10,37},{19,28},{45,32},{51,20},{43,9},{20,10},{8,17},{55,39}},
        {{10,37},{19,29},{45,34},{53,20},{33,8},{15,17},{6,8},{55,8}},
        {{10,37},{12,24},{25,10},{47,10},{53,27},{42,38},{8,8},{56,39}},
        {{10,37},{16,25},{23,11},{44,10},{51,25},{43,37},{6,12},{56,39}}
    };
    const int (*points)[2] = nodes[level - 1];
    for (int n = 0; n < 8; n++) {
        land(map, points[n][0], points[n][1], n < 6 ? 8 : 5, n < 6 ? 6 : 4);
    }
    for (int n = 0; n < 6; n++) {
        int next = (n + 1) % 6;
        path(map, points[n][0], points[n][1], points[next][0], points[next][1], 3);
    }
    path(map, points[5][0], points[5][1], points[6][0], points[6][1], 2);
    path(map, points[3][0], points[3][1], points[7][0], points[7][1], 2);
    // A crosswind shortcut crosses the sheltered outer loop.
    path(map, points[1][0], points[1][1], 20, 24, 2);
    path(map, 20, 24, 45, 24, 2);
    path(map, 45, 24, points[3][0], points[3][1], 2);
    if (level >= 2) {
        for (int n = 1; n < 6; n += level == 3 ? 2 : 1) {
            room(map, points[n][0], points[n][1], 4, 3);
        }
        for (int y = 21; y <= 27; y++) {
            if (map_is_walkable(map, 32, y)) {
                map->tiles[y][32] = TILE_STORM_WALL;
            }
        }
        map->tiles[24][32] = TILE_STORM_SHUTTER_UP;
        map->tiles[23][31] = TILE_STORM_CRANK;
        map->tiles[23][33] = TILE_STORM_CRANK;
    } else {
        room(map, 24, 33, 3, 2);
    }
    for (int n = 6; n < 8; n++) {
        map->tiles[points[n][1]][points[n][0]] = TILE_STORM_SUPPLIES;
    }
    if (level == 3) {
        room(map, 55, 8, 3, 3);
        map->tiles[8][55] = TILE_STORM_SUPPLIES;
    }
    // Both gates sit on the land boundary, framed by actual masonry.
    for (int y = 37; y < STORM_H; y++) {
        map->tiles[y][10] = TILE_STORM_STONE;
    }
    for (int y = 0; y <= points[4][1]; y++) {
        map->tiles[y][points[4][0]] = TILE_STORM_STONE;
    }
    for (int y = 0; y < 2; y++) {
        map->tiles[y][points[4][0] - 1] = TILE_STORM_WALL;
        map->tiles[y][points[4][0] + 1] = TILE_STORM_WALL;
    }
    for (int y = STORM_H - 2; y < STORM_H; y++) {
        map->tiles[y][9] = TILE_STORM_WALL;
        map->tiles[y][11] = TILE_STORM_WALL;
    }
    map->stairs_up_x = 10;
    map->stairs_up_y = STORM_H - 1;
    map->stairs_down_x = points[4][0];
    map->stairs_down_y = 0;
    map->tiles[map->stairs_up_y][map->stairs_up_x] = TILE_STORM_ENTRANCE;
    map->tiles[map->stairs_down_y][map->stairs_down_x] = TILE_STORM_EXIT;
}

void stormwatch_pack(StormwatchStage *state, const Map *map) {
    memset(state->explored, 0, sizeof(state->explored));
    for (int y = 0; y < STORM_H; y++) {
        for (int x = 0; x < STORM_W; x++) {
            int bit = y * STORM_W + x;
            state->tiles[y][x] = map->tiles[y][x];
            if (map_is_explored(map, x, y)) {
                state->explored[bit / 8] |= 1u << (bit % 8);
            }
        }
    }
}

void stormwatch_unpack(const StormwatchStage *state, Map *map) {
    memset(map, 0, sizeof(*map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            map->tiles[y][x] = inside(x, y) ? state->tiles[y][x] : TILE_STORM_WATER;
            if (inside(x, y)) {
                int bit = y * STORM_W + x;
                if (state->explored[bit / 8] & (1u << (bit % 8))) {
                    map_mark_explored(map, x, y);
                }
                if (state->tiles[y][x] == TILE_STORM_EXIT) {
                    map->stairs_down_x = x;
                    map->stairs_down_y = y;
                }
            }
        }
    }
    map->stairs_up_x = 10;
    map->stairs_up_y = STORM_H - 1;
}

void stormwatch_capture(const GameState *g, StormwatchStage *state) {
    if (g->location != LOCATION_STORMWATCH) {
        return;
    }
    stormwatch_pack(state, &g->map);
    state->enemy_count = g->enemy_count;
    memcpy(state->enemies, g->enemies, sizeof(Enemy) * state->enemy_count);
    state->loot_count = g->floor_item_count;
    memcpy(state->loot, g->floor_items, sizeof(FloorItem) * state->loot_count);
    // Items keep their exact floor underlay, rather than being baked into terrain.
    for (int n = 0; n < state->loot_count; n++) {
        const FloorItem *item = &state->loot[n];
        if (item->active && inside(item->x, item->y)) {
            state->tiles[item->y][item->x] = item->underlying_tile;
        }
    }
    state->valid = 1;
}

void stormwatch_store(GameState *g) {
    StormwatchStage *state = current(g);
    if (state) {
        if (g->level == 4) {
            Enemy guards[3];
            stormbell_capture(g, guards);
            memcpy(g->stormbell_guards, guards, sizeof(guards));
        }
        stormwatch_capture(g, state);
    }
}

static int occupied(const GameState *g, const Enemy *self, int x, int y) {
    if (self && x == g->player.x && y == g->player.y) {
        return 1;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (&g->enemies[i] != self && g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y) {
            return 1;
        }
    }
    return 0;
}

static int safe(const GameState *g, const Enemy *self, int x, int y) {
    if (!inside(x, y) || !map_is_walkable(&g->map, x, y) || occupied(g, self, x, y)) {
        return 0;
    }
    TileType tile = g->map.tiles[y][x];
    return tile != TILE_STORM_ENTRANCE && tile != TILE_STORM_EXIT && tile != TILE_PORTAL;
}

static void make_enemy(Enemy *enemy, EnemyType type, int x, int y, int level, int summoned) {
    int kind = type - ENEMY_HARPY_RAIDER;
    const char *names[] = {"Harpy Raider", "Cliff Gargoyle", "Harpy Stormcaller", "Storm Elemental", "Tempest Matriarch"};
    const int hp[] = {36, 64, 46, 72, 320};
    const int attack[] = {12, 16, 15, 18, 24};
    const int defense[] = {2, 7, 3, 4, 7};
    *enemy = (Enemy){0};
    enemy->active = 1;
    enemy->type = type;
    enemy->x = x;
    enemy->y = y;
    enemy->hp = enemy->max_hp = hp[kind] + (kind == 4 ? 0 : level * 3);
    enemy->attack = attack[kind] + level;
    enemy->defense = defense[kind];
    enemy->experience = summoned ? 0 : kind == 4 ? 450 : 24 + kind * 14;
    enemy->is_boss = kind == 4;
    enemy->revived = summoned;
    enemy->facing_dy = 1;
    snprintf(enemy->name, sizeof(enemy->name), "%s", names[kind]);
}

Enemy *stormwatch_spawn(GameState *g, EnemyType type, int x, int y, int summoned) {
    if (type < ENEMY_HARPY_RAIDER || type > ENEMY_TEMPEST_MATRIARCH || !safe(g, NULL, x, y) || (g->player.x == x && g->player.y == y)) {
        return NULL;
    }
    int slot = g->enemy_count;
    if (slot >= AREA_ENEMY_LIMIT) {
        for (int n = 0; n < g->enemy_count; n++) {
            if (!g->enemies[n].active) {
                slot = n;
                break;
            }
        }
    }
    if (slot >= AREA_ENEMY_LIMIT) {
        return NULL;
    }
    Enemy *enemy = &g->enemies[slot];
    make_enemy(enemy, type, x, y, g->level, summoned);
    if (slot == g->enemy_count) {
        g->enemy_count++;
    }
    return enemy;
}

static void repair_boss(Map *map, Enemy *enemies, int *count, int px, int py) {
    for (int n = 0; n < *count; n++) {
        if (enemies[n].type == ENEMY_TEMPEST_MATRIARCH) {
            return;
        }
    }
    int slot = *count;
    if (slot >= AREA_ENEMY_LIMIT) {
        for (int n = 0; n < *count; n++) {
            if (!enemies[n].active) {
                slot = n;
                break;
            }
        }
    }
    if (slot >= AREA_ENEMY_LIMIT) {
        return;
    }
    for (int radius = 0; radius < STORM_W + STORM_H; radius++) {
        for (int y = 1; y < STORM_H - 1; y++) {
            for (int x = 1; x < STORM_W - 1; x++) {
                if (abs(x - 43) + abs(y - 10) != radius || !map_is_walkable(map, x, y) ||
                    map->tiles[y][x] == TILE_PORTAL || map->tiles[y][x] == TILE_ITEM || (x == px && y == py)) {
                    continue;
                }
                int taken = 0;
                for (int n = 0; n < *count; n++) {
                    taken |= enemies[n].active && enemies[n].x == x && enemies[n].y == y;
                }
                if (!taken) {
                    make_enemy(&enemies[slot], ENEMY_TEMPEST_MATRIARCH, x, y, 5, 0);
                    if (slot == *count) {
                        (*count)++;
                    }
                    return;
                }
            }
        }
    }
}

void stormwatch_repair_boss(GameState *g) {
    if (g->stormwatch_defeated) {
        return;
    }
    if (g->location == LOCATION_STORMWATCH && g->level == 5) {
        repair_boss(&g->map, g->enemies, &g->enemy_count, g->player.x, g->player.y);
        for (int n = 0; n < g->enemy_count; n++) {
            if (g->enemies[n].active && g->enemies[n].is_boss) {
                g->level_cleared = 0;
            }
        }
        stormwatch_store(g);
    } else if (g->stormwatch[4].valid) {
        Map map;
        stormwatch_unpack(&g->stormwatch[4], &map);
        for (int n = 0; n < g->stormwatch[4].loot_count; n++) {
            const FloorItem *item = &g->stormwatch[4].loot[n];
            if (item->active) {
                map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        if (g->portal_active && g->portal_location == LOCATION_STORMWATCH && g->portal_level == 5 && inside(g->portal_x, g->portal_y)) {
            map.tiles[g->portal_y][g->portal_x] = TILE_PORTAL;
        }
        repair_boss(&map, g->stormwatch[4].enemies, &g->stormwatch[4].enemy_count, -1, -1);
    }
}

static void load_stage(GameState *g) {
    StormwatchStage *state = current(g);
    if (state->valid) {
        stormwatch_unpack(state, &g->map);
        g->enemy_count = state->enemy_count;
        memcpy(g->enemies, state->enemies, sizeof(Enemy) * state->enemy_count);
        g->floor_item_count = state->loot_count;
        memcpy(g->floor_items, state->loot, sizeof(FloorItem) * state->loot_count);
        for (int n = 0; n < g->floor_item_count; n++) {
            FloorItem *item = &g->floor_items[n];
            if (item->active) {
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
    } else {
        stormwatch_generate(&g->map, g->level);
        g->enemy_count = 0;
        g->floor_item_count = 0;
        int limit = g->level == STORM_DEPTH ? 10 : 8 + g->level;
        const int guaranteed[4][2] = {{24,33}, {19,28}, {33,8}, {47,10}};
        if (g->level < STORM_DEPTH) {
            stormwatch_spawn(g, ENEMY_HARPY_RAIDER + g->level - 1, guaranteed[g->level - 1][0], guaranteed[g->level - 1][1], 0);
        }
        for (int y = 5; y < STORM_H - 6 && g->enemy_count < limit; y += 5) {
            int start = 5 + rand() % 8;
            for (int x = start; x < STORM_W - 5 && g->enemy_count < limit; x += 9) {
                if (!(g->level == 5 && x == 43 && y == 10) && rand() % 3 == 0 && (safe(g, NULL, x, y) && (g->player.x != x || g->player.y != y))) {
                    int kind = rand() % (g->level < 4 ? g->level : 4);
                    stormwatch_spawn(g, ENEMY_HARPY_RAIDER + kind, x, y, 0);
                }
            }
        }
        if (g->level == STORM_DEPTH) {
            stormwatch_spawn(g, ENEMY_TEMPEST_MATRIARCH, 43, 10, 0);
            repair_boss(&g->map, g->enemies, &g->enemy_count, g->player.x, g->player.y);
        }
        state->phase[0] = 5;
        state->phase[1] = 1;
        state->direction[0] = state->next_direction[0] = 0;
        state->direction[1] = state->next_direction[1] = 1;
        stormwatch_store(g);
    }
    g->level_cleared = 1;
    for (int n = 0; n < g->enemy_count; n++) {
        if (g->enemies[n].active) {
            g->level_cleared = 0;
        }
    }
    map_mark_explored(&g->map, g->player.x, g->player.y);
}

void stormwatch_enter(GameState *g, int portal) {
    if (!portal) {
        Enemy guards[3];
        stormbell_capture(g, guards);
        memcpy(g->stormbell_guards, guards, sizeof(guards));
        memset(g->stormwatch, 0, sizeof(g->stormwatch));
    }
    g->location = LOCATION_STORMWATCH;
    g->level = portal ? g->portal_level : 1;
    load_stage(g);
    g->player.x = portal ? g->portal_x : g->map.stairs_up_x;
    g->player.y = portal ? g->portal_y : g->map.stairs_up_y - 1;
    stormbell_restore(g);
    g->player.last_dx = 0;
    g->player.last_dy = -1;
    if (portal || (g->portal_active && g->portal_location == LOCATION_STORMWATCH)) {
        g->portal_active = 0;
    }
    map_mark_explored(&g->map, g->player.x, g->player.y);
    push_message(g, "Stormwatch: gusts push even while you stand still. Press A in the final warning second to brace.");
}

void stormwatch_leave(GameState *g) {
    stormwatch_store(g);
    approach_enter_region(g, 18);
    g->player.x = g->map.stairs_down_x;
    g->player.y = g->map.stairs_down_y + 1;
    g->player.last_dx = 0;
    g->player.last_dy = 1;
    map_mark_explored(&g->map, g->player.x, g->player.y);
}

void stormwatch_change_level(GameState *g, int forward) {
    if (g->location != LOCATION_STORMWATCH) {
        return;
    }
    if ((!forward && g->level == 1) || (forward && g->level == STORM_DEPTH)) {
        if (forward && !g->stormwatch_defeated && !g->debug_passages_unlocked) {
            push_message(g, "The Matriarch holds the final departure. You can retreat by the southern gate.");
            return;
        }
        stormwatch_leave(g);
        return;
    }
    stormwatch_store(g);
    g->level += forward ? 1 : -1;
    load_stage(g);
    g->player.x = forward ? g->map.stairs_up_x : g->map.stairs_down_x;
    g->player.y = forward ? g->map.stairs_up_y - 1 : g->map.stairs_down_y + 1;
    stormbell_restore(g);
    map_mark_explored(&g->map, g->player.x, g->player.y);
    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message), "Stormwatch %d: %s", g->level, stormwatch_stage_name(g->level));
    push_message(g, message);
}

int stormwatch_travel(GameState *g, int x, int y) {
    if (g->location != LOCATION_STORMWATCH || !inside(x, y)) {
        return 0;
    }
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_STORM_ENTRANCE || tile == TILE_STORM_EXIT) {
        stormwatch_change_level(g, tile == TILE_STORM_EXIT);
        return 1;
    }
    return 0;
}

static int nearby(const GameState *g, TileType tile, int *tx, int *ty) {
    for (int y = g->player.y - 1; y <= g->player.y + 1; y++) {
        for (int x = g->player.x - 1; x <= g->player.x + 1; x++) {
            if (inside(x, y) && abs(x - g->player.x) + abs(y - g->player.y) <= 1 && g->map.tiles[y][x] == tile) {
                *tx = x;
                *ty = y;
                return 1;
            }
        }
    }
    return 0;
}

int stormwatch_wind_marks(const GameState *g, int lane, int x, int y) {
    if (g->location != LOCATION_STORMWATCH || lane < 0 || lane > 1 || !inside(x, y) || g->level < 1 || g->level > STORM_DEPTH) {
        return 0;
    }
    const StormwatchStage *state = &g->stormwatch[g->level - 1];
    int bit = y * STORM_W + x;
    return state->phase[lane] >= 2 && state->phase[lane] <= 4 && (state->warned[lane][bit / 8] & (1u << (bit % 8)));
}

static int exposed(const GameState *g, int lane, int direction, int x, int y) {
    if ((lane == 0 && (y < 22 || y > 26 || x < 4 || x > 59)) ||
        (lane == 1 && (x < 29 || x > 33 || y < 4 || y > 43)) || !map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    int dx = wind_dx[direction];
    int dy = wind_dy[direction];
    for (int n = 1; n < STORM_W; n++) {
        int nx = x - dx * n;
        int ny = y - dy * n;
        if (!inside(nx, ny)) {
            break;
        }
        TileType tile = g->map.tiles[ny][nx];
        if (tile == TILE_STORM_WALL || tile == TILE_STORM_SHUTTER_UP) {
            return 0;
        }
    }
    return 1;
}

static void mask(GameState *g, int lane, int shrink) {
    StormwatchStage *state = current(g);
    for (int y = 0; y < STORM_H; y++) {
        for (int x = 0; x < STORM_W; x++) {
            int bit = y * STORM_W + x;
            unsigned char flag = 1u << (bit % 8);
            if (!exposed(g, lane, state->direction[lane], x, y)) {
                state->warned[lane][bit / 8] &= ~flag;
            } else if (!shrink) {
                state->warned[lane][bit / 8] |= flag;
            }
        }
    }
}

static TileType threshold(const GameState *g, int *loot) {
    TileType tile = g->map.tiles[24][32];
    *loot = 0;
    for (int n = 0; n < g->floor_item_count; n++) {
        const FloorItem *item = &g->floor_items[n];
        if (item->active && item->x == 32 && item->y == 24) {
            tile = item->underlying_tile;
            *loot = 1;
        }
    }
    return tile;
}

const char *stormwatch_hint(const GameState *g) {
    if (g->location != LOCATION_STORMWATCH) {
        return NULL;
    }
    int x;
    int y;
    if (nearby(g, TILE_STORM_CRANK, &x, &y)) {
        int loot;
        return threshold(g, &loot) == TILE_STORM_SHUTTER_UP ? "A: LOWER STORM SHUTTER" : "A: RAISE STORM SHUTTER";
    }
    if (containers_hint(g)) {
        return containers_hint(g);
    }
    const StormwatchStage *state = &g->stormwatch[g->level - 1];
    for (int lane = 0; lane < (g->level >= 3 ? 2 : 1); lane++) {
        if (state->phase[lane] == 3 && stormwatch_wind_marks(g, lane, g->player.x, g->player.y)) {
            return state->braced ? "BRACED: HOLD STILL" : "A: BRACE AGAINST THE NEXT GUST";
        }
    }
    return stormbell_hint(g);
}

int stormwatch_interact(GameState *g) {
    StormwatchStage *state = current(g);
    if (!state) {
        return 0;
    }
    state->braced = 0;
    int x;
    int y;
    if (nearby(g, TILE_STORM_CRANK, &x, &y)) {
        int loot;
        int raising = threshold(g, &loot) == TILE_STORM_SHUTTER_DOWN;
        if (raising && ((occupied(g, NULL, 32, 24) || (g->player.x == 32 && g->player.y == 24)) || loot ||
            (g->portal_active && g->portal_location == LOCATION_STORMWATCH && g->portal_level == g->level && g->portal_x == 32 && g->portal_y == 24))) {
            push_message(g, "The shutter threshold must be clear before you raise it.");
            return 1;
        }
        g->map.tiles[24][32] = raising ? TILE_STORM_SHUTTER_UP : TILE_STORM_SHUTTER_DOWN;
        if (raising) {
            mask(g, 0, 1);
            mask(g, 1, 1);
        }
        push_message(g, raising ? "The raised shutter blocks the passage and wind. The outer route remains open." : "The shutter lowers into a timber shortcut. New exposure begins with the next warning.");
    } else if (containers_hint(g)) {
        return containers_interact(g);
    } else if (stormbell_hint(g) && !(state->phase[0] == 3 && stormwatch_wind_marks(g, 0, g->player.x, g->player.y)) &&
        !(state->phase[1] == 3 && stormwatch_wind_marks(g, 1, g->player.x, g->player.y))) {
        return stormbell_interact(g);
    } else if (stormwatch_hint(g)) {
        state->braced = 1;
        push_message(g, "You plant your feet against the wind for this enemy phase.");
    } else {
        return 0;
    }
    return 1;
}

static int incoming(const GameState *g, int x, int y) {
    for (int n = 0; n < g->enemy_count; n++) {
        if (stormwatch_attack_marks(g, &g->enemies[n], x, y)) {
            return 1;
        }
    }
    return 0;
}

void stormwatch_wind(GameState *g) {
    StormwatchStage *state = current(g);
    if (!state) {
        return;
    }
    for (int lane = 0; lane < (g->level >= 3 ? 2 : 1); lane++) {
        state->phase[lane] = (state->phase[lane] + 1) % 6;
        if (state->phase[lane] == 2) {
            state->direction[lane] = state->next_direction[lane];
            mask(g, lane, 0);
        }
        if (state->phase[lane] != 4) {
            continue;
        }
        int dx = wind_dx[state->direction[lane]];
        int dy = wind_dy[state->direction[lane]];
        // No collision damage and no forced transition, water landing or overlap.
        if (!state->braced && stormwatch_wind_marks(g, lane, g->player.x, g->player.y) && safe(g, NULL, g->player.x + dx, g->player.y + dy) &&
            !incoming(g, g->player.x + dx, g->player.y + dy)) {
            g->player.x += dx;
            g->player.y += dy;
            map_mark_explored(&g->map, g->player.x, g->player.y);
            combat_feedback_add(g, FEEDBACK_PLAYER_PUSH, FEEDBACK_NOW, g->player.x, g->player.y, state->direction[lane]);
            const char *directions[] = {"east", "south", "west", "north"};
            char message[MAX_MESSAGE_LEN];
            snprintf(message, sizeof(message), "The gust pushes you one tile %s!", directions[state->direction[lane]]);
            push_message(g, message);
        } else if (stormwatch_wind_marks(g, lane, g->player.x, g->player.y)) {
            push_message(g, state->braced ? "You brace and hold your ground against the gust." :
                "The gust cannot move you: its landing is blocked or threatened.");
        }
        for (int n = 0; n < g->enemy_count; n++) {
            Enemy *enemy = &g->enemies[n];
            if (enemy->active && enemy->type == ENEMY_HARPY_RAIDER && stormwatch_wind_marks(g, lane, enemy->x, enemy->y) && safe(g, enemy, enemy->x + dx, enemy->y + dy)) {
                enemy->x += dx;
                enemy->y += dy;
                if (enemy->attack_phase) {
                    enemy->attack_phase = 0;
                    enemy->move_timer = -1;
                }
            }
        }
    }
    state->braced = 0;
}

int stormwatch_advance_wind(GameState *g, Uint32 elapsed) {
    StormwatchStage *state = current(g);
    if (!state || g->player.hp <= 0 || g->game_won) {
        return 0;
    }
    // A delayed frame cannot deliver a backlog of gusts all at once.
    if (elapsed > STORM_WIND_STEP_MS) {
        elapsed = STORM_WIND_STEP_MS;
    }
    state->wind_elapsed_ms += (int)elapsed;
    if (state->wind_elapsed_ms < STORM_WIND_STEP_MS) {
        return 0;
    }
    state->wind_elapsed_ms -= STORM_WIND_STEP_MS;
    stormwatch_wind(g);
    return 1;
}

void stormwatch_defeat(GameState *g) {
    if (!g->stormwatch_defeated) {
        g->stormwatch_defeated = 1;
        g->gold += 220;
        g->score += 2600;
        push_message(g, "The Tempest Matriarch falls! The departure opens. You recover 220 gold.");
    }
}

int stormwatch_valid(const GameState *g) {
    if (g->stormwatch_defeated < 0 || g->stormwatch_defeated > 1) {
        return 0;
    }
    for (int n = 0; n < STORM_DEPTH; n++) {
        const StormwatchStage *state = &g->stormwatch[n];
        if (state->enemy_count < 0 || state->enemy_count > AREA_ENEMY_LIMIT || state->loot_count < 0 || state->loot_count > MAX_FLOOR_ITEMS ||
            state->valid < 0 || state->valid > 1 || state->braced < 0 || state->braced > 1 ||
            state->wind_elapsed_ms < 0 || state->wind_elapsed_ms >= STORM_WIND_STEP_MS) {
            return 0;
        }
        if (state->valid) {
            for (int y = 0; y < STORM_H; y++) {
                for (int x = 0; x < STORM_W; x++) {
                    TileType tile = state->tiles[y][x];
                    if (tile < TILE_STORM_GRASS || tile > TILE_STORM_EXIT) {
                        return 0;
                    }
                }
            }
            for (int i = 0; i < state->enemy_count; i++) {
                const Enemy *enemy = &state->enemies[i];
                if (!inside(enemy->x, enemy->y) || enemy->type < ENEMY_HARPY_RAIDER || enemy->type > ENEMY_TEMPEST_MATRIARCH) {
                    return 0;
                }
            }
            for (int i = 0; i < state->loot_count; i++) {
                const FloorItem *item = &state->loot[i];
                if (!inside(item->x, item->y) || item->underlying_tile < TILE_STORM_GRASS || item->underlying_tile > TILE_STORM_EXIT) {
                    return 0;
                }
            }
        }
        for (int lane = 0; lane < 2; lane++) {
            if (state->phase[lane] < 0 || state->phase[lane] > 5 || state->direction[lane] < 0 || state->direction[lane] > 3 ||
                state->next_direction[lane] < 0 || state->next_direction[lane] > 3) {
                return 0;
            }
        }
    }
    return g->location != LOCATION_STORMWATCH || (g->level >= 1 && g->level <= STORM_DEPTH && inside(g->player.x, g->player.y) &&
        g->enemy_count >= 0 && g->enemy_count <= AREA_ENEMY_LIMIT && g->floor_item_count >= 0 && g->floor_item_count <= MAX_FLOOR_ITEMS &&
        g->stormwatch[g->level - 1].valid);
}
