#include "containers.h"
#include "crownroad_north.h"
#include "combat_feedback.h"
#include "elderwood.h"
#include "approach.h"
#include "town_road.h"
#include <stdlib.h>
#include <string.h>

static const int camps[NORTH_SQUADS][2] = {{32,82},{23,69},{41,56},{25,43},{41,30},{31,17}};
static const int steps[4][2] = {{0,-1},{1,0},{0,1},{-1,0}};

static int inside(int x, int y) {
    return x > 0 && y > 0 && x < CROWN_NORTH_W - 1 && y < CROWN_NORTH_H - 1;
}

static int occupied(const GameState *g, const Enemy *self, int x, int y) {
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *other = &g->enemies[i];
        if (other != self && other->active && other->x == x && other->y == y) {
            return 1;
        }
    }
    return 0;
}

static void clearing(Map *map, int cx, int cy, int rx, int ry) {
    for (int y = cy - ry; y <= cy + ry; y++) {
        for (int x = cx - rx; x <= cx + rx; x++) {
            if (inside(x, y) && map->tiles[y][x] == TILE_FOREST_WALL && (x - cx) * (x - cx) * ry * ry + (y - cy) * (y - cy) * rx * rx <= rx * rx * ry * ry) {
                map->tiles[y][x] = TILE_FOREST_FLOOR;
            }
        }
    }
}

static void trail(Map *map, int x, int y, int tx, int ty, int paved) {
    while (x != tx || y != ty) {
        clearing(map, x, y, 2, 2);
        if (paved) {
            map->tiles[y][x] = TILE_TOWN_PATH;
        }
        if (y != ty) {
            y += (ty > y) - (ty < y);
        } else {
            x += (tx > x) - (tx < x);
        }
    }
    clearing(map, tx, ty, 2, 2);
    if (paved) {
        map->tiles[ty][tx] = TILE_TOWN_PATH;
    }
}

void northroad_generate(Map *map) {
    memset(map, 0, sizeof(*map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            map->tiles[y][x] = TILE_FOREST_WALL;
        }
    }
    int px = CROWN_NORTH_X;
    int py = CROWN_NORTH_H - 2;
    for (int i = 0; i < NORTH_SQUADS; i++) {
        int x = camps[i][0];
        int y = camps[i][1];
        clearing(map, x, y, 10, 6);
        trail(map, px, py, x, y, 1);
        // Parallel woodland trails reconnect beyond each occupied checkpoint.
        int side = i % 2 ? x + 14 : x - 14;
        trail(map, x, y + 5, side, y + 5, 0);
        trail(map, side, y + 5, side, y - 6, 0);
        trail(map, side, y - 6, x, y - 6, 0);
        clearing(map, side, y, 4, 3);
        map->tiles[y][side] = TILE_CROWN_SUPPLIES;
        for (int offset = -7; offset <= 7; offset++) {
            if (abs(offset) >= 3) {
                map->tiles[y + 4][x + offset] = TILE_CROWN_BARRICADE;
            }
        }
        px = x;
        py = y;
    }
    trail(map, px, py, CROWN_NORTH_X, 7, 1);
    trail(map, CROWN_NORTH_X, 7, CROWN_NORTH_X, 1, 1);
    map->tiles[0][CROWN_NORTH_X] = TILE_CROWN_EXIT;
    map->tiles[CROWN_NORTH_H - 1][CROWN_NORTH_X] = TILE_CROWN_EXIT;
    map->stairs_up_x = CROWN_NORTH_X;
    map->stairs_up_y = CROWN_NORTH_H - 1;
    map->stairs_down_x = CROWN_NORTH_X;
    map->stairs_down_y = 0;
}

static int spawn_open(const GameState *g, int x, int y) {
    return inside(x, y) && map_is_walkable(&g->map, x, y) && g->map.tiles[y][x] != TILE_ITEM &&
        !(x == g->player.x && y == g->player.y) && !occupied(g, NULL, x, y) &&
        !(g->portal_active && g->portal_location == LOCATION_CROWNROAD_NORTH && x == g->portal_x && y == g->portal_y);
}

static int nearest_open(const GameState *g, int *x, int *y, int limit) {
    for (int radius = 0; radius <= limit; radius++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (abs(dx) + abs(dy) == radius && spawn_open(g, *x + dx, *y + dy)) {
                    *x += dx;
                    *y += dy;
                    return 1;
                }
            }
        }
    }
    return 0;
}

static void spawn_patrols(GameState *g) {
    static const EnemyType types[NORTH_SQUAD_SIZE] = {
        ENEMY_HOBGOBLIN_GUARD, ENEMY_HOBGOBLIN_GUARD, ENEMY_HOBGOBLIN_GUARD, ENEMY_HOBGOBLIN_GUARD,
        ENEMY_GOBLIN_SCOUT, ENEMY_GOBLIN_SCOUT, ENEMY_GOBLIN_SCOUT,
        ENEMY_GOBLIN_ARCHER, ENEMY_GOBLIN_ARCHER, ENEMY_GOBLIN_BOMBER, ENEMY_GOBLIN_SHAMAN,
        ENEMY_BLIGHTED_WOLF, ENEMY_BLIGHTED_WOLF, ENEMY_BLIGHTED_WOLF, ENEMY_BLIGHTED_WOLF
    };
    static const int offsets[NORTH_SQUAD_SIZE][2] = {
        {-3,2},{-1,3},{1,3},{3,2},{-5,0},{5,0},{0,0},{-4,-2},{4,-2},{0,-4},{0,-2},
        {-8,1},{-7,-2},{7,-2},{8,1}
    };
    g->enemy_count = 0;
    for (int squad = 0; squad < NORTH_SQUADS; squad++) {
        for (int i = 0; i < NORTH_SQUAD_SIZE; i++) {
            int x = camps[squad][0] + offsets[i][0];
            int y = camps[squad][1] + offsets[i][1];
            if (!nearest_open(g, &x, &y, 6)) {
                continue;
            }
            Enemy *enemy = &g->enemies[g->enemy_count];
            game_spawn_enemy(g, enemy, types[i], x, y);
            enemy->max_hp += 40;
            enemy->hp = enemy->max_hp;
            enemy->attack += 8;
            enemy->defense += 3;
            enemy->experience *= 2;
            // Existing serialized boss metadata stores squad identity and alarm.
            enemy->boss_aux_x = squad;
            enemy->boss_aux_y = i;
            enemy->facing_dy = 1;
            g->enemy_count++;
        }
    }
    g->level_cleared = 0;
}

void northroad_capture(const GameState *g, NorthRoadCache *state) {
    if (g->location != LOCATION_CROWNROAD_NORTH) {
        return;
    }
    state->valid = 1;
    state->map = g->map;
    state->enemy_count = g->enemy_count;
    state->loot_count = g->floor_item_count;
    state->level_cleared = g->level_cleared;
    memcpy(state->enemies, g->enemies, g->enemy_count * sizeof(Enemy));
    memcpy(state->loot, g->floor_items, g->floor_item_count * sizeof(FloorItem));
}

void northroad_store(GameState *g) {
    northroad_capture(g, &g->northroad);
}

void northroad_enter(GameState *g, int from_castle, int portal) {
    const TownApproach *route = approach_route(g);
    int castle_access = from_castle || (route && route->town == LOCATION_CASTLE);
    if (portal && (!g->northroad.valid || !inside(g->portal_x, g->portal_y) ||
        !map_is_walkable(&g->northroad.map, g->portal_x, g->portal_y))) {
        return;
    }
    if (!g->northroad.valid) {
        northroad_generate(&g->northroad.map);
        g->northroad.valid = 1;
    }
    g->location = LOCATION_CROWNROAD_NORTH;
    g->level = 1;
    g->approach_route = -1;
    g->map = g->northroad.map;
    g->floor_item_count = g->northroad.loot_count;
    memcpy(g->floor_items, g->northroad.loot, g->floor_item_count * sizeof(FloorItem));
    g->player.x = portal ? g->portal_x : CROWN_NORTH_X;
    g->player.y = portal ? g->portal_y : from_castle ? 1 : CROWN_NORTH_H - 2;
    // Crown Road North is exempt from expedition respawns: patrols spawn only on the first crossing.
    g->enemy_count = g->northroad.enemy_count;
    g->level_cleared = g->northroad.level_cleared;
    memcpy(g->enemies, g->northroad.enemies, g->enemy_count * sizeof(Enemy));
    if (portal) {
        g->portal_active = 0;
    } else {
        g->northroad.from_castle = castle_access;
        if (!g->enemy_count) {
            spawn_patrols(g);
        }
        for (int i = 0; i < g->enemy_count; i++) {
            Enemy *enemy = &g->enemies[i];
            // Survivors forget shots marked during the last crossing and clear the entry tile.
            enemy->attack_phase = 0;
            if (enemy->active && enemy->x == g->player.x && enemy->y == g->player.y) {
                nearest_open(g, &enemy->x, &enemy->y, CROWN_NORTH_W + CROWN_NORTH_H);
            }
        }
        g->player.last_dx = 0;
        g->player.last_dy = from_castle ? 1 : -1;
    }
    g->dialogue_active = 0;
    map_mark_explored(&g->map, g->player.x, g->player.y);
    push_message(g, "Crown Road North: goblin shield lines and wolf packs occupy the castle road.");
    push_message(g, "Side trails bypass firing lanes. Flank shields; leave marked shots and bombs.");
}

void northroad_leave(GameState *g, int castle) {
    northroad_store(g);
    if (castle) {
        game_enter_town_gate(g, LOCATION_CASTLE, CROWNROAD_X, TOWN_H - 1);
    } else {
        northern_return(g, g->northroad.from_castle ? LOCATION_CASTLE : LOCATION_TOWN, 1);
        g->player.x = NORTH_ROAD_X;
        g->player.y = 1;
        g->player.last_dx = 0;
        g->player.last_dy = 1;
    }
}

int northroad_travel(GameState *g, int x, int y) {
    if (g->location != LOCATION_CROWNROAD_NORTH || g->map.tiles[y][x] != TILE_CROWN_EXIT) {
        return 0;
    }
    northroad_leave(g, y == 0);
    return 1;
}

static int supplies(const GameState *g, int *x, int *y) {
    if (g->location != LOCATION_CROWNROAD_NORTH) {
        return 0;
    }
    for (int i = 0; i < 4; i++) {
        int tx = g->player.x + steps[i][0];
        int ty = g->player.y + steps[i][1];
        if (inside(tx, ty) && g->map.tiles[ty][tx] == TILE_CROWN_SUPPLIES) {
            *x = tx;
            *y = ty;
            return 1;
        }
    }
    return 0;
}

int northroad_has_interaction(const GameState *g) {
    if (g->location == LOCATION_CROWNROAD_NORTH && containers_hint(g)) {
        return 1;
    }
    int x;
    int y;
    return supplies(g, &x, &y);
}

int northroad_interact(GameState *g) {
    if (g->location == LOCATION_CROWNROAD_NORTH && containers_hint(g)) {
        return containers_interact(g);
    }
    int x;
    int y;
    if (!supplies(g, &x, &y)) {
        return 0;
    }
    return containers_interact(g);
}

static int line_clear(const Map *map, int x, int y, int tx, int ty) {
    int dx = abs(tx - x);
    int dy = abs(ty - y);
    int sx = (tx > x) - (tx < x);
    int sy = (ty > y) - (ty < y);
    int error = dx - dy;
    while (x != tx || y != ty) {
        int twice = 2 * error;
        if (twice > -dy) {
            error -= dy;
            x += sx;
        }
        if (twice < dx) {
            error += dx;
            y += sy;
        }
        if (!map_is_walkable(map, x, y)) {
            return 0;
        }
    }
    return 1;
}

static int step_toward(GameState *g, Enemy *enemy, int tx, int ty) {
    if (!inside(tx, ty) || !map_is_walkable(&g->map, tx, ty)) {
        return 0;
    }
    unsigned char blocked[CROWN_NORTH_H][CROWN_NORTH_W] = {{0}};
    short distance[CROWN_NORTH_H][CROWN_NORTH_W];
    int queue[CROWN_NORTH_W * CROWN_NORTH_H];
    memset(distance, -1, sizeof(distance));
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *other = &g->enemies[i];
        if (other != enemy && other->active && inside(other->x, other->y)) {
            blocked[other->y][other->x] = 1;
        }
    }
    if (blocked[ty][tx]) {
        return 0;
    }
    int head = 0;
    int tail = 1;
    distance[ty][tx] = 0;
    queue[0] = ty * CROWN_NORTH_W + tx;
    while (head < tail) {
        int cell = queue[head++];
        int x = cell % CROWN_NORTH_W;
        int y = cell / CROWN_NORTH_W;
        for (int i = 0; i < 4; i++) {
            int nx = x + steps[i][0];
            int ny = y + steps[i][1];
            if (inside(nx, ny) && distance[ny][nx] < 0 && !blocked[ny][nx] && map_is_walkable(&g->map, nx, ny)) {
                distance[ny][nx] = distance[y][x] + 1;
                queue[tail++] = ny * CROWN_NORTH_W + nx;
            }
        }
    }
    int best = distance[enemy->y][enemy->x];
    if (best < 0) {
        return 0;
    }
    for (int i = 0; i < 4; i++) {
        int x = enemy->x + steps[i][0];
        int y = enemy->y + steps[i][1];
        if (inside(x, y) && distance[y][x] >= 0 && distance[y][x] < best && !blocked[y][x] &&
            !(x == g->player.x && y == g->player.y)) {
            enemy->x = x;
            enemy->y = y;
            return 1;
        }
    }
    return 0;
}

static void ranged_position(GameState *g, Enemy *enemy) {
    int best = MAP_W + MAP_H;
    int tx = enemy->x;
    int ty = enemy->y;
    for (int dir = 0; dir < 4; dir++) {
        for (int range = 4; range <= 7; range++) {
            int x = g->player.x + steps[dir][0] * range;
            int y = g->player.y + steps[dir][1] * range;
            int distance = abs(enemy->x - x) + abs(enemy->y - y);
            if (inside(x, y) && map_is_walkable(&g->map, x, y) && !occupied(g, enemy, x, y) &&
                distance < best && line_clear(&g->map, x, y, g->player.x, g->player.y)) {
                best = distance;
                tx = x;
                ty = y;
            }
        }
    }
    step_toward(g, enemy, tx, ty);
}

int northroad_attack_marks(const GameState *g, const Enemy *enemy, int x, int y) {
    if (g->location != LOCATION_CROWNROAD_NORTH || !enemy->active || !enemy->attack_phase ||
        !inside(x, y) || !map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    int distance = abs(x - enemy->attack_target_x) + abs(y - enemy->attack_target_y);
    return enemy->boss_attack == NORTH_ARROW ? distance == 0 && line_clear(&g->map, enemy->x, enemy->y, x, y) :
        enemy->boss_attack == NORTH_BOMB && distance <= 1 && line_clear(&g->map, enemy->attack_target_x, enemy->attack_target_y, x, y);
}

int northroad_shielded(const GameState *g, const Enemy *enemy) {
    if (g->location != LOCATION_CROWNROAD_NORTH || enemy->type != ENEMY_HOBGOBLIN_GUARD ||
        !enemy->active || enemy->frozen_turns > 0) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *ally = &g->enemies[i];
        if (ally != enemy && ally->active && ally->boss_aux_x == enemy->boss_aux_x &&
            abs(ally->x - enemy->x) + abs(ally->y - enemy->y) <= 5 &&
            (ally->type == ENEMY_GOBLIN_ARCHER || ally->type == ENEMY_GOBLIN_SHAMAN)) {
            return 1;
        }
    }
    return 0;
}

int northroad_weapon_damage(const GameState *g, const Enemy *enemy, int damage) {
    int dx = g->player.x - enemy->x;
    int dy = g->player.y - enemy->y;
    int front = dx * enemy->facing_dx + dy * enemy->facing_dy;
    int side = abs(dx * enemy->facing_dy - dy * enemy->facing_dx);
    if (northroad_shielded(g, enemy) && front > 0 && side <= front) {
        return damage > 1 ? (damage + 1) / 2 : damage;
    }
    return damage;
}

static void face_player(const GameState *g, Enemy *enemy) {
    int dx = g->player.x - enemy->x;
    int dy = g->player.y - enemy->y;
    enemy->facing_dx = abs(dx) > abs(dy) ? (dx > 0) - (dx < 0) : 0;
    enemy->facing_dy = enemy->facing_dx ? 0 : (dy > 0) - (dy < 0);
}

int northroad_tick(GameState *g, Enemy *enemy, int *damage) {
    if (g->location != LOCATION_CROWNROAD_NORTH) {
        return 0;
    }
    *damage = 0;
    int dx = g->player.x - enemy->x;
    int dy = g->player.y - enemy->y;
    int distance = abs(dx) + abs(dy);
    if (!enemy->boss_wave_used) {
        if (distance > 11 || !line_clear(&g->map, enemy->x, enemy->y, g->player.x, g->player.y)) {
            return 1;
        }
        for (int i = 0; i < g->enemy_count; i++) {
            if (g->enemies[i].boss_aux_x == enemy->boss_aux_x) {
                g->enemies[i].boss_wave_used = 1;
                face_player(g, &g->enemies[i]);
            }
        }
        push_message(g, "A goblin patrol signals its shield line. Wolves split toward the flanks!");
    }
    if (enemy->attack_phase) {
        if (--enemy->move_timer <= 0) {
            if (northroad_attack_marks(g, enemy, g->player.x, g->player.y)) {
                *damage = enemy->attack;
            }
            enemy->attack_phase = 0;
            enemy->move_timer = -1;
        }
        return 1;
    }
    if (enemy->move_timer < 0) {
        enemy->move_timer++;
        return 1;
    }
    enemy->move_timer++;
    if (!northroad_shielded(g, enemy)) {
        face_player(g, enemy);
    }
    if (enemy->type == ENEMY_GOBLIN_SHAMAN && enemy->move_timer % 4 == 0) {
        for (int i = 0; i < g->enemy_count; i++) {
            Enemy *ally = &g->enemies[i];
            if (ally->active && ally->boss_aux_x == enemy->boss_aux_x && ally->hp < ally->max_hp &&
                abs(ally->x - enemy->x) + abs(ally->y - enemy->y) <= 5) {
                int healed = ally->max_hp - ally->hp;
                if (healed > 10) {
                    healed = 10;
                }
                ally->hp += healed;
                combat_feedback_add(g, FEEDBACK_HEAL, FEEDBACK_NOW, ally->x, ally->y, healed);
                push_message(g, "A Goblin Shaman rallies a wounded squadmate.");
                return 1;
            }
        }
    }
    if (distance > 2 && distance <= 8 && line_clear(&g->map, enemy->x, enemy->y, g->player.x, g->player.y) &&
        (enemy->type == ENEMY_GOBLIN_BOMBER || (enemy->type == ENEMY_GOBLIN_ARCHER && (!dx || !dy)))) {
        enemy->boss_attack = enemy->type == ENEMY_GOBLIN_ARCHER ? NORTH_ARROW : NORTH_BOMB;
        enemy->attack_phase = 1;
        enemy->move_timer = enemy->boss_attack == NORTH_BOMB ? 2 : 1;
        enemy->attack_target_x = g->player.x;
        enemy->attack_target_y = g->player.y;
        push_message(g, enemy->boss_attack == NORTH_BOMB ? "A goblin bomb is marked: two actions to leave the blast!" : "A Goblin Archer aims at the marked square!");
        return 1;
    }
    if (enemy->type == ENEMY_BLIGHTED_WOLF && abs(dx) <= 1 && abs(dy) <= 1) {
        int flank = enemy->boss_aux_y % 4;
        int x = g->player.x + steps[flank][0];
        int y = g->player.y + steps[flank][1];
        if ((enemy->x != x || enemy->y != y) && step_toward(g, enemy, x, y)) {
            return 1;
        }
    }
    if (abs(dx) <= 1 && abs(dy) <= 1 && (dx || dy)) {
        *damage = enemy->attack;
        return 1;
    }
    if (enemy->type == ENEMY_GOBLIN_ARCHER || enemy->type == ENEMY_GOBLIN_BOMBER || enemy->type == ENEMY_GOBLIN_SHAMAN) {
        ranged_position(g, enemy);
        return 1;
    }
    if (enemy->type == ENEMY_BLIGHTED_WOLF || enemy->type == ENEMY_GOBLIN_SCOUT) {
        int flank = enemy->boss_aux_y % 4;
        int x = g->player.x + steps[flank][0];
        int y = g->player.y + steps[flank][1];
        int moves = enemy->type == ENEMY_BLIGHTED_WOLF && distance > 3 ? 2 : 1;
        for (int i = 0; i < moves; i++) {
            if (!step_toward(g, enemy, x, y)) {
                step_toward(g, enemy, g->player.x, g->player.y);
            }
        }
    } else {
        // Shield bearers retain their screen until the player closes or outflanks it.
        if (!northroad_shielded(g, enemy) || distance <= 5) {
            step_toward(g, enemy, g->player.x, g->player.y);
        }
    }
    return 1;
}

static int valid_type(EnemyType type) {
    return type == ENEMY_GOBLIN_SCOUT || type == ENEMY_GOBLIN_ARCHER || type == ENEMY_GOBLIN_BOMBER ||
        type == ENEMY_GOBLIN_SHAMAN || type == ENEMY_HOBGOBLIN_GUARD || type == ENEMY_BLIGHTED_WOLF;
}

static int valid_actors(const Map *map, const Enemy *enemies, int count, const FloorItem *loot, int loot_count) {
    if (count < 0 || count > NORTH_ENEMIES || loot_count < 0 || loot_count > MAX_FLOOR_ITEMS ||
        map->stairs_up_x != CROWN_NORTH_X || map->stairs_up_y != CROWN_NORTH_H - 1 ||
        map->stairs_down_x != CROWN_NORTH_X || map->stairs_down_y != 0 ||
        map->tiles[0][CROWN_NORTH_X] != TILE_CROWN_EXIT || map->tiles[CROWN_NORTH_H - 1][CROWN_NORTH_X] != TILE_CROWN_EXIT) {
        return 0;
    }
    for (int i = 0; i < count; i++) {
        const Enemy *enemy = &enemies[i];
        if (!valid_type(enemy->type) || enemy->boss_aux_x < 0 || enemy->boss_aux_x >= NORTH_SQUADS ||
            enemy->boss_aux_y < 0 || enemy->boss_aux_y >= NORTH_SQUAD_SIZE ||
            (enemy->attack_phase && (enemy->attack_phase != 1 ||
            (enemy->boss_attack != NORTH_ARROW && enemy->boss_attack != NORTH_BOMB) ||
            enemy->move_timer < 1 || enemy->move_timer > 2 || !inside(enemy->attack_target_x, enemy->attack_target_y))) ||
            (enemy->active && (!inside(enemy->x, enemy->y) || !map_is_walkable(map, enemy->x, enemy->y)))) {
            return 0;
        }
        for (int j = 0; j < i; j++) {
            if (enemy->active && enemies[j].active && enemy->x == enemies[j].x && enemy->y == enemies[j].y) {
                return 0;
            }
        }
    }
    for (int i = 0; i < loot_count; i++) {
        if (loot[i].active && (!inside(loot[i].x, loot[i].y) || !map_is_walkable(map, loot[i].x, loot[i].y))) {
            return 0;
        }
    }
    return 1;
}

int northroad_valid(const GameState *g) {
    const NorthRoadCache *cache = &g->northroad;
    if ((cache->valid != 0 && cache->valid != 1) || (cache->level_cleared != 0 && cache->level_cleared != 1) ||
        (cache->from_castle != 0 && cache->from_castle != 1)) {
        return 0;
    }
    if (cache->valid && !valid_actors(&cache->map, cache->enemies, cache->enemy_count, cache->loot, cache->loot_count)) {
        return 0;
    }
    if (g->location == LOCATION_CROWNROAD_NORTH) {
        if (!cache->valid || g->level != 1 || !inside(g->player.x, g->player.y) ||
            !map_is_walkable(&g->map, g->player.x, g->player.y) ||
            !valid_actors(&g->map, g->enemies, g->enemy_count, g->floor_items, g->floor_item_count)) {
            return 0;
        }
    }
    return !g->portal_active || g->portal_location != LOCATION_CROWNROAD_NORTH ||
        (cache->valid && g->portal_level == 1 && inside(g->portal_x, g->portal_y) && map_is_walkable(&cache->map, g->portal_x, g->portal_y));
}
