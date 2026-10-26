#include "stormwatch.h"
#include <stdlib.h>

static int sign(int value) {
    return (value > 0) - (value < 0);
}

static int ground(const GameState *g, int x, int y) {
    if (x < 0 || x >= STORM_W || y < 0 || y >= STORM_H || !map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    TileType tile = g->map.tiles[y][x];
    return tile != TILE_STORM_ENTRANCE && tile != TILE_STORM_EXIT && tile != TILE_PORTAL;
}

static int occupied(const GameState *g, const Enemy *self, int x, int y) {
    if (x == g->player.x && y == g->player.y) {
        return 1;
    }
    for (int n = 0; n < g->enemy_count; n++) {
        const Enemy *enemy = &g->enemies[n];
        if (enemy != self && enemy->active && enemy->x == x && enemy->y == y) {
            return 1;
        }
    }
    return 0;
}

static int line(const GameState *g, int x, int y, int tx, int ty) {
    int steps = abs(tx - x) + abs(ty - y);
    for (int n = 0; n <= steps; n++) {
        if (!ground(g, x, y)) {
            return 0;
        }
        if (x == tx && y == ty) {
            return 1;
        }
        // Cardinal rays; crosswind fan checks its forward segment first.
        if (x != tx) {
            x += sign(tx - x);
        } else {
            y += sign(ty - y);
        }
    }
    return 0;
}

int stormwatch_attack_marks(const GameState *g, const Enemy *enemy, int x, int y) {
    if (g->location != LOCATION_STORMWATCH || !enemy->active || !enemy->attack_phase || enemy->move_timer <= 0 || !ground(g, x, y)) {
        return 0;
    }
    if (enemy->boss_attack == STORM_LIGHTNING) {
        return (x == enemy->attack_target_x && y == enemy->attack_target_y) ||
            (enemy->is_boss && x == enemy->boss_aux_x && y == enemy->boss_aux_y);
    }
    if (enemy->boss_attack == STORM_DIVE) {
        int length = abs(enemy->boss_aux_x - enemy->attack_target_x) + abs(enemy->boss_aux_y - enemy->attack_target_y);
        for (int n = 1; n <= length; n++) {
            int nx = enemy->attack_target_x + enemy->facing_dx * n;
            int ny = enemy->attack_target_y + enemy->facing_dy * n;
            if (!ground(g, nx, ny)) {
                break;
            }
            if (nx == x && ny == y) {
                return 1;
            }
        }
        return 0;
    }
    int dx = x - enemy->attack_target_x;
    int dy = y - enemy->attack_target_y;
    int forward = dx * enemy->facing_dx + dy * enemy->facing_dy;
    int side = dx * -enemy->facing_dy + dy * enemy->facing_dx;
    int reach = enemy->boss_attack == STORM_BURST ? 3 : 1;
    return forward >= 1 && forward <= reach && abs(side) <= 1 &&
        line(g, enemy->attack_target_x, enemy->attack_target_y,
            enemy->attack_target_x + forward * enemy->facing_dx, enemy->attack_target_y + forward * enemy->facing_dy) &&
        line(g, enemy->attack_target_x + forward * enemy->facing_dx, enemy->attack_target_y + forward * enemy->facing_dy, x, y);
}

static void step(GameState *g, Enemy *enemy) {
    // Follow the actual walkable fort graph, including the permanent detours.
    short distance[STORM_H][STORM_W];
    int queue[STORM_W * STORM_H];
    for (int y = 0; y < STORM_H; y++) {
        for (int x = 0; x < STORM_W; x++) {
            distance[y][x] = -1;
        }
    }
    int front = 0;
    int count = 1;
    queue[0] = g->player.y * STORM_W + g->player.x;
    distance[g->player.y][g->player.x] = 0;
    const int dx[] = {1, 0, -1, 0};
    const int dy[] = {0, 1, 0, -1};
    while (front < count) {
        int cell = queue[front++];
        int x = cell % STORM_W;
        int y = cell / STORM_W;
        for (int n = 0; n < 4; n++) {
            int nx = x + dx[n];
            int ny = y + dy[n];
            if (ground(g, nx, ny) && distance[ny][nx] < 0) {
                distance[ny][nx] = distance[y][x] + 1;
                queue[count++] = ny * STORM_W + nx;
            }
        }
    }
    int best = distance[enemy->y][enemy->x];
    if (best < 0 || best > 22) {
        return;
    }
    int tx = enemy->x;
    int ty = enemy->y;
    for (int n = 0; n < 4; n++) {
        int x = enemy->x + dx[n];
        int y = enemy->y + dy[n];
        if (ground(g, x, y) && !occupied(g, enemy, x, y) && distance[y][x] >= 0 && distance[y][x] < best) {
            best = distance[y][x];
            tx = x;
            ty = y;
        }
    }
    if (tx != enemy->x || ty != enemy->y) {
        enemy->facing_dx = tx - enemy->x;
        enemy->facing_dy = ty - enemy->y;
        enemy->x = tx;
        enemy->y = ty;
    }
}

static void summon(GameState *g, const Enemy *boss) {
    int made = 0;
    for (int radius = 2; radius <= 5 && made < 2; radius++) {
        for (int dx = -radius; dx <= radius && made < 2; dx++) {
            int x = boss->x + dx;
            int y = boss->y + radius;
            if (g->enemy_count < AREA_ENEMY_LIMIT && ground(g, x, y) && !occupied(g, boss, x, y)) {
                Enemy *add = &g->enemies[g->enemy_count++];
                *add = (Enemy){.active = 1, .type = ENEMY_HARPY_RAIDER, .x = x, .y = y, .hp = 40, .max_hp = 40,
                    .attack = 14, .defense = 2, .revived = 1, .move_timer = -2, .facing_dy = 1};
                snprintf(add->name, sizeof(add->name), "Harpy Raider");
                made++;
            }
        }
    }
}

int stormwatch_tick(GameState *g, Enemy *enemy, int *damage) {
    if (g->location != LOCATION_STORMWATCH || enemy->type < ENEMY_HARPY_RAIDER || enemy->type > ENEMY_TEMPEST_MATRIARCH) {
        return 0;
    }
    *damage = 0;
    if (enemy->move_timer < 0) {
        enemy->move_timer++;
        if (!enemy->move_timer) {
            enemy->defense = enemy->type == ENEMY_CLIFF_GARGOYLE ? 7 : enemy->is_boss ? 7 : enemy->defense;
        }
        return 1;
    }
    if (enemy->attack_phase) {
        if (enemy->move_timer > 1) {
            enemy->move_timer--;
            return 1;
        }
        if (stormwatch_attack_marks(g, enemy, g->player.x, g->player.y)) {
            *damage = enemy->attack;
        }
        if (enemy->boss_attack == STORM_DIVE) {
            int length = abs(enemy->boss_aux_x - enemy->attack_target_x) + abs(enemy->boss_aux_y - enemy->attack_target_y);
            for (int n = 1; n <= length; n++) {
                int x = enemy->attack_target_x + n * enemy->facing_dx;
                int y = enemy->attack_target_y + n * enemy->facing_dy;
                if (!ground(g, x, y) || occupied(g, enemy, x, y)) {
                    break;
                }
                enemy->x = x;
                enemy->y = y;
            }
        } else if (enemy->boss_attack == STORM_SWEEP && *damage > 0 && !g->stormwatch[g->level - 1].braced) {
            int x = g->player.x + enemy->facing_dx;
            int y = g->player.y + enemy->facing_dy;
            if (ground(g, x, y) && !occupied(g, NULL, x, y)) {
                g->player.x = x;
                g->player.y = y;
                map_mark_explored(&g->map, x, y);
            }
        }
        enemy->attack_phase = 0;
        enemy->move_timer = enemy->is_boss ? -2 : -1;
        if (enemy->type == ENEMY_CLIFF_GARGOYLE || enemy->is_boss) {
            enemy->defense = 1;
        }
        return 1;
    }
    int dx = g->player.x - enemy->x;
    int dy = g->player.y - enemy->y;
    int distance = abs(dx) + abs(dy);
    if (distance > 18) {
        return 1;
    }
    if (enemy->is_boss && enemy->hp * 2 < enemy->max_hp && !enemy->boss_wave_used) {
        enemy->boss_wave_used = 1;
        summon(g, enemy);
        enemy->move_timer = -2;
        push_message(g, "The Matriarch calls her last two raiders!");
        return 1;
    }
    int previous = enemy->boss_attack;
    int kind = enemy->type == ENEMY_HARPY_STORMCALLER ? STORM_LIGHTNING :
        enemy->type == ENEMY_STORM_ELEMENTAL ? STORM_BURST : STORM_DIVE;
    if (enemy->is_boss) {
        kind = previous == STORM_DIVE ? STORM_SWEEP : STORM_DIVE;
        if (enemy->hp * 2 < enemy->max_hp && previous == STORM_SWEEP) {
            kind = STORM_LIGHTNING;
        }
    }
    int range = enemy->is_boss ? 6 : enemy->type == ENEMY_CLIFF_GARGOYLE ? 3 : 2;
    if ((kind == STORM_DIVE && ((dx && dy) || distance > range)) ||
        (kind == STORM_SWEEP && distance > 2) || (kind == STORM_BURST && distance > 4) ||
        (kind == STORM_LIGHTNING && (distance > 7 || !line(g, enemy->x, enemy->y, g->player.x, g->player.y)))) {
        step(g, enemy);
        return 1;
    }
    enemy->boss_attack = kind;
    enemy->facing_dx = abs(dx) >= abs(dy) ? sign(dx) : 0;
    enemy->facing_dy = enemy->facing_dx ? 0 : sign(dy);
    enemy->attack_target_x = kind == STORM_LIGHTNING ? g->player.x : enemy->x;
    enemy->attack_target_y = kind == STORM_LIGHTNING ? g->player.y : enemy->y;
    enemy->boss_aux_x = enemy->x;
    enemy->boss_aux_y = enemy->y;
    if (kind == STORM_DIVE) {
        for (int n = 1; n <= range; n++) {
            int x = enemy->x + n * enemy->facing_dx;
            int y = enemy->y + n * enemy->facing_dy;
            if (!ground(g, x, y)) {
                break;
            }
            enemy->boss_aux_x = x;
            enemy->boss_aux_y = y;
        }
    } else if (kind == STORM_LIGHTNING) {
        const int ox[] = {2, 0, -2, 0, 1, 0, -1, 0};
        const int oy[] = {0, 2, 0, -2, 0, 1, 0, -1};
        for (int n = 0; n < 8; n++) {
            int x = g->player.x + ox[n];
            int y = g->player.y + oy[n];
            if (ground(g, x, y)) {
                enemy->boss_aux_x = x;
                enemy->boss_aux_y = y;
                break;
            }
        }
        StormwatchStage *state = &g->stormwatch[g->level - 1];
        state->next_direction[0] = dx >= 0 ? 0 : 2;
        state->next_direction[1] = dy >= 0 ? 1 : 3;
    }
    enemy->attack_phase = 1;
    enemy->move_timer = enemy->is_boss && kind != STORM_SWEEP ? 2 : 1;
    push_message(g, kind == STORM_DIVE ? "A swoop is aimed down the marked lane!" :
        kind == STORM_LIGHTNING ? "Lightning gathers over the fixed marked ground!" : "A wing sweep is coming! Step outside the marked fan.");
    return 1;
}

int stormwatch_defense(const Enemy *enemy, int x, int y) {
    if (enemy->type != ENEMY_CLIFF_GARGOYLE || enemy->move_timer < 0) {
        return enemy->defense;
    }
    int front = (x - enemy->x) * enemy->facing_dx + (y - enemy->y) * enemy->facing_dy;
    return front > 0 ? enemy->defense : 2;
}
