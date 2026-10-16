#include "lord_veyr.h"
#include "castle.h"
#include <stdlib.h>

// attack_phase: latched health phase; boss_attack: signature; revived: spent escort call.
// boss_wave_used: cycle (low two bits), deferred legacy fire (4), queued escort (8).
// boss_aux_x: fixed footprint; boss_aux_y: barrier plan; revive_timer: extra recovery.
static int inside(int x, int y) {
    return x >= 34 && x < 56 && y >= 44 && y < 58;
}

static TileType tile_at(const GameState *g, int x, int y) {
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_ITEM) {
        for (int i = 0; i < g->floor_item_count; i++) {
            const FloorItem *f = &g->floor_items[i];
            if (f->active && f->x == x && f->y == y) {
                return f->underlying_tile;
            }
        }
    }
    return tile;
}

static int wards(const GameState *g) {
    return (tile_at(g, 37, 54) == TILE_CASTLE_WARD) + (tile_at(g, 53, 54) == TILE_CASTLE_WARD);
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

int lord_veyr_escort_marks(const Enemy *e, int x, int y) {
    return e->active && e->type == ENEMY_LORD_VEYR && e->attack_target_x >= 0 &&
        (e->boss_wave_used & VEYR_ESCORT_PENDING) && y == 46 && (x == 42 || x == 48);
}

int lord_veyr_marks(const Enemy *e, int x, int y) {
    if (!e->active || e->type != ENEMY_LORD_VEYR || e->attack_target_x < 0 || e->attack_target_y < 0) {
        return 0;
    }
    int bit;
    if (e->boss_attack == VEYR_ADVANCE) {
        int dx = x - e->x;
        int dy = y - e->y;
        int step = dx * e->facing_dx + dy * e->facing_dy;
        if (step < 1 || step > 3 || dx != step * e->facing_dx || dy != step * e->facing_dy) {
            return 0;
        }
        bit = step - 1;
    } else if (e->boss_attack == VEYR_FIRE) {
        if (y != 51 || x < 41 || x > 49) {
            return 0;
        }
        bit = x - 41;
    } else if (e->boss_attack == VEYR_WEST_LANE) {
        if (y != e->attack_target_y || x < 34 || x >= 56) {
            return 0;
        }
        bit = x - 34;
    } else if (e->boss_attack == VEYR_EAST_LANE) {
        if (x != e->attack_target_x || y < 44 || y >= 58) {
            return 0;
        }
        bit = y - 44;
    } else {
        int dx = x - e->attack_target_x;
        int dy = y - e->attack_target_y;
        if (abs(dx) > 2 || abs(dy) > 2) {
            return 0;
        }
        bit = (dy + 2) * 5 + dx + 2;
    }
    return (e->boss_aux_x & (1 << bit)) != 0;
}

int lord_veyr_barrier_change(const Enemy *e, int x, int y) {
    if (!e->active || e->type != ENEMY_LORD_VEYR || e->attack_target_x < 0 || (x != 40 && x != 50) || y < 47 || y > 49) {
        return 0;
    }
    int bit = (x == 50 ? 3 : 0) + y - 47;
    return (e->boss_aux_y & (1 << bit)) ? -1 : (e->boss_aux_y & (1 << (bit + 6))) ? 1 : 0;
}

static void lock_footprint(const Map *m, Enemy *e) {
    e->boss_aux_x = 0;
    e->boss_aux_y = 0;
    int radius = e->boss_attack == VEYR_CROWN ? 2 : e->boss_attack == VEYR_BLAST ? 1 : 0;
    for (int bit = 0; bit < 25; bit++) {
        int dx = bit % 5 - 2;
        int dy = bit / 5 - 2;
        int x = e->attack_target_x + dx;
        int y = e->attack_target_y + dy;
        int mark = abs(dx) + abs(dy) <= radius;
        if (e->boss_attack == VEYR_ADVANCE) {
            x = e->x + e->facing_dx * (bit + 1);
            y = e->y + e->facing_dy * (bit + 1);
            if (bit >= 3 || !inside(x, y) || !map_is_walkable(m, x, y)) {
                break;
            }
            mark = 1;
        } else if (e->boss_attack == VEYR_FIRE) {
            x = 41 + bit;
            y = 51;
            mark = bit < 9;
        } else if (e->boss_attack == VEYR_WEST_LANE) {
            x = 34 + bit;
            y = e->attack_target_y;
            mark = bit < 22;
        } else if (e->boss_attack == VEYR_EAST_LANE) {
            x = e->attack_target_x;
            y = 44 + bit;
            mark = bit < 14;
        }
        if (mark && map_is_walkable(m, x, y)) {
            e->boss_aux_x |= 1 << bit;
        }
    }
}

static void plan_barriers(const Map *m, Enemy *e) {
    e->boss_aux_y = 0;
    if (e->boss_attack < VEYR_BLAST || e->boss_attack == VEYR_FIRE) {
        return;
    }
    for (int bit = 0; bit < 6; bit++) {
        TileType tile = m->tiles[47 + bit % 3][bit < 3 ? 40 : 50];
        if (tile == TILE_CASTLE_GATE_OPEN) {
            e->boss_aux_y |= 1 << bit;
        } else if (tile == TILE_CASTLE_GATE) {
            e->boss_aux_y |= 1 << (bit + 6);
        }
    }
}

static int can_escape(const GameState *g, const Enemy *e, int x, int y, int turns) {
    if (!lord_veyr_marks(e, x, y)) {
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
        if (map_is_walkable(&g->map, nx, ny) && !actor_at(g, nx, ny) && lord_veyr_barrier_change(e, nx, ny) >= 0 && can_escape(g, e, nx, ny, turns - 1)) {
            return 1;
        }
    }
    return 0;
}

void lord_veyr_cancel(GameState *g, Enemy *e) {
    e->attack_target_x = -1;
    e->attack_target_y = -1;
    e->boss_aux_x = 0;
    e->boss_aux_y = 0;
    e->revive_timer = 0;
    e->move_timer = 0;
    e->boss_wave_used &= ~VEYR_ESCORT_PENDING;
    g->castle_fire_phase[5] = 0;
}

static void declare(GameState *g, Enemy *e, int attack) {
    e->boss_attack = attack;
    e->attack_target_x = g->player.x;
    e->attack_target_y = g->player.y;
    e->move_timer = attack == VEYR_CROWN ? 4 : attack == VEYR_SWING || attack == VEYR_ADVANCE ? 1 : 3;
    int dx = g->player.x - e->x;
    int dy = g->player.y - e->y;
    e->facing_dx = abs(dx) >= abs(dy) ? (dx > 0 ? 1 : -1) : 0;
    e->facing_dy = e->facing_dx ? 0 : (dy > 0 ? 1 : -1);
    if (attack == VEYR_ADVANCE) {
        e->attack_target_x = e->x + e->facing_dx * 3;
        e->attack_target_y = e->y + e->facing_dy * 3;
    }
    if (!e->revived) {
        e->revived = 1;
        e->boss_wave_used |= VEYR_ESCORT_PENDING;
        if (e->move_timer == 1) {
            e->move_timer = 3;
        }
        push_message(g, "Veyr calls his last two escorts! Marked G stones; arrivals wait until this attack resolves, then act next turn.");
    }
    lock_footprint(&g->map, e);
    plan_barriers(&g->map, e);
    for (int bit = 0; bit < 6; bit++) {
        if (actor_at(g, bit < 3 ? 40 : 50, 47 + bit % 3)) {
            e->boss_aux_y &= ~(1 << bit);
        }
    }
    int turns = e->move_timer >= 3 ? e->move_timer - 1 : 1;
    if (!can_escape(g, e, g->player.x, g->player.y, turns)) {
        e->boss_aux_x = 0;
        e->boss_aux_y &= 63 << 6;
        push_message(g, "Veyr's blocked preparation leaves no walking escape, so its strike will miss.");
    }
    g->castle_fire_phase[5] = attack == VEYR_FIRE ? 1 : 0;
    static const char *names[] = {"", "Oathblade Swing", "Throne Advance", "Royal Edict", "West Ward Lane", "East Ward Lane", "Unbound Crown", "Throne Fire"};
    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message), "%s: %d action%s! Fixed stones and gate changes are marked.", names[attack], turns, turns == 1 ? "" : "s");
    push_message(g, message);
}

void lord_veyr_disable_ward(GameState *g, int x, int y) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level != 6 || y != 54 || (x != 37 && x != 53)) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (e->active && e->type == ENEMY_LORD_VEYR && e->attack_target_x >= 0 && e->boss_attack == (x == 37 ? VEYR_WEST_LANE : VEYR_EAST_LANE)) {
            // Preserve a separately announced escort arrival until the canceled lane resolves.
            e->boss_aux_x = 0;
            e->boss_aux_y = 0;
            push_message(g, "Veyr's linked ward lane goes dark. Its strike and barrier change are canceled.");
        }
    }
}

static int resolve(GameState *g, Enemy *e) {
    int hit = lord_veyr_marks(e, g->player.x, g->player.y);
    if (e->boss_attack == VEYR_ADVANCE) {
        hit = 0;
        for (int step = 0; step < 3 && (e->boss_aux_x & (1 << step)); step++) {
            int x = e->x + e->facing_dx;
            int y = e->y + e->facing_dy;
            if (!map_is_walkable(&g->map, x, y)) {
                break;
            }
            if (actor_at(g, x, y)) {
                hit = x == g->player.x && y == g->player.y;
                break;
            }
            e->x = x;
            e->y = y;
        }
    }
    int damage = e->boss_attack == VEYR_FIRE ? 24 - g->player.defense / 2 : e->attack - g->player.defense + wards(g) * 4;
    if (e->boss_attack != VEYR_FIRE) {
        for (int i = 0; i < g->enemy_count; i++) {
            const Enemy *hexer = &g->enemies[i];
            if (hexer->active && hexer->type == ENEMY_COURT_HEXER && abs(hexer->x - e->x) + abs(hexer->y - e->y) <= 4) {
                damage += 5;
                break;
            }
        }
    }
    for (int y = 47; y <= 49; y++) {
        for (int x = 40; x <= 50; x += 10) {
            int change = lord_veyr_barrier_change(e, x, y);
            if (change > 0 && g->map.tiles[y][x] == TILE_CASTLE_GATE) {
                g->map.tiles[y][x] = TILE_CASTLE_GATE_OPEN;
            } else if (change < 0 && g->map.tiles[y][x] == TILE_CASTLE_GATE_OPEN && !actor_at(g, x, y)) {
                g->map.tiles[y][x] = TILE_CASTLE_GATE;
            }
        }
    }
    if (e->boss_wave_used & VEYR_ESCORT_PENDING) {
        castle_summon_guard(g, ENEMY_IRON_WARDEN, 42, 46);
        castle_summon_guard(g, ENEMY_ROYAL_MARKSMAN, 48, 46);
        push_message(g, "Veyr's finite escorts arrive on empty marked stones. They yield no gold, items or XP.");
    }
    int extra = e->boss_attack == VEYR_CROWN && wards(g) == 0;
    int fire = e->boss_attack == VEYR_FIRE;
    int cycle = e->boss_attack == VEYR_CROWN || e->boss_attack == VEYR_SWING ? 1 :
        e->boss_attack == VEYR_BLAST && e->attack_phase == 2 ? 1 : e->boss_attack == VEYR_WEST_LANE ? 2 : 0;
    lord_veyr_cancel(g, e);
    e->boss_wave_used = (e->boss_wave_used & ~3) | cycle;
    e->move_timer = 2;
    e->revive_timer = extra;
    g->castle_fire_phase[5] = fire ? 3 : 0;
    push_message(g, extra ? "The crown shatters against the broken wards! Two full recovery actions." : "Veyr's declared strike ends. One full recovery action.");
    int minimum = fire ? 6 : 3;
    return hit ? (damage > minimum ? damage : minimum) : 0;
}

int lord_veyr_tick(GameState *g, Enemy *e) {
    if (e->move_timer >= 3) {
        e->move_timer = e->move_timer == 3 ? 1 : e->move_timer - 1;
        if (e->boss_attack == VEYR_FIRE) {
            g->castle_fire_phase[5] = 2;
        }
        return 0;
    }
    if (e->move_timer == 1) {
        return resolve(g, e);
    }
    if (e->move_timer == 2) {
        if (e->revive_timer > 0) {
            e->revive_timer--;
        } else {
            e->move_timer = 0;
        }
        return 0;
    }
    int phase = e->hp * 3 > e->max_hp * 2 ? 1 : e->hp * 3 > e->max_hp ? 2 : 3;
    if (phase > e->attack_phase) {
        e->attack_phase = phase;
        e->boss_wave_used &= ~3;
        static const char *names[] = {"", "Armored Ruler! Flank his shield and warned throne advance.", "Broken Throne! Royal Edicts and ward-linked lanes reshape the arena.", "Unbound Crown! Three-action crowns alternate with separately warned throne fire."};
        push_message(g, names[phase]);
    }
    int cycle = e->boss_wave_used & 3;
    int attack = VEYR_BLAST;
    int distance = abs(g->player.x - e->x) + abs(g->player.y - e->y);
    if (e->boss_wave_used & VEYR_DEFERRED_FIRE) {
        e->boss_wave_used &= ~VEYR_DEFERRED_FIRE;
        attack = VEYR_FIRE;
    } else if (e->attack_phase == 1) {
        attack = !inside(g->player.x, g->player.y) ? VEYR_BLAST : cycle || distance > 2 ? VEYR_ADVANCE : VEYR_SWING;
    } else if (e->attack_phase == 2 && inside(g->player.x, g->player.y)) {
        if (cycle == 1 && tile_at(g, 37, 54) == TILE_CASTLE_WARD) {
            attack = VEYR_WEST_LANE;
        } else if (cycle && tile_at(g, 53, 54) == TILE_CASTLE_WARD) {
            attack = VEYR_EAST_LANE;
        }
    } else if (e->attack_phase == 3) {
        attack = cycle && inside(g->player.x, g->player.y) ? VEYR_FIRE : VEYR_CROWN;
    }
    declare(g, e, attack);
    return 0;
}

static void migrate_floor(const Map *m, Enemy *enemies, int count, int fire) {
    for (int i = 0; i < count; i++) {
        Enemy *e = &enemies[i];
        if (e->type != ENEMY_LORD_VEYR) {
            continue;
        }
        e->boss_wave_used = fire == 1 || fire == 2 ? VEYR_DEFERRED_FIRE : 0;
        e->boss_aux_x = 0;
        e->boss_aux_y = 0;
        e->revive_timer = 0;
        if (e->attack_target_x >= 0 && e->attack_target_y >= 0) {
            e->boss_attack = e->attack_phase == 3 ? VEYR_CROWN : e->attack_phase == 2 ? VEYR_BLAST : VEYR_SWING;
            lock_footprint(m, e);
            plan_barriers(m, e);
        }
    }
}

void lord_veyr_migrate(GameState *g) {
    g->crown_of_unbound = g->game_won != 0;
    int fire = g->castle_fire_phase[5];
    if (g->location == LOCATION_CASTLE_INTERIOR && g->level == 6) {
        migrate_floor(&g->map, g->enemies, g->enemy_count, fire);
    }
    if (g->castle_cache[5].valid) {
        LevelCache *c = &g->castle_cache[5];
        migrate_floor(&c->map, c->enemies, c->enemy_count, fire);
    }
    g->castle_fire_phase[5] = 0;
}
