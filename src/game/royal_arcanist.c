#include "royal_arcanist.h"
#include <stdlib.h>

// attack_phase: latched health phase; boss_attack: signature;
// boss_wave_used: lanes follow the cross; boss_aux_x/y: fixed footprint bits.
// revive_timer: six closing + six opening barrier bits while preparing,
// or extra recovery actions after its target has been cleared.
static int inside(int x, int y) {
    return x >= 34 && x < 56 && y >= 44 && y < 58;
}

static int ward_active(const GameState *g, int east) {
    int x = east ? 53 : 37;
    TileType tile = g->map.tiles[54][x];
    if (tile == TILE_ITEM) {
        for (int i = 0; i < g->floor_item_count; i++) {
            const FloorItem *f = &g->floor_items[i];
            if (f->active && f->x == x && f->y == 54) {
                tile = f->underlying_tile;
                break;
            }
        }
    }
    return tile == TILE_CASTLE_WARD;
}

static int actor_at(const GameState *g, int x, int y) {
    if (g->player.x == x && g->player.y == y) {
        return 1;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y) {
            return 1;
        }
    }
    return 0;
}

int royal_arcanist_marks(const Enemy *e, int x, int y) {
    if (!e->active || e->type != ENEMY_ROYAL_ARCANIST || e->attack_target_x < 0 || e->attack_target_y < 0) {
        return 0;
    }
    int bit;
    if (e->boss_attack == ARCANIST_WEST_LANE) {
        if (y != e->attack_target_y || x < 34 || x >= 56) {
            return 0;
        }
        bit = x - 34;
    } else if (e->boss_attack == ARCANIST_EAST_LANE) {
        if (x != e->attack_target_x || y < 44 || y >= 58) {
            return 0;
        }
        bit = y - 44;
    } else {
        int dx = x - e->attack_target_x;
        int dy = y - e->attack_target_y;
        if (abs(dx) > 3 || abs(dy) > 3) {
            return 0;
        }
        bit = (dy + 3) * 7 + dx + 3;
    }
    return bit < 25 ? (e->boss_aux_x & (1 << bit)) != 0 : (e->boss_aux_y & (1 << (bit - 25))) != 0;
}

int royal_arcanist_barrier_change(const Enemy *e, int x, int y) {
    if (!e->active || e->type != ENEMY_ROYAL_ARCANIST || e->attack_target_x < 0 || (x != 40 && x != 50) || y < 47 || y > 49) {
        return 0;
    }
    int bit = (x == 50 ? 3 : 0) + y - 47;
    return (e->revive_timer & (1 << bit)) ? -1 : (e->revive_timer & (1 << (bit + 6))) ? 1 : 0;
}

static void lock_footprint(const Map *m, Enemy *e) {
    e->boss_aux_x = 0;
    e->boss_aux_y = 0;
    for (int bit = 0; bit < 49; bit++) {
        int dx = bit % 7 - 3;
        int dy = bit / 7 - 3;
        int x = e->attack_target_x + dx;
        int y = e->attack_target_y + dy;
        int marks = e->boss_attack == ARCANIST_BLAST ? abs(dx) + abs(dy) <= 1 : dx == 0 || dy == 0;
        if (e->boss_attack == ARCANIST_WEST_LANE) {
            x = 34 + bit;
            y = e->attack_target_y;
            marks = bit < 22;
        } else if (e->boss_attack == ARCANIST_EAST_LANE) {
            x = e->attack_target_x;
            y = 44 + bit;
            marks = bit < 14;
        }
        if (marks && map_is_walkable(m, x, y)) {
            if (bit < 25) {
                e->boss_aux_x |= 1 << bit;
            } else {
                e->boss_aux_y |= 1 << (bit - 25);
            }
        }
    }
}

static void plan_barriers(const Map *m, Enemy *e) {
    e->revive_timer = 0;
    for (int bit = 0; bit < 6; bit++) {
        TileType tile = m->tiles[47 + bit % 3][bit < 3 ? 40 : 50];
        if (tile == TILE_CASTLE_GATE_OPEN) {
            e->revive_timer |= 1 << bit;
        } else if (tile == TILE_CASTLE_GATE) {
            e->revive_timer |= 1 << (bit + 6);
        }
    }
}

static int future_walkable(const GameState *g, const Enemy *e, int x, int y) {
    if (actor_at(g, x, y)) {
        return 0;
    }
    int change = royal_arcanist_barrier_change(e, x, y);
    if (change < 0) {
        return 0;
    }
    // An escape must be walkable now, before the future barrier change too.
    return map_is_walkable(&g->map, x, y);
}

static int can_escape(const GameState *g, const Enemy *e, int x, int y, int turns) {
    if (!royal_arcanist_marks(e, x, y)) {
        return 1;
    }
    if (!turns) {
        return 0;
    }
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    for (int d = 0; d < 4; d++) {
        int nx = x + dx[d];
        int ny = y + dy[d];
        if (future_walkable(g, e, nx, ny) && can_escape(g, e, nx, ny, turns - 1)) {
            return 1;
        }
    }
    return 0;
}

void royal_arcanist_cancel(Enemy *e) {
    e->attack_target_x = -1;
    e->attack_target_y = -1;
    e->boss_aux_x = 0;
    e->boss_aux_y = 0;
    e->revive_timer = 0;
    e->move_timer = 0;
}

static void declare(GameState *g, Enemy *e, int attack) {
    e->boss_attack = attack;
    e->attack_target_x = g->player.x;
    e->attack_target_y = g->player.y;
    e->move_timer = 3;
    lock_footprint(&g->map, e);
    plan_barriers(&g->map, e);
    for (int bit = 0; bit < 6; bit++) {
        if (actor_at(g, bit < 3 ? 40 : 50, 47 + bit % 3)) {
            e->revive_timer &= ~(1 << bit);
        }
    }
    if (!can_escape(g, e, g->player.x, g->player.y, 2)) {
        e->boss_aux_x = 0;
        e->boss_aux_y = 0;
        e->revive_timer &= 63 << 6;
        push_message(g, "The chapel has no safe escape lane. The Arcanist's blocked attack will miss.");
    }
    static const char *names[] = {"", "Warded Blast", "Chapel Cross", "West Ward Lane", "East Ward Lane"};
    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message), "%s: two actions! Fixed stones and barrier changes are marked.%s", names[attack],
        attack >= ARCANIST_WEST_LANE ? " Disable its linked pedestal to interrupt." : "");
    push_message(g, message);
}

static void change_barriers(GameState *g, const Enemy *e) {
    for (int y = 47; y <= 49; y++) {
        for (int x = 40; x <= 50; x += 10) {
            int change = royal_arcanist_barrier_change(e, x, y);
            if (change > 0 && g->map.tiles[y][x] == TILE_CASTLE_GATE) {
                g->map.tiles[y][x] = TILE_CASTLE_GATE_OPEN;
            } else if (change < 0 && g->map.tiles[y][x] == TILE_CASTLE_GATE_OPEN && !actor_at(g, x, y)) {
                g->map.tiles[y][x] = TILE_CASTLE_GATE;
            }
        }
    }
}

void royal_arcanist_disable_ward(GameState *g, int x, int y) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level != 4 || y != 54 || (x != 37 && x != 53)) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        int source = x == 37 ? ARCANIST_WEST_LANE : ARCANIST_EAST_LANE;
        if (e->active && e->type == ENEMY_ROYAL_ARCANIST && e->attack_target_x >= 0 && e->boss_attack == source) {
            royal_arcanist_cancel(e);
            e->move_timer = 2;
            e->revive_timer = 2; // Interact's enemy tick leaves two full recovery actions.
            e->boss_wave_used = 0;
            push_message(g, "The linked ward lane collapses! Arcanist exposed for two full actions; barrier change canceled.");
        }
    }
}

int royal_arcanist_tick(GameState *g, Enemy *e) {
    if (e->move_timer >= 3) {
        e->move_timer = e->move_timer == 3 ? 1 : e->move_timer - 1;
        return 0;
    }
    if (e->move_timer == 2) {
        if (e->revive_timer > 0) {
            e->revive_timer--;
        } else {
            e->move_timer = 0;
        }
        return 0;
    }
    if (e->move_timer == 1) {
        int damage = 0;
        if (royal_arcanist_marks(e, g->player.x, g->player.y)) {
            damage = e->attack - g->player.defense + 4 * (ward_active(g, 0) + ward_active(g, 1));
            for (int i = 0; i < g->enemy_count; i++) {
                const Enemy *hexer = &g->enemies[i];
                if (hexer->active && hexer->type == ENEMY_COURT_HEXER && abs(hexer->x - e->x) + abs(hexer->y - e->y) <= 4) {
                    damage += 5;
                    break;
                }
            }
            if (damage < 3) {
                damage = 3;
            }
        }
        int next_lane = e->boss_attack == ARCANIST_WEST_LANE && ward_active(g, 1) && inside(g->player.x, g->player.y);
        change_barriers(g, e);
        royal_arcanist_cancel(e);
        push_message(g, "The Arcanist strikes only the marked stones. Warned barriers shift safely.");
        if (next_lane) {
            declare(g, e, ARCANIST_EAST_LANE);
        } else {
            e->move_timer = 2;
            e->boss_wave_used = e->boss_attack == ARCANIST_CROSS;
            push_message(g, "The Arcanist recovers for one full action. Both pedestals remain usable.");
        }
        return damage;
    }
    int phase = e->hp * 2 <= e->max_hp ? 2 : 1;
    if (phase > e->attack_phase) {
        e->attack_phase = phase;
        push_message(g, phase == 2 ? "Shattered Liturgy! Chapel crosses alternate with separately warned ward lanes." :
            "Warded Scholar! Two-action blasts; each active pedestal grants protection and strengthens attacks.");
    }
    int attack = ARCANIST_BLAST;
    if (e->attack_phase == 2) {
        attack = ARCANIST_CROSS;
        if (e->boss_wave_used) {
            if (!ward_active(g, 0) && !ward_active(g, 1)) {
                attack = ARCANIST_BLAST;
            } else if (inside(g->player.x, g->player.y) && ward_active(g, 0)) {
                attack = ARCANIST_WEST_LANE;
            } else if (inside(g->player.x, g->player.y) && ward_active(g, 1)) {
                attack = ARCANIST_EAST_LANE;
            }
        }
    }
    declare(g, e, attack);
    return 0;
}

static void migrate_floor(const Map *m, Enemy *enemies, int count) {
    for (int i = 0; i < count; i++) {
        Enemy *e = &enemies[i];
        if (e->type != ENEMY_ROYAL_ARCANIST) {
            continue;
        }
        e->attack_phase = 0;
        e->boss_attack = ARCANIST_BLAST;
        e->boss_wave_used = 0;
        e->boss_aux_x = 0;
        e->boss_aux_y = 0;
        e->revive_timer = 0;
        if (e->attack_target_x >= 0 && e->attack_target_y >= 0) {
            e->attack_phase = 1;
            lock_footprint(m, e);
            plan_barriers(m, e);
        }
    }
}

void royal_arcanist_migrate(GameState *g) {
    g->arcanist_reward_state = (g->castle_minibosses & 2) ? 1 : 0;
    if (g->location == LOCATION_CASTLE_INTERIOR && g->level == 4) {
        migrate_floor(&g->map, g->enemies, g->enemy_count);
    }
    if (g->castle_cache[3].valid) {
        LevelCache *c = &g->castle_cache[3];
        migrate_floor(&c->map, c->enemies, c->enemy_count);
    }
}
