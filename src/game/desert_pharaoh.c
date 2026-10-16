#include "desert_pharaoh.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int chamber(const Map *m, int x, int y) {
    if (!m->room_count) {
        return 0;
    }
    const Room *room = &m->rooms[m->room_count - 1];
    return x >= room->x && x < room->x + room->w && y >= room->y && y < room->y + room->h;
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

void desert_pharaoh_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py) {
    int living = 0;
    for (int i = 0; i < count; i++) {
        living |= enemies[i].active && enemies[i].type == ENEMY_DESERT_PHARAOH;
    }
    if (!living || !m->room_count) {
        return;
    }
    const Room *room = &m->rooms[m->room_count - 1];
    int existing = 0;
    for (int y = room->y; y < room->y + room->h; y++) {
        for (int x = room->x; x < room->x + room->w; x++) {
            existing += m->tiles[y][x] == TILE_PHARAOH_OBELISK;
        }
    }
    for (int n = existing; n < 4; n++) {
        int tx = room->x + (n % 2 ? room->w - 3 : 2);
        int ty = room->y + (n / 2 ? room->h - 3 : 2);
        int bx = -1;
        int by = -1;
        int best = MAP_W + MAP_H;
        for (int y = room->y + 2; y < room->y + room->h - 2; y++) {
            for (int x = room->x + 2; x < room->x + room->w - 2; x++) {
                int distance = abs(x - tx) + abs(y - ty);
                if (m->tiles[y][x] != TILE_DESERT_FLOOR || occupied(enemies, count, x, y) ||
                    (x == px && y == py) || distance >= best) {
                    continue;
                }
                int open = 1;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        open &= m->tiles[y + dy][x + dx] == TILE_DESERT_FLOOR;
                    }
                }
                if (open) {
                    bx = x;
                    by = y;
                    best = distance;
                }
            }
        }
        if (bx >= 0) {
            m->tiles[by][bx] = TILE_PHARAOH_OBELISK;
        }
    }
}

// A beam ends at its locked target. Obelisks, terrain and living actors block it.
static int beam_marks(const GameState *g, const Enemy *e, int tx, int ty) {
    int x = e->x;
    int y = e->y;
    int dx = abs(e->attack_target_x - x);
    int dy = -abs(e->attack_target_y - y);
    int sx = x < e->attack_target_x ? 1 : -1;
    int sy = y < e->attack_target_y ? 1 : -1;
    int error = dx + dy;
    for (int n = 0; n < MAP_W + MAP_H; n++) {
        if (x == e->attack_target_x && y == e->attack_target_y) {
            return 0;
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
        if (!map_is_walkable(&g->map, x, y) || tile_at(g, x, y) == TILE_PHARAOH_OBELISK ||
            occupied(g->enemies, g->enemy_count, x, y) ||
            (x != ox && y != oy && (!map_is_walkable(&g->map, x, oy) || !map_is_walkable(&g->map, ox, y)))) {
            return 0;
        }
        if (x == tx && y == ty) {
            return 1;
        }
    }
    return 0;
}

int desert_pharaoh_shadow(const GameState *g, const Enemy *e, int x, int y) {
    if (!e->active || e->type != ENEMY_DESERT_PHARAOH || e->boss_attack != PHARAOH_STORM ||
        e->move_timer <= 0 || e->attack_target_x < 0 || !chamber(&g->map, x, y) ||
        !map_is_walkable(&g->map, x, y) || abs(e->facing_dx) + abs(e->facing_dy) != 1) {
        return 0;
    }
    for (int step = 1; step <= 2; step++) {
        int bx = x - e->facing_dx * step;
        int by = y - e->facing_dy * step;
        if (tile_at(g, bx, by) == TILE_PHARAOH_OBELISK) {
            return 1;
        }
        if (!map_is_walkable(&g->map, bx, by)) {
            return 0;
        }
    }
    return 0;
}

int desert_pharaoh_attack_marks(const GameState *g, const Enemy *e, int x, int y) {
    if (!e->active || e->type != ENEMY_DESERT_PHARAOH || e->move_timer <= 0 ||
        e->attack_target_x < 0 || !map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    if (e->boss_attack == PHARAOH_BEAM) {
        return beam_marks(g, e, x, y);
    }
    if (e->boss_attack == PHARAOH_BURIAL) {
        return chamber(&g->map, x, y) && abs(x - e->attack_target_x) + abs(y - e->attack_target_y) <= 1;
    }
    return e->boss_attack == PHARAOH_STORM && chamber(&g->map, x, y) &&
        !desert_pharaoh_shadow(g, e, x, y) &&
        (e->facing_dx ? abs(x - e->attack_target_x) <= 1 : abs(y - e->attack_target_y) <= 1);
}

int desert_pharaoh_sand_turns(TileType tile) {
    return tile == TILE_PHARAOH_SAND_2 ? 2 : tile == TILE_PHARAOH_SAND_1 ? 1 : 0;
}

int desert_pharaoh_tick_floor(GameState *g) {
    if (g->location != LOCATION_DESERT || g->level != DESERT_DEPTH || !g->map.room_count) {
        return 0;
    }
    int hit = desert_pharaoh_sand_turns(tile_at(g, g->player.x, g->player.y));
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    for (int y = room->y; y < room->y + room->h; y++) {
        for (int x = room->x; x < room->x + room->w; x++) {
            int turns = desert_pharaoh_sand_turns(tile_at(g, x, y));
            if (turns) {
                set_tile(g, x, y, turns == 2 ? TILE_PHARAOH_SAND_1 : TILE_DESERT_FLOOR);
            }
        }
    }
    if (hit) {
        push_message_kind(g, "Burial sand drags at your feet! Leave the numbered sand patches.", MESSAGE_DAMAGE_TAKEN);
    }
    return hit ? 8 : 0;
}

static void clear_targets(Enemy *e) {
    e->attack_target_x = -1;
    e->attack_target_y = -1;
    e->boss_aux_x = -1;
    e->boss_aux_y = -1;
}

static int can_escape(const GameState *g, const Enemy *e, int x, int y, int turns) {
    if (!desert_pharaoh_attack_marks(g, e, x, y) && !desert_pharaoh_sand_turns(tile_at(g, x, y))) {
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

int desert_pharaoh_tick(GameState *g, Enemy *e, EnemyProjectiles *shots) {
    if (g->location != LOCATION_DESERT || g->level != DESERT_DEPTH || !g->map.room_count) {
        return 0;
    }
    int distance = abs(g->player.x - e->x) + abs(g->player.y - e->y);
    if (!chamber(&g->map, g->player.x, g->player.y) &&
        (distance > 12 || (e->hp == e->max_hp && !e->boss_attack))) {
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
        int hit = desert_pharaoh_attack_marks(g, e, g->player.x, g->player.y);
        if (e->boss_attack == PHARAOH_BURIAL) {
            const Room *room = &g->map.rooms[g->map.room_count - 1];
            for (int y = room->y; y < room->y + room->h; y++) {
                for (int x = room->x; x < room->x + room->w; x++) {
                    if (desert_pharaoh_attack_marks(g, e, x, y) && tile_at(g, x, y) == TILE_DESERT_FLOOR) {
                        set_tile(g, x, y, TILE_PHARAOH_SAND_2);
                    }
                }
            }
        } else if (e->boss_attack == PHARAOH_BEAM && e->attack_target_x >= 0 && shots && shots->count < MAX_ENEMIES) {
            if (beam_marks(g, e, e->attack_target_x, e->attack_target_y)) {
                shots->shots[shots->count++] = (EnemyProjectile){e->type, e->x, e->y, e->attack_target_x, e->attack_target_y};
            }
        }
        push_message(g, e->boss_attack == PHARAOH_BEAM ? "The curse beam strikes its fixed line! Pharaoh exposed." :
            e->boss_attack == PHARAOH_BURIAL ? "Burial sand erupts! Patches last 2 actions; Pharaoh exposed." :
            "The storm crosses its marked band! Obelisk shadows shelter you; Pharaoh exposed.");
        clear_targets(e);
        e->move_timer = -1;
        int damage = e->attack - g->player.defense / 2;
        return hit ? (damage > 3 ? damage : 3) : 0;
    }
    if (!e->attack_phase && e->hp <= e->max_hp / 2) {
        e->attack_phase = 1;
        push_message(g, "Sandbound! Moving storm bands join the curses. Shelter on S tiles behind obelisks.");
    }
    e->boss_attack = e->boss_attack == PHARAOH_BEAM ? PHARAOH_BURIAL :
        e->boss_attack == PHARAOH_BURIAL && e->attack_phase ? PHARAOH_STORM : PHARAOH_BEAM;
    clear_targets(e);
    e->move_timer = e->boss_attack == PHARAOH_BEAM ? 1 : 2;
    e->attack_target_x = g->player.x;
    e->attack_target_y = g->player.y;
    if (e->boss_attack == PHARAOH_STORM) {
        static const int winds[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
        int pass = e->boss_wave_used % 12;
        e->facing_dx = winds[pass / 3][0];
        e->facing_dy = winds[pass / 3][1];
        const Room *room = &g->map.rooms[g->map.room_count - 1];
        int edge = pass % 3;
        if (e->facing_dx < 0 || e->facing_dy < 0) {
            edge = 2 - edge;
        }
        e->attack_target_x = e->facing_dx ? (edge == 0 ? room->x + 3 : edge == 1 ? e->x : room->x + room->w - 3) : e->x;
        e->attack_target_y = e->facing_dy ? (edge == 0 ? room->y + 3 : edge == 1 ? e->y : room->y + room->h - 3) : e->y;
        e->boss_wave_used = (pass + 1) % 12;
    }
    if (e->boss_attack == PHARAOH_BEAM && !beam_marks(g, e, e->attack_target_x, e->attack_target_y)) {
        clear_targets(e);
    }
    if (!can_escape(g, e, g->player.x, g->player.y, e->move_timer)) {
        clear_targets(e);
    }
    push_message(g, e->boss_attack == PHARAOH_BEAM ? "Curse beam in 1 action! Step off its fixed line; obelisks block the beam." :
        e->boss_attack == PHARAOH_BURIAL ? "Burial sand in 2 actions! Leave the fixed cross; the sand then lasts 2 actions." :
        "Storm band in 2 actions! Leave the marked strip or shelter on S tiles in the wind's obelisk shadows.");
    return 0;
}

void desert_pharaoh_cancel_landing(GameState *g, int x, int y) {
    if (g->location != LOCATION_DESERT || g->level != DESERT_DEPTH) {
        return;
    }
    if (desert_pharaoh_sand_turns(tile_at(g, x, y))) {
        set_tile(g, x, y, TILE_DESERT_FLOOR);
    }
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (desert_pharaoh_attack_marks(g, e, x, y)) {
            clear_targets(e);
            e->move_timer = -1;
        }
    }
}

void desert_pharaoh_restore_reward(GameState *g) {
    if (g->location != LOCATION_DESERT || g->level != DESERT_DEPTH || !g->map.room_count ||
        !(g->defeated_bosses & (1 << LOCATION_DESERT))) {
        return;
    }
    if (g->sandstorm_staff_unclaimed && !g->desert_pharaoh_reward_state) {
        g->desert_pharaoh_reward_state = 1;
    }
    int pending = g->desert_pharaoh_reward_state == 1;
    g->sandstorm_staff_unclaimed = pending;
    int floor_reward = 0;
    for (int i = 0; i < g->floor_item_count; i++) {
        floor_reward |= g->floor_items[i].active && strcmp(g->floor_items[i].item.name, "Sandstorm Staff") == 0;
    }
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    int cache = 0;
    for (int y = room->y; y < room->y + room->h; y++) {
        for (int x = room->x; x < room->x + room->w; x++) {
            if (tile_at(g, x, y) == TILE_PHARAOH_RELIQUARY) {
                if (!pending || floor_reward) {
                    set_tile(g, x, y, TILE_DESERT_FLOOR);
                } else {
                    cache = 1;
                }
            }
        }
    }
    if (!pending || floor_reward || cache) {
        return;
    }
    for (int n = 0; n <= room->w * room->h; n++) {
        int x = n ? room->x + (n - 1) % room->w : room->x + room->w / 2;
        int y = n ? room->y + (n - 1) / room->w : room->y + room->h / 2;
        if (tile_at(g, x, y) != TILE_DESERT_FLOOR || occupied(g->enemies, g->enemy_count, x, y)) {
            continue;
        }
        if (g->floor_item_count < MAX_FLOOR_ITEMS) {
            FloorItem *item = &g->floor_items[g->floor_item_count++];
            *item = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_DESERT_FLOOR, .item = item_make_sandstorm_staff()};
            g->map.tiles[y][x] = TILE_ITEM;
        } else {
            set_tile(g, x, y, TILE_PHARAOH_RELIQUARY);
            push_message(g, "Sandstorm Staff waits in the sandstone reliquary. Interact beside it or press P on it.");
        }
        return;
    }
}

static int cache_nearby(const GameState *g, int *x, int *y) {
    if (g->location != LOCATION_DESERT || g->level != DESERT_DEPTH || g->desert_pharaoh_reward_state != 1) {
        return 0;
    }
    static const int offsets[5][2] = {{0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int i = 0; i < 5; i++) {
        *x = g->player.x + offsets[i][0];
        *y = g->player.y + offsets[i][1];
        if (tile_at(g, *x, *y) == TILE_PHARAOH_RELIQUARY) {
            return 1;
        }
    }
    return 0;
}

int desert_pharaoh_has_interaction(const GameState *g) {
    int x;
    int y;
    return cache_nearby(g, &x, &y);
}

int desert_pharaoh_interact(GameState *g) {
    int x;
    int y;
    if (!cache_nearby(g, &x, &y)) {
        return 0;
    }
    if (g->inventory_count >= MAX_INVENTORY) {
        push_message(g, "Inventory full. Sandstorm Staff remains in the sandstone reliquary.");
    } else {
        Item reward = item_make_sandstorm_staff();
        g->inventory[g->inventory_count++] = reward;
        g->score += reward.value;
        g->desert_pharaoh_reward_state = 2;
        g->sandstorm_staff_unclaimed = 0;
        set_tile(g, x, y, TILE_DESERT_FLOOR);
        push_message(g, "Sandstorm Staff recovered from the sandstone reliquary!");
    }
    return 1;
}

int desert_pharaoh_pickup(GameState *g) {
    return tile_at(g, g->player.x, g->player.y) == TILE_PHARAOH_RELIQUARY && desert_pharaoh_interact(g);
}

static void reset_enemies(Enemy *enemies, int count) {
    for (int i = 0; i < count; i++) {
        Enemy *e = &enemies[i];
        if (e->type == ENEMY_DESERT_PHARAOH) {
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
    int protect = g->portal_active && g->portal_location == LOCATION_DESERT && g->portal_level == DESERT_DEPTH &&
        x >= 0 && x < MAP_W && y >= 0 && y < MAP_H && m->tiles[y][x] == TILE_DESERT_FLOOR;
    if (protect) {
        m->tiles[y][x] = TILE_PORTAL;
    }
    desert_pharaoh_prepare_map(m, enemies, count, px, py);
    if (protect) {
        m->tiles[y][x] = TILE_DESERT_FLOOR;
    }
}

void desert_pharaoh_migrate(GameState *g) {
    reset_enemies(g->enemies, g->enemy_count);
    LevelCache *cache = &g->desert_cache[DESERT_DEPTH - 1];
    if (cache->valid) {
        reset_enemies(cache->enemies, cache->enemy_count);
        prepare_saved_map(g, &cache->map, cache->enemies, cache->enemy_count, -1, -1);
    }
    if (g->location == LOCATION_DESERT && g->level == DESERT_DEPTH) {
        prepare_saved_map(g, &g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    g->desert_pharaoh_reward_state = g->sandstorm_staff_unclaimed ? 1 :
        (g->defeated_bosses & (1 << LOCATION_DESERT)) ? 2 : 0;
}
