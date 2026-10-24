#include "mangrove.h"
#include <stdlib.h>
#include <string.h>

enum { REED_THRUST = 1, VENOM_SPIT, ROOT_CHARGE, SPEAR_SWEEP, CALL_HUNTERS };
static const int steps[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

static int sign(int value) {
    return (value > 0) - (value < 0);
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

static int clear_ground(const GameState *g, int x, int y) {
    if (x < 1 || y < 1 || x >= MANGROVE_W - 1 || y >= MANGROVE_H - 1 || !map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    for (int i = 0; i < 4; i++) {
        int lx;
        int ly;
        mangrove_landing(g->level, i, &lx, &ly);
        if (abs(lx - x) + abs(ly - y) <= 1) {
            return 0;
        }
    }
    return abs(x - g->map.stairs_up_x) + abs(y - g->map.stairs_up_y) > 3 &&
        abs(x - g->map.stairs_down_x) + abs(y - g->map.stairs_down_y) > 3;
}

static void add_enemy(GameState *g, EnemyType type, int x, int y) {
    if (g->enemy_count >= AREA_ENEMY_LIMIT) {
        return;
    }
    static const char *names[] = {"Lizardfolk Scout", "Reed Spearhunter", "Marsh Spitter", "Mudback Turtle", "Lizardfolk Scaleguard", "Sszareth"};
    static const int health[] = {32, 44, 38, 105, 90, 480};
    static const int attack[] = {9, 12, 11, 16, 17, 25};
    static const int armor[] = {2, 3, 2, 8, 6, 10};
    static const int xp[] = {35, 48, 45, 95, 90, 900};
    int i = type - ENEMY_LIZARDFOLK_SCOUT;
    Enemy *enemy = &g->enemies[g->enemy_count++];
    *enemy = (Enemy){.active = 1, .type = type, .x = x, .y = y, .is_boss = type == ENEMY_SSZARETH,
        .max_hp = health[i] + (type == ENEMY_SSZARETH ? 0 : 7 * (g->level - 1)),
        .attack = attack[i] + g->level - 1, .defense = armor[i], .experience = xp[i],
        .attack_target_x = -1, .attack_target_y = -1, .facing_dy = 1};
    enemy->hp = enemy->max_hp;
    snprintf(enemy->name, sizeof(enemy->name), "%s", names[i]);
}

static int place_near(GameState *g, EnemyType type, int cx, int cy, int summoned) {
    for (int radius = 0; radius < 8; radius++) {
        for (int y = cy - radius; y <= cy + radius; y++) {
            for (int x = cx - radius; x <= cx + radius; x++) {
                if (abs(x - cx) + abs(y - cy) != radius || !clear_ground(g, x, y) || occupied(g, NULL, x, y) ||
                    (x == g->player.x && y == g->player.y) ||
                    (summoned && abs(x - g->player.x) + abs(y - g->player.y) < 5)) {
                    continue;
                }
                int old = g->enemy_count;
                add_enemy(g, type, x, y);
                if (g->enemy_count > old && summoned) {
                    g->enemies[old].revived = 1;
                    g->enemies[old].experience = 0;
                }
                return g->enemy_count > old;
            }
        }
    }
    return 0;
}

void mangrove_spawn(GameState *g, int fresh) {
    Enemy boss = {0};
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].type == ENEMY_SSZARETH && g->enemies[i].active) {
            boss = g->enemies[i];
        }
    }
    g->enemy_count = 0;
    if (!fresh && boss.active && !g->mangrove_defeated) {
        g->enemies[g->enemy_count++] = boss;
    }
    for (int island = 1; island < 4; island++) {
        int x;
        int y;
        mangrove_island(g->level, island, &x, &y);
        EnemyType first = g->level == 3 ? ENEMY_MUDBACK_TURTLE :
            g->level >= 4 ? ENEMY_LIZARDFOLK_SCALEGUARD : ENEMY_LIZARDFOLK_SCOUT;
        place_near(g, first, x - 2, y, 0);
        place_near(g, ENEMY_REED_SPEARHUNTER, x + 2, y - 1, 0);
        if (g->level >= 2) {
            place_near(g, ENEMY_MARSH_SPITTER, x, y + 3, 0);
        }
        if (g->level == MANGROVE_DEPTH && island == 3 && !g->mangrove_defeated) {
            if (fresh || !boss.active) {
                place_near(g, ENEMY_SSZARETH, x, y, 0);
            }
        }
    }
}

// Follow only the connected shore component; roots and water are real cover.
static void step_toward(GameState *g, Enemy *enemy) {
    short distance[MANGROVE_H][MANGROVE_W];
    int queue[MANGROVE_W * MANGROVE_H];
    memset(distance, -1, sizeof(distance));
    int head = 0;
    int tail = 0;
    distance[g->player.y][g->player.x] = 0;
    queue[tail++] = g->player.y * MANGROVE_W + g->player.x;
    while (head < tail) {
        int cell = queue[head++];
        int x = cell % MANGROVE_W;
        int y = cell / MANGROVE_W;
        for (int i = 0; i < 4; i++) {
            int nx = x + steps[i][0];
            int ny = y + steps[i][1];
            if (nx > 0 && ny > 0 && nx < MANGROVE_W - 1 && ny < MANGROVE_H - 1 &&
                distance[ny][nx] < 0 && map_is_walkable(&g->map, nx, ny)) {
                distance[ny][nx] = distance[y][x] + 1;
                queue[tail++] = ny * MANGROVE_W + nx;
            }
        }
    }
    int best = distance[enemy->y][enemy->x];
    if (best < 0 || best > 18) {
        return;
    }
    int tx = enemy->x;
    int ty = enemy->y;
    for (int i = 0; i < 4; i++) {
        int x = enemy->x + steps[i][0];
        int y = enemy->y + steps[i][1];
        if (clear_ground(g, x, y) && distance[y][x] > 0 && distance[y][x] < best && !occupied(g, enemy, x, y)) {
            best = distance[y][x];
            tx = x;
            ty = y;
        }
    }
    enemy->x = tx;
    enemy->y = ty;
}

static int clear_lane(const GameState *g, const Enemy *enemy, int range) {
    int dx = g->player.x - enemy->x;
    int dy = g->player.y - enemy->y;
    int distance = abs(dx) + abs(dy);
    if ((dx && dy) || distance < 1 || distance > range) {
        return 0;
    }
    for (int i = 1; i < distance; i++) {
        if (!map_is_walkable(&g->map, enemy->x + sign(dx) * i, enemy->y + sign(dy) * i)) {
            return 0;
        }
    }
    return 1;
}

int mangrove_attack_marks(const Enemy *enemy, int x, int y) {
    if (!enemy->active || !enemy->attack_phase) {
        return 0;
    }
    int dx = x - enemy->x;
    int dy = y - enemy->y;
    if (enemy->boss_attack == VENOM_SPIT) {
        return x == enemy->attack_target_x && y == enemy->attack_target_y;
    }
    if (enemy->boss_attack == SPEAR_SWEEP) {
        return abs(dx) <= 2 && abs(dy) <= 2 && (dx || dy);
    }
    if (enemy->boss_attack == REED_THRUST || enemy->boss_attack == ROOT_CHARGE) {
        int length = enemy->boss_attack == REED_THRUST ? 2 : enemy->type == ENEMY_SSZARETH ? 6 : 3;
        int along = dx * enemy->facing_dx + dy * enemy->facing_dy;
        return along > 0 && along <= length && dx * enemy->facing_dy == dy * enemy->facing_dx;
    }
    return 0;
}

int mangrove_tick(GameState *g, Enemy *enemy, int *damage) {
    if (g->location != LOCATION_MANGROVES) {
        return 0;
    }
    *damage = 0;
    if (enemy->move_timer < 0) {
        enemy->move_timer++;
        if (!enemy->move_timer) {
            enemy->defense = enemy->type == ENEMY_SSZARETH ? 10 : enemy->type == ENEMY_MUDBACK_TURTLE ? 8 : enemy->defense;
        }
        return 1;
    }
    if (enemy->attack_phase) {
        if (--enemy->move_timer > 0) {
            return 1;
        }
        if (enemy->boss_attack == ROOT_CHARGE) {
            int collision = 0;
            int length = enemy->is_boss ? 6 : 3;
            for (int n = 0; n < length; n++) {
                int x = enemy->x + enemy->facing_dx;
                int y = enemy->y + enemy->facing_dy;
                if (!clear_ground(g, x, y) || occupied(g, enemy, x, y)) {
                    collision = 1;
                    break;
                }
                if (x == g->player.x && y == g->player.y) {
                    *damage = enemy->attack;
                    break;
                }
                enemy->x = x;
                enemy->y = y;
            }
            enemy->move_timer = collision ? -3 : -2;
            enemy->defense = 0;
            if (collision && enemy->is_boss) {
                push_message(g, "Sszareth's charge is stopped! His guard is open.");
            }
        } else if (enemy->boss_attack == CALL_HUNTERS) {
            place_near(g, ENEMY_REED_SPEARHUNTER, enemy->x - 5, enemy->y, 1);
            place_near(g, ENEMY_REED_SPEARHUNTER, enemy->x + 5, enemy->y, 1);
            enemy->move_timer = -2;
        } else {
            if (mangrove_attack_marks(enemy, g->player.x, g->player.y)) {
                *damage = enemy->attack;
            }
            enemy->move_timer = -2;
        }
        enemy->attack_phase = 0;
        return 1;
    }
    int dx = g->player.x - enemy->x;
    int dy = g->player.y - enemy->y;
    int distance = abs(dx) + abs(dy);
    if (enemy->is_boss && distance <= 12 && enemy->hp * 2 < enemy->max_hp && !enemy->boss_wave_used) {
        enemy->boss_wave_used = 1;
        enemy->boss_attack = CALL_HUNTERS;
    } else if ((enemy->type == ENEMY_MUDBACK_TURTLE || enemy->is_boss) && clear_lane(g, enemy, enemy->is_boss ? 6 : 3)) {
        enemy->boss_attack = ROOT_CHARGE;
    } else if (enemy->type == ENEMY_MARSH_SPITTER && clear_lane(g, enemy, 6)) {
        enemy->boss_attack = VENOM_SPIT;
    } else if ((enemy->type == ENEMY_LIZARDFOLK_SCALEGUARD || enemy->is_boss) && abs(dx) <= 2 && abs(dy) <= 2) {
        enemy->boss_attack = SPEAR_SWEEP;
    } else if (enemy->type == ENEMY_REED_SPEARHUNTER && clear_lane(g, enemy, 2)) {
        enemy->boss_attack = REED_THRUST;
    } else if (enemy->type == ENEMY_LIZARDFOLK_SCOUT && distance == 1) {
        *damage = enemy->attack;
        enemy->move_timer = -1;
        return 1;
    } else {
        step_toward(g, enemy);
        if (enemy->type == ENEMY_MUDBACK_TURTLE) {
            enemy->move_timer = -1;
        }
        return 1;
    }
    enemy->attack_phase = 1;
    enemy->attack_target_x = g->player.x;
    enemy->attack_target_y = g->player.y;
    enemy->facing_dx = sign(dx);
    enemy->facing_dy = dx ? 0 : sign(dy);
    enemy->move_timer = enemy->is_boss || enemy->boss_attack == SPEAR_SWEEP ? 2 : 1;
    push_message(g, enemy->boss_attack == CALL_HUNTERS ? "Sszareth calls two spearhunters!" :
        enemy->boss_attack == VENOM_SPIT ? "Venom is aimed at the marked ground!" :
        enemy->boss_attack == ROOT_CHARGE ? "A charge is coming! Leave the marked lane." : "A spear strike is coming! Step away from the marked ground.");
    return 1;
}

void mangrove_defeat(GameState *g) {
    if (!g->mangrove_defeated) {
        g->mangrove_defeated = 1;
        g->gold += 200;
        g->score += 2400;
        push_message(g, "Sszareth falls! The sanctuary's departure opens. You recover 200 gold.");
    }
}
