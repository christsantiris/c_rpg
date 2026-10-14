#include "grave_marshal.h"
#include "catacombs.h"
#include <stdlib.h>
#include <stdio.h>
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

static int chamber(const Map *m, int x, int y) {
    return m->room_count > 0 && catacombs_room_at(m, x, y) == m->room_count - 1;
}

void grave_marshal_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py) {
    int living = 0;
    for (int i = 0; i < count; i++) {
        living |= enemies[i].active && enemies[i].type == ENEMY_GRAVE_MARSHAL;
    }
    if (!living || !m->room_count) {
        return;
    }
    const Room *room = &m->rooms[m->room_count - 1];
    for (int y = room->y; y < room->y + room->h; y++) {
        for (int x = room->x; x < room->x + room->w; x++) {
            if (m->tiles[y][x] == TILE_MARSHAL_NICHE || m->tiles[y][x] == TILE_MARSHAL_NICHE_SPENT) {
                return;
            }
        }
    }
    for (int n = 0; n < 2; n++) {
        int tx = room->x + (n ? room->w - 3 : 2);
        int ty = room->y + (n ? room->h - 3 : 2);
        int bx = -1;
        int by = -1;
        int best = MAP_W + MAP_H;
        for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
            for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
                int distance = abs(x - tx) + abs(y - ty);
                if (m->tiles[y][x] == TILE_CATACOMBS_FLOOR && !occupied(enemies, count, x, y) &&
                    !(x == px && y == py) && distance < best) {
                    bx = x;
                    by = y;
                    best = distance;
                }
            }
        }
        if (bx >= 0) {
            m->tiles[by][bx] = TILE_MARSHAL_NICHE;
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

int grave_marshal_attack_marks(const Map *m, const Enemy *e, int x, int y) {
    if (!e->active || e->type != ENEMY_GRAVE_MARSHAL || e->move_timer <= 0 ||
        e->attack_target_x < 0 || !map_is_walkable(m, x, y)) {
        return 0;
    }
    if (e->boss_attack == MARSHAL_GUARDS) {
        return (x == e->attack_target_x && y == e->attack_target_y) || (x == e->boss_aux_x && y == e->boss_aux_y);
    }
    if (e->boss_attack == MARSHAL_CROSS || e->boss_attack == MARSHAL_CHANNEL) {
        int dx = abs(x - e->attack_target_x);
        int dy = abs(y - e->attack_target_y);
        int radius = e->boss_attack == MARSHAL_CHANNEL ? 3 : 2;
        if ((dx != 0 && dy != 0) || dx + dy > radius) {
            return 0;
        }
        int sx = (x > e->attack_target_x) - (x < e->attack_target_x);
        int sy = (y > e->attack_target_y) - (y < e->attack_target_y);
        for (int step = 0; step <= dx + dy; step++) {
            if (!map_is_walkable(m, e->attack_target_x + sx * step, e->attack_target_y + sy * step)) {
                return 0;
            }
        }
        return 1;
    }
    int dx = e->attack_target_x - e->x;
    int dy = e->attack_target_y - e->y;
    return e->boss_attack == MARSHAL_SWEEP && (dx ? x - e->x == dx && abs(y - e->y) <= 1 :
        y - e->y == dy && abs(x - e->x) <= 1);
}

static void clear_targets(Enemy *e) {
    e->attack_target_x = -1;
    e->attack_target_y = -1;
    e->boss_aux_x = -1;
    e->boss_aux_y = -1;
}

static int can_escape(const GameState *g, const Enemy *e, int x, int y, int turns) {
    if (!grave_marshal_attack_marks(&g->map, e, x, y)) {
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

int grave_marshal_tick(GameState *g, Enemy *e) {
    if (g->location != LOCATION_CATACOMBS || !g->map.room_count) {
        return 0;
    }
    int nearby = abs(g->player.x - e->x) <= 12 && abs(g->player.y - e->y) <= 12;
    if (!chamber(&g->map, g->player.x, g->player.y) && (!nearby || (e->hp == e->max_hp && !e->boss_attack))) {
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
        int hit = e->boss_attack != MARSHAL_GUARDS && grave_marshal_attack_marks(&g->map, e, g->player.x, g->player.y);
        if (e->boss_attack == MARSHAL_GUARDS) {
            int spawned = 0;
            int targets[2][2] = {{e->attack_target_x, e->attack_target_y}, {e->boss_aux_x, e->boss_aux_y}};
            for (int n = 0; n < 2; n++) {
                int x = targets[n][0];
                int y = targets[n][1];
                if (x >= 0 && tile_at(g, x, y) == TILE_MARSHAL_NICHE) {
                    set_tile(g, x, y, TILE_MARSHAL_NICHE_SPENT);
                    if (!(g->player.x == x && g->player.y == y) && !occupied(g->enemies, g->enemy_count, x, y)) {
                        spawned += game_spawn_marshal_guard(g, x, y, (int)(e - g->enemies)) != NULL;
                    }
                }
            }
            char message[MAX_MESSAGE_LEN];
            snprintf(message, sizeof(message), "%d honor guard%s emerge from the marked niches. The only wave is spent; Marshal exposed!", spawned, spawned == 1 ? "" : "s");
            push_message(g, message);
        } else {
            push_message(g, e->boss_attack == MARSHAL_SWEEP ? "The polearm sweeps its fixed marked front!" :
                e->boss_attack == MARSHAL_CROSS ? "The funeral cross sweeps its locked stones!" : "The funeral channel strikes its fixed marked crossing!");
        }
        clear_targets(e);
        e->move_timer = -1;
        int damage = e->attack - g->player.defense;
        return hit ? (damage > 1 ? damage : 1) : 0;
    }
    if (!e->attack_phase && e->hp <= e->max_hp / 2) {
        e->attack_phase = 1;
        push_message(g, "Funeral March! Cross sweeps, a channel and one honor guard wave join the battle.");
    }
    clear_targets(e);
    if (e->attack_phase && !e->boss_wave_used) {
        e->boss_attack = MARSHAL_GUARDS;
        e->boss_wave_used = 1;
        const Room *room = &g->map.rooms[g->map.room_count - 1];
        for (int y = room->y; y < room->y + room->h; y++) {
            for (int x = room->x; x < room->x + room->w; x++) {
                if (tile_at(g, x, y) == TILE_MARSHAL_NICHE && !occupied(g->enemies, g->enemy_count, x, y) &&
                    abs(x - g->player.x) + abs(y - g->player.y) >= 4 && abs(x - e->x) + abs(y - e->y) >= 3) {
                    if (e->attack_target_x < 0) {
                        e->attack_target_x = x;
                        e->attack_target_y = y;
                    } else {
                        e->boss_aux_x = x;
                        e->boss_aux_y = y;
                    }
                }
            }
        }
        e->move_timer = 2;
        push_message(g, "Honor guard in 2 actions! H marks fixed tomb niches; new guards cannot act immediately.");
        return 0;
    }
    if (!e->attack_phase && (abs(g->player.x - e->x) > 1 || abs(g->player.y - e->y) > 1)) {
        return -1; // Request one existing pathfinding step, confined to the royal tomb.
    }
    e->boss_attack = e->attack_phase ? (e->boss_attack == MARSHAL_CROSS ? MARSHAL_CHANNEL : MARSHAL_CROSS) : MARSHAL_SWEEP;
    e->move_timer = e->attack_phase ? 2 : 1;
    if (line_clear(g, e, g->player.x, g->player.y)) {
        if (e->attack_phase) {
            e->attack_target_x = g->player.x;
            e->attack_target_y = g->player.y;
        } else {
            int dx = g->player.x - e->x;
            int dy = g->player.y - e->y;
            e->attack_target_x = e->x + (abs(dx) >= abs(dy) ? (dx < 0 ? -1 : 1) : 0);
            e->attack_target_y = e->y + (abs(dx) < abs(dy) ? (dy < 0 ? -1 : 1) : 0);
        }
        if (!can_escape(g, e, g->player.x, g->player.y, e->move_timer)) {
            clear_targets(e);
        }
    }
    push_message(g, e->boss_attack == MARSHAL_SWEEP ? "Polearm front in 1 action! Leave the three marked tiles." :
        e->boss_attack == MARSHAL_CROSS ? "Funeral cross in 2 actions! Walk out of the marked crossing." :
        "Funeral channel in 2 actions! Leave the crossing or extinguish a tomb brazier to interrupt.");
    return 0;
}

void grave_marshal_interrupt(GameState *g, int x, int y) {
    if (g->level != CATACOMBS_DEPTH || !chamber(&g->map, x, y)) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (e->active && e->type == ENEMY_GRAVE_MARSHAL && e->boss_attack == MARSHAL_CHANNEL && e->move_timer > 0) {
            clear_targets(e);
            e->move_timer = -3;
            push_message(g, "The tomb flame dies and interrupts the funeral channel! Attack during 2 full actions of recovery.");
        }
    }
}

void grave_marshal_cancel_landing(GameState *g, Enemy *e, int x, int y) {
    if (e->boss_attack != MARSHAL_GUARDS && grave_marshal_attack_marks(&g->map, e, x, y)) {
        clear_targets(e);
        e->move_timer = -1;
    }
}

void grave_marshal_restore_reward(GameState *g) {
    if (g->location != LOCATION_CATACOMBS || g->level != CATACOMBS_DEPTH || !g->map.room_count ||
        !(g->defeated_bosses & (1 << LOCATION_CATACOMBS))) {
        return;
    }
    if (g->catacombs_mantle_unclaimed && !g->grave_marshal_reward_state) {
        g->grave_marshal_reward_state = 1;
    }
    int pending = g->grave_marshal_reward_state == 1;
    g->catacombs_mantle_unclaimed = pending;
    int floor_reward = 0;
    for (int i = 0; i < g->floor_item_count; i++) {
        floor_reward |= g->floor_items[i].active && strcmp(g->floor_items[i].item.name, "Gravekeeper's Mantle") == 0;
    }
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    int cache = 0;
    for (int y = room->y; y < room->y + room->h; y++) {
        for (int x = room->x; x < room->x + room->w; x++) {
            if (tile_at(g, x, y) == TILE_MARSHAL_RELIQUARY) {
                if (!pending || floor_reward) {
                    set_tile(g, x, y, TILE_CATACOMBS_FLOOR);
                } else {
                    cache = 1;
                }
            }
        }
    }
    if (!pending || floor_reward || cache) {
        return;
    }
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            if (tile_at(g, x, y) != TILE_CATACOMBS_FLOOR || occupied(g->enemies, g->enemy_count, x, y)) {
                continue;
            }
            if (g->floor_item_count < MAX_FLOOR_ITEMS) {
                FloorItem *item = &g->floor_items[g->floor_item_count++];
                *item = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_CATACOMBS_FLOOR, .item = item_make_gravekeeper_mantle()};
                g->map.tiles[y][x] = TILE_ITEM;
            } else {
                set_tile(g, x, y, TILE_MARSHAL_RELIQUARY);
                push_message(g, "Gravekeeper's Mantle waits in the royal reliquary. Interact beside it or press P on it.");
            }
            return;
        }
    }
}

static int cache_nearby(const GameState *g, int *x, int *y) {
    if (g->location != LOCATION_CATACOMBS || g->level != CATACOMBS_DEPTH || g->grave_marshal_reward_state != 1) {
        return 0;
    }
    static const int offsets[5][2] = {{0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int i = 0; i < 5; i++) {
        *x = g->player.x + offsets[i][0];
        *y = g->player.y + offsets[i][1];
        if (tile_at(g, *x, *y) == TILE_MARSHAL_RELIQUARY) {
            return 1;
        }
    }
    return 0;
}

int grave_marshal_has_interaction(const GameState *g) {
    int x;
    int y;
    return cache_nearby(g, &x, &y);
}

int grave_marshal_interact(GameState *g) {
    int x;
    int y;
    if (!cache_nearby(g, &x, &y)) {
        return 0;
    }
    if (g->inventory_count >= MAX_INVENTORY) {
        push_message(g, "Inventory full. Gravekeeper's Mantle remains in the royal reliquary.");
    } else {
        Item reward = item_make_gravekeeper_mantle();
        g->inventory[g->inventory_count++] = reward;
        g->score += reward.value;
        g->grave_marshal_reward_state = 2;
        g->catacombs_mantle_unclaimed = 0;
        set_tile(g, x, y, TILE_CATACOMBS_FLOOR);
        push_message(g, "Gravekeeper's Mantle recovered from the royal reliquary!");
    }
    return 1;
}

int grave_marshal_pickup(GameState *g) {
    return tile_at(g, g->player.x, g->player.y) == TILE_MARSHAL_RELIQUARY && grave_marshal_interact(g);
}

static void reset_enemies(Enemy *enemies, int count) {
    for (int i = 0; i < count; i++) {
        Enemy *e = &enemies[i];
        if (e->type == ENEMY_GRAVE_MARSHAL) {
            e->move_timer = 0;
            e->attack_phase = 0;
            e->boss_attack = 0;
            e->boss_wave_used = 0;
            clear_targets(e);
        }
    }
}

static void prepare_saved_map(const GameState *g, Map *m, const Enemy *enemies, int count, int px, int py) {
    int x = g->portal_x;
    int y = g->portal_y;
    int protect = g->portal_active && g->portal_location == LOCATION_CATACOMBS && g->portal_level == CATACOMBS_DEPTH &&
        x >= 0 && x < MAP_W && y >= 0 && y < MAP_H && m->tiles[y][x] == TILE_CATACOMBS_FLOOR;
    if (protect) {
        m->tiles[y][x] = TILE_PORTAL;
    }
    grave_marshal_prepare_map(m, enemies, count, px, py);
    if (protect) {
        m->tiles[y][x] = TILE_CATACOMBS_FLOOR;
    }
}

void grave_marshal_migrate(GameState *g) {
    reset_enemies(g->enemies, g->enemy_count);
    LevelCache *cache = &g->catacombs_cache[CATACOMBS_DEPTH - 1];
    if (cache->valid) {
        reset_enemies(cache->enemies, cache->enemy_count);
        prepare_saved_map(g, &cache->map, cache->enemies, cache->enemy_count, -1, -1);
    }
    if (g->location == LOCATION_CATACOMBS && g->level == CATACOMBS_DEPTH) {
        prepare_saved_map(g, &g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    g->grave_marshal_reward_state = g->catacombs_mantle_unclaimed ? 1 :
        (g->defeated_bosses & (1 << LOCATION_CATACOMBS)) ? 2 : 0;
}
