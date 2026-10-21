#include "sun_guardian.h"
#include <stdlib.h>

static int summit(const GameState *g) {
    return g->location == LOCATION_TEMPLE && g->level == TEMPLE_DEPTH;
}

static int sanctuary(int x, int y) {
    return x >= 22 && x <= 42 && y >= 7 && y <= 11;
}

static TileType tile_at(const GameState *g, int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
        return TILE_WALL;
    }
    if (g->map.tiles[y][x] == TILE_ITEM) {
        for (int i = 0; i < g->floor_item_count; i++) {
            const FloorItem *f = &g->floor_items[i];
            if (f->active && f->x == x && f->y == y) {
                return (TileType)f->underlying_tile;
            }
        }
    }
    return g->map.tiles[y][x];
}

static void set_tile(GameState *g, int x, int y, TileType tile) {
    if (g->map.tiles[y][x] != TILE_ITEM) {
        g->map.tiles[y][x] = tile;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *f = &g->floor_items[i];
        if (f->active && f->x == x && f->y == y) {
            f->underlying_tile = tile;
        }
    }
}

static int occupied(const Enemy *enemies, int count, int x, int y) {
    for (int i = 0; i < count; i++) {
        if (enemies[i].active && enemies[i].x == x && enemies[i].y == y) {
            return 1;
        }
    }
    return 0;
}

void sun_guardian_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py) {
    int living = 0;
    for (int i = 0; i < count; i++) {
        living |= enemies[i].active && enemies[i].type == ENEMY_FALLEN_SUN_GUARDIAN;
    }
    if (!living) {
        return;
    }
    // Only ordinary summit floor is replaced; doors, traps, treasure and actors stay intact.
    static const int positions[6][2] = {{29, 9}, {35, 9}, {29, 8}, {29, 10}, {35, 8}, {35, 10}};
    for (int i = 0; i < 6; i++) {
        int x = positions[i][0];
        int y = positions[i][1];
        if (m->tiles[y][x] == TILE_TEMPLE_FLOOR && !(x == px && y == py) && !occupied(enemies, count, x, y)) {
            m->tiles[y][x] = i < 2 ? TILE_TEMPLE_ALTAR : i < 4 ? TILE_GUARDIAN_MOON_SANCTUARY : TILE_GUARDIAN_SUN_SANCTUARY;
        }
    }
}

int sun_guardian_sanctuary_protects(const GameState *g, const Enemy *e, int x, int y) {
    TileType tile = tile_at(g, x, y);
    return e->boss_attack == GUARDIAN_LUNAR ? tile == TILE_GUARDIAN_SUN_SANCTUARY : tile == TILE_GUARDIAN_MOON_SANCTUARY;
}

// The lance retains its declared terminal cell. Current cover can only shorten it.
static int lance_marks(const GameState *g, const Enemy *e, int x, int y) {
    int length = abs(e->boss_aux_x - e->attack_target_x) + abs(e->boss_aux_y - e->attack_target_y);
    for (int n = 1; n <= length; n++) {
        int tx = e->attack_target_x + e->facing_dx * n;
        int ty = e->attack_target_y + e->facing_dy * n;
        if (!map_is_walkable(&g->map, tx, ty) || occupied(g->enemies, g->enemy_count, tx, ty)) {
            break;
        }
        if (tx == x && ty == y) {
            return 1;
        }
    }
    return 0;
}

int sun_guardian_attack_marks(const GameState *g, const Enemy *e, int x, int y) {
    if (!summit(g) || !e->active || e->type != ENEMY_FALLEN_SUN_GUARDIAN ||
        e->move_timer <= 0 || e->attack_target_x < 0 || sun_guardian_sanctuary_protects(g, e, x, y)) {
        return 0;
    }
    if (e->boss_attack == GUARDIAN_LANCE) {
        return lance_marks(g, e, x, y);
    }
    if (!map_is_walkable(&g->map, x, y) || (!sanctuary(x, y) && e->boss_attack != GUARDIAN_LUNAR)) {
        return 0;
    }
    int dx = abs(x - e->attack_target_x);
    int dy = abs(y - e->attack_target_y);
    if (e->boss_attack == GUARDIAN_RING) {
        return dx + dy >= 2 && dx + dy <= 4;
    }
    if (e->boss_attack == GUARDIAN_LANES) {
        return (dx == 0 || dy == 0) && dx + dy <= 4;
    }
    return e->boss_attack == GUARDIAN_LUNAR && dx + dy <= 1;
}

static int can_escape(const GameState *g, const Enemy *e, int x, int y, int turns) {
    if (!sun_guardian_attack_marks(g, e, x, y)) {
        return 1;
    }
    if (!turns) {
        return 0;
    }
    static const int steps[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (int i = 0; i < 4; i++) {
        int nx = x + steps[i][0];
        int ny = y + steps[i][1];
        if (map_is_walkable(&g->map, nx, ny) && !occupied(g->enemies, g->enemy_count, nx, ny) &&
            can_escape(g, e, nx, ny, turns - 1)) {
            return 1;
        }
    }
    return 0;
}

int sun_guardian_enemy_damage(const GameState *g, const Enemy *e, int damage) {
    if (!summit(g) || e->type != ENEMY_FALLEN_SUN_GUARDIAN || g->temple_alignment) {
        return damage;
    }
    damage = damage * 2 / 3;
    return damage > 0 ? damage : 1;
}

static void declare_lance(const GameState *g, Enemy *e) {
    int dx = g->player.x - e->x;
    int dy = g->player.y - e->y;
    e->attack_target_x = e->x;
    e->attack_target_y = e->y;
    e->facing_dx = abs(dx) >= abs(dy) ? (dx < 0 ? -1 : 1) : 0;
    e->facing_dy = abs(dx) < abs(dy) ? (dy < 0 ? -1 : 1) : 0;
    e->boss_aux_x = e->x;
    e->boss_aux_y = e->y;
    for (int n = 1; n <= 10; n++) {
        int x = e->x + e->facing_dx * n;
        int y = e->y + e->facing_dy * n;
        if (!map_is_walkable(&g->map, x, y) || occupied(g->enemies, g->enemy_count, x, y)) {
            break;
        }
        e->boss_aux_x = x;
        e->boss_aux_y = y;
    }
}

int sun_guardian_tick(GameState *g, Enemy *e, EnemyProjectiles *shots) {
    if (!summit(g) || (e->hp == e->max_hp && !e->boss_attack && g->player.y > 12) ||
        abs(g->player.x - e->x) + abs(g->player.y - e->y) > 16) {
        return 0;
    }
    if (e->move_timer < 0) {
        e->move_timer++;
        return 0;
    }
    if (e->move_timer > 0) {
        e->move_timer--;
        if (e->move_timer > 0) {
            return 0;
        }
        e->move_timer = 1;
        int hit = sun_guardian_attack_marks(g, e, g->player.x, g->player.y);
        if (e->boss_attack == GUARDIAN_LANCE && shots && shots->count < MAX_ENEMIES) {
            int x = e->attack_target_x;
            int y = e->attack_target_y;
            int length = abs(e->boss_aux_x - x) + abs(e->boss_aux_y - y);
            for (int n = 1; n <= length; n++) {
                int nx = x + e->facing_dx;
                int ny = y + e->facing_dy;
                if (!map_is_walkable(&g->map, nx, ny) || occupied(g->enemies, g->enemy_count, nx, ny)) {
                    break;
                }
                x = nx;
                y = ny;
            }
            if (x != e->attack_target_x || y != e->attack_target_y) {
                shots->shots[shots->count++] = (EnemyProjectile){e->type, e->attack_target_x, e->attack_target_y, x, y};
            }
        }
        e->attack_target_x = -1;
        e->attack_target_y = -1;
        e->move_timer = -1;
        e->boss_wave_used++;
        push_message(g, "The Guardian's declared strike resolves. One full counterattack action!");
        int damage = e->attack + (e->attack_phase ? 5 : 0) - g->player.defense / 2;
        if (damage < 6) {
            damage = 6;
        }
        // Alignment changes affect protection, never the announced footprint.
        if (g->temple_alignment == (e->boss_attack == GUARDIAN_LUNAR ? 0 : 1)) {
            damage /= 2;
        }
        return hit ? damage : 0;
    }
    int phase = e->hp <= e->max_hp / 4 ? 2 : e->hp <= e->max_hp / 2 ? 1 : 0;
    if (phase > e->attack_phase) {
        e->attack_phase = phase;
        push_message(g, phase == 2 ? "Eclipse! Solar lanes alternate with lunar bursts. The next alignment is announced." :
            "Cracked Core! Two-action sunburst rings join the lances. Match sanctuaries to the announced element.");
    }
    e->boss_attack = e->attack_phase == 2 ? (e->boss_wave_used % 2 ? GUARDIAN_LUNAR : GUARDIAN_LANES) :
        e->boss_wave_used % 2 ? (e->attack_phase == 1 ? GUARDIAN_RING : GUARDIAN_LUNAR) : GUARDIAN_LANCE;
    if (!sanctuary(g->player.x, g->player.y)) {
        e->boss_attack = GUARDIAN_LANCE;
    }
    // Aim at diagonal attackers and counter a refuge only before declaring.
    if ((e->boss_attack == GUARDIAN_LANCE && g->player.x != e->x && g->player.y != e->y) ||
        sun_guardian_sanctuary_protects(g, e, g->player.x, g->player.y)) {
        e->boss_attack = e->boss_attack == GUARDIAN_LUNAR ? GUARDIAN_LANES : GUARDIAN_LUNAR;
    }
    e->move_timer = e->boss_attack == GUARDIAN_LANCE && e->attack_phase < 2 ? 1 : 2;
    if (e->boss_attack == GUARDIAN_LANCE) {
        declare_lance(g, e);
    } else {
        e->attack_target_x = e->boss_attack == GUARDIAN_RING ? e->x : g->player.x;
        e->attack_target_y = e->boss_attack == GUARDIAN_RING ? e->y : g->player.y;
    }
    if (!can_escape(g, e, g->player.x, g->player.y, e->move_timer)) {
        e->attack_target_x = -1;
        e->attack_target_y = -1;
        push_message(g, "The Guardian's light fades: no open walking escape.");
    } else {
        push_message(g, e->boss_attack == GUARDIAN_LUNAR ? "LUNAR BURST: 2 actions! Sun sanctuaries are safe; Sun alignment halves damage." :
            e->boss_attack == GUARDIAN_LANES ? "SOLAR LANES: 2 actions! Moon sanctuaries are safe; Moon alignment halves damage." :
            e->boss_attack == GUARDIAN_RING ? "SUNBURST RING: 2 actions! Leave the marked ring; Moon sanctuaries are safe." :
            e->attack_phase == 2 ? "SOLAR LANCE: 2 actions! Leave its fixed route; Moon alignment halves damage." :
            "SOLAR LANCE: 1 action! Leave its fixed route; Moon alignment halves damage.");
    }
    return 0;
}

void sun_guardian_restore_reward(GameState *g) {
    if (!summit(g)) {
        return;
    }
    for (int y = 7; y <= 11; y++) {
        for (int x = 22; x <= 42; x++) {
            if (tile_at(g, x, y) == TILE_GUARDIAN_RELIQUARY) {
                if (g->sun_guardian_reward_state == 1) {
                    return;
                }
                set_tile(g, x, y, TILE_TEMPLE_FLOOR);
            }
        }
    }
    if (g->sun_guardian_reward_state != 1) {
        return;
    }
    int bx = -1;
    int by = -1;
    int best = MAP_W + MAP_H;
    for (int y = 7; y <= 11; y++) {
        for (int x = 22; x <= 42; x++) {
            int distance = abs(x - 32) + abs(y - 9);
            if (tile_at(g, x, y) == TILE_TEMPLE_FLOOR && !occupied(g->enemies, g->enemy_count, x, y) && distance < best) {
                bx = x;
                by = y;
                best = distance;
            }
        }
    }
    if (bx >= 0) {
        set_tile(g, bx, by, TILE_GUARDIAN_RELIQUARY);
    }
}

static int claim_reward(GameState *g) {
    if (g->inventory_count >= MAX_INVENTORY) {
        push_message(g, "Inventory full. Sunwarden Mantle waits in the summit reliquary.");
        return 0;
    }
    Item mantle = item_make_sunwarden_mantle();
    g->inventory[g->inventory_count++] = mantle;
    g->score += mantle.value;
    g->sun_guardian_reward_state = 2;
    sun_guardian_restore_reward(g);
    push_message(g, "Sunwarden Mantle received! The Buried Sun remains Nahla's separate quest.");
    return 1;
}

void sun_guardian_award_reward(GameState *g) {
    if (g->sun_guardian_reward_state) {
        return;
    }
    g->sun_guardian_reward_state = 1;
    if (!claim_reward(g)) {
        sun_guardian_restore_reward(g);
    }
}

static int nearby_cache(const GameState *g, int pickup) {
    if (!summit(g) || g->sun_guardian_reward_state != 1) {
        return 0;
    }
    static const int offsets[5][2] = {{0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int i = 0; i < (pickup ? 1 : 5); i++) {
        if (tile_at(g, g->player.x + offsets[i][0], g->player.y + offsets[i][1]) == TILE_GUARDIAN_RELIQUARY) {
            return 1;
        }
    }
    return 0;
}

int sun_guardian_has_interaction(const GameState *g) {
    return nearby_cache(g, 0);
}

int sun_guardian_interact(GameState *g, int pickup) {
    if (!nearby_cache(g, pickup)) {
        return 0;
    }
    claim_reward(g);
    return 1;
}

void sun_guardian_cancel_landing(GameState *g, int x, int y) {
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        int threatens = sun_guardian_attack_marks(g, e, x, y);
        if (summit(g) && e->active && e->type == ENEMY_FALLEN_SUN_GUARDIAN &&
            e->move_timer > 0 && e->boss_attack == GUARDIAN_LANCE && e->attack_target_x >= 0) {
            // A temporary blocker may move earlier in the arrival's enemy phase.
            int dx = x - e->attack_target_x;
            int dy = y - e->attack_target_y;
            int step = dx * e->facing_dx + dy * e->facing_dy;
            int length = abs(e->boss_aux_x - e->attack_target_x) + abs(e->boss_aux_y - e->attack_target_y);
            threatens |= step > 0 && step <= length && dx == step * e->facing_dx && dy == step * e->facing_dy;
        }
        if (threatens) {
            e->attack_target_x = -1;
            e->attack_target_y = -1;
            e->move_timer = -1;
        }
    }
}

static void migrate_enemies(Enemy *enemies, int count) {
    for (int i = 0; i < count; i++) {
        Enemy *e = &enemies[i];
        if (e->type == ENEMY_FALLEN_SUN_GUARDIAN) {
            e->move_timer = 0;
            e->attack_phase = 0;
            e->boss_attack = 0;
            e->boss_wave_used = 0;
            e->attack_target_x = -1;
            e->attack_target_y = -1;
        }
    }
}

static void migrate_map(const GameState *g, Map *m, Enemy *enemies, int count, int px, int py) {
    migrate_enemies(enemies, count);
    int x = g->portal_x;
    int y = g->portal_y;
    int protect = g->portal_active && g->portal_location == LOCATION_TEMPLE && g->portal_level == TEMPLE_DEPTH &&
        x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && m->tiles[y][x] == TILE_TEMPLE_FLOOR;
    if (protect) {
        m->tiles[y][x] = TILE_PORTAL;
    }
    sun_guardian_prepare_map(m, enemies, count, px, py);
    if (protect) {
        m->tiles[y][x] = TILE_TEMPLE_FLOOR;
    }
}

void sun_guardian_migrate(GameState *g) {
    if (summit(g)) {
        migrate_map(g, &g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    LevelCache *cache = &g->temple_cache[TEMPLE_DEPTH - 1];
    if (cache->valid) {
        migrate_map(g, &cache->map, cache->enemies, cache->enemy_count, -1, -1);
    }
    // Previous victories keep their treasure and never receive a replacement mantle.
    g->sun_guardian_reward_state = 0;
}
