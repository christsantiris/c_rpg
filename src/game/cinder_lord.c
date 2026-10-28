#include "cinder_lord.h"
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

// A lance ends at its locked target. Basalt, terrain and living actors block it.
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
        if (!map_is_walkable(&g->map, x, y) || tile_at(g, x, y) == TILE_ASHEN_LAVA ||
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
    return g->location == LOCATION_ASHEN && g->level == ASHEN_DEPTH && g->map.room_count > 0;
}

int cinder_lord_hot_turns(TileType tile) {
    return tile == TILE_CINDER_HOT_2 ? 2 : tile == TILE_CINDER_HOT_1 ? 1 : 0;
}

int cinder_lord_cool_turns(TileType tile) {
    return tile >= TILE_CINDER_COOL_1 && tile <= TILE_CINDER_COOL_5 ? tile - TILE_CINDER_COOL_1 + 1 : 0;
}

int cinder_lord_sluice_turns(TileType tile) {
    return tile >= TILE_CINDER_SLUICE_1 && tile <= TILE_CINDER_SLUICE_5 ? tile - TILE_CINDER_SLUICE_1 + 1 : 0;
}

static int sluice(TileType tile) {
    return tile == TILE_CINDER_SLUICE || cinder_lord_sluice_turns(tile);
}

void cinder_lord_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py) {
    const Enemy *boss = NULL;
    for (int i = 0; i < count; i++) {
        if (enemies[i].active && enemies[i].type == ENEMY_CINDER_LORD) {
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
            existing += sluice(m->tiles[y][x]);
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
                if (m->tiles[y][x] != TILE_ASHEN_FLOOR || occupied(enemies, count, x, y) ||
                    (x == px && y == py) || distance >= best) {
                    continue;
                }
                int open = 1;
                for (int dx = -1; dx <= 1; dx++) {
                    open &= m->tiles[y + sy][x + dx] == TILE_ASHEN_FLOOR &&
                        !(x + dx == px && y + sy == py) && !occupied(enemies, count, x + dx, y + sy);
                }
                if (open) {
                    bx = x;
                    by = y;
                    best = distance;
                }
            }
        }
        if (bx >= 0) {
            m->tiles[by][bx] = TILE_CINDER_SLUICE;
            int sy = by < boss->y ? 1 : -1;
            for (int dx = -1; dx <= 1; dx++) {
                m->tiles[by + sy][bx + dx] = TILE_CINDER_CHANNEL;
            }
        }
    }
}

// Both ring fronts are locked at declaration; the inner front resolves one action earlier.
int cinder_lord_attack_turns(const GameState *g, const Enemy *e, int x, int y) {
    if (!e->active || e->type != ENEMY_CINDER_LORD || e->move_timer <= 0 ||
        e->attack_target_x < 0 || !map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    if (e->boss_attack == CINDER_LANCE) {
        return lance_marks(g, e, x, y) ? e->move_timer : 0;
    }
    if (!chamber(&g->map, x, y)) {
        return 0;
    }
    int distance = abs(x - e->attack_target_x) + abs(y - e->attack_target_y);
    if (e->boss_attack == CINDER_ERUPTION) {
        return distance <= 1 && !cinder_lord_cool_turns(tile_at(g, x, y)) ? e->move_timer : 0;
    }
    if (e->boss_attack == CINDER_RING && distance >= 1 && distance <= 2) {
        int turns = distance - (2 - e->move_timer);
        return turns > 0 ? turns : 0;
    }
    return 0;
}

static void clear_targets(Enemy *e) {
    e->attack_target_x = -1;
    e->attack_target_y = -1;
    e->boss_aux_x = -1;
    e->boss_aux_y = -1;
}

static int can_escape(const GameState *g, const Enemy *e, int x, int y, int turns) {
    if (!cinder_lord_attack_turns(g, e, x, y) && !cinder_lord_hot_turns(tile_at(g, x, y))) {
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

int cinder_lord_tick_floor(GameState *g) {
    if (!arena(g)) {
        return 0;
    }
    int hit = cinder_lord_hot_turns(tile_at(g, g->player.x, g->player.y));
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    for (int y = room->y; y < room->y + room->h; y++) {
        for (int x = room->x; x < room->x + room->w; x++) {
            TileType tile = tile_at(g, x, y);
            int hot = cinder_lord_hot_turns(tile);
            int cool = cinder_lord_cool_turns(tile);
            int timer = cinder_lord_sluice_turns(tile);
            if (hot) {
                set_tile(g, x, y, hot == 2 ? TILE_CINDER_HOT_1 : TILE_ASHEN_FLOOR);
            } else if (cool) {
                set_tile(g, x, y, cool == 1 ? TILE_CINDER_CHANNEL : tile - 1);
            } else if (timer) {
                set_tile(g, x, y, timer == 1 ? TILE_CINDER_SLUICE : tile - 1);
            }
        }
    }
    if (hit) {
        push_message_kind(g, "Lingering embers burn your feet! Leave the H-numbered hot tiles.", MESSAGE_DAMAGE_TAKEN);
    }
    return hit ? 8 : 0;
}

static int resolve_attack(GameState *g, Enemy *e, EnemyProjectiles *shots) {
    int hit = cinder_lord_attack_turns(g, e, g->player.x, g->player.y) == 1;
    if (e->boss_attack == CINDER_LANCE) {
        if (e->attack_target_x >= 0 && shots && shots->count < MAX_ENEMIES && lance_marks(g, e, e->attack_target_x, e->attack_target_y)) {
            shots->shots[shots->count++] = (EnemyProjectile){e->type, e->x, e->y, e->attack_target_x, e->attack_target_y};
        }
    } else {
        const Room *room = &g->map.rooms[g->map.room_count - 1];
        for (int y = room->y; y < room->y + room->h; y++) {
            for (int x = room->x; x < room->x + room->w; x++) {
                if (cinder_lord_attack_turns(g, e, x, y) == 1 && tile_at(g, x, y) == TILE_ASHEN_FLOOR) {
                    set_tile(g, x, y, TILE_CINDER_HOT_2);
                }
            }
        }
    }
    int damage = e->attack - g->player.defense / 2;
    return hit ? (damage > 3 ? damage : 3) : 0;
}

int cinder_lord_tick(GameState *g, Enemy *e, EnemyProjectiles *shots) {
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
        if (e->boss_attack == CINDER_RING && e->move_timer == 2) {
            int damage = resolve_attack(g, e, shots);
            e->move_timer = 1;
            push_message(g, "The inner ember ring bursts! The already marked outer ring follows in 1 action.");
            return damage;
        }
        e->move_timer--;
        if (e->move_timer > 0) {
            return 0;
        }
        // Keep the warning's final shape while resolving it, then enter recovery.
        e->move_timer = 1;
        int damage = resolve_attack(g, e, shots);
        push_message(g, e->boss_attack == CINDER_LANCE ? "The cinder lance strikes its fixed line! Cinder Lord exposed." :
            e->boss_attack == CINDER_ERUPTION ? "Lava erupts at its locked cross! Cinder Lord exposed." :
            "The outer ember ring bursts! Cinder Lord exposed.");
        clear_targets(e);
        e->move_timer = -1;
        return damage;
    }
    if (!e->attack_phase && e->hp <= e->max_hp / 2) {
        e->attack_phase = 1;
        push_message(g, "Broken Crucible! Ember rings join the eruptions. Inner 1, outer 2: both fronts are already marked.");
    }
    e->boss_attack = !e->attack_phase ? (e->boss_attack == CINDER_LANCE ? CINDER_ERUPTION : CINDER_LANCE) :
        (e->boss_attack == CINDER_ERUPTION ? CINDER_RING : CINDER_ERUPTION);
    clear_targets(e);
    e->move_timer = e->boss_attack == CINDER_LANCE ? 1 : 2;
    e->attack_target_x = e->boss_attack == CINDER_RING ? e->x : g->player.x;
    e->attack_target_y = e->boss_attack == CINDER_RING ? e->y : g->player.y;
    if ((e->boss_attack == CINDER_LANCE && !lance_marks(g, e, e->attack_target_x, e->attack_target_y)) ||
        !can_escape(g, e, g->player.x, g->player.y, e->move_timer)) {
        clear_targets(e);
    }
    push_message(g, e->boss_attack == CINDER_LANCE ? "Cinder lance in 1 action! Step off its fixed line." :
        e->boss_attack == CINDER_ERUPTION ? "Lava eruption in 2 actions! Leave the cross or open a sluice feeding its marked channel." :
        "Expanding embers! Inner ring in 1 action, outer in 2. Walk outward; both footprints are fixed.");
    return 0;
}
void cinder_lord_restore_reward(GameState *g) {
    if (!arena(g) || !(g->defeated_bosses & (1 << LOCATION_ASHEN))) {
        return;
    }
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    int cache = 0;
    for (int y = room->y; y < room->y + room->h; y++) {
        for (int x = room->x; x < room->x + room->w; x++) {
            if (tile_at(g, x, y) == TILE_CINDER_RELIQUARY) {
                if (!g->cinder_lord_reward_pending) {
                    set_tile(g, x, y, TILE_ASHEN_FLOOR);
                } else {
                    cache = 1;
                }
            }
        }
    }
    if (cache || !g->cinder_lord_reward_pending) {
        return;
    }
    int bx = -1;
    int by = -1;
    int best = MAP_W + MAP_H;
    for (int y = room->y; y < room->y + room->h; y++) {
        for (int x = room->x; x < room->x + room->w; x++) {
            int d = abs(x - (room->x + room->w / 2)) + abs(y - (room->y + room->h / 2));
            if (tile_at(g, x, y) == TILE_ASHEN_FLOOR && !occupied(g->enemies, g->enemy_count, x, y) && d < best) {
                bx = x;
                by = y;
                best = d;
            }
        }
    }
    if (bx >= 0) {
        set_tile(g, bx, by, TILE_CINDER_RELIQUARY);
        push_message(g, "Cinder Lord rewards wait in the forge reliquary. Open with Interact, then Pick Up the released loot.");
    }
}

static TileType interaction_at(const GameState *g, int *x, int *y, int pickup) {
    if (!arena(g)) {
        return TILE_ASHEN_FLOOR;
    }
    static const int offsets[5][2] = {{0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int n = 0; n < (pickup ? 1 : 5); n++) {
        *x = g->player.x + offsets[n][0];
        *y = g->player.y + offsets[n][1];
        TileType tile = tile_at(g, *x, *y);
        if ((tile == TILE_CINDER_RELIQUARY && g->cinder_lord_reward_pending) || (!pickup && sluice(tile))) {
            return tile;
        }
    }
    return TILE_ASHEN_FLOOR;
}

int cinder_lord_has_interaction(const GameState *g) {
    int x;
    int y;
    return interaction_at(g, &x, &y, 0) != TILE_ASHEN_FLOOR;
}

static int interact_at(GameState *g, int pickup) {
    int x;
    int y;
    TileType tile = interaction_at(g, &x, &y, pickup);
    if (tile == TILE_ASHEN_FLOOR) {
        return 0;
    }
    if (tile == TILE_CINDER_RELIQUARY) {
        for (int flag = CINDER_WEAPON_PENDING; flag <= CINDER_POTION_PENDING; flag <<= 1) {
            if ((g->cinder_lord_reward_pending & flag) && g->inventory_count < MAX_INVENTORY) {
                Item reward = flag == CINDER_WEAPON_PENDING ? item_make_embercleaver() : item_make_strength_potion();
                g->inventory[g->inventory_count++] = reward;
                g->score += reward.value;
                g->cinder_lord_reward_pending &= ~flag;
                char message[MAX_MESSAGE_LEN];
                snprintf(message, sizeof(message), "%s recovered from the forge reliquary!", reward.name);
                push_message(g, message);
            }
        }
        if (g->cinder_lord_reward_pending) {
            push_message(g, "Inventory full. Remaining Cinder Lord rewards stay in the forge reliquary.");
        } else {
            set_tile(g, x, y, TILE_ASHEN_FLOOR);
        }
        return 1;
    }
    if (cinder_lord_sluice_turns(tile)) {
        push_message(g, "This forge sluice is cooling. Its number shows the remaining cooldown.");
        return 1;
    }
    int redirected = 0;
    // Inspect the warning before cooling the channel, since cooling makes eruption cells safe.
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_CINDER_LORD || e->boss_attack != CINDER_ERUPTION || e->move_timer <= 0) {
            continue;
        }
        int overlap = 0;
        for (int dy = -1; dy <= 1; dy += 2) {
            for (int dx = -1; dx <= 1; dx++) {
                TileType channel = tile_at(g, x + dx, y + dy);
                overlap |= (channel == TILE_CINDER_CHANNEL || cinder_lord_cool_turns(channel)) &&
                    cinder_lord_attack_turns(g, e, x + dx, y + dy);
            }
        }
        if (overlap) {
            clear_targets(e);
            e->move_timer = -3; // This action's enemy phase leaves two complete counterattacks.
            redirected = 1;
        }
    }
    for (int dy = -1; dy <= 1; dy += 2) {
        for (int dx = -1; dx <= 1; dx++) {
            TileType channel = tile_at(g, x + dx, y + dy);
            if (channel == TILE_CINDER_CHANNEL || cinder_lord_cool_turns(channel)) {
                set_tile(g, x + dx, y + dy, TILE_CINDER_COOL_5);
            }
        }
    }
    set_tile(g, x, y, TILE_CINDER_SLUICE_5);
    push_message(g, redirected ? "Lava redirected into the cooling channel! Cinder Lord exposed for 2 full counterattacks." :
        "Sluice opened; its channel cools for 4 actions. No warned eruption was feeding this channel.");
    return 1;
}

int cinder_lord_interact(GameState *g) {
    return interact_at(g, 0);
}

int cinder_lord_pickup(GameState *g) {
    return interact_at(g, 1);
}

void cinder_lord_cancel_landing(GameState *g, int x, int y) {
    if (!arena(g)) {
        return;
    }
    if (cinder_lord_hot_turns(tile_at(g, x, y))) {
        set_tile(g, x, y, TILE_ASHEN_FLOOR);
    }
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (cinder_lord_attack_turns(g, e, x, y)) {
            clear_targets(e);
            e->move_timer = -1;
        }
    }
}

static void reset_enemies(Enemy *enemies, int count) {
    for (int i = 0; i < count; i++) {
        Enemy *e = &enemies[i];
        if (e->type == ENEMY_CINDER_LORD) {
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
    int protect = g->portal_active && g->portal_location == LOCATION_ASHEN && g->portal_level == ASHEN_DEPTH &&
        x >= 0 && x < MAP_W && y >= 0 && y < MAP_H && m->tiles[y][x] == TILE_ASHEN_FLOOR;
    if (protect) {
        m->tiles[y][x] = TILE_PORTAL;
    }
    cinder_lord_prepare_map(m, enemies, count, px, py);
    if (protect) {
        m->tiles[y][x] = TILE_ASHEN_FLOOR;
    }
}

void cinder_lord_migrate(GameState *g) {
    reset_enemies(g->enemies, g->enemy_count);
    LevelCache *cache = &g->ashen_cache[ASHEN_DEPTH - 1];
    if (cache->valid) {
        reset_enemies(cache->enemies, cache->enemy_count);
        prepare_saved_map(g, &cache->map, cache->enemies, cache->enemy_count, -1, -1);
    }
    if (arena(g)) {
        prepare_saved_map(g, &g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    // Older victories keep their original drops, never a new repeat Strength Potion.
    g->cinder_lord_reward_pending = 0;
}
