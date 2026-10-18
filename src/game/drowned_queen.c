#include "drowned_queen.h"
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
    return g->location == LOCATION_COAST && g->level == COAST_DEPTH && g->map.room_count > 0;
}

static int ordinary_floor(TileType tile) {
    return tile == TILE_COAST_FLOOR || tile == TILE_COAST_SHALLOW_WATER;
}

void drowned_queen_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py) {
    if (m->room_count < 1) {
        return;
    }
    const Enemy *queen = NULL;
    for (int i = 0; i < count; i++) {
        if (enemies[i].active && enemies[i].type == ENEMY_DROWNED_QUEEN) {
            queen = &enemies[i];
        }
    }
    if (!queen) {
        return;
    }
    const Room *room = &m->rooms[m->room_count - 1];
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            if (m->tiles[y][x] >= TILE_QUEEN_WATER && m->tiles[y][x] <= TILE_QUEEN_CONTROL_V_DRAINED) {
                return;
            }
        }
    }
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            if (ordinary_floor(m->tiles[y][x]) || m->tiles[y][x] == TILE_TRAP_HIDDEN) {
                m->tiles[y][x] = TILE_QUEEN_WATER;
            }
        }
    }
    for (int n = 0; n < 2; n++) {
        int tx = n == 0 ? room->x + 2 : queen->x - 2;
        int ty = n == 0 ? queen->y + 2 : room->y + 2;
        int bx = -1;
        int by = -1;
        int distance = MAP_W + MAP_H;
        for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
            for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
                int d = abs(x - tx) + abs(y - ty);
                if ((n == 0 && abs(y - queen->y) < 2) || (n == 1 && abs(x - queen->x) < 2)) {
                    continue;
                }
                if (m->tiles[y][x] == TILE_QUEEN_WATER && !occupied(enemies, count, x, y) &&
                    !(x == px && y == py) && d < distance) {
                    bx = x;
                    by = y;
                    distance = d;
                }
            }
        }
        if (bx >= 0) {
            m->tiles[by][bx] = n == 0 ? TILE_QUEEN_CONTROL_H : TILE_QUEEN_CONTROL_V;
        }
    }
}

static int shelter(TileType tile) {
    return tile == TILE_QUEEN_DRY || (tile >= TILE_QUEEN_CONTROL_H && tile <= TILE_QUEEN_CONTROL_V_DRAINED);
}

static void remove_islands(Map *m) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (m->tiles[y][x] == TILE_QUEEN_ISLAND) {
                m->tiles[y][x] = TILE_QUEEN_WATER;
            }
        }
    }
}

void drowned_queen_remove_islands(GameState *g) {
    if (arena(g)) {
        remove_islands(&g->map);
    }
    for (int i = 0; i < COAST_DEPTH; i++) {
        if (g->coast_cache[i].valid) {
            remove_islands(&g->coast_cache[i].map);
        }
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        if (g->floor_items[i].underlying_tile == TILE_QUEEN_ISLAND) {
            g->floor_items[i].underlying_tile = TILE_QUEEN_WATER;
        }
    }
    if (g->portal_origin_tile == TILE_QUEEN_ISLAND) {
        g->portal_origin_tile = TILE_QUEEN_WATER;
    }
}

int drowned_queen_controls(const GameState *g) {
    if (!arena(g)) {
        return 0;
    }
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    int count = 0;
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            TileType tile = tile_at(g, x, y);
            count += tile == TILE_QUEEN_CONTROL_H || tile == TILE_QUEEN_CONTROL_V;
        }
    }
    return count;
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

static int wave(const GameState *g, const Enemy *e, int x, int y) {
    int row = e->boss_attack == QUEEN_ROW;
    if ((row ? y != e->attack_target_y : x != e->attack_target_x) ||
        abs(x - e->x) > 12 || abs(y - e->y) > 12) {
        return 0;
    }
    int axis = row ? x : y;
    return axis >= e->boss_aux_x && axis <= e->boss_aux_y;
}

static void wave_bounds(const GameState *g, Enemy *e) {
    int row = e->boss_attack == QUEEN_ROW;
    int low = row ? e->attack_target_x : e->attack_target_y;
    int high = low;
    int center = row ? e->x : e->y;
    while (low - 1 >= center - 12 && map_is_walkable(&g->map, row ? low - 1 : e->attack_target_x, row ? e->attack_target_y : low - 1)) {
        low--;
    }
    while (high + 1 <= center + 12 && map_is_walkable(&g->map, row ? high + 1 : e->attack_target_x, row ? e->attack_target_y : high + 1)) {
        high++;
    }
    e->boss_aux_x = low;
    e->boss_aux_y = high;
}

int drowned_queen_attack_marks(const GameState *g, const Enemy *e, int x, int y) {
    if (!e->active || e->type != ENEMY_DROWNED_QUEEN || e->move_timer <= 0 ||
        e->attack_target_x < 0 || !map_is_walkable(&g->map, x, y) || shelter(tile_at(g, x, y))) {
        return 0;
    }
    if (e->boss_attack == QUEEN_POCKETS) {
        return pocket(e->attack_target_x, e->attack_target_y, x, y) || pocket(e->boss_aux_x, e->boss_aux_y, x, y);
    }
    return (e->boss_attack == QUEEN_ROW || e->boss_attack == QUEEN_COLUMN) && wave(g, e, x, y);
}

static void clear_targets(Enemy *e) {
    e->attack_target_x = -1;
    e->attack_target_y = -1;
    e->boss_aux_x = -1;
    e->boss_aux_y = -1;
}

int drowned_queen_tick(GameState *g, Enemy *e) {
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
        int hit = drowned_queen_attack_marks(g, e, g->player.x, g->player.y);
        push_message(g, e->boss_attack == QUEEN_POCKETS ? "Storm pools flood the marked pockets!" : "The tidal wave sweeps its fixed marked lane!");
        e->move_timer = -1;
        clear_targets(e);
        int damage = e->attack - g->player.defense / 2;
        return hit ? (damage > 5 ? damage : 5) : 0;
    }
    if (!e->attack_phase && e->hp <= e->max_hp / 2) {
        e->attack_phase = 1;
        push_message(g, "The Drowned Queen becomes Storm Throne! Flooded pockets join the waves.");
    }
    e->boss_attack = e->boss_attack == QUEEN_ROW ? QUEEN_COLUMN :
        e->boss_attack == QUEEN_COLUMN && e->attack_phase ? QUEEN_POCKETS : QUEEN_ROW;
    clear_targets(e);
    if (line_clear(g, e, g->player.x, g->player.y)) {
        e->attack_target_x = g->player.x;
        e->attack_target_y = g->player.y;
        if (e->boss_attack == QUEEN_POCKETS) {
            const Room *room = &g->map.rooms[g->map.room_count - 1];
            int distance = MAP_W + MAP_H;
            int tx = g->player.x + (e->x > g->player.x ? 4 : -4);
            for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
                for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
                    int d = abs(x - tx) + abs(y - g->player.y);
                    if (map_is_walkable(&g->map, x, y) && !shelter(tile_at(g, x, y)) &&
                        abs(x - g->player.x) + abs(y - g->player.y) >= 4 && d < distance) {
                        e->boss_aux_x = x;
                        e->boss_aux_y = y;
                        distance = d;
                    }
                }
            }
        } else {
            wave_bounds(g, e);
        }
    }
    e->move_timer = 2;
    push_message(g, e->boss_attack == QUEEN_POCKETS ? "Flooded pockets in 2 turns! Leave both fixed storm pools." :
        e->boss_attack == QUEEN_ROW ? "Tidal row in 2 turns! Sidestep or shelter on dry stone." : "Tidal column in 2 turns! Sidestep or shelter on dry stone.");
    return 0;
}

void drowned_queen_restore_reward(GameState *g) {
    if (g->location != LOCATION_COAST || g->level != COAST_DEPTH || g->map.room_count < 1 || !(g->defeated_bosses & (1 << LOCATION_COAST))) {
        return;
    }
    int floor_robes = 0;
    for (int i = 0; i < g->floor_item_count; i++) {
        floor_robes |= g->floor_items[i].active && strcmp(g->floor_items[i].item.name, "Tidecaller Robes") == 0;
    }
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    int cache = 0;
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            if (tile_at(g, x, y) == TILE_QUEEN_RELIQUARY) {
                if (g->drowned_queen_reward_state != 1 || floor_robes) {
                    set_tile(g, x, y, TILE_COAST_FLOOR);
                } else {
                    cache = 1;
                }
            }
        }
    }
    if (g->drowned_queen_reward_state != 1 || floor_robes || cache) {
        return;
    }
    for (int y = room->y + room->h - 2; y > room->y; y--) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            if ((g->map.tiles[y][x] == TILE_COAST_FLOOR || g->map.tiles[y][x] == TILE_COAST_SHALLOW_WATER || g->map.tiles[y][x] == TILE_QUEEN_WATER || g->map.tiles[y][x] == TILE_QUEEN_DRY) && !occupied(g->enemies, g->enemy_count, x, y)) {
                g->map.tiles[y][x] = TILE_QUEEN_RELIQUARY;
                push_message(g, "Unclaimed Tidecaller Robes wait in the Queen's reliquary. Interact beside it or press P on it.");
                return;
            }
        }
    }
}

static TileType interaction_at(const GameState *g, int *x, int *y) {
    if (g->location != LOCATION_COAST || g->level != COAST_DEPTH) {
        return TILE_COAST_FLOOR;
    }
    // Prefer the tile underfoot, so P on a reliquary cannot select a nearby tide control.
    for (int radius = 0; radius <= 1; radius++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (abs(dx) + abs(dy) != radius) {
                    continue;
                }
                TileType tile = tile_at(g, g->player.x + dx, g->player.y + dy);
                if ((tile == TILE_QUEEN_CONTROL_H || tile == TILE_QUEEN_CONTROL_V) || (tile == TILE_QUEEN_RELIQUARY && g->drowned_queen_reward_state == 1)) {
                    *x = g->player.x + dx;
                    *y = g->player.y + dy;
                    return tile;
                }
            }
        }
    }
    return TILE_COAST_FLOOR;
}

int drowned_queen_has_interaction(const GameState *g) {
    int x;
    int y;
    return interaction_at(g, &x, &y) != TILE_COAST_FLOOR;
}

int drowned_queen_interact(GameState *g) {
    int x;
    int y;
    TileType tile = interaction_at(g, &x, &y);
    if (tile == TILE_COAST_FLOOR) {
        return 0;
    }
    if (tile == TILE_QUEEN_RELIQUARY) {
        if (g->inventory_count >= MAX_INVENTORY) {
            push_message(g, "Inventory full. Tidecaller Robes remains safe in the reliquary.");
        } else {
            Item reward = item_make_tidecaller_robes();
            g->inventory[g->inventory_count++] = reward;
            g->score += reward.value;
            g->drowned_queen_reward_state = 2;
            set_tile(g, x, y, TILE_COAST_FLOOR);
            push_message(g, "Tidecaller Robes recovered from the Queen's reliquary!");
        }
        return 1;
    }
    int row = tile == TILE_QUEEN_CONTROL_H;
    set_tile(g, x, y, row ? TILE_QUEEN_CONTROL_H_DRAINED : TILE_QUEEN_CONTROL_V_DRAINED);
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    for (int yy = room->y + 1; yy < room->y + room->h - 1; yy++) {
        for (int xx = room->x + 1; xx < room->x + room->w - 1; xx++) {
            if ((row ? yy == y : xx == x) && (tile_at(g, xx, yy) == TILE_QUEEN_WATER || ordinary_floor(tile_at(g, xx, yy)))) {
                set_tile(g, xx, yy, TILE_QUEEN_DRY);
                map_mark_explored(&g->map, xx, yy);
            }
        }
    }
    push_message(g, row ? "The throne's horizontal lane drains into a safe stone approach!" : "The throne's vertical lane drains into a safe stone approach!");
    return 1;
}

static void prepare_saved_map(const GameState *g, Map *m, const Enemy *enemies, int count, int px, int py) {
    int x = g->portal_x;
    int y = g->portal_y;
    int protect = g->portal_active && g->portal_location == LOCATION_COAST &&
        g->portal_level == COAST_DEPTH && x >= 0 && x < MAP_W && y >= 0 && y < MAP_H &&
        (m->tiles[y][x] == TILE_COAST_FLOOR || m->tiles[y][x] == TILE_COAST_SHALLOW_WATER);
    TileType original = protect ? m->tiles[y][x] : TILE_COAST_FLOOR;
    // Hidden portal landings stay traversable even when their marker is absent.
    if (protect) {
        m->tiles[y][x] = TILE_PORTAL;
    }
    drowned_queen_prepare_map(m, enemies, count, px, py);
    if (protect) {
        m->tiles[y][x] = original;
    }
}

void drowned_queen_migrate(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (e->type == ENEMY_DROWNED_QUEEN) {
            e->move_timer = 0;
            e->attack_phase = 0;
            e->boss_attack = 0;
            e->boss_wave_used = 0;
            clear_targets(e);
        }
    }
    LevelCache *cache = &g->coast_cache[COAST_DEPTH - 1];
    if (cache->valid) {
        for (int i = 0; i < cache->enemy_count; i++) {
            Enemy *e = &cache->enemies[i];
            if (e->type == ENEMY_DROWNED_QUEEN) {
                e->move_timer = 0;
                e->attack_phase = 0;
                e->boss_attack = 0;
                e->boss_wave_used = 0;
                clear_targets(e);
            }
        }
        prepare_saved_map(g, &cache->map, cache->enemies, cache->enemy_count, -1, -1);
    }
    if (g->location == LOCATION_COAST && g->level == COAST_DEPTH) {
        prepare_saved_map(g, &g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    // Only known, currently dropped legacy rewards establish an unclaimed gift.
    if (g->defeated_bosses & (1 << LOCATION_COAST)) {
        g->drowned_queen_reward_state = 2;
        int known_claim = 0;
        for (int i = 0; i < g->inventory_count; i++) {
            known_claim |= strcmp(g->inventory[i].name, "Tidecaller Robes") == 0;
        }
        if (g->location == LOCATION_COAST && g->level == COAST_DEPTH) {
            for (int i = 0; i < g->floor_item_count; i++) {
                if (!known_claim && g->floor_items[i].active &&
                    strcmp(g->floor_items[i].item.name, "Tidecaller Robes") == 0) {
                    g->drowned_queen_reward_state = 1;
                }
            }
        }
    }
}
