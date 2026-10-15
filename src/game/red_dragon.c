#include "red_dragon.h"
#include <stdlib.h>
#include <string.h>

static int arena(const GameState *g) {
    return g->location == LOCATION_DRAGONSPINE && g->level == DRAGONSPINE_DEPTH && g->map.room_count > 0;
}

static int chamber(const Map *m, int x, int y) {
    if (!m->room_count) {
        return 0;
    }
    const Room *r = &m->rooms[m->room_count - 1];
    return x >= r->x && y >= r->y && x < r->x + r->w && y < r->y + r->h;
}

static int column(TileType tile) {
    return tile >= TILE_DRAGON_BASALT && tile <= TILE_DRAGON_CRACKED_COLUMN;
}

static int cache_tile(TileType tile) {
    return tile >= TILE_DRAGON_RELIQUARY_FLOOR && tile <= TILE_DRAGON_RELIQUARY_HOARD;
}

static int plain(TileType tile) {
    return tile == TILE_DRAGON_FLOOR || tile == TILE_DRAGON_ASH || tile == TILE_DRAGON_HOARD;
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

int red_dragon_hot_turns(TileType tile) {
    return tile >= TILE_DRAGON_HOT_FLOOR_1 && tile <= TILE_DRAGON_HOT_HOARD_3 ? (tile - TILE_DRAGON_HOT_FLOOR_1) % 3 + 1 : 0;
}

TileType red_dragon_base_tile(TileType tile) {
    if (red_dragon_hot_turns(tile)) {
        int terrain = (tile - TILE_DRAGON_HOT_FLOOR_1) / 3;
        return terrain == 0 ? TILE_DRAGON_FLOOR : terrain == 1 ? TILE_DRAGON_ASH : TILE_DRAGON_HOARD;
    }
    if (cache_tile(tile)) {
        return tile == TILE_DRAGON_RELIQUARY_FLOOR ? TILE_DRAGON_FLOOR :
            tile == TILE_DRAGON_RELIQUARY_ASH ? TILE_DRAGON_ASH : TILE_DRAGON_HOARD;
    }
    return tile;
}

void red_dragon_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py) {
    const Enemy *boss = NULL;
    for (int i = 0; i < count; i++) {
        if (enemies[i].active && enemies[i].type == ENEMY_RED_DRAGON) {
            boss = &enemies[i];
        }
    }
    if (!boss || !m->room_count) {
        return;
    }
    const Room *r = &m->rooms[m->room_count - 1];
    for (int y = r->y; y < r->y + r->h; y++) {
        for (int x = r->x; x < r->x + r->w; x++) {
            if (column(m->tiles[y][x]) || m->tiles[y][x] == TILE_DRAGON_COLUMN_RUBBLE) {
                return;
            }
        }
    }
    int cx;
    int cy;
    map_room_center(r, &cx, &cy);
    for (int n = 0; n < 4; n++) {
        int tx = cx + (n % 2 ? 3 : -3);
        int ty = cy + (n == 0 || n == 3 ? -2 : 2);
        int bx = -1;
        int by = -1;
        int best = MAP_W + MAP_H;
        for (int y = r->y + 1; y < r->y + r->h - 1; y++) {
            for (int x = r->x + 1; x < r->x + r->w - 1; x++) {
                int d = abs(x - tx) + abs(y - ty);
                if (m->tiles[y][x] != TILE_DRAGON_ASH || (x == px && y == py) || occupied(enemies, count, x, y) ||
                    abs(x - boss->x) + abs(y - boss->y) < 3 || abs(x - cx - 2) + abs(y - cy - 1) <= 1 || d >= best) {
                    continue;
                }
                // An open 3x3 ring preserves every existing walking route around a new column.
                int open = 1;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        open &= plain(m->tiles[y + dy][x + dx]);
                    }
                }
                if (open) {
                    bx = x;
                    by = y;
                    best = d;
                }
            }
        }
        if (bx >= 0) {
            m->tiles[by][bx] = n < 2 ? TILE_DRAGON_BASALT : TILE_DRAGON_WEAK_COLUMN;
        }
    }
}

static void clear_warning(Enemy *e) {
    memset(e->dragon_warning, 0, sizeof(e->dragon_warning));
    e->attack_target_x = -1;
    e->attack_target_y = -1;
}

static int warned(const Enemy *e, int x, int y) {
    int dx = x - e->boss_aux_x;
    int dy = y - e->boss_aux_y;
    int depth = dx * e->facing_dx + dy * e->facing_dy;
    int side = -dx * e->facing_dy + dy * e->facing_dx;
    if (e->facing_dx && e->facing_dy) {
        dx *= e->facing_dx;
        dy *= e->facing_dy;
        if (dx == dy) {
            depth = dx;
            side = 0;
        } else if (dx == dy + 1) {
            depth = dx;
            side = -1;
        } else if (dy == dx + 1) {
            depth = dy;
            side = 1;
        } else {
            return 0;
        }
    }
    if (depth < -1 || depth > DRAGON_WARNING_RADIUS || side < -1 || side > 1) {
        return 0;
    }
    int index = (depth + 1) * 3 + side + 1;
    return (e->dragon_warning[index / DRAGON_WARNING_WORD_BITS] & (1u << (index % DRAGON_WARNING_WORD_BITS))) != 0;
}

static void warning_cell(const Enemy *e, int depth, int side, int *x, int *y) {
    *x = e->boss_aux_x + e->facing_dx * depth;
    *y = e->boss_aux_y + e->facing_dy * depth;
    if (e->facing_dx && e->facing_dy) {
        *x -= side > 0 ? e->facing_dx : 0;
        *y -= side < 0 ? e->facing_dy : 0;
    } else {
        *x -= e->facing_dy * side;
        *y += e->facing_dx * side;
    }
}

int red_dragon_attack_marks(const GameState *g, const Enemy *e, int x, int y) {
    return arena(g) && e->active && e->type == ENEMY_RED_DRAGON && e->move_timer > 0 && warned(e, x, y);
}

// Terrain clips a cone at declaration. Its saved cells never expand when a column breaks.
static int clear_ray(const GameState *g, int sx, int sy, int tx, int ty) {
    int x = sx;
    int y = sy;
    int dx = abs(tx - x);
    int dy = -abs(ty - y);
    int step_x = x < tx ? 1 : -1;
    int step_y = y < ty ? 1 : -1;
    int error = dx + dy;
    for (int n = 0; n <= DRAGON_WARNING_RADIUS * 2; n++) {
        if (x == tx && y == ty) {
            return 1;
        }
        int ox = x;
        int oy = y;
        int twice = error * 2;
        if (twice >= dy) {
            error += dy;
            x += step_x;
        }
        if (twice <= dx) {
            error += dx;
            y += step_y;
        }
        if (x != ox && y != oy && (!map_is_walkable(&g->map, x, oy) || !map_is_walkable(&g->map, ox, y))) {
            return 0;
        }
        if (!map_is_walkable(&g->map, x, y)) {
            return x == tx && y == ty && column(tile_at(g, x, y));
        }
    }
    return 0;
}

static int can_escape(const GameState *g, const Enemy *e, int x, int y, int turns, int elapsed) {
    if (elapsed > 0 && red_dragon_hot_turns(tile_at(g, x, y)) >= elapsed) {
        return 0;
    }
    if (!warned(e, x, y) && !red_dragon_hot_turns(tile_at(g, x, y))) {
        return 1;
    }
    if (!turns) {
        return !warned(e, x, y) && red_dragon_hot_turns(tile_at(g, x, y)) < elapsed;
    }
    static const int moves[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (int n = 0; n < 4; n++) {
        int nx = x + moves[n][0];
        int ny = y + moves[n][1];
        if (map_is_walkable(&g->map, nx, ny) && !occupied(g->enemies, g->enemy_count, nx, ny) &&
            can_escape(g, e, nx, ny, turns - 1, elapsed + 1)) {
            return 1;
        }
    }
    return 0;
}

static void declare(GameState *g, Enemy *e, int attack, int rotate) {
    int old_dx = e->facing_dx;
    int old_dy = e->facing_dy;
    clear_warning(e);
    e->boss_attack = attack;
    e->move_timer = attack == DRAGON_SWEEP ? 1 : 2;
    int dx = g->player.x - e->x;
    int dy = g->player.y - e->y;
    e->facing_dx = rotate ? -old_dy : abs(dx) >= abs(dy) ? (dx < 0 ? -1 : 1) : 0;
    e->facing_dy = rotate ? old_dx : abs(dx) < abs(dy) ? (dy < 0 ? -1 : 1) : 0;
    if (!rotate && attack != DRAGON_SWEEP && attack != DRAGON_WINGBEAT && dx && abs(dx) == abs(dy)) {
        e->facing_dx = dx < 0 ? -1 : 1;
        e->facing_dy = dy < 0 ? -1 : 1;
    }
    e->attack_target_x = attack == DRAGON_WINGBEAT ? g->player.x : e->x;
    e->attack_target_y = attack == DRAGON_WINGBEAT ? g->player.y : e->y;
    e->boss_aux_x = e->attack_target_x;
    e->boss_aux_y = e->attack_target_y;
    for (int d = -1; d <= DRAGON_WARNING_RADIUS; d++) {
        for (int s = -1; s <= 1; s++) {
            int x;
            int y;
            warning_cell(e, d, s, &x, &y);
            int rx = x - e->x;
            int ry = y - e->y;
            int shape = attack == DRAGON_WINGBEAT ? abs(x - e->attack_target_x) + abs(y - e->attack_target_y) <= 1 :
                attack == DRAGON_SWEEP ? d >= -1 && d <= 1 && (rx || ry) :
                d >= 1 && d <= DRAGON_WARNING_RADIUS && abs(s) <= (d >= 3 ? 1 : 0);
            int open = map_is_walkable(&g->map, x, y);
            if (!shape || (!open && !(attack != DRAGON_SWEEP && attack != DRAGON_WINGBEAT && column(tile_at(g, x, y))))) {
                continue;
            }
            if (attack == DRAGON_WINGBEAT) {
                open &= chamber(&g->map, x, y);
            } else {
                open = clear_ray(g, e->x, e->y, x, y);
            }
            if (open) {
                int index = (d + 1) * 3 + s + 1;
                e->dragon_warning[index / DRAGON_WARNING_WORD_BITS] |= 1u << (index % DRAGON_WARNING_WORD_BITS);
            }
        }
    }
    if (!can_escape(g, e, g->player.x, g->player.y, e->move_timer, 0)) {
        clear_warning(e);
        push_message(g, "The dragon's strike falters: no open walking escape through the cover and embers.");
    } else {
        push_message(g, attack == DRAGON_SWEEP ? "Claw and tail sweep: 1 action! Leave the marked lanes beside the dragon." :
            attack == DRAGON_WINGBEAT ? "Wingbeat landing: 2 actions! Leave its fixed cross; marked ground will burn for 3 actions." :
            attack == DRAGON_INFERNO_FIRST ? "Last Inferno: first cone in 2 actions! A separately warned quarter-turn breath follows." :
            attack == DRAGON_INFERNO_SECOND ? "Last Inferno: second cone in 2 actions! Its new direction is fixed; long recovery follows." :
            "Dragon breath: 2 actions! Leave the fixed cone or shelter behind basalt. Weak columns crack before breaking.");
    }
}

int red_dragon_tick_floor(GameState *g) {
    if (!arena(g)) {
        return 0;
    }
    int hit = red_dragon_hot_turns(tile_at(g, g->player.x, g->player.y));
    const Room *r = &g->map.rooms[g->map.room_count - 1];
    for (int y = r->y; y < r->y + r->h; y++) {
        for (int x = r->x; x < r->x + r->w; x++) {
            TileType tile = tile_at(g, x, y);
            int turns = red_dragon_hot_turns(tile);
            if (turns) {
                set_tile(g, x, y, turns == 1 ? red_dragon_base_tile(tile) : tile - 1);
            }
        }
    }
    if (hit) {
        push_message_kind(g, "Dragonfire burns the H-numbered ground! Leave the embers.", MESSAGE_DAMAGE_TAKEN);
    }
    return hit ? 8 : 0;
}

static int resolve(GameState *g, Enemy *e, EnemyProjectiles *shots) {
    int hit = warned(e, g->player.x, g->player.y);
    if (shots && e->boss_attack != DRAGON_SWEEP && e->boss_attack != DRAGON_WINGBEAT) {
        for (int side = -1; side <= 1; side++) {
            for (int depth = DRAGON_WARNING_RADIUS; depth >= 1; depth--) {
                int x;
                int y;
                warning_cell(e, depth, side, &x, &y);
                if (warned(e, x, y) && map_is_walkable(&g->map, x, y) && shots->count < MAX_ENEMIES) {
                    shots->shots[shots->count++] = (EnemyProjectile){e->type, e->x, e->y, x, y};
                    break;
                }
            }
        }
    }
    for (int depth = -1; depth <= DRAGON_WARNING_RADIUS; depth++) {
        for (int side = -1; side <= 1; side++) {
            int x;
            int y;
            warning_cell(e, depth, side, &x, &y);
            if (!warned(e, x, y)) {
                continue;
            }
            TileType tile = tile_at(g, x, y);
            if (e->boss_attack == DRAGON_WINGBEAT && plain(tile)) {
                set_tile(g, x, y, tile == TILE_DRAGON_FLOOR ? TILE_DRAGON_HOT_FLOOR_3 :
                    tile == TILE_DRAGON_ASH ? TILE_DRAGON_HOT_ASH_3 : TILE_DRAGON_HOT_HOARD_3);
            } else if (e->boss_attack != DRAGON_SWEEP && e->boss_attack != DRAGON_WINGBEAT) {
                if (tile == TILE_DRAGON_WEAK_COLUMN) {
                    set_tile(g, x, y, TILE_DRAGON_CRACKED_COLUMN);
                    push_message(g, "A weak basalt column cracks! Another warned breath will break it.");
                } else if (tile == TILE_DRAGON_CRACKED_COLUMN) {
                    set_tile(g, x, y, TILE_DRAGON_COLUMN_RUBBLE);
                    push_message(g, "The cracked column crumbles. This cone cannot spread beyond its original warning.");
                }
            }
        }
    }
    if (e->boss_attack == DRAGON_WINGBEAT && e->attack_target_x >= 0 &&
        map_is_walkable(&g->map, e->attack_target_x, e->attack_target_y) &&
        !occupied(g->enemies, g->enemy_count, e->attack_target_x, e->attack_target_y) &&
        !(g->player.x == e->attack_target_x && g->player.y == e->attack_target_y) &&
        warned(e, e->attack_target_x, e->attack_target_y)) {
        e->x = e->attack_target_x;
        e->y = e->attack_target_y;
    }
    int damage = e->attack - g->player.defense / 2;
    return hit ? (damage > 3 ? damage : 3) : 0;
}

int red_dragon_tick(GameState *g, Enemy *e, EnemyProjectiles *shots) {
    if (!arena(g)) {
        return 0;
    }
    int dx = abs(g->player.x - e->x);
    int dy = abs(g->player.y - e->y);
    int distance = dx > dy ? dx : dy;
    if (!chamber(&g->map, g->player.x, g->player.y) && (distance > 16 || (e->hp == e->max_hp && !e->boss_attack))) {
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
        int damage = resolve(g, e, shots);
        if (e->boss_attack == DRAGON_INFERNO_FIRST) {
            declare(g, e, DRAGON_INFERNO_SECOND, 1);
        } else {
            int recovery = e->boss_attack == DRAGON_INFERNO_SECOND ? 2 : 1;
            clear_warning(e);
            e->move_timer = -recovery;
            e->boss_wave_used = (e->boss_wave_used + 1) % 2;
            push_message(g, recovery == 2 ? "Last Inferno ends. Two full counterattack actions!" : "Dragon exposed for one full counterattack action!");
        }
        return damage;
    }
    int phase = e->hp <= e->max_hp / 3 ? 2 : e->hp <= e->max_hp * 2 / 3 ? 1 : 0;
    if (phase > e->attack_phase) {
        e->attack_phase = phase;
        push_message(g, phase == 2 ? "Last Inferno! Two fixed breath cones, each warned separately, followed by a long opening." :
            "Scorched Wings! Warned landings leave three-action burning patches. The dragon remains vulnerable.");
    }
    int attack = e->attack_phase == 2 ? DRAGON_INFERNO_FIRST :
        e->attack_phase == 1 && e->boss_wave_used ? DRAGON_WINGBEAT :
        e->attack_phase == 0 && e->boss_wave_used && distance <= 1 ? DRAGON_SWEEP : DRAGON_BREATH;
    if (attack == DRAGON_WINGBEAT && (!chamber(&g->map, g->player.x, g->player.y) || !plain(tile_at(g, g->player.x, g->player.y)))) {
        attack = DRAGON_BREATH;
    }
    declare(g, e, attack, 0);
    return 0;
}

static int mantle(const Item *item) {
    return item->type == ITEM_ARMOR && strcmp(item->name, "Dragon Scale Mantle") == 0;
}

void red_dragon_restore_reward(GameState *g) {
    if (!arena(g)) {
        return;
    }
    int floor_mantle = 0;
    for (int i = 0; i < g->floor_item_count; i++) {
        floor_mantle |= g->floor_items[i].active && mantle(&g->floor_items[i].item);
    }
    const Room *r = &g->map.rooms[g->map.room_count - 1];
    int cache = 0;
    for (int y = r->y; y < r->y + r->h; y++) {
        for (int x = r->x; x < r->x + r->w; x++) {
            TileType tile = tile_at(g, x, y);
            if (cache_tile(tile)) {
                if (g->red_dragon_reward_state != 1 || floor_mantle) {
                    set_tile(g, x, y, red_dragon_base_tile(tile));
                } else {
                    cache = 1;
                }
            }
        }
    }
    if (g->red_dragon_reward_state != 1 || floor_mantle || cache) {
        return;
    }
    int bx = -1;
    int by = -1;
    int best = MAP_W + MAP_H;
    for (int y = r->y; y < r->y + r->h; y++) {
        for (int x = r->x; x < r->x + r->w; x++) {
            int d = abs(x - r->x - r->w / 2) + abs(y - r->y - r->h / 2);
            if (plain(tile_at(g, x, y)) && !occupied(g->enemies, g->enemy_count, x, y) && d < best) {
                bx = x;
                by = y;
                best = d;
            }
        }
    }
    if (bx >= 0) {
        TileType tile = tile_at(g, bx, by);
        set_tile(g, bx, by, tile == TILE_DRAGON_FLOOR ? TILE_DRAGON_RELIQUARY_FLOOR :
            tile == TILE_DRAGON_ASH ? TILE_DRAGON_RELIQUARY_ASH : TILE_DRAGON_RELIQUARY_HOARD);
        push_message(g, "Unclaimed Dragon Scale Mantle waits in the hoard reliquary. Use A nearby or P on it.");
    }
}

void red_dragon_award_reward(GameState *g, int x, int y) {
    if (g->red_dragon_reward_state) {
        return;
    }
    g->red_dragon_reward_state = 1;
    if (g->floor_item_count < MAX_FLOOR_ITEMS) {
        TileType tile = tile_at(g, x, y);
        FloorItem *f = &g->floor_items[g->floor_item_count++];
        *f = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = tile, .item = item_make_dragon_scale_mantle()};
        g->map.tiles[y][x] = TILE_ITEM;
        push_message(g, "Dragon Scale Mantle dropped! Unclaimed equipment remains recoverable on return.");
    }
    red_dragon_restore_reward(g);
}

static int nearby_cache(const GameState *g, int pickup) {
    if (!arena(g) || g->red_dragon_reward_state != 1) {
        return 0;
    }
    static const int offsets[5][2] = {{0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int i = 0; i < (pickup ? 1 : 5); i++) {
        if (cache_tile(tile_at(g, g->player.x + offsets[i][0], g->player.y + offsets[i][1]))) {
            return 1;
        }
    }
    return 0;
}

int red_dragon_has_interaction(const GameState *g) {
    return nearby_cache(g, 0);
}

int red_dragon_interact(GameState *g, int pickup) {
    if (!nearby_cache(g, pickup)) {
        return 0;
    }
    if (g->inventory_count >= MAX_INVENTORY) {
        push_message(g, "Inventory full. The Dragon Scale Mantle remains reserved in the hoard.");
        return 1;
    }
    g->inventory[g->inventory_count++] = item_make_dragon_scale_mantle();
    g->red_dragon_reward_state = 2;
    red_dragon_restore_reward(g);
    push_message(g, "Dragon Scale Mantle recovered! Ilya's golden goblet remains a separate quest.");
    return 1;
}

void red_dragon_cancel_landing(GameState *g, int x, int y) {
    if (!arena(g)) {
        return;
    }
    TileType tile = tile_at(g, x, y);
    if (red_dragon_hot_turns(tile)) {
        set_tile(g, x, y, red_dragon_base_tile(tile));
    }
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (red_dragon_attack_marks(g, e, x, y)) {
            clear_warning(e);
            e->move_timer = -1;
        }
    }
}

static void migrate_map(const GameState *g, Map *m, Enemy *enemies, int count, int px, int py) {
    for (int i = 0; i < count; i++) {
        Enemy *e = &enemies[i];
        if (e->type != ENEMY_RED_DRAGON) {
            continue;
        }
        e->move_timer = 0;
        e->attack_phase = 0;
        e->boss_attack = 0;
        e->boss_wave_used = 0;
        clear_warning(e);
        // Older pursuit could leave the lair; relocate only that legacy case without healing.
        if (e->active && !chamber(m, e->x, e->y) && m->room_count > 0) {
            const Room *r = &m->rooms[m->room_count - 1];
            int best = MAP_W + MAP_H;
            for (int y = r->y; y < r->y + r->h; y++) {
                for (int x = r->x; x < r->x + r->w; x++) {
                    int d = abs(x - r->x - r->w / 2) + abs(y - r->y - r->h / 2);
                    if (plain(m->tiles[y][x]) && !occupied(enemies, count, x, y) && !(x == px && y == py) &&
                        !(g->portal_active && g->portal_location == LOCATION_DRAGONSPINE && g->portal_level == DRAGONSPINE_DEPTH &&
                        x == g->portal_x && y == g->portal_y) && d < best) {
                        e->x = x;
                        e->y = y;
                        best = d;
                    }
                }
            }
        }
    }
    int x = g->portal_x;
    int y = g->portal_y;
    int protect = g->portal_active && g->portal_location == LOCATION_DRAGONSPINE && g->portal_level == DRAGONSPINE_DEPTH &&
        x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && plain(m->tiles[y][x]);
    TileType original = protect ? m->tiles[y][x] : TILE_DRAGON_FLOOR;
    if (protect) {
        m->tiles[y][x] = TILE_PORTAL;
    }
    red_dragon_prepare_map(m, enemies, count, px, py);
    if (protect) {
        m->tiles[y][x] = original;
    }
}

void red_dragon_migrate(GameState *g) {
    if (arena(g)) {
        migrate_map(g, &g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    LevelCache *cache = &g->dragonspine_cache[DRAGONSPINE_DEPTH - 1];
    if (cache->valid) {
        migrate_map(g, &cache->map, cache->enemies, cache->enemy_count, -1, -1);
    }
    g->red_dragon_reward_state = (g->defeated_bosses & (1 << LOCATION_DRAGONSPINE)) ? 2 : 0;
    for (int i = 0; i < g->floor_item_count; i++) {
        if (g->floor_items[i].active && mantle(&g->floor_items[i].item)) {
            g->red_dragon_reward_state = 1;
        }
    }
    for (int i = 0; i < g->inventory_count; i++) {
        if (mantle(&g->inventory[i])) {
            g->red_dragon_reward_state = 2;
        }
    }
}
