#include "minotaur.h"
#include <stdlib.h>
#include <string.h>

static TileType tile_at(const GameState *g, int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
        return TILE_WALL;
    }
    if (g->map.tiles[y][x] == TILE_ITEM) {
        for (int i = 0; i < g->floor_item_count; i++) {
            const FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == x && item->y == y) {
                return (TileType)item->underlying_tile;
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
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            item->underlying_tile = tile;
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

static int arena(const GameState *g) {
    return g->location == LOCATION_LABYRINTH && g->level == LABYRINTH_DEPTH;
}

static int pillar(TileType tile) {
    return tile == TILE_MINOTAUR_PILLAR || tile == TILE_MINOTAUR_CRACKED;
}

void minotaur_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py) {
    int living = 0;
    for (int i = 0; i < count; i++) {
        living |= enemies[i].active && enemies[i].type == ENEMY_MINOTAUR;
    }
    int cy = m->stairs_up_y;
    if (!living || cy < 3 || cy >= LABYRINTH_H - 3) {
        return;
    }
    for (int y = cy - 2; y <= cy + 2; y++) {
        for (int x = 37; x <= 41; x++) {
            if (m->tiles[y][x] >= TILE_MINOTAUR_PILLAR && m->tiles[y][x] <= TILE_MINOTAUR_BROKEN) {
                return;
            }
        }
    }
    for (int n = 0; n < 2; n++) {
        int tx = n ? 40 : 38;
        int ty = cy + (n ? 1 : -1);
        int bx = -1;
        int by = -1;
        int distance = MAP_W + MAP_H;
        for (int y = cy - 2; y <= cy + 2; y++) {
            for (int x = 37; x <= 41; x++) {
                int d = abs(x - tx) + abs(y - ty);
                if (y != cy && m->tiles[y][x] == TILE_LABYRINTH_FLOOR &&
                    !occupied(enemies, count, x, y) && !(x == px && y == py) && d < distance) {
                    bx = x;
                    by = y;
                    distance = d;
                }
            }
        }
        if (bx >= 0) {
            m->tiles[by][bx] = TILE_MINOTAUR_PILLAR;
        }
    }
}

static int line_clear(const GameState *g, const Enemy *e, int tx, int ty) {
    int x = e->x;
    int y = e->y;
    int dx = abs(tx - x);
    int dy = -abs(ty - y);
    int sx = x < tx ? 1 : -1;
    int sy = y < ty ? 1 : -1;
    int error = dx + dy;
    for (int n = 0; n < MAP_W + MAP_H; n++) {
        if (x == tx && y == ty) {
            return 1;
        }
        int ox = x;
        int oy = y;
        int twice = error * 2;
        if (twice >= dy) {
            error += dy;
            x += sx;
        }
        if (twice <= dx) {
            error += dx;
            y += sy;
        }
        if (!map_is_walkable(&g->map, x, y) || occupied(g->enemies, g->enemy_count, x, y) ||
            (x != ox && y != oy && (!map_is_walkable(&g->map, x, oy) || !map_is_walkable(&g->map, ox, y)))) {
            return 0;
        }
    }
    return 0;
}

int minotaur_attack_marks(const GameState *g, const Enemy *e, int x, int y) {
    if (!e->active || e->type != ENEMY_MINOTAUR || e->move_timer <= 0 ||
        e->attack_target_x < 0 || !map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    int dx = e->facing_dx;
    int dy = e->facing_dy;
    int forward = (x - e->x) * dx + (y - e->y) * dy;
    int side = abs((x - e->x) * dy - (y - e->y) * dx);
    if (e->boss_attack == MINOTAUR_CHARGE) {
        int length = abs(e->attack_target_x - e->x) + abs(e->attack_target_y - e->y);
        return side == 0 && forward >= 1 && forward <= length;
    }
    if (e->boss_attack == MINOTAUR_AXE) {
        return forward == 1 && side <= 1;
    }
    return e->boss_attack == MINOTAUR_WIDE_AXE && forward >= 0 && forward <= 1 &&
        side <= 2 && (forward != 0 || side != 0);
}

static int can_escape(const GameState *g, const Enemy *e, int x, int y, int turns) {
    if (!minotaur_attack_marks(g, e, x, y)) {
        return 1;
    }
    if (!turns) {
        return 0;
    }
    static const int steps[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (int n = 0; n < 4; n++) {
        int nx = x + steps[n][0];
        int ny = y + steps[n][1];
        if (map_is_walkable(&g->map, nx, ny) && !occupied(g->enemies, g->enemy_count, nx, ny) &&
            can_escape(g, e, nx, ny, turns - 1)) {
            return 1;
        }
    }
    return 0;
}

static void clear_targets(Enemy *e) {
    e->attack_target_x = -1;
    e->attack_target_y = -1;
    e->boss_aux_x = -1;
    e->boss_aux_y = -1;
}

static int charge_floor(const GameState *g, int x, int y) {
    TileType tile = tile_at(g, x, y);
    return x >= 0 && x < LABYRINTH_W && y >= 0 && y < LABYRINTH_H &&
        (tile == TILE_LABYRINTH_FLOOR || tile == TILE_MINOTAUR_BROKEN);
}

static void prepare_charge(GameState *g, Enemy *e) {
    e->attack_target_x = e->x;
    e->attack_target_y = e->y;
    for (int step = 1; step <= 6; step++) {
        int x = e->x + e->facing_dx * step;
        int y = e->y + e->facing_dy * step;
        if (!charge_floor(g, x, y) || occupied(g->enemies, g->enemy_count, x, y)) {
            TileType tile = tile_at(g, x, y);
            if (tile == TILE_LABYRINTH_WALL || pillar(tile)) {
                e->boss_aux_x = x;
                e->boss_aux_y = y;
            }
            break;
        }
        e->attack_target_x = x;
        e->attack_target_y = y;
    }
    // A single-file corridor still allows a normal step back: stop at the aimed tile.
    if (!can_escape(g, e, g->player.x, g->player.y, 1) &&
        minotaur_attack_marks(g, e, g->player.x, g->player.y)) {
        e->attack_target_x = g->player.x;
        e->attack_target_y = g->player.y;
        e->boss_aux_x = -1;
        e->boss_aux_y = -1;
    }
}

static int resolve_charge(GameState *g, Enemy *e) {
    int marked = minotaur_attack_marks(g, e, g->player.x, g->player.y);
    int length = abs(e->attack_target_x - e->x) + abs(e->attack_target_y - e->y);
    int hit = 0;
    for (int step = 0; step < length; step++) {
        int nx = e->x + e->facing_dx;
        int ny = e->y + e->facing_dy;
        if (!charge_floor(g, nx, ny) || occupied(g->enemies, g->enemy_count, nx, ny)) {
            break;
        }
        if (g->player.x == nx && g->player.y == ny) {
            hit = marked;
            break;
        }
        e->x = nx;
        e->y = ny;
    }
    int impact = e->boss_aux_x >= 0 && e->x == e->attack_target_x && e->y == e->attack_target_y;
    TileType obstacle = impact ? tile_at(g, e->boss_aux_x, e->boss_aux_y) : TILE_LABYRINTH_FLOOR;
    impact &= obstacle == TILE_LABYRINTH_WALL || pillar(obstacle);
    e->move_timer = impact ? -2 : -1;
    if (impact && pillar(obstacle)) {
        int breaks = obstacle == TILE_MINOTAUR_CRACKED && e->attack_phase;
        set_tile(g, e->boss_aux_x, e->boss_aux_y, breaks ? TILE_MINOTAUR_BROKEN : TILE_MINOTAUR_CRACKED);
        push_message(g, breaks ? "The warned pillar cap breaks! A flank opens; Minotaur staggered for 2 actions." :
            "The reinforced pillar cracks! A warned phase-two impact can break it. Staggered for 2 actions.");
    } else {
        push_message(g, impact ? "The Minotaur strikes solid stone! Staggered for 2 actions." : "The Minotaur charges its fixed marked lane!");
    }
    return hit;
}

int minotaur_tick(GameState *g, Enemy *e) {
    if (!arena(g)) {
        return 0;
    }
    int distance = abs(g->player.x - e->x) > abs(g->player.y - e->y) ? abs(g->player.x - e->x) : abs(g->player.y - e->y);
    int clear = line_clear(g, e, g->player.x, g->player.y);
    if (distance > 12 || (e->hp == e->max_hp && !e->boss_attack && (distance > 8 || !clear))) {
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
        int hit = 0;
        if (e->attack_target_x >= 0) {
            if (e->boss_attack == MINOTAUR_CHARGE) {
                hit = resolve_charge(g, e);
            } else {
                hit = minotaur_attack_marks(g, e, g->player.x, g->player.y);
                e->move_timer = -1;
                push_message(g, e->boss_attack == MINOTAUR_WIDE_AXE ? "The bloodied axe sweeps its fixed wide arc!" : "The Minotaur sweeps its marked axe front!");
            }
        } else {
            e->move_timer = -1;
            push_message(g, "The Minotaur's empty swing leaves a recovery opening.");
        }
        if (e->attack_phase) {
            e->boss_wave_used = e->boss_attack == MINOTAUR_CHARGE ? e->boss_wave_used + 1 : 0;
        }
        clear_targets(e);
        int damage = e->attack - g->player.defense;
        return hit ? (damage > 1 ? damage : 1) : 0;
    }
    if (!e->attack_phase && e->hp <= e->max_hp / 2) {
        e->attack_phase = 1;
        e->boss_wave_used = 0;
        push_message(g, "Bloodied Bull! Two separately warned charges, then a wide axe sweep.");
    }
    int dx = g->player.x - e->x;
    int dy = g->player.y - e->y;
    e->facing_dx = abs(dx) >= abs(dy) ? (dx < 0 ? -1 : 1) : 0;
    e->facing_dy = e->facing_dx == 0 ? (dy < 0 ? -1 : 1) : 0;
    clear_targets(e);
    e->boss_attack = e->attack_phase ? (e->boss_wave_used < 2 ? MINOTAUR_CHARGE : MINOTAUR_WIDE_AXE) :
        (abs(dx) <= 1 && abs(dy) <= 1 ? MINOTAUR_AXE : MINOTAUR_CHARGE);
    e->move_timer = e->boss_attack == MINOTAUR_WIDE_AXE ? 2 : 1;
    if (clear) {
        if (e->boss_attack == MINOTAUR_CHARGE) {
            prepare_charge(g, e);
        } else {
            e->attack_target_x = e->x + e->facing_dx;
            e->attack_target_y = e->y + e->facing_dy;
        }
        if (!can_escape(g, e, g->player.x, g->player.y, e->move_timer)) {
            clear_targets(e);
        }
    }
    push_message(g, e->boss_attack == MINOTAUR_CHARGE ? "Charge in 1 action! Leave the fixed lane; bait stone for a stagger." :
        e->boss_attack == MINOTAUR_AXE ? "Axe front in 1 action! Step back from the marked sweep." : "Wide axe in 2 actions! Leave the marked arc.");
    if (e->boss_aux_x >= 0 && tile_at(g, e->boss_aux_x, e->boss_aux_y) == TILE_MINOTAUR_PILLAR) {
        push_message(g, "Reinforced pillar cap will CRACK on this marked charge!");
    }
    if (e->boss_aux_x >= 0 && tile_at(g, e->boss_aux_x, e->boss_aux_y) == TILE_MINOTAUR_CRACKED && e->attack_phase) {
        push_message(g, "Cracked pillar cap will BREAK on this marked charge!");
    }
    return 0;
}

static int pending_reward(const GameState *g) {
    return g->minotaur_reward_state == MINOTAUR_REWARD_AEGIS || g->minotaur_reward_state == MINOTAUR_REWARD_LEGACY;
}

static const char *reward_name(const GameState *g) {
    return g->minotaur_reward_state == MINOTAUR_REWARD_LEGACY ? "Magic Shield" : "Labyrinth Aegis";
}

void minotaur_restore_reward(GameState *g) {
    if (!arena(g) || !(g->defeated_bosses & (1 << LOCATION_LABYRINTH))) {
        return;
    }
    int floor_reward = 0;
    for (int i = 0; i < g->floor_item_count; i++) {
        floor_reward |= g->floor_items[i].active && strcmp(g->floor_items[i].item.name, reward_name(g)) == 0;
    }
    int cache = 0;
    for (int y = 0; y < LABYRINTH_H; y++) {
        for (int x = 0; x < LABYRINTH_W; x++) {
            if (tile_at(g, x, y) == TILE_MINOTAUR_RELIQUARY) {
                if (!pending_reward(g) || floor_reward) {
                    set_tile(g, x, y, TILE_LABYRINTH_FLOOR);
                } else {
                    cache = 1;
                }
            }
        }
    }
    if (!pending_reward(g) || floor_reward || cache) {
        return;
    }
    int cy = g->map.stairs_up_y;
    // Prefer uncovered floor, then reserve beneath loot without deleting an item.
    for (int covered = 0; covered <= 1; covered++) {
        for (int y = cy + 2; y >= cy - 2; y--) {
            for (int x = 37; x <= 41; x++) {
                if (y >= 0 && y < LABYRINTH_H && tile_at(g, x, y) == TILE_LABYRINTH_FLOOR &&
                    (covered || g->map.tiles[y][x] != TILE_ITEM) && !occupied(g->enemies, g->enemy_count, x, y)) {
                    set_tile(g, x, y, TILE_MINOTAUR_RELIQUARY);
                    push_message(g, "Your unclaimed shield waits in the vault's bull-marked cache. Open with Interact, then Pick Up the released loot.");
                    return;
                }
            }
        }
    }
}

static int cache_nearby(const GameState *g, int *x, int *y) {
    if (!arena(g) || !pending_reward(g)) {
        return 0;
    }
    static const int offsets[5][2] = {{0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int n = 0; n < 5; n++) {
        *x = g->player.x + offsets[n][0];
        *y = g->player.y + offsets[n][1];
        if (tile_at(g, *x, *y) == TILE_MINOTAUR_RELIQUARY) {
            return 1;
        }
    }
    return 0;
}

int minotaur_has_interaction(const GameState *g) {
    int x;
    int y;
    return cache_nearby(g, &x, &y);
}

int minotaur_interact(GameState *g) {
    int x;
    int y;
    if (!cache_nearby(g, &x, &y)) {
        return 0;
    }
    if (g->inventory_count >= MAX_INVENTORY) {
        push_message(g, "Inventory full. Your shield remains safe in the vault cache.");
    } else {
        Item reward = g->minotaur_reward_state == MINOTAUR_REWARD_LEGACY ? item_make_magic_shield() : item_make_labyrinth_aegis();
        g->inventory[g->inventory_count++] = reward;
        g->score += reward.value;
        g->minotaur_reward_state = MINOTAUR_REWARD_CLAIMED;
        set_tile(g, x, y, TILE_LABYRINTH_FLOOR);
        push_message(g, "Shield recovered from the Minotaur's vault cache!");
    }
    return 1;
}

int minotaur_pickup(GameState *g) {
    return arena(g) && tile_at(g, g->player.x, g->player.y) == TILE_MINOTAUR_RELIQUARY && minotaur_interact(g);
}

void minotaur_record_pickup(GameState *g, const Item *item) {
    if (pending_reward(g) && strcmp(item->name, reward_name(g)) == 0) {
        g->minotaur_reward_state = MINOTAUR_REWARD_CLAIMED;
        minotaur_restore_reward(g);
    }
}

static void migrate_enemies(Enemy *enemies, int count) {
    for (int i = 0; i < count; i++) {
        Enemy *e = &enemies[i];
        if (e->type == ENEMY_MINOTAUR) {
            e->move_timer = 0;
            e->attack_phase = 0;
            e->boss_attack = 0;
            e->boss_wave_used = 0;
            clear_targets(e);
        }
    }
}

void minotaur_migrate(GameState *g) {
    migrate_enemies(g->enemies, g->enemy_count);
    LevelCache *cache = &g->labyrinth_cache[LABYRINTH_DEPTH - 1];
    if (cache->valid) {
        migrate_enemies(cache->enemies, cache->enemy_count);
        minotaur_prepare_map(&cache->map, cache->enemies, cache->enemy_count, -1, -1);
    }
    if (arena(g)) {
        minotaur_prepare_map(&g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    if (g->defeated_bosses & (1 << LOCATION_LABYRINTH)) {
        // Future victories receive Aegis; existing Magic Shields keep their identity.
        g->minotaur_reward_state = MINOTAUR_REWARD_CLAIMED;
        int known_claim = 0;
        for (int i = 0; i < g->inventory_count; i++) {
            known_claim |= strcmp(g->inventory[i].name, "Magic Shield") == 0;
        }
        if (arena(g) && !known_claim) {
            for (int i = 0; i < g->floor_item_count; i++) {
                if (g->floor_items[i].active && strcmp(g->floor_items[i].item.name, "Magic Shield") == 0) {
                    g->minotaur_reward_state = MINOTAUR_REWARD_LEGACY;
                }
            }
        }
    }
}
