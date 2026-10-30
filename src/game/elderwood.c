#include "combat_feedback.h"
#include "elderwood.h"
#include "boss_gates.h"
#include "crownroad_north.h"
#include "approach.h"
#include "heartseed.h"
#include <stdlib.h>
#include <string.h>

static void refresh_population(GameState *g);

static int sign(int value) {
    return (value > 0) - (value < 0);
}

static int inside(int x, int y) {
    return x > 0 && x < ELDERWOOD_W - 1 && y > 0 && y < ELDERWOOD_H - 1;
}

static void discard_loot(GameState *g) {
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && g->map.tiles[item->y][item->x] == TILE_ITEM) {
            g->map.tiles[item->y][item->x] = item->underlying_tile;
        }
    }
    g->floor_item_count = 0;
}

void elderwood_pack(ElderwoodCache *cache, const Map *map) {
    memset(cache->explored, 0, sizeof(cache->explored));
    for (int y = 0; y < ELDERWOOD_H; y++) {
        for (int x = 0; x < ELDERWOOD_W; x++) {
            cache->tiles[y][x] = map->tiles[y][x];
            int bit = y * ELDERWOOD_W + x;
            if (map_is_explored(map, x, y)) {
                cache->explored[bit / 8] |= 1u << (bit % 8);
            }
        }
    }
}

void elderwood_unpack(const ElderwoodCache *cache, Map *map) {
    memset(map, 0, sizeof(*map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            map->tiles[y][x] = TILE_ELDERWOOD_WALL;
            if (x < ELDERWOOD_W && y < ELDERWOOD_H) {
                map->tiles[y][x] = cache->tiles[y][x];
                int bit = y * ELDERWOOD_W + x;
                if (cache->explored[bit / 8] & (1u << (bit % 8))) {
                    int dest = y * MAP_W + x;
                    map->explored[dest / 8] |= 1u << (dest % 8);
                }
            }
        }
    }
    map->stairs_up_x = ELDERWOOD_W / 2;
    map->stairs_up_y = ELDERWOOD_H - 2;
    map->stairs_down_x = ELDERWOOD_W / 2;
    map->stairs_down_y = 1;
}

void elderwood_store(GameState *g) {
    if (g->location != LOCATION_ELDERWOOD || g->level < 1 || g->level > ELDERWOOD_DEPTH) {
        return;
    }
    ElderwoodCache *cache = &g->elderwood_cache[g->level - 1];
    elderwood_pack(cache, &g->map);
    for (int i = 0; i < g->floor_item_count; i++) {
        const FloorItem *item = &g->floor_items[i];
        if (item->active && inside(item->x, item->y) && cache->tiles[item->y][item->x] == TILE_ITEM) {
            cache->tiles[item->y][item->x] = item->underlying_tile;
        }
    }
    cache->enemy_count = g->enemy_count < AREA_ENEMY_LIMIT ? g->enemy_count : AREA_ENEMY_LIMIT;
    memcpy(cache->enemies, g->enemies, cache->enemy_count * sizeof(Enemy));
    cache->level_cleared = g->level_cleared;
    cache->valid = 1;
}

static void carve(Map *map, int x, int y, int tx, int ty) {
    while (1) {
        for (int oy = -1; oy <= 1; oy++) {
            for (int ox = -1; ox <= 1; ox++) {
                if (inside(x + ox, y + oy)) {
                    map->tiles[y + oy][x + ox] = TILE_ELDERWOOD_FLOOR;
                }
            }
        }
        if (x == tx && y == ty) {
            break;
        }
        if (x != tx) {
            x += sign(tx - x);
        } else {
            y += sign(ty - y);
        }
    }
}

static void make_enemy(Enemy *enemy, EnemyType type, int x, int y, int level) {
    *enemy = (Enemy){.active = 1, .type = type, .x = x, .y = y,
        .attack_target_x = -1, .attack_target_y = -1};
    const char *name = "Blighted Wolf";
    int hp = 26;
    int attack = 7;
    int defense = 1;
    int xp = 28;
    if (type == ENEMY_DARK_ELF_RANGER) {
        name = "Dark Elf Ranger";
        hp = 42; attack = 10; defense = 3; xp = 40;
    } else if (type == ENEMY_THORN_DRYAD) {
        name = "Thorn Dryad";
        hp = 32; attack = 9; defense = 2; xp = 38;
    } else if (type == ENEMY_TREEFOLK) {
        name = "Treefolk";
        hp = 92; attack = 15; defense = 8; xp = 90;
    } else if (type == ENEMY_DARK_ELF_ARCHER) {
        name = "Dark Elf Archer";
        hp = 22; attack = 9; defense = 3; xp = 28;
    } else if (type == ENEMY_ELDERTHORN) {
        name = "Elderthorn";
        hp = 450; attack = 22; defense = 10; xp = 720;
        enemy->is_boss = 1;
    }
    snprintf(enemy->name, sizeof(enemy->name), "%s", name);
    enemy->max_hp = hp + (enemy->is_boss ? 0 : (level - 1) * 7);
    enemy->hp = enemy->max_hp;
    enemy->attack = attack + (enemy->is_boss ? 0 : level - 1);
    enemy->defense = defense;
    enemy->experience = xp + level * 4;
}

static void add_enemy(GameState *g, EnemyType type, int x, int y) {
    if (g->enemy_count < AREA_ENEMY_LIMIT) {
        make_enemy(&g->enemies[g->enemy_count++], type, x, y, g->level);
    }
}

static const int clearing_x[6] = {12, 32, 52, 13, 34, 53};
static const int clearing_y[6] = {11, 10, 12, 33, 32, 34};

int elderwood_in_clearing(int x, int y, int x0, int y0, int x1, int y1) {
    // The ellipse inscribed in the rectangle, in doubled units so even spans stay centred.
    int dx = 2 * x - x0 - x1;
    int dy = 2 * y - y0 - y1;
    int rx = x1 - x0;
    int ry = y1 - y0;
    return dx * dx * ry * ry + dy * dy * rx * rx <= rx * rx * ry * ry;
}

static void layout(Map *map, const int width[6], const int height[6], int boss) {
    for (int y = 0; y < ELDERWOOD_H; y++) {
        for (int x = 0; x < ELDERWOOD_W; x++) {
            map->tiles[y][x] = TILE_ELDERWOOD_WALL;
        }
    }
    // Rounded clearings, as in the Haunted Forest.
    for (int i = 0; i < 6; i++) {
        int x0 = clearing_x[i] - width[i];
        int y0 = clearing_y[i] - height[i];
        int x1 = clearing_x[i] + width[i];
        int y1 = clearing_y[i] + height[i];
        for (int y = y0; y <= y1; y++) {
            for (int x = x0; x <= x1; x++) {
                if (elderwood_in_clearing(x, y, x0, y0, x1, y1)) {
                    map->tiles[y][x] = TILE_ELDERWOOD_FLOOR;
                }
            }
        }
    }
    carve(map, 32, 46, 13, 33);
    carve(map, 13, 33, 53, 34);
    carve(map, 13, 33, 12, 11);
    carve(map, 53, 34, 52, 12);
    carve(map, 12, 11, 52, 12);
    carve(map, 32, 10, 32, 1);
    carve(map, 34, 32, 32, 10);
    for (int y = 20; y <= 22; y++) {
        for (int x = 31; x <= 33; x++) {
            map->tiles[y][x] = TILE_ELDERWOOD_BRAMBLE;
        }
    }
    for (int i = 0; i < 6; i++) {
        map->tiles[clearing_y[i] - 2][clearing_x[i] + 2] = TILE_ELDERWOOD_LOG;
    }
    if (boss) {
        for (int y = 7; y <= 38; y++) {
            for (int x = 17; x <= 47; x++) {
                if (elderwood_in_clearing(x, y, 17, 7, 47, 38)) {
                    map->tiles[y][x] = TILE_ELDERWOOD_FLOOR;
                }
            }
        }
        for (int y = 16; y <= 30; y += 14) {
            map->tiles[y][25] = TILE_ELDERWOOD_LOG;
            map->tiles[y][39] = TILE_ELDERWOOD_LOG;
        }
    }
    map->tiles[46][32] = TILE_ELDERWOOD_ENTRANCE;
    map->tiles[1][32] = TILE_ELDERWOOD_EXIT;
}

static void generate(GameState *g) {
    ElderwoodCache blank = {0};
    elderwood_unpack(&blank, &g->map);
    int width[6];
    int height[6];
    for (int i = 0; i < 6; i++) {
        width[i] = 5 + rand() % 2;
        height[i] = 4 + rand() % 2;
    }
    layout(&g->map, width, height, g->level == ELDERWOOD_DEPTH);
    g->enemy_count = 0;
    g->level_cleared = 0;
    if (g->level == ELDERWOOD_DEPTH) {
        if (!g->elderwood_defeated) {
            add_enemy(g, ENEMY_ELDERTHORN, 32, 13);
        }
        add_enemy(g, ENEMY_TREEFOLK, 21, 27);
        add_enemy(g, ENEMY_DARK_ELF_ARCHER, 43, 26);
        add_enemy(g, ENEMY_BLIGHTED_WOLF, 26, 35);
        add_enemy(g, ENEMY_THORN_DRYAD, 38, 34);
    } else {
        const EnemyType types[5] = {ENEMY_BLIGHTED_WOLF, ENEMY_DARK_ELF_RANGER, ENEMY_THORN_DRYAD, ENEMY_DARK_ELF_ARCHER, ENEMY_TREEFOLK};
        for (int i = 0; i < 4 + g->level * 2; i++) {
            int room = i % 6;
            int x = clearing_x[room] + (i / 6) * 2 - 2;
            int y = clearing_y[room] + 1;
            add_enemy(g, types[(i + g->level - 1) % 5], x, y);
        }
    }
}

static int saved_extent(const Map *map, int x, int y, int dx, int dy, int low, int high) {
    // Count unbroken floor from a clearing's centre along a line no trail crosses.
    int n = 0;
    while (n < high && map->tiles[y + dy * (n + 1)][x + dx * (n + 1)] != TILE_ELDERWOOD_WALL) {
        n++;
    }
    return n < low ? low : n;
}

static int removable(const Map *map, const Map *shape, int x, int y) {
    return inside(x, y) && map->tiles[y][x] == TILE_ELDERWOOD_FLOOR && shape->tiles[y][x] == TILE_ELDERWOOD_WALL;
}

static void keep_path(const Map *map, const Map *shape, unsigned char keep[ELDERWOOD_H][ELDERWOOD_W], int x, int y) {
    // Spare the shortest run of corner tiles linking an occupant to the rounded floor.
    if (!inside(x, y) || shape->tiles[y][x] != TILE_ELDERWOOD_WALL) {
        return;
    }
    static const int steps[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    int from[ELDERWOOD_W * ELDERWOOD_H];
    int queue[ELDERWOOD_W * ELDERWOOD_H];
    memset(from, -1, sizeof(from));
    int start = y * ELDERWOOD_W + x;
    int head = 0;
    int tail = 0;
    from[start] = start;
    queue[tail++] = start;
    keep[y][x] = 1;
    while (head < tail) {
        int cell = queue[head++];
        for (int i = 0; i < 4; i++) {
            int nx = cell % ELDERWOOD_W + steps[i][0];
            int ny = cell / ELDERWOOD_W + steps[i][1];
            int next = ny * ELDERWOOD_W + nx;
            if (!inside(nx, ny) || from[next] >= 0) {
                continue;
            }
            if (map->tiles[ny][nx] != TILE_ELDERWOOD_WALL && shape->tiles[ny][nx] != TILE_ELDERWOOD_WALL) {
                for (int path = cell; path != start; path = from[path]) {
                    keep[path / ELDERWOOD_W][path % ELDERWOOD_W] = 1;
                }
                return;
            }
            if (removable(map, shape, nx, ny)) {
                from[next] = cell;
                queue[tail++] = next;
            }
        }
    }
}

static void round_floor(const GameState *g, Map *map, int level, const Enemy *enemies, int count, int live) {
    int width[6];
    int height[6];
    for (int i = 0; i < 6; i++) {
        width[i] = saved_extent(map, clearing_x[i], clearing_y[i] + 3, 1, 0, 5, 6);
        height[i] = saved_extent(map, clearing_x[i] + 3, clearing_y[i], 0, 1, 4, 5);
    }
    Map shape;
    layout(&shape, width, height, level == ELDERWOOD_DEPTH);
    if (level == 4 && (g->heartseed_sites & 2)) {
        heartseed_carve_grove(&shape);
    }
    unsigned char keep[ELDERWOOD_H][ELDERWOOD_W] = {{0}};
    for (int i = 0; i < count; i++) {
        if (enemies[i].active) {
            keep_path(map, &shape, keep, enemies[i].x, enemies[i].y);
        }
    }
    if (g->portal_location == LOCATION_ELDERWOOD && g->portal_level == level) {
        keep_path(map, &shape, keep, g->portal_x, g->portal_y);
    }
    if (live) {
        keep_path(map, &shape, keep, g->player.x, g->player.y);
        for (int i = 0; i < g->floor_item_count; i++) {
            if (g->floor_items[i].active) {
                keep_path(map, &shape, keep, g->floor_items[i].x, g->floor_items[i].y);
            }
        }
    }
    for (int y = 0; y < ELDERWOOD_H; y++) {
        for (int x = 0; x < ELDERWOOD_W; x++) {
            if (removable(map, &shape, x, y) && !keep[y][x]) {
                map->tiles[y][x] = TILE_ELDERWOOD_WALL;
            }
        }
    }
}

void elderwood_round_saved_clearings(GameState *g) {
    // Floors saved before rounded clearings keep their trails, objects and occupants.
    for (int level = 1; level <= ELDERWOOD_DEPTH; level++) {
        ElderwoodCache *cache = &g->elderwood_cache[level - 1];
        if (cache->valid) {
            Map map;
            elderwood_unpack(cache, &map);
            round_floor(g, &map, level, cache->enemies, cache->enemy_count, 0);
            elderwood_pack(cache, &map);
        }
    }
    if (g->location == LOCATION_ELDERWOOD && g->level >= 1 && g->level <= ELDERWOOD_DEPTH) {
        round_floor(g, &g->map, g->level, g->enemies, g->enemy_count, 1);
    }
}

static void load_floor(GameState *g) {
    ElderwoodCache *cache = &g->elderwood_cache[g->level - 1];
    if (cache->valid) {
        elderwood_unpack(cache, &g->map);
        g->enemy_count = cache->enemy_count;
        memcpy(g->enemies, cache->enemies, cache->enemy_count * sizeof(Enemy));
        g->level_cleared = cache->level_cleared;
    } else {
        generate(g);
    }
    if (g->max_elderwood_level_reached < g->level) {
        g->max_elderwood_level_reached = g->level;
    }
    g->floor_item_count = 0;
    heartseed_prepare(g);
}

void elderwood_enter(GameState *g) {
    const TownApproach *route = approach_route(g);
    g->elderwood_entry_town = route ? route->town : LOCATION_TOWN;
    discard_loot(g);
    g->location = LOCATION_ELDERWOOD;
    g->approach_route = -1;
    g->player.x = 32;
    g->player.y = 46;
    for (int level = 1; level <= ELDERWOOD_DEPTH; level++) {
        if (g->elderwood_cache[level - 1].valid) {
            g->level = level;
            load_floor(g);
            refresh_population(g);
            elderwood_store(g);
        }
    }
    g->level = 1;
    load_floor(g);
    g->player.x = 32;
    g->player.y = 46;
    g->player.last_dx = 0;
    g->player.last_dy = -1;
    g->dialogue_active = 0;
    map_mark_explored(&g->map, g->player.x, g->player.y);
    push_message(g, "You enter Elderwood Forest. Ancient roots shelter the living woods.");
    push_message(g, "Interact beside brambles to cut a path. Logs stop charging creatures.");
}

void elderwood_change_level(GameState *g, int descending) {
    int next = g->level + (descending ? 1 : -1);
    if (next < 1 || next > ELDERWOOD_DEPTH) {
        return;
    }
    elderwood_store(g);
    discard_loot(g);
    g->level = next;
    load_floor(g);
    g->player.x = 32;
    g->player.y = descending ? 46 : 1;
    map_mark_explored(&g->map, g->player.x, g->player.y);
}

int elderwood_travel(GameState *g, int x, int y) {
    if (g->location != LOCATION_ELDERWOOD) {
        return 0;
    }
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_ELDERWOOD_ENTRANCE) {
        if (g->level == 1) {
            elderwood_leave(g);
        } else {
            elderwood_change_level(g, 0);
        }
        return 1;
    }
    if (tile == TILE_ELDERWOOD_EXIT) {
        if (g->level == ELDERWOOD_DEPTH) {
            if (boss_gate_crossing(g, LOCATION_ELDERWOOD)) {
                elderwood_leave(g);
            }
        } else {
            elderwood_change_level(g, 1);
        }
        return 1;
    }
    return 0;
}

int elderwood_interact(GameState *g) {
    if (heartseed_interact(g)) {
        return 1;
    }
    if (g->location != LOCATION_ELDERWOOD) {
        return 0;
    }
    for (int y = g->player.y - 1; y <= g->player.y + 1; y++) {
        for (int x = g->player.x - 1; x <= g->player.x + 1; x++) {
            if (inside(x, y) && g->map.tiles[y][x] == TILE_ELDERWOOD_BRAMBLE) {
                g->map.tiles[y][x] = TILE_ELDERWOOD_FLOOR;
                push_message(g, "You cut through the brambles, opening a flanking path.");
                return 1;
            }
        }
    }
    return 0;
}

static int occupied(const GameState *g, const Enemy *self, int x, int y) {
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *enemy = &g->enemies[i];
        if (enemy != self && enemy->active && enemy->x == x && enemy->y == y) {
            return 1;
        }
    }
    return self && x == g->player.x && y == g->player.y;
}

static void refresh_population(GameState *g) {
    unsigned char regular[AREA_ENEMY_LIMIT] = {0};
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *enemy = &g->enemies[i];
        Enemy fresh;
        make_enemy(&fresh, enemy->type, enemy->x, enemy->y, g->level);
        if (!enemy->is_boss && !enemy->dain_fragment && enemy->experience > 0 && strcmp(enemy->name, fresh.name) == 0) {
            regular[i] = 1;
            enemy->active = 0;
        }
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (!regular[i]) {
            continue;
        }
        Enemy *enemy = &g->enemies[i];
        int x = enemy->x;
        int y = enemy->y;
        int found = 0;
        for (int n = -1; n < ELDERWOOD_W * ELDERWOOD_H; n++) {
            int tx = n < 0 ? x : n % ELDERWOOD_W;
            int ty = n < 0 ? y : n / ELDERWOOD_W;
            if (inside(tx, ty) && map_is_walkable(&g->map, tx, ty) && !occupied(g, enemy, tx, ty) &&
                g->map.tiles[ty][tx] != TILE_ELDERWOOD_ENTRANCE && g->map.tiles[ty][tx] != TILE_ELDERWOOD_EXIT) {
                make_enemy(enemy, enemy->type, tx, ty, g->level);
                found = 1;
                break;
            }
        }
        if (found) {
            g->level_cleared = 0;
        }
    }
}

static void step(GameState *g, Enemy *enemy, int tx, int ty) {
    int dx = sign(tx - enemy->x);
    int dy = sign(ty - enemy->y);
    int nx[4] = {enemy->x + dx, enemy->x, enemy->x - dy, enemy->x + dy};
    int ny[4] = {enemy->y, enemy->y + dy, enemy->y, enemy->y};
    for (int i = 0; i < 4; i++) {
        if ((nx[i] != enemy->x || ny[i] != enemy->y) && inside(nx[i], ny[i]) &&
            map_is_walkable(&g->map, nx[i], ny[i]) && !occupied(g, enemy, nx[i], ny[i])) {
            enemy->facing_dx = nx[i] - enemy->x;
            enemy->facing_dy = ny[i] - enemy->y;
            enemy->x = nx[i];
            enemy->y = ny[i];
            return;
        }
    }
}

int elderwood_marks(const Enemy *enemy, int x, int y) {
    if (!enemy->active || enemy->move_timer <= 0 || !enemy->attack_phase) {
        return 0;
    }
    if (enemy->boss_attack == ELDER_CHARGE) {
        int dx = sign(enemy->attack_target_x - enemy->x);
        int dy = sign(enemy->attack_target_y - enemy->y);
        for (int n = 1; n <= 7; n++) {
            if (x == enemy->x + n * dx && y == enemy->y + n * dy) {
                return 1;
            }
        }
    } else if (enemy->boss_attack == ELDER_ROOTS) {
        return abs(x - enemy->attack_target_x) + abs(y - enemy->attack_target_y) <= (enemy->is_boss ? 2 : 1);
    }
    return 0;
}

int elderwood_tick(GameState *g, Enemy *enemy, int *damage) {
    if (g->location != LOCATION_ELDERWOOD || (enemy->type != ENEMY_TREEFOLK && enemy->type != ENEMY_ELDERTHORN)) {
        return 0;
    }
    *damage = 0;
    if (enemy->move_timer < 0) {
        enemy->move_timer++;
        if (!enemy->move_timer && enemy->is_boss) {
            enemy->defense = enemy->hp * 2 < enemy->max_hp ? 6 : 10;
        }
        return 1;
    }
    if (enemy->attack_phase) {
        if (enemy->move_timer > 1) {
            enemy->move_timer--;
            return 1;
        }
        if (enemy->boss_attack == ELDER_CHARGE) {
            int dx = sign(enemy->attack_target_x - enemy->x);
            int dy = sign(enemy->attack_target_y - enemy->y);
            int collision = 0;
            for (int n = 0; n < 7; n++) {
                int x = enemy->x + dx;
                int y = enemy->y + dy;
                if (!inside(x, y)) {
                    collision = 1;
                    break;
                }
                if (g->map.tiles[y][x] == TILE_ELDERWOOD_BRAMBLE) {
                    g->map.tiles[y][x] = TILE_ELDERWOOD_FLOOR;
                }
                if (!map_is_walkable(&g->map, x, y)) {
                    collision = 1;
                    break;
                }
                if (x == g->player.x && y == g->player.y) {
                    *damage = enemy->attack;
                    break;
                }
                if (occupied(g, enemy, x, y)) {
                    break;
                }
                enemy->x = x;
                enemy->y = y;
            }
            enemy->move_timer = collision ? -3 : -1;
            if (collision && enemy->is_boss) {
                enemy->defense = 0;
                push_message(g, "Elderthorn strikes a trunk! Its heart is exposed.");
            }
        } else if (enemy->boss_attack == ELDER_ROOTS) {
            if (elderwood_marks(enemy, g->player.x, g->player.y)) {
                *damage = enemy->attack;
            }
            combat_feedback_add(g, FEEDBACK_ROOT_BURST, FEEDBACK_NOW, enemy->attack_target_x, enemy->attack_target_y, enemy->is_boss ? 2 : 1);
            enemy->move_timer = -1;
        } else {
            if (g->enemy_count < AREA_ENEMY_LIMIT - 1) {
                for (int ox = -2; ox <= 2; ox += 4) {
                    int x = enemy->x + ox;
                    int y = enemy->y + 2;
                    if (inside(x, y) && map_is_walkable(&g->map, x, y) && !occupied(g, enemy, x, y)) {
                        add_enemy(g, ENEMY_BLIGHTED_WOLF, x, y);
                        g->enemies[g->enemy_count - 1].experience = 0;
                        g->enemies[g->enemy_count - 1].revived = 1;
                    }
                }
            }
            enemy->move_timer = -1;
        }
        enemy->attack_phase = 0;
        return 1;
    }
    int dx = g->player.x - enemy->x;
    int dy = g->player.y - enemy->y;
    int distance = abs(dx) + abs(dy);
    if (heartseed_is_guardian(g, enemy) && distance > 6) {
        return 1;
    }
    if (distance > 12) {
        return 1;
    }
    if (enemy->is_boss && enemy->hp * 2 < enemy->max_hp && !(enemy->boss_wave_used & 1)) {
        enemy->boss_attack = ELDER_SUMMON;
        enemy->boss_wave_used |= 1;
    } else if (enemy->type != ENEMY_TREEFOLK && (!dx || !dy) && distance > 1 && distance <= 7) {
        enemy->boss_attack = ELDER_CHARGE;
    } else if ((enemy->type == ENEMY_TREEFOLK || enemy->is_boss) && distance <= 6) {
        enemy->boss_attack = ELDER_ROOTS;
    } else if (distance <= 2 && abs(dx) <= 1 && abs(dy) <= 1) {
        *damage = enemy->attack;
        enemy->move_timer = -1;
        return 1;
    } else {
        step(g, enemy, g->player.x, g->player.y);
        return 1;
    }
    enemy->attack_phase = 1;
    enemy->attack_target_x = g->player.x;
    enemy->attack_target_y = g->player.y;
    enemy->move_timer = 2;
    push_message(g, enemy->boss_attack == ELDER_CHARGE ? "A charge is coming! Leave the marked lane." :
        enemy->boss_attack == ELDER_ROOTS ? "Roots stir beneath the marked ground!" : "Elderthorn calls the grove's wolves!");
    return 1;
}

void elderwood_defeat(GameState *g) {
    if (!g->elderwood_defeated) {
        g->elderwood_defeated = 1;
        g->gold += 180;
        g->score += 2000;
        push_message(g, "Elderthorn falls. The northern road to the Royal Catacombs opens!");
    }
}

int northern_approach(const GameState *g) {
    const TownApproach *route = approach_route(g);
    return route && (route->area == LOCATION_ELDERWOOD || route->area == LOCATION_CATACOMBS);
}

void northern_refresh(GameState *g) {
    if (!northern_approach(g)) {
        return;
    }
    const TownApproach *route = approach_route(g);
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g->map.tiles[y][x] = inside(x, y) ? TILE_TOWN_FLOOR : TILE_ELDERWOOD_WALL;
        }
    }
    for (int y = 1; y < ELDERWOOD_H - 1; y++) {
        g->map.tiles[y][NORTH_ROAD_X] = TILE_TOWN_PATH;
    }
    // Irregular woodland banks form two narrow passes, rather than a tree wall.
    const int passes[] = {NORTH_FOREST_BARRIER_Y, NORTH_CAT_Y - 2};
    for (int pass = 0; pass < 2; pass++) {
        for (int x = 1; x < ELDERWOOD_W - 1; x++) {
            int distance = x - NORTH_ROAD_X;
            int width = 1 + abs(distance) / 3;
            for (int dy = -width; dy <= width; dy++) {
                int y = passes[pass] + dy;
                if (x != NORTH_ROAD_X && y > 0 && y < ELDERWOOD_H - 1) {
                    g->map.tiles[y][x] = TILE_ROAD_BANK;
                }
            }
        }
    }
    for (int x = NORTH_ROAD_X; x <= NORTH_ELDER_X; x++) {
        g->map.tiles[NORTH_ELDER_Y + 1][x] = TILE_TOWN_PATH;
        g->map.tiles[NORTH_ELDER_Y - 6][x] = TILE_TOWN_PATH;
    }
    for (int y = NORTH_ELDER_Y - 5; y <= NORTH_ELDER_Y; y++) {
        for (int x = NORTH_ELDER_X - 3; x <= NORTH_ELDER_X + 3; x++) {
            g->map.tiles[y][x] = TILE_NORTH_ELDERWOOD;
        }
    }
    for (int x = NORTH_CAT_X; x <= NORTH_ROAD_X; x++) {
        g->map.tiles[NORTH_CAT_Y + 1][x] = TILE_TOWN_PATH;
        g->map.tiles[NORTH_CAT_Y - 6][x] = TILE_TOWN_PATH;
    }
    for (int y = NORTH_CAT_Y - 5; y <= NORTH_CAT_Y; y++) {
        for (int x = NORTH_CAT_X - 3; x <= NORTH_CAT_X + 3; x++) {
            g->map.tiles[y][x] = TILE_NORTH_CATACOMBS;
        }
    }
    g->map.tiles[ELDERWOOD_H - 1][NORTH_ROAD_X] = TILE_NORTH_OAKHAVEN;
    g->map.tiles[0][NORTH_ROAD_X] = TILE_NORTH_CASTLE;
    g->map.stairs_up_x = NORTH_ROAD_X;
    g->map.stairs_up_y = route->town == LOCATION_CASTLE ? 0 : ELDERWOOD_H - 1;
    g->map.stairs_down_x = route->area == LOCATION_CATACOMBS ? NORTH_CAT_X : NORTH_ELDER_X;
    g->map.stairs_down_y = route->area == LOCATION_CATACOMBS ? NORTH_CAT_Y : NORTH_ELDER_Y;
    approach_grow_scenery(&g->map, route->area);
    boss_gate_refresh(g);
}

void northern_return(GameState *g, Location town, int catacombs) {
    int route = -1;
    for (int i = 0; i < town_approach_count; i++) {
        if (town_approaches[i].town == town && town_approaches[i].area == (town == LOCATION_CASTLE ? LOCATION_CATACOMBS : LOCATION_ELDERWOOD)) {
            route = i;
            break;
        }
    }
    if (route < 0) {
        return;
    }
    approach_enter_region(g, route);
    g->player.x = catacombs ? NORTH_CAT_X : NORTH_ELDER_X;
    int entrance_y = catacombs ? NORTH_CAT_Y : NORTH_ELDER_Y;
    g->player.y = entrance_y + (town == LOCATION_CASTLE ? -6 : 1);
    g->player.last_dx = 0;
    g->player.last_dy = 1;
    map_mark_explored(&g->map, g->player.x, g->player.y);
}

void elderwood_leave(GameState *g) {
    Location town = g->elderwood_entry_town;
    elderwood_store(g);
    discard_loot(g);
    northern_return(g, town, 0);
}

int northern_travel(GameState *g, int x, int y) {
    if (!northern_approach(g)) {
        return 0;
    }
    const TownApproach *route = approach_route(g);
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_NORTH_ELDERWOOD) {
        elderwood_enter(g);
    } else if (tile == TILE_NORTH_CATACOMBS) {
        g->catacombs_entry_town = route->town;
        game_enter_catacombs(g);
    } else if (tile == TILE_NORTH_OAKHAVEN) {
        if (route->town == LOCATION_CASTLE && (!boss_gate_crossing(g, LOCATION_CATACOMBS) || !boss_gate_crossing(g, LOCATION_ELDERWOOD))) {
            return 1;
        }
        game_leave_approach_to_town(g, LOCATION_TOWN);
    } else if (tile == TILE_NORTH_CASTLE) {
        if (route->town == LOCATION_TOWN && (!boss_gate_crossing(g, LOCATION_ELDERWOOD) || !boss_gate_crossing(g, LOCATION_CATACOMBS))) {
            return 1;
        }
        northroad_enter(g, 0, 0);
    } else if (x == NORTH_ROAD_X && y == 0) {
        push_message(g, "Clear Elderwood and the Royal Catacombs to reach the Castle Grounds.");
    } else {
        return 0;
    }
    return 1;
}

void elderwood_hide_portal(GameState *g) {
    if (g->portal_location != LOCATION_ELDERWOOD || g->portal_level < 1 || g->portal_level > ELDERWOOD_DEPTH || !inside(g->portal_x, g->portal_y)) {
        return;
    }
    ElderwoodCache *cache = &g->elderwood_cache[g->portal_level - 1];
    if (cache->tiles[g->portal_y][g->portal_x] == TILE_PORTAL) {
        cache->tiles[g->portal_y][g->portal_x] = g->portal_origin_tile;
    }
}

void elderwood_portal(GameState *g) {
    if (g->portal_level < 1 || g->portal_level > ELDERWOOD_DEPTH || !g->elderwood_cache[g->portal_level - 1].valid) {
        return;
    }
    g->location = LOCATION_ELDERWOOD;
    g->portal_active = 0;
    g->approach_route = -1;
    g->elderwood_entry_town = g->northern_portal_town;
    g->level = g->portal_level;
    load_floor(g);
    int x = g->portal_x;
    int y = g->portal_y;
    if (inside(x, y) && g->map.tiles[y][x] == TILE_PORTAL) {
        g->map.tiles[y][x] = g->portal_origin_tile;
    }
    if (!inside(x, y) || !map_is_walkable(&g->map, x, y) || occupied(g, NULL, x, y)) {
        int nearest = MAP_W + MAP_H;
        for (int py = 1; py < ELDERWOOD_H - 1; py++) {
            for (int px = 1; px < ELDERWOOD_W - 1; px++) {
                TileType tile = g->map.tiles[py][px];
                int distance = abs(px - g->portal_x) + abs(py - g->portal_y);
                if (distance < nearest && tile != TILE_ELDERWOOD_ENTRANCE && tile != TILE_ELDERWOOD_EXIT &&
                    map_is_walkable(&g->map, px, py) && !occupied(g, NULL, px, py)) {
                    nearest = distance;
                    x = px;
                    y = py;
                }
            }
        }
    }
    g->player.x = x;
    g->player.y = y;
    for (int i = 0; i < g->enemy_count; i++) {
        if (elderwood_marks(&g->enemies[i], x, y)) {
            g->enemies[i].attack_phase = 0;
            g->enemies[i].move_timer = -1;
        }
    }
    map_mark_explored(&g->map, x, y);
    push_message(g, "Returned through the Elderwood portal.");
}
