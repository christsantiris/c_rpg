#include "containers.h"
#include "giants_crown.h"
#include "crown_quest.h"
#include "approach.h"
#include <stdlib.h>
#include <string.h>

static int inside(int x, int y) {
    return x >= 0 && x < GIANTS_W && y >= 0 && y < GIANTS_H;
}

static GiantsStage *current(GameState *g) {
    return g->location == LOCATION_GIANTS_CROWN && g->level >= 1 && g->level <= GIANTS_DEPTH ? &g->giants_crown[g->level - 1] : NULL;
}

const char *giants_stage_name(int level) {
    static const char *names[] = {"Terraced Foothills", "The Fallen Village", "Broken Aqueduct", "Winter Halls", "The Jarl's Court"};
    return level >= 1 && level <= GIANTS_DEPTH ? names[level - 1] : "Giant's Peaks";
}

static void clearing(Map *map, int cx, int cy, int rx, int ry, TileType floor, int hall) {
    for (int y = cy - ry; y <= cy + ry; y++) {
        for (int x = cx - rx; x <= cx + rx; x++) {
            if (x > 0 && x < GIANTS_W - 1 && y > 0 && y < GIANTS_H - 1 &&
                (hall || (x - cx) * (x - cx) * ry * ry + (y - cy) * (y - cy) * rx * rx <= rx * rx * ry * ry)) {
                map->tiles[y][x] = floor;
            }
        }
    }
}

static void path(Map *map, int x, int y, int tx, int ty, TileType floor) {
    while (1) {
        clearing(map, x, y, 1, 1, floor, 1);
        if (x == tx && y == ty) {
            break;
        }
        if (abs(tx - x) > abs(ty - y)) {
            x += (tx > x) - (tx < x);
        } else {
            y += (ty > y) - (ty < y);
        }
    }
}

void giants_generate(Map *map, int level) {
    memset(map, 0, sizeof(*map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            map->tiles[y][x] = TILE_GIANTS_WALL;
        }
    }
    int nodes[7][2] = {{12,36},{32,37},{51,35},{12,12},{32,10},{51,12},{32,24}};
    for (int n = 0; n < 7; n++) {
        nodes[n][0] += rand() % 3 - 1;
        nodes[n][1] += rand() % 3 - 1;
        TileType floor = level >= 4 ? TILE_GIANTS_SNOW : TILE_GIANTS_GRASS;
        int hall = (level == 2 && n != 6) || (level >= 4 && n >= 3);
        clearing(map, nodes[n][0], nodes[n][1], 6 + rand() % 2, 4 + rand() % 2, hall ? TILE_GIANTS_PAVING : floor, hall);
    }
    static const int edges[5][10][2] = {
        {{0,1},{1,2},{2,5},{5,4},{4,3},{3,0},{0,6},{6,4},{6,2},{1,6}},
        {{0,3},{3,4},{4,5},{5,2},{2,1},{1,0},{3,6},{6,5},{1,6},{4,6}},
        {{0,3},{3,4},{4,5},{5,2},{2,1},{1,0},{0,6},{6,5},{1,6},{3,6}},
        {{0,1},{1,2},{2,5},{5,4},{4,3},{3,0},{0,6},{6,4},{6,2},{3,6}},
        {{0,1},{1,2},{2,5},{5,4},{4,3},{3,0},{1,6},{6,4},{0,6},{2,6}}
    };
    for (int e = 0; e < 10; e++) {
        int a = edges[level - 1][e][0];
        int b = edges[level - 1][e][1];
        path(map, nodes[a][0], nodes[a][1], nodes[b][0], nodes[b][1], TILE_GIANTS_PAVING);
    }
    path(map, 32, GIANTS_H - 2, nodes[1][0], nodes[1][1], TILE_GIANTS_PAVING);
    path(map, 32, 1, nodes[4][0], nodes[4][1], TILE_GIANTS_PAVING);
    if (level == 3) {
        // A broad ruined aqueduct divides the central court; both ends reconnect.
        for (int x = 22; x <= 42; x++) {
            map->tiles[22][x] = x % 5 == 0 ? TILE_GIANTS_COLUMN : TILE_GIANTS_WALL;
        }
        path(map, 20, 19, 20, 26, TILE_GIANTS_PAVING);
        path(map, 44, 19, 44, 26, TILE_GIANTS_PAVING);
        path(map, 20, 19, nodes[4][0], nodes[4][1], TILE_GIANTS_PAVING);
        path(map, 44, 26, nodes[2][0], nodes[2][1], TILE_GIANTS_PAVING);
    }
    for (int n = 0; n < 7; n++) {
        for (int side = -1; side <= 1; side += 2) {
            int x = nodes[n][0] + side * 3;
            int y = nodes[n][1] - 2;
            if (map_is_walkable(map, x, y)) {
                map->tiles[y][x] = TILE_GIANTS_COLUMN;
            }
        }
    }
    // The optional vault is deliberately sealed; no enemies spawn inside it.
    path(map, 9, 23, nodes[0][0], nodes[0][1], TILE_GIANTS_PAVING);
    for (int y = 20; y <= 26; y++) {
        for (int x = 2; x <= 8; x++) {
            map->tiles[y][x] = x == 2 || x == 8 || y == 20 || y == 26 ? TILE_GIANTS_WALL : TILE_GIANTS_PAVING;
        }
    }
    map->tiles[23][8] = TILE_GIANTS_CRACK;
    map->tiles[23][4] = TILE_GIANTS_CACHE;
    map->tiles[nodes[5][1] + 2][nodes[5][0]] = TILE_GIANTS_CACHE;
    map->stairs_up_x = 32;
    map->stairs_up_y = GIANTS_H - 1;
    map->stairs_down_x = 32;
    map->stairs_down_y = 0;
    map->tiles[GIANTS_H - 1][32] = TILE_GIANTS_ENTRANCE;
    map->tiles[0][32] = TILE_GIANTS_EXIT;
}

void giants_pack(GiantsStage *state, const Map *map) {
    memset(state->explored, 0, sizeof(state->explored));
    for (int y = 0; y < GIANTS_H; y++) {
        for (int x = 0; x < GIANTS_W; x++) {
            state->tiles[y][x] = map->tiles[y][x];
            int bit = y * GIANTS_W + x;
            if (map_is_explored(map, x, y)) {
                state->explored[bit / 8] |= 1u << (bit % 8);
            }
        }
    }
}

void giants_unpack(const GiantsStage *state, Map *map) {
    memset(map, 0, sizeof(*map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            map->tiles[y][x] = TILE_GIANTS_WALL;
            if (inside(x, y)) {
                map->tiles[y][x] = state->tiles[y][x];
                int bit = y * GIANTS_W + x;
                if (state->explored[bit / 8] & (1u << (bit % 8))) {
                    int dest = y * MAP_W + x;
                    map->explored[dest / 8] |= 1u << (dest % 8);
                }
            }
        }
    }
    map->stairs_up_x = map->stairs_down_x = 32;
    map->stairs_up_y = GIANTS_H - 1;
    map->stairs_down_y = 0;
}

void giants_capture(const GameState *g, GiantsStage *state) {
    if (g->location != LOCATION_GIANTS_CROWN) {
        return;
    }
    giants_pack(state, &g->map);
    state->valid = 1;
    state->enemy_count = g->enemy_count;
    memcpy(state->enemies, g->enemies, sizeof(Enemy) * g->enemy_count);
    state->loot_count = g->floor_item_count;
    memcpy(state->loot, g->floor_items, sizeof(FloorItem) * g->floor_item_count);
    for (int n = 0; n < state->loot_count; n++) {
        FloorItem *item = &state->loot[n];
        if (item->active && inside(item->x, item->y)) {
            state->tiles[item->y][item->x] = item->underlying_tile;
        }
    }
}

void giants_store(GameState *g) {
    GiantsStage *state = current(g);
    if (state) {
        giants_capture(g, state);
    }
}

static int occupied(const GameState *g, int x, int y) {
    for (int n = 0; n < g->enemy_count; n++) {
        if (g->enemies[n].active && g->enemies[n].x == x && g->enemies[n].y == y) {
            return 1;
        }
    }
    for (int n = 0; n < g->floor_item_count; n++) {
        if (g->floor_items[n].active && g->floor_items[n].x == x && g->floor_items[n].y == y) {
            return 1;
        }
    }
    return g->portal_active && g->portal_location == LOCATION_GIANTS_CROWN && g->portal_level == g->level && g->portal_x == x && g->portal_y == y;
}

static void populate(GameState *g) {
    unsigned char seen[GIANTS_H][GIANTS_W] = {{0}};
    int queue[GIANTS_W * GIANTS_H];
    int head = 0;
    int tail = 1;
    queue[0] = (GIANTS_H - 1) * GIANTS_W + 32;
    seen[GIANTS_H - 1][32] = 1;
    const int dx[] = {1,0,-1,0};
    const int dy[] = {0,1,0,-1};
    while (head < tail) {
        int cell = queue[head++];
        for (int n = 0; n < 4; n++) {
            int x = cell % GIANTS_W + dx[n];
            int y = cell / GIANTS_W + dy[n];
            if (inside(x, y) && !seen[y][x] && map_is_walkable(&g->map, x, y)) {
                seen[y][x] = 1;
                queue[tail++] = y * GIANTS_W + x;
            }
        }
    }
    g->enemy_count = 0;
    if (g->level == GIANTS_DEPTH && !g->giants_crown_defeated) {
        game_spawn_enemy(g, &g->enemies[g->enemy_count++], ENEMY_LAST_JARL, 32, 10);
    }
    static const EnemyType types[] = {ENEMY_OGRE, ENEMY_CAVE_TROLL, ENEMY_HILL_GIANT, ENEMY_STONE_GIANT, ENEMY_SHARD_GOLEM, ENEMY_FROST_GIANT};
    int limit = g->level == 1 ? 2 : g->level == 2 ? 3 : g->level == 3 ? 5 : 6;
    int count = 7 + g->level;
    for (int n = g->enemy_count; n < count; n++) {
        for (int tries = 0; tries < 2000; tries++) {
            int cell = queue[rand() % tail];
            int x = cell % GIANTS_W;
            int y = cell / GIANTS_W;
            TileType tile = g->map.tiles[y][x];
            if (y > GIANTS_H - 9 || y < 4 || occupied(g, x, y) ||
                (tile != TILE_GIANTS_GRASS && tile != TILE_GIANTS_PAVING && tile != TILE_GIANTS_SNOW)) {
                continue;
            }
            EnemyType type = types[n % limit];
            game_spawn_enemy(g, &g->enemies[g->enemy_count++], type, x, y);
            break;
        }
    }
}

static void load_stage(GameState *g, int refresh) {
    GiantsStage *state = current(g);
    if (!state->valid) {
        giants_generate(&g->map, g->level);
        g->floor_item_count = 0;
        crown_quest_prepare(g);
        populate(g);
    } else {
        giants_unpack(state, &g->map);
        g->enemy_count = state->enemy_count;
        memcpy(g->enemies, state->enemies, sizeof(Enemy) * state->enemy_count);
        g->floor_item_count = state->loot_count;
        memcpy(g->floor_items, state->loot, sizeof(FloorItem) * state->loot_count);
        crown_quest_prepare(g);
        if (refresh) {
            populate(g);
        }
    }
    giants_store(g);
    g->level_cleared = 0;
    game_update_level_progress(g);
}

void giants_enter(GameState *g, int portal) {
    g->location = LOCATION_GIANTS_CROWN;
    g->approach_route = -1;
    if (!portal) {
        for (int n = 0; n < GIANTS_DEPTH; n++) {
            if (g->giants_crown[n].valid) {
                g->level = n + 1;
                load_stage(g, 1);
            }
        }
    }
    g->level = portal ? g->portal_level : 1;
    load_stage(g, 0);
    g->player.x = portal ? g->portal_x : 32;
    g->player.y = portal ? g->portal_y : GIANTS_H - 2;
    g->player.last_dx = 0;
    g->player.last_dy = -1;
    g->dialogue_active = 0;
    if (g->portal_location == LOCATION_GIANTS_CROWN) {
        g->portal_active = 0;
    }
    map_mark_explored(&g->map, g->player.x, g->player.y);
    push_message(g, "Giant's Peaks: ruined halls and terraces. Heavy strikes break cracked masonry and pillars.");
    push_message(g, "Leave the amber warnings before a strike. A clears cracked walls or searches stone coffers.");
}

void giants_leave(GameState *g) {
    giants_store(g);
    approach_enter_region(g, GIANTS_APPROACH_ROUTE);
    g->player.x = g->map.stairs_down_x;
    g->player.y = g->map.stairs_down_y + 1;
    map_mark_explored(&g->map, g->player.x, g->player.y);
}

void giants_change_level(GameState *g, int forward) {
    if (!current(g)) {
        return;
    }
    if ((!forward && g->level == 1) || (forward && g->level == GIANTS_DEPTH)) {
        if (forward && !g->giants_crown_defeated && !g->debug_passages_unlocked) {
            push_message(g, "The Last Jarl guards the final departure. You can retreat through the southern entrance.");
        } else {
            giants_leave(g);
        }
        return;
    }
    giants_store(g);
    g->level += forward ? 1 : -1;
    load_stage(g, 0);
    g->player.x = 32;
    g->player.y = forward ? GIANTS_H - 2 : 1;
    map_mark_explored(&g->map, g->player.x, g->player.y);
}

int giants_travel(GameState *g, int x, int y) {
    if (!current(g) || !inside(x, y)) {
        return 0;
    }
    TileType tile = g->map.tiles[y][x];
    if (tile != TILE_GIANTS_ENTRANCE && tile != TILE_GIANTS_EXIT) {
        return 0;
    }
    giants_change_level(g, tile == TILE_GIANTS_EXIT);
    return 1;
}

static TileType nearby(const GameState *g, int *tx, int *ty) {
    const int dx[] = {0,1,0,-1,0};
    const int dy[] = {0,0,1,0,-1};
    for (int n = 0; n < 5; n++) {
        int x = g->player.x + dx[n];
        int y = g->player.y + dy[n];
        if (inside(x, y)) {
            TileType tile = g->map.tiles[y][x];
            if (tile == TILE_GIANTS_CACHE || tile == TILE_GIANTS_CRACK) {
                *tx = x;
                *ty = y;
                return tile;
            }
        }
    }
    return TILE_GIANTS_GRASS;
}

const char *giants_hint(const GameState *g) {
    if (crown_quest_hint(g)) {
        return crown_quest_hint(g);
    }
    if (g->location != LOCATION_GIANTS_CROWN) {
        return NULL;
    }
    if (containers_hint(g)) {
        return containers_hint(g);
    }
    int x;
    int y;
    TileType tile = nearby(g, &x, &y);
    return tile == TILE_GIANTS_CACHE ? "A: SEARCH STONE COFFER" : tile == TILE_GIANTS_CRACK ? "A: CLEAR CRACKED MASONRY" : NULL;
}

int giants_interact(GameState *g) {
    if (crown_quest_interact(g)) {
        return 1;
    }
    if (!current(g)) {
        return 0;
    }
    if (containers_hint(g)) {
        return containers_interact(g);
    }
    int x;
    int y;
    TileType tile = nearby(g, &x, &y);
    if (tile == TILE_GIANTS_CACHE) {
        return containers_interact(g);
    } else if (tile == TILE_GIANTS_CRACK) {
        g->map.tiles[y][x] = TILE_GIANTS_PAVING;
        push_message(g, "You clear the fractured stones. A forgotten chamber opens.");
    } else {
        return 0;
    }
    return 1;
}

int giants_attack_marks(const Enemy *enemy, int x, int y) {
    if (!enemy->active || !enemy->attack_phase || enemy->move_timer <= 0) {
        return 0;
    }
    int dx = abs(x - enemy->attack_target_x);
    int dy = abs(y - enemy->attack_target_y);
    return dx + dy <= (enemy->is_boss ? 2 : 1);
}

static int clear_view(const GameState *g, const Enemy *enemy) {
    int dx = g->player.x - enemy->x;
    int dy = g->player.y - enemy->y;
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
    for (int n = 1; n < steps; n++) {
        int x = enemy->x + dx * n / steps;
        int y = enemy->y + dy * n / steps;
        if (!map_is_walkable(&g->map, x, y)) {
            return 0;
        }
    }
    return 1;
}

int giants_tick(GameState *g, Enemy *enemy, int *damage) {
    if (g->location != LOCATION_GIANTS_CROWN) {
        return 0;
    }
    *damage = 0;
    if (enemy->move_timer < 0) {
        enemy->move_timer++;
        return 1;
    }
    if (enemy->attack_phase) {
        if (enemy->move_timer > 1) {
            enemy->move_timer--;
            return 1;
        }
        if (giants_attack_marks(enemy, g->player.x, g->player.y)) {
            *damage = enemy->attack;
        }
        for (int y = enemy->attack_target_y - 2; y <= enemy->attack_target_y + 2; y++) {
            for (int x = enemy->attack_target_x - 2; x <= enemy->attack_target_x + 2; x++) {
                if (inside(x, y) && giants_attack_marks(enemy, x, y) &&
                    (g->map.tiles[y][x] == TILE_GIANTS_COLUMN || g->map.tiles[y][x] == TILE_GIANTS_CRACK)) {
                    g->map.tiles[y][x] = TILE_GIANTS_PAVING;
                }
            }
        }
        enemy->attack_phase = 0;
        enemy->move_timer = -2;
        return 1;
    }
    int distance = abs(enemy->x - g->player.x) + abs(enemy->y - g->player.y);
    int reach = enemy->type == ENEMY_HILL_GIANT || enemy->is_boss ? 6 : 3;
    if (distance > reach || !clear_view(g, enemy)) {
        return 0;
    }
    enemy->attack_phase = 1;
    enemy->attack_target_x = g->player.x;
    enemy->attack_target_y = g->player.y;
    enemy->move_timer = 2;
    push_message(g, enemy->type == ENEMY_HILL_GIANT ? "A Hill Giant lifts a boulder! Leave the amber mark before it lands." :
        enemy->is_boss ? "The Last Jarl raises his stone maul! Clear the marked ground." : "A heavy strike is coming! Leave the marked ground.");
    return 1;
}

void giants_defeat(GameState *g) {
    if (!g->giants_crown_defeated) {
        g->giants_crown_defeated = 1;
        g->gold += 200;
        g->score += 2200;
        push_message(g, "The Last Jarl falls. The final departure opens. You recover 200 gold.");
    }
    crown_quest_sync(g);
}

static int valid_stage(const GiantsStage *state) {
    if (state->valid < 0 || state->valid > 1 || state->enemy_count < 0 || state->enemy_count > AREA_ENEMY_LIMIT ||
        state->loot_count < 0 || state->loot_count > MAX_FLOOR_ITEMS) {
        return 0;
    }
    if (!state->valid) {
        return 1;
    }
    for (int y = 0; y < GIANTS_H; y++) {
        for (int x = 0; x < GIANTS_W; x++) {
            if (state->tiles[y][x] < TILE_GIANTS_GRASS || state->tiles[y][x] > TILE_GIANTS_EXIT) {
                return 0;
            }
        }
    }
    for (int n = 0; n < state->enemy_count; n++) {
        const Enemy *enemy = &state->enemies[n];
        if (!inside(enemy->x, enemy->y) || enemy->max_hp <= 0 || enemy->attack_phase < 0 || enemy->attack_phase > 1 ||
            (enemy->attack_phase && (!inside(enemy->attack_target_x, enemy->attack_target_y) || enemy->move_timer < 1 || enemy->move_timer > 2))) {
            return 0;
        }
    }
    for (int n = 0; n < state->loot_count; n++) {
        if (state->loot[n].active && !inside(state->loot[n].x, state->loot[n].y)) {
            return 0;
        }
    }
    return state->tiles[GIANTS_H - 1][32] == TILE_GIANTS_ENTRANCE && state->tiles[0][32] == TILE_GIANTS_EXIT;
}

int giants_valid(const GameState *g) {
    if (g->giants_crown_defeated < 0 || g->giants_crown_defeated > 1) {
        return 0;
    }
    for (int n = 0; n < GIANTS_DEPTH; n++) {
        if (!valid_stage(&g->giants_crown[n])) {
            return 0;
        }
    }
    if (g->location == LOCATION_GIANTS_CROWN) {
        if (g->level < 1 || g->level > GIANTS_DEPTH || !inside(g->player.x, g->player.y) ||
            g->enemy_count < 0 || g->enemy_count > AREA_ENEMY_LIMIT || g->floor_item_count < 0 || g->floor_item_count > MAX_FLOOR_ITEMS) {
            return 0;
        }
        GiantsStage state = {0};
        giants_capture(g, &state);
        return valid_stage(&state);
    }
    return 1;
}

void giants_place_castle_road(Map *map) {
    for (int x = CASTLE_ASHEN_GATE_X; x <= CROWNROAD_X; x++) {
        map->tiles[CASTLE_ROAD_Y][x] = TILE_TOWN_PATH;
    }
    for (int y = 1; y < CASTLE_ROAD_Y; y++) {
        map->tiles[y][CASTLE_ASHEN_GATE_X] = TILE_TOWN_PATH;
    }
    map->tiles[0][CASTLE_ASHEN_GATE_X] = TILE_TOWN_EXIT;
}

static void relocate_point(const Map *old, const Map *map, int *x, int *y) {
    int vertical = old->stairs_up_y == 0 || old->stairs_up_y == APPROACH_H - 1;
    int dx = vertical ? 0 : old->stairs_up_x == 0 ? 1 : -1;
    int dy = vertical ? old->stairs_up_y == 0 ? 1 : -1 : 0;
    int along = (*x - old->stairs_up_x) * dx + (*y - old->stairs_up_y) * dy;
    int side = (*y - old->stairs_up_y) * dx - (*x - old->stairs_up_x) * dy;
    *x = map->stairs_up_x + side;
    *y = map->stairs_up_y - along;
}

void giants_migrate_geography(GameState *g) {
    if (g->location == LOCATION_CASTLE) {
        // Preserve dropped objects while exposing the new perimeter path.
        giants_place_castle_road(&g->map);
        for (int n = 0; n < g->floor_item_count; n++) {
            FloorItem *item = &g->floor_items[n];
            if (item->active && ((item->x == CASTLE_ASHEN_GATE_X && item->y < CASTLE_ROAD_Y) ||
                (item->y == CASTLE_ROAD_Y && item->x >= CASTLE_ASHEN_GATE_X && item->x <= CROWNROAD_X))) {
                item->underlying_tile = g->map.tiles[item->y][item->x];
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
    }
    if (g->location == LOCATION_APPROACH && g->approach_route == ASHEN_APPROACH_ROUTE) {
        Map old = g->map;
        FloorItem loot[MAX_FLOOR_ITEMS];
        int count = g->floor_item_count;
        memcpy(loot, g->floor_items, sizeof(FloorItem) * count);
        int px = g->player.x;
        int py = g->player.y;
        approach_enter_region(g, ASHEN_APPROACH_ROUTE);
        relocate_point(&old, &g->map, &px, &py);
        if (px >= 0 && px < APPROACH_W && py >= 0 && py < APPROACH_H && map_is_walkable(&g->map, px, py)) {
            g->player.x = px;
            g->player.y = py;
        }
        g->floor_item_count = count;
        for (int n = 0; n < count; n++) {
            g->floor_items[n] = loot[n];
            if (loot[n].active) {
                int x = loot[n].x;
                int y = loot[n].y;
                relocate_point(&old, &g->map, &x, &y);
                if (x < 0 || x >= APPROACH_W || y < 0 || y >= APPROACH_H || !map_is_walkable(&g->map, x, y)) {
                    x = g->player.x;
                    y = g->player.y;
                }
                g->floor_items[n].x = x;
                g->floor_items[n].y = y;
                g->floor_items[n].underlying_tile = g->map.tiles[y][x];
                for (int i = 0; i < n; i++) {
                    if (g->floor_items[i].active && g->floor_items[i].x == x && g->floor_items[i].y == y) {
                        g->floor_items[n].underlying_tile = g->floor_items[i].underlying_tile;
                    }
                }
                g->map.tiles[y][x] = TILE_ITEM;
            }
        }
        for (int y = 0; y < APPROACH_H; y++) {
            for (int x = 0; x < APPROACH_W; x++) {
                if (map_is_explored(&old, x, y)) {
                    int nx = x;
                    int ny = y;
                    relocate_point(&old, &g->map, &nx, &ny);
                    if (nx >= 0 && nx < APPROACH_W && ny >= 0 && ny < APPROACH_H) {
                        map_mark_explored(&g->map, nx, ny);
                    }
                }
            }
        }
        map_mark_explored(&g->map, g->player.x, g->player.y);
    }
    if (g->dialogue_active && strstr(g->dialogue_text, "Ashen")) {
        g->dialogue_active = 0;
    }
}
