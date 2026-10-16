#include "thorn_regent.h"
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

// A lance ends at its locked target. Hedges, terrain and living actors block it.
static int lance_marks(const GameState *g, const Enemy *e, int tx, int ty) {
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
        if (!map_is_walkable(&g->map, x, y) || tile_at(g, x, y) == TILE_MOONVEIL_POOL ||
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

static int arena(const GameState *g) {
    return g->location == LOCATION_MOONVEIL && g->level == MOONVEIL_DEPTH && g->map.room_count > 0;
}

int thorn_regent_root_turns(TileType tile) {
    return tile == TILE_REGENT_ROOT_2 ? 2 : tile == TILE_REGENT_ROOT_1 ? 1 : 0;
}

int thorn_regent_wall_turns(TileType tile) {
    return tile >= TILE_REGENT_WALL_1 && tile <= TILE_REGENT_WALL_4 ? tile - TILE_REGENT_WALL_1 + 1 : 0;
}

int thorn_regent_node_turns(TileType tile) {
    return tile >= TILE_REGENT_NODE_1 && tile <= TILE_REGENT_NODE_5 ? tile - TILE_REGENT_NODE_1 + 1 : 0;
}

static int node(TileType tile) {
    return tile == TILE_REGENT_NODE || thorn_regent_node_turns(tile);
}

void thorn_regent_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py) {
    const Enemy *boss = NULL;
    for (int i = 0; i < count; i++) {
        if (enemies[i].active && enemies[i].type == ENEMY_THORN_REGENT) {
            boss = &enemies[i];
        }
    }
    if (!boss || !m->room_count) {
        return;
    }
    const Room *room = &m->rooms[m->room_count - 1];
    int existing = 0;
    for (int y = room->y; y < room->y + room->h; y++) {
        for (int x = room->x; x < room->x + room->w; x++) {
            existing += node(m->tiles[y][x]);
        }
    }
    for (int n = existing; n < 2; n++) {
        int tx = boss->x;
        int ty = room->y + (n ? room->h - 3 : 2);
        int bx = -1;
        int by = -1;
        int best = MAP_W + MAP_H;
        for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
            for (int x = room->x + 2; x < room->x + room->w - 2; x++) {
                int distance = abs(x - tx) + abs(y - ty);
                int sy = y < boss->y ? 1 : -1;
                if (m->tiles[y][x] != TILE_MOONVEIL_FLOOR || occupied(enemies, count, x, y) ||
                    (x == px && y == py) || distance >= best) {
                    continue;
                }
                int open = 1;
                for (int dx = -1; dx <= 1; dx++) {
                    open &= m->tiles[y + sy][x + dx] == TILE_MOONVEIL_FLOOR;
                }
                if (open) {
                    bx = x;
                    by = y;
                    best = distance;
                }
            }
        }
        if (bx >= 0) {
            m->tiles[by][bx] = TILE_REGENT_NODE;
        }
    }
}

int thorn_regent_attack_marks(const GameState *g, const Enemy *e, int x, int y) {
    if (!e->active || e->type != ENEMY_THORN_REGENT || e->move_timer <= 0 ||
        e->attack_target_x < 0 || !map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    if (e->boss_attack == REGENT_LANCE) {
        return lance_marks(g, e, x, y);
    }
    if (e->boss_attack == REGENT_ROOTS) {
        return chamber(&g->map, x, y) && abs(x - e->attack_target_x) + abs(y - e->attack_target_y) <= 1;
    }
    int offset = x - e->attack_target_x + 1;
    return e->boss_attack == REGENT_REGROW && y == e->attack_target_y && offset >= 0 && offset < 3 &&
        (e->boss_aux_x & (1 << offset));
}

static void clear_targets(Enemy *e) {
    e->attack_target_x = -1;
    e->attack_target_y = -1;
    e->boss_aux_x = 0;
    e->boss_aux_y = -1;
}

static int can_escape(const GameState *g, const Enemy *e, int x, int y, int turns) {
    if (!thorn_regent_attack_marks(g, e, x, y) && !thorn_regent_root_turns(tile_at(g, x, y))) {
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

// Preserve every existing route in the boss's connected component, not just one exit.
static int growth_connected(const GameState *g, const Enemy *e, int mask) {
    static unsigned char visited[MAP_H][MAP_W];
    static int queue[MAP_W * MAP_H];
    static const int steps[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    memset(visited, 0, sizeof(visited));
    for (int pass = 0; pass < 2; pass++) {
        int flag = 1 << pass;
        int head = 0;
        int tail = 0;
        queue[tail++] = e->y * MAP_W + e->x;
        visited[e->y][e->x] |= flag;
        while (head < tail) {
            int x = queue[head] % MAP_W;
            int y = queue[head++] / MAP_W;
            for (int n = 0; n < 4; n++) {
                int nx = x + steps[n][0];
                int ny = y + steps[n][1];
                int offset = nx - e->attack_target_x + 1;
                int blocked = pass && ny == e->attack_target_y && offset >= 0 && offset < 3 && (mask & (1 << offset));
                if (!blocked && map_is_walkable(&g->map, nx, ny) && !(visited[ny][nx] & flag)) {
                    visited[ny][nx] |= flag;
                    queue[tail++] = ny * MAP_W + nx;
                }
            }
        }
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            int offset = x - e->attack_target_x + 1;
            int blocked = y == e->attack_target_y && offset >= 0 && offset < 3 && (mask & (1 << offset));
            if (visited[y][x] == 1 && !blocked) {
                return 0;
            }
        }
    }
    return 1;
}

static int growth_mask(const GameState *g, const Enemy *e) {
    int mask = 0;
    for (int n = 0; n < 3; n++) {
        int x = e->attack_target_x + n - 1;
        int y = e->attack_target_y;
        if (chamber(&g->map, x, y) && tile_at(g, x, y) == TILE_MOONVEIL_FLOOR &&
            g->map.tiles[y][x] != TILE_ITEM && !(x == g->player.x && y == g->player.y) &&
            !occupied(g->enemies, g->enemy_count, x, y)) {
            mask |= 1 << n;
        }
    }
    return mask;
}

int thorn_regent_tick_floor(GameState *g) {
    if (!arena(g)) {
        return 0;
    }
    int hit = thorn_regent_root_turns(tile_at(g, g->player.x, g->player.y));
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    for (int y = room->y; y < room->y + room->h; y++) {
        for (int x = room->x; x < room->x + room->w; x++) {
            TileType tile = tile_at(g, x, y);
            int roots = thorn_regent_root_turns(tile);
            int wall = thorn_regent_wall_turns(tile);
            int cooldown = thorn_regent_node_turns(tile);
            if (roots) {
                set_tile(g, x, y, roots == 2 ? TILE_REGENT_ROOT_1 : TILE_MOONVEIL_FLOOR);
            } else if (wall) {
                set_tile(g, x, y, wall == 1 ? TILE_MOONVEIL_FLOOR : tile - 1);
            } else if (cooldown) {
                set_tile(g, x, y, cooldown == 1 ? TILE_REGENT_NODE : tile - 1);
            }
        }
    }
    if (hit) {
        push_message_kind(g, "Creeping roots sting your feet! Leave the numbered root patches.", MESSAGE_DAMAGE_TAKEN);
    }
    return hit ? 8 : 0;
}

int thorn_regent_tick(GameState *g, Enemy *e, EnemyProjectiles *shots) {
    if (!arena(g)) {
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
        int hit = e->boss_attack != REGENT_REGROW && thorn_regent_attack_marks(g, e, g->player.x, g->player.y);
        const Room *room = &g->map.rooms[g->map.room_count - 1];
        if (e->boss_attack == REGENT_REGROW && e->attack_target_x >= 0) {
            int mask = growth_mask(g, e) & e->boss_aux_x;
            if (growth_connected(g, e, mask)) {
                for (int n = 0; n < 3; n++) {
                    if (mask & (1 << n)) {
                        set_tile(g, e->attack_target_x + n - 1, e->attack_target_y, TILE_REGENT_WALL_4);
                    }
                }
            }
        } else if (e->boss_attack == REGENT_ROOTS) {
            for (int y = room->y; y < room->y + room->h; y++) {
                for (int x = room->x; x < room->x + room->w; x++) {
                    if (thorn_regent_attack_marks(g, e, x, y) && tile_at(g, x, y) == TILE_MOONVEIL_FLOOR) {
                        set_tile(g, x, y, TILE_REGENT_ROOT_2);
                    }
                }
            }
        } else if (e->boss_attack == REGENT_LANCE && e->attack_target_x >= 0 && shots && shots->count < MAX_ENEMIES) {
            if (lance_marks(g, e, e->attack_target_x, e->attack_target_y)) {
                shots->shots[shots->count++] = (EnemyProjectile){e->type, e->x, e->y, e->attack_target_x, e->attack_target_y};
            }
        }
        push_message(g, e->boss_attack == REGENT_LANCE ? "The thorn lance strikes its fixed line! Regent exposed." :
            e->boss_attack == REGENT_ROOTS ? "Creeping roots erupt for 2 actions! Regent exposed." :
            "The marked briars regrow for 4 actions! Tend a flowering node to open the flank; Regent exposed.");
        clear_targets(e);
        e->move_timer = -1;
        int damage = e->attack - g->player.defense / 2;
        return hit ? (damage > 3 ? damage : 3) : 0;
    }
    if (!e->attack_phase && e->hp <= e->max_hp / 2) {
        e->attack_phase = 1;
        push_message(g, "Briar Crown! Warned thorn strips reshape the flanks. Tend the flowering nodes to clear them.");
    }
    e->boss_attack = e->boss_attack == REGENT_LANCE ? REGENT_ROOTS :
        e->boss_attack == REGENT_ROOTS && e->attack_phase ? REGENT_REGROW : REGENT_LANCE;
    clear_targets(e);
    e->move_timer = e->boss_attack == REGENT_LANCE ? 1 : 2;
    e->attack_target_x = g->player.x;
    e->attack_target_y = g->player.y;
    if (e->boss_attack == REGENT_REGROW) {
        int nodes[2][2];
        int count = 0;
        const Room *room = &g->map.rooms[g->map.room_count - 1];
        for (int y = room->y; y < room->y + room->h && count < 2; y++) {
            for (int x = room->x; x < room->x + room->w && count < 2; x++) {
                if (node(tile_at(g, x, y))) {
                    nodes[count][0] = x;
                    nodes[count++][1] = y;
                }
            }
        }
        if (count) {
            int n = e->boss_wave_used % count;
            e->facing_dy = nodes[n][1] < e->y ? 1 : -1;
            e->attack_target_x = nodes[n][0];
            e->attack_target_y = nodes[n][1] + e->facing_dy;
            e->boss_aux_x = growth_mask(g, e);
            e->boss_wave_used = (n + 1) % count;
            if (!growth_connected(g, e, e->boss_aux_x)) {
                clear_targets(e);
            }
        } else {
            clear_targets(e);
        }
    }
    if (e->boss_attack == REGENT_LANCE && !lance_marks(g, e, e->attack_target_x, e->attack_target_y)) {
        clear_targets(e);
    }
    if (!can_escape(g, e, g->player.x, g->player.y, e->move_timer)) {
        clear_targets(e);
    }
    push_message(g, e->boss_attack == REGENT_LANCE ? "Thorn lance in 1 action! Step off its fixed line." :
        e->boss_attack == REGENT_ROOTS ? "Creeping roots in 2 actions! Walk out of the fixed cross." :
        "Briar regrowth in 2 actions! G marks the strip; tend its flowering node to cancel and open a flank.");
    return 0;
}
void thorn_regent_restore_reward(GameState *g) {
    if (!arena(g) || !(g->defeated_bosses & (1 << LOCATION_MOONVEIL))) {
        return;
    }
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    int cache = 0;
    for (int y = room->y; y < room->y + room->h; y++) {
        for (int x = room->x; x < room->x + room->w; x++) {
            if (tile_at(g, x, y) == TILE_REGENT_RELIQUARY) {
                if (!g->thorn_regent_reward_pending) {
                    set_tile(g, x, y, TILE_MOONVEIL_FLOOR);
                } else {
                    cache = 1;
                }
            }
        }
    }
    if (cache || !g->thorn_regent_reward_pending) {
        return;
    }
    int bx = -1;
    int by = -1;
    int best = MAP_W + MAP_H;
    for (int y = room->y; y < room->y + room->h; y++) {
        for (int x = room->x; x < room->x + room->w; x++) {
            int d = abs(x - (room->x + room->w / 2)) + abs(y - (room->y + room->h / 2));
            if (tile_at(g, x, y) == TILE_MOONVEIL_FLOOR && !occupied(g->enemies, g->enemy_count, x, y) && d < best) {
                bx = x;
                by = y;
                best = d;
            }
        }
    }
    if (bx >= 0) {
        set_tile(g, bx, by, TILE_REGENT_RELIQUARY);
        push_message(g, "Regent rewards wait in the flowered reliquary. Interact beside it or press P on it.");
    }
}

static TileType interaction_at(const GameState *g, int *x, int *y, int pickup) {
    if (!arena(g)) {
        return TILE_MOONVEIL_FLOOR;
    }
    static const int offsets[5][2] = {{0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int n = 0; n < (pickup ? 1 : 5); n++) {
        *x = g->player.x + offsets[n][0];
        *y = g->player.y + offsets[n][1];
        TileType tile = tile_at(g, *x, *y);
        if ((tile == TILE_REGENT_RELIQUARY && g->thorn_regent_reward_pending) || (!pickup && node(tile))) {
            return tile;
        }
    }
    return TILE_MOONVEIL_FLOOR;
}

int thorn_regent_has_interaction(const GameState *g) {
    int x;
    int y;
    return interaction_at(g, &x, &y, 0) != TILE_MOONVEIL_FLOOR;
}

static int interact_at(GameState *g, int pickup) {
    int x;
    int y;
    TileType tile = interaction_at(g, &x, &y, pickup);
    if (tile == TILE_MOONVEIL_FLOOR) {
        return 0;
    }
    if (tile == TILE_REGENT_RELIQUARY) {
        for (int flag = REGENT_CLOAK_PENDING; flag <= REGENT_POTION_PENDING; flag <<= 1) {
            if ((g->thorn_regent_reward_pending & flag) && g->inventory_count < MAX_INVENTORY) {
                Item reward = flag == REGENT_CLOAK_PENDING ? item_make_briarweave_cloak() : item_make_strength_potion();
                g->inventory[g->inventory_count++] = reward;
                g->score += reward.value;
                g->thorn_regent_reward_pending &= ~flag;
                char message[MAX_MESSAGE_LEN];
                snprintf(message, sizeof(message), "%s recovered from the flowered reliquary!", reward.name);
                push_message(g, message);
            }
        }
        if (g->thorn_regent_reward_pending) {
            push_message(g, "Inventory full. Remaining Regent rewards stay in the flowered reliquary.");
        } else {
            set_tile(g, x, y, TILE_MOONVEIL_FLOOR);
        }
        return 1;
    }
    if (thorn_regent_node_turns(tile)) {
        push_message(g, "This flowering node is resting. Its number shows the cooldown.");
        return 1;
    }
    int removed = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx;
            int ny = y + dy;
            if (thorn_regent_wall_turns(tile_at(g, nx, ny))) {
                set_tile(g, nx, ny, TILE_MOONVEIL_FLOOR);
                removed++;
            }
        }
    }
    int interrupted = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (e->active && e->type == ENEMY_THORN_REGENT && e->boss_attack == REGENT_REGROW && e->move_timer > 0 &&
            e->attack_target_x == x && e->attack_target_y - e->facing_dy == y) {
            clear_targets(e);
            e->move_timer = -3; // This action's enemy phase leaves two complete counterattacks.
            interrupted = 1;
        } else if (removed && e->active && e->type == ENEMY_THORN_REGENT && e->move_timer <= 0) {
            e->move_timer = -3;
        }
    }
    set_tile(g, x, y, TILE_REGENT_NODE_5);
    push_message(g, interrupted ? "Flower tended! Regrowth canceled; Regent exposed for 2 full counterattacks." :
        removed ? "Flower tended! The adjacent thorn strip recedes, opening the flank." :
        "Flower tended. It rests for 4 actions; use it beside a regrowing or grown thorn strip.");
    return 1;
}

int thorn_regent_interact(GameState *g) {
    return interact_at(g, 0);
}

int thorn_regent_pickup(GameState *g) {
    return interact_at(g, 1);
}

void thorn_regent_cancel_landing(GameState *g, int x, int y) {
    if (!arena(g)) {
        return;
    }
    if (thorn_regent_root_turns(tile_at(g, x, y))) {
        set_tile(g, x, y, TILE_MOONVEIL_FLOOR);
    }
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (thorn_regent_attack_marks(g, e, x, y)) {
            clear_targets(e);
            e->move_timer = -1;
        }
    }
}

static void reset_enemies(Enemy *enemies, int count) {
    for (int i = 0; i < count; i++) {
        Enemy *e = &enemies[i];
        if (e->type == ENEMY_THORN_REGENT) {
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
    int protect = g->portal_active && g->portal_location == LOCATION_MOONVEIL && g->portal_level == MOONVEIL_DEPTH &&
        x >= 0 && x < MAP_W && y >= 0 && y < MAP_H && m->tiles[y][x] == TILE_MOONVEIL_FLOOR;
    if (protect) {
        m->tiles[y][x] = TILE_PORTAL;
    }
    thorn_regent_prepare_map(m, enemies, count, px, py);
    if (protect) {
        m->tiles[y][x] = TILE_MOONVEIL_FLOOR;
    }
}

void thorn_regent_migrate(GameState *g) {
    reset_enemies(g->enemies, g->enemy_count);
    LevelCache *cache = &g->moonveil_cache[MOONVEIL_DEPTH - 1];
    if (cache->valid) {
        reset_enemies(cache->enemies, cache->enemy_count);
        prepare_saved_map(g, &cache->map, cache->enemies, cache->enemy_count, -1, -1);
    }
    if (arena(g)) {
        prepare_saved_map(g, &g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    // Older victories keep their original drops, never a new repeat Strength Potion.
    g->thorn_regent_reward_pending = 0;
}
