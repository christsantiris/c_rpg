#include "swamp_demon.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int in_chamber(const Map *m, int x, int y) {
    if (m->room_count < 1) {
        return 0;
    }
    const Room *room = &m->rooms[m->room_count - 1];
    return x > room->x && x < room->x + room->w - 1 &&
        y > room->y && y < room->y + room->h - 1;
}

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
    return g->location == LOCATION_SWAMP && g->level == SWAMP_BOSS_LEVEL && g->map.room_count > 0;
}

// Vents and rot clocks live in map tiles, including cached maps and loot underlays.
int swamp_demon_rot_turns(TileType tile) {
    return tile == TILE_DEMON_ROT_2 ? 2 : tile == TILE_DEMON_ROT_1 ? 1 : 0;
}

int swamp_demon_vent_turns(TileType tile) {
    return tile >= TILE_DEMON_VENT_1 && tile <= TILE_DEMON_VENT_5 ? tile - TILE_DEMON_VENT_1 + 1 : 0;
}

void swamp_demon_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py) {
    const Enemy *demon = NULL;
    for (int i = 0; i < count; i++) {
        if (enemies[i].active && enemies[i].type == ENEMY_SWAMP_DEMON) {
            demon = &enemies[i];
        }
    }
    if (!demon || !m->room_count) {
        return;
    }
    const Room *room = &m->rooms[m->room_count - 1];
    for (int y = room->y; y < room->y + room->h; y++) {
        for (int x = room->x; x < room->x + room->w; x++) {
            if (m->tiles[y][x] >= TILE_DEMON_VENT && m->tiles[y][x] <= TILE_DEMON_VENT_5) {
                return;
            }
            if (m->tiles[y][x] == TILE_TRAP_HIDDEN) {
                m->tiles[y][x] = TILE_SWAMP_FLOOR;
            }
        }
    }
    for (int n = 0; n < 3; n++) {
        int tx = demon->x + (n == 0 ? -3 : n == 1 ? 3 : 0);
        int ty = demon->y + (n == 2 ? -3 : 2);
        int bx = -1;
        int by = -1;
        int distance = MAP_W + MAP_H;
        for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
            for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
                int d = abs(x - tx) + abs(y - ty);
                if (m->tiles[y][x] == TILE_SWAMP_FLOOR && !occupied(enemies, count, x, y) &&
                    !(x == px && y == py) && d < distance) {
                    bx = x;
                    by = y;
                    distance = d;
                }
            }
        }
        if (bx >= 0) {
            m->tiles[by][bx] = TILE_DEMON_VENT;
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

static int pocket(int cx, int cy, int x, int y) {
    return cx >= 0 && cy >= 0 && abs(cx - x) + abs(cy - y) <= 1;
}

int swamp_demon_attack_marks(const GameState *g, const Enemy *e, int x, int y) {
    if (!e->active || e->type != ENEMY_SWAMP_DEMON || e->move_timer <= 0 ||
        e->attack_target_x < 0 || !map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    if (e->boss_attack == DEMON_POOL) {
        return pocket(e->attack_target_x, e->attack_target_y, x, y) || pocket(e->boss_aux_x, e->boss_aux_y, x, y);
    }
    int dx = e->attack_target_x - e->x;
    int dy = e->attack_target_y - e->y;
    int forward = (x - e->x) * dx + (y - e->y) * dy;
    int side = abs((x - e->x) * dy - (y - e->y) * dx);
    return e->boss_attack == DEMON_CLEAVE ? forward == 1 && side <= 1 :
        e->boss_attack == DEMON_FAN && forward >= 1 && forward <= 3 && side <= (forward == 3 ? 2 : 1);
}

static void clear_targets(Enemy *e) {
    e->attack_target_x = -1;
    e->attack_target_y = -1;
    e->boss_aux_x = -1;
    e->boss_aux_y = -1;
}

// A rejected warning becomes an empty swing, never an unavoidable hit in a narrow causeway.
static int can_escape(const GameState *g, const Enemy *e, int x, int y, int turns) {
    if (!swamp_demon_attack_marks(g, e, x, y) && !swamp_demon_rot_turns(tile_at(g, x, y))) {
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

int swamp_demon_tick_floor(GameState *g) {
    if (!arena(g)) {
        return 0;
    }
    int hit = swamp_demon_rot_turns(tile_at(g, g->player.x, g->player.y));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = tile_at(g, x, y);
            int rot = swamp_demon_rot_turns(tile);
            int vent = swamp_demon_vent_turns(tile);
            if (rot) {
                set_tile(g, x, y, rot == 2 ? TILE_DEMON_ROT_1 : TILE_SWAMP_FLOOR);
            } else if (vent) {
                set_tile(g, x, y, vent == 1 ? TILE_DEMON_VENT : tile - 1);
            }
        }
    }
    if (hit) {
        push_message_kind(g, "Lingering rot burns your feet! Move off the numbered pools.", MESSAGE_DAMAGE_TAKEN);
    }
    return hit ? 8 : 0;
}

int swamp_demon_tick(GameState *g, Enemy *e) {
    if (!arena(g)) {
        return 0;
    }
    int nearby = abs(g->player.x - e->x) <= 12 && abs(g->player.y - e->y) <= 12;
    if (!in_chamber(&g->map, g->player.x, g->player.y) &&
        (!nearby || (e->hp == e->max_hp && !e->boss_attack))) {
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
        int hit = swamp_demon_attack_marks(g, e, g->player.x, g->player.y);
        if (e->boss_attack == DEMON_POOL) {
            for (int y = 0; y < MAP_H; y++) {
                for (int x = 0; x < MAP_W; x++) {
                    if (tile_at(g, x, y) == TILE_SWAMP_FLOOR && swamp_demon_attack_marks(g, e, x, y)) {
                        set_tile(g, x, y, TILE_DEMON_ROT_2);
                    }
                }
            }
        }
        push_message(g, e->boss_attack == DEMON_POOL ? "Rot pools erupt! Numbered rot lasts two more actions." :
            e->boss_attack == DEMON_CLEAVE ? "The Demon cleaves the marked front!" : "The miasma fan washes over the marked ground!");
        clear_targets(e);
        e->move_timer = -1;
        int damage = e->attack - g->player.defense / 2;
        return hit ? (damage > 4 ? damage : 4) : 0;
    }
    if (!e->attack_phase && e->hp <= e->max_hp / 2) {
        e->attack_phase = 1;
        push_message(g, "The Swamp Demon is unshackled! Miasma fans and twin rot pockets join its attacks.");
    }
    e->boss_attack = e->boss_attack == DEMON_CLEAVE ? DEMON_POOL :
        e->boss_attack == DEMON_POOL && e->attack_phase ? DEMON_FAN : DEMON_CLEAVE;
    clear_targets(e);
    e->move_timer = e->boss_attack == DEMON_CLEAVE ? 1 : 2;
    if (line_clear(g, e, g->player.x, g->player.y)) {
        int dx = g->player.x - e->x;
        int dy = g->player.y - e->y;
        if (e->boss_attack == DEMON_POOL) {
            e->attack_target_x = g->player.x;
            e->attack_target_y = g->player.y;
            if (e->attack_phase) {
                const Room *room = &g->map.rooms[g->map.room_count - 1];
                int distance = MAP_W + MAP_H;
                int tx = g->player.x + (dx < 0 ? 4 : -4);
                for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
                    for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
                        int d = abs(x - tx) + abs(y - g->player.y);
                        if (tile_at(g, x, y) == TILE_SWAMP_FLOOR &&
                            abs(x - g->player.x) + abs(y - g->player.y) >= 4 && d < distance) {
                            e->boss_aux_x = x;
                            e->boss_aux_y = y;
                            distance = d;
                        }
                    }
                }
            }
        } else {
            e->attack_target_x = e->x + (abs(dx) >= abs(dy) ? (dx < 0 ? -1 : 1) : 0);
            e->attack_target_y = e->y + (abs(dx) < abs(dy) ? (dy < 0 ? -1 : 1) : 0);
        }
        if (!can_escape(g, e, g->player.x, g->player.y, e->move_timer)) {
            clear_targets(e);
        }
    }
    push_message(g, e->boss_attack == DEMON_CLEAVE ? "Frontal cleave in 1 turn! Step sideways or back." :
        e->boss_attack == DEMON_POOL ? "Rot pools in 2 turns! Leave the marked crosses." : "Miasma fan in 2 turns! Leave the marked wedge.");
    return 0;
}

void swamp_demon_restore_reward(GameState *g) {
    if (g->location != LOCATION_SWAMP || g->level != SWAMP_BOSS_LEVEL || g->map.room_count < 1 || !(g->defeated_bosses & (1 << LOCATION_SWAMP))) {
        return;
    }
    int floor_sword = 0;
    for (int i = 0; i < g->floor_item_count; i++) {
        floor_sword |= g->floor_items[i].active && strcmp(g->floor_items[i].item.name, "Demonic Sword") == 0;
    }
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    int cache = 0;
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            if (tile_at(g, x, y) == TILE_DEMON_RELIQUARY) {
                if (g->swamp_demon_reward_state != 1 || floor_sword) {
                    set_tile(g, x, y, TILE_SWAMP_FLOOR);
                } else {
                    cache = 1;
                }
            }
        }
    }
    if (g->swamp_demon_reward_state != 1 || floor_sword || cache) {
        return;
    }
    for (int y = room->y + room->h - 2; y > room->y; y--) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            if (g->map.tiles[y][x] == TILE_SWAMP_FLOOR && !occupied(g->enemies, g->enemy_count, x, y)) {
                g->map.tiles[y][x] = TILE_DEMON_RELIQUARY;
                push_message(g, "Unclaimed Demonic Sword waits in the Demon's root cache. Interact beside it or press P on it.");
                return;
            }
        }
    }
}

static TileType interaction_at(const GameState *g, int *x, int *y) {
    if (!arena(g)) {
        return TILE_SWAMP_FLOOR;
    }
    for (int radius = 0; radius <= 1; radius++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (abs(dx) + abs(dy) != radius) {
                    continue;
                }
                TileType tile = tile_at(g, g->player.x + dx, g->player.y + dy);
                if ((tile >= TILE_DEMON_VENT && tile <= TILE_DEMON_VENT_5) ||
                    (tile == TILE_DEMON_RELIQUARY && g->swamp_demon_reward_state == 1)) {
                    *x = g->player.x + dx;
                    *y = g->player.y + dy;
                    return tile;
                }
            }
        }
    }
    return TILE_SWAMP_FLOOR;
}

int swamp_demon_has_interaction(const GameState *g) {
    int x;
    int y;
    return interaction_at(g, &x, &y) != TILE_SWAMP_FLOOR;
}

int swamp_demon_interact(GameState *g) {
    int x;
    int y;
    TileType tile = interaction_at(g, &x, &y);
    if (tile == TILE_SWAMP_FLOOR) {
        return 0;
    }
    if (tile == TILE_DEMON_RELIQUARY) {
        if (g->inventory_count >= MAX_INVENTORY) {
            push_message(g, "Inventory full. Demonic Sword remains safe in the root cache.");
        } else {
            Item reward = item_make_demonic_sword();
            g->inventory[g->inventory_count++] = reward;
            g->score += reward.value;
            g->swamp_demon_reward_state = 2;
            set_tile(g, x, y, TILE_SWAMP_FLOOR);
            push_message(g, "Demonic Sword recovered from the Demon's root cache!");
        }
        return 1;
    }
    if (swamp_demon_vent_turns(tile)) {
        push_message(g, "The root vent is recovering. Its number shows actions until ready.");
        return 1;
    }
    set_tile(g, x, y, TILE_DEMON_VENT_5);
    for (int yy = y - 2; yy <= y + 2; yy++) {
        for (int xx = x - 2; xx <= x + 2; xx++) {
            if (abs(xx - x) + abs(yy - y) <= 2 && swamp_demon_rot_turns(tile_at(g, xx, yy))) {
                set_tile(g, xx, yy, TILE_SWAMP_FLOOR);
            }
        }
    }
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (e->active && e->type == ENEMY_SWAMP_DEMON) {
            clear_targets(e);
            // This interaction consumes the first tick; two full counterattacks remain.
            e->move_timer = -3;
        }
    }
    push_message(g, "The root vent drains nearby rot! Demon exposed for 2 actions; vent recovers in 4.");
    return 1;
}

static void prepare_saved_map(const GameState *g, Map *m, const Enemy *enemies, int count, int px, int py) {
    int x = g->portal_x;
    int y = g->portal_y;
    int protect = g->portal_active && g->portal_location == LOCATION_SWAMP &&
        g->portal_level == SWAMP_BOSS_LEVEL && x >= 0 && x < MAP_W && y >= 0 && y < MAP_H &&
        m->tiles[y][x] == TILE_SWAMP_FLOOR;
    TileType original = protect ? m->tiles[y][x] : TILE_SWAMP_FLOOR;
    // Hidden portal landings stay traversable even when their marker is absent.
    if (protect) {
        m->tiles[y][x] = TILE_PORTAL;
    }
    swamp_demon_prepare_map(m, enemies, count, px, py);
    if (protect) {
        m->tiles[y][x] = original;
    }
}

void swamp_demon_migrate(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (e->type == ENEMY_SWAMP_DEMON) {
            e->move_timer = 0;
            e->attack_phase = 0;
            e->boss_attack = 0;
            e->boss_wave_used = 0;
            clear_targets(e);
        }
    }
    LevelCache *cache = &g->swamp_cache[SWAMP_BOSS_LEVEL - 1];
    if (cache->valid) {
        for (int i = 0; i < cache->enemy_count; i++) {
            Enemy *e = &cache->enemies[i];
            if (e->type == ENEMY_SWAMP_DEMON) {
                e->move_timer = 0;
                e->attack_phase = 0;
                e->boss_attack = 0;
                e->boss_wave_used = 0;
                clear_targets(e);
            }
        }
        prepare_saved_map(g, &cache->map, cache->enemies, cache->enemy_count, -1, -1);
    }
    if (g->location == LOCATION_SWAMP && g->level == SWAMP_BOSS_LEVEL) {
        prepare_saved_map(g, &g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    // Only known, currently dropped legacy rewards establish an unclaimed gift.
    if (g->defeated_bosses & (1 << LOCATION_SWAMP)) {
        g->swamp_demon_reward_state = 2;
        int known_claim = 0;
        for (int i = 0; i < g->inventory_count; i++) {
            known_claim |= strcmp(g->inventory[i].name, "Demonic Sword") == 0;
        }
        if (g->location == LOCATION_SWAMP && g->level == SWAMP_BOSS_LEVEL) {
            for (int i = 0; i < g->floor_item_count; i++) {
                if (!known_claim && g->floor_items[i].active &&
                    strcmp(g->floor_items[i].item.name, "Demonic Sword") == 0 && !g->floor_items[i].item.sharpened) {
                    g->swamp_demon_reward_state = 1;
                }
            }
        }
    }
}
