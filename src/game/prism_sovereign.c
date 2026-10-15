#include "prism_sovereign.h"
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

static int arena(const GameState *g) {
    return g->location == LOCATION_GLASSDEEP && g->level == GLASSDEEP_DEPTH && g->map.room_count > 0;
}

static int reflector(TileType tile) {
    return tile >= TILE_PRISM_LEFT && tile <= TILE_PRISM_RETURN;
}

static int reliquary(TileType tile) {
    return tile == TILE_PRISM_RELIQUARY || tile == TILE_PRISM_PAVED_RELIQUARY;
}

static int plain(TileType tile) {
    return tile == TILE_GLASSDEEP_FLOOR || tile == TILE_GLASSDEEP_RUIN;
}

void prism_sovereign_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py) {
    const Enemy *boss = NULL;
    for (int i = 0; i < count; i++) {
        if (enemies[i].active && enemies[i].type == ENEMY_PRISM_SOVEREIGN) {
            boss = &enemies[i];
        }
    }
    if (!boss || !m->room_count) {
        return;
    }
    const Room *room = &m->rooms[m->room_count - 1];
    for (int y = room->y; y < room->y + room->h; y++) {
        for (int x = room->x; x < room->x + room->w; x++) {
            if (reflector(m->tiles[y][x])) {
                return;
            }
        }
    }
    // Prefer a north/south pair; a paved, walkable east/west pair is the fallback.
    for (int axis = 0; axis < 2; axis++) {
        int points[2][2];
        int found = 0;
        for (int side = -1; side <= 1; side += 2) {
            for (int step = 1; step <= 4; step++) {
                int x = boss->x + (axis ? side * step : 0);
                int y = boss->y + (axis ? 0 : side * step);
                if (x < room->x || y < room->y || x >= room->x + room->w || y >= room->y + room->h || !map_is_walkable(m, x, y)) {
                    break;
                }
                if (step >= 2 && plain(m->tiles[y][x]) && !(x == px && y == py) && !occupied(enemies, count, x, y)) {
                    points[found][0] = x;
                    points[found++][1] = y;
                    break;
                }
            }
        }
        if (found == 2) {
            for (int n = 0; n < 2; n++) {
                m->tiles[points[n][1]][points[n][0]] = TILE_PRISM_LEFT;
            }
            return;
        }
    }
}

// One visible bounce per branch; the budget includes the incoming and reflected legs.
// Saved terminal cells bound the route, so removing an actor never adds unannounced cells.
static int trace(const GameState *g, const Enemy *e, int dx, int dy, int ex, int ey, int locked, PrismSegment *out, int *tx, int *ty) {
    int x = e->x;
    int y = e->y;
    int sx = x;
    int sy = y;
    int bounced = 0;
    int count = 0;
    for (int step = 0; step < 8; step++) {
        int nx = x + dx;
        int ny = y + dy;
        int crown = nx == e->x && ny == e->y;
        if (!map_is_walkable(&g->map, nx, ny) || (!crown && occupied(g->enemies, g->enemy_count, nx, ny))) {
            break;
        }
        x = nx;
        y = ny;
        if ((locked && x == ex && y == ey) || crown) {
            break;
        }
        TileType face = tile_at(g, x, y);
        if (!bounced && reflector(face)) {
            out[count++] = (PrismSegment){sx, sy, x, y};
            sx = x;
            sy = y;
            int old_dx = dx;
            if (face == TILE_PRISM_RETURN) {
                dx = -dx;
                dy = -dy;
            } else if (face == TILE_PRISM_LEFT) {
                dx = dy;
                dy = -old_dx;
            } else {
                dx = -dy;
                dy = old_dx;
            }
            bounced = 1;
        }
    }
    if (sx != x || sy != y) {
        out[count++] = (PrismSegment){sx, sy, x, y};
    }
    *tx = x;
    *ty = y;
    return count;
}

int prism_sovereign_segments(const GameState *g, const Enemy *e, PrismSegment out[4]) {
    if (!e->active || e->type != ENEMY_PRISM_SOVEREIGN || e->move_timer <= 0 || abs(e->facing_dx) + abs(e->facing_dy) != 1) {
        return 0;
    }
    int count = 0;
    int endpoints[2][2] = {{e->attack_target_x, e->attack_target_y}, {e->boss_aux_x, e->boss_aux_y}};
    for (int n = 0; n < (e->boss_attack == PRISM_SPLIT ? 2 : 1); n++) {
        if (endpoints[n][0] >= 0) {
            int tx;
            int ty;
            int sign = n ? -1 : 1;
            count += trace(g, e, e->facing_dx * sign, e->facing_dy * sign, endpoints[n][0], endpoints[n][1], 1, out + count, &tx, &ty);
        }
    }
    return count;
}

static int segment_has(const PrismSegment *s, int x, int y) {
    int dx = (s->x2 > s->x1) - (s->x2 < s->x1);
    int dy = (s->y2 > s->y1) - (s->y2 < s->y1);
    int length = abs(s->x2 - s->x1) + abs(s->y2 - s->y1);
    for (int n = 1; n <= length; n++) {
        if (s->x1 + dx * n == x && s->y1 + dy * n == y) {
            return 1;
        }
    }
    return 0;
}

int prism_sovereign_attack_marks(const GameState *g, const Enemy *e, int x, int y) {
    PrismSegment segments[4];
    int count = prism_sovereign_segments(g, e, segments);
    for (int n = 0; n < count; n++) {
        if (segment_has(&segments[n], x, y)) {
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

static int can_escape(const GameState *g, const Enemy *e, int x, int y, int turns) {
    if (!prism_sovereign_attack_marks(g, e, x, y)) {
        return 1;
    }
    if (!turns) {
        return 0;
    }
    static const int steps[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (int n = 0; n < 4; n++) {
        int nx = x + steps[n][0];
        int ny = y + steps[n][1];
        if (map_is_walkable(&g->map, nx, ny) && !occupied(g->enemies, g->enemy_count, nx, ny) && can_escape(g, e, nx, ny, turns - 1)) {
            return 1;
        }
    }
    return 0;
}
int prism_sovereign_tick(GameState *g, Enemy *e, EnemyProjectiles *shots) {
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
        PrismSegment segments[4];
        int count = prism_sovereign_segments(g, e, segments);
        int hit = 0;
        int returned = 0;
        for (int n = 0; n < count; n++) {
            hit |= segment_has(&segments[n], g->player.x, g->player.y);
            returned |= segments[n].x2 == e->x && segments[n].y2 == e->y;
            if (shots && shots->count < MAX_ENEMIES) {
                shots->shots[shots->count++] = (EnemyProjectile){e->type, segments[n].x1, segments[n].y1, segments[n].x2, segments[n].y2};
            }
        }
        clear_targets(e);
        e->move_timer = returned ? -2 : -1;
        push_message(g, returned ? "Reflected light strikes the crown! Sovereign staggered for 2 full counterattack actions." :
            "The fixed prismatic route fires! Sovereign exposed for 1 counterattack action.");
        int damage = e->attack - g->player.defense / 2;
        return hit ? (damage > 3 ? damage : 3) : 0;
    }
    if (!e->attack_phase && e->hp <= e->max_hp / 2) {
        e->attack_phase = 1;
        push_message(g, "Fractured Crown! Split beams pass through the crystal faces. Every segment is marked before firing.");
    }
    e->boss_attack = e->attack_phase ? PRISM_SPLIT : PRISM_BEAM;
    e->move_timer = e->attack_phase ? 2 : 1;
    int dx = g->player.x - e->x;
    int dy = g->player.y - e->y;
    e->facing_dx = abs(dx) >= abs(dy) ? (dx < 0 ? -1 : 1) : 0;
    e->facing_dy = abs(dx) < abs(dy) ? (dy < 0 ? -1 : 1) : 0;
    if (e->attack_phase) {
        const Room *room = &g->map.rooms[g->map.room_count - 1];
        for (int y = room->y; y < room->y + room->h; y++) {
            for (int x = room->x; x < room->x + room->w; x++) {
                if (reflector(tile_at(g, x, y)) && (x == e->x || y == e->y)) {
                    e->facing_dx = (x > e->x) - (x < e->x);
                    e->facing_dy = (y > e->y) - (y < e->y);
                }
            }
        }
    }
    clear_targets(e);
    for (int n = 0; n < (e->attack_phase ? 2 : 1); n++) {
        PrismSegment segments[2];
        int tx;
        int ty;
        int sign = n ? -1 : 1;
        if (trace(g, e, e->facing_dx * sign, e->facing_dy * sign, 0, 0, 0, segments, &tx, &ty)) {
            if (n) {
                e->boss_aux_x = tx;
                e->boss_aux_y = ty;
            } else {
                e->attack_target_x = tx;
                e->attack_target_y = ty;
            }
        }
    }
    if (!can_escape(g, e, g->player.x, g->player.y, e->move_timer)) {
        clear_targets(e);
    }
    push_message(g, e->attack_phase ? "Split beams in 2 actions! Both reflected routes and endpoints are fixed; faces are locked." :
        "Prismatic beam in 1 action! Leave its fixed route; crystal faces are locked until it fires.");
    return 0;
}
void prism_sovereign_restore_reward(GameState *g) {
    if (!arena(g) || !(g->defeated_bosses & (1 << LOCATION_GLASSDEEP))) {
        return;
    }
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    int cache = 0;
    for (int y = room->y; y < room->y + room->h; y++) {
        for (int x = room->x; x < room->x + room->w; x++) {
            if (reliquary(tile_at(g, x, y))) {
                if (!g->prism_sovereign_reward_pending) {
                    set_tile(g, x, y, tile_at(g, x, y) == TILE_PRISM_PAVED_RELIQUARY ? TILE_GLASSDEEP_RUIN : TILE_GLASSDEEP_FLOOR);
                } else {
                    cache = 1;
                }
            }
        }
    }
    if (cache || !g->prism_sovereign_reward_pending) {
        return;
    }
    int bx = -1;
    int by = -1;
    int best = MAP_W + MAP_H;
    for (int y = room->y; y < room->y + room->h; y++) {
        for (int x = room->x; x < room->x + room->w; x++) {
            int d = abs(x - (room->x + room->w / 2)) + abs(y - (room->y + room->h / 2));
            if (plain(tile_at(g, x, y)) && !occupied(g->enemies, g->enemy_count, x, y) && d < best) {
                bx = x;
                by = y;
                best = d;
            }
        }
    }
    if (bx >= 0) {
        set_tile(g, bx, by, tile_at(g, bx, by) == TILE_GLASSDEEP_RUIN ? TILE_PRISM_PAVED_RELIQUARY : TILE_PRISM_RELIQUARY);
        push_message(g, "Prism Sovereign rewards wait in the prismatic reliquary. Interact beside it or press P on it.");
    }
}

static TileType interaction_at(const GameState *g, int *x, int *y, int pickup) {
    if (!arena(g)) {
        return TILE_GLASSDEEP_FLOOR;
    }
    static const int offsets[5][2] = {{0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int n = 0; n < (pickup ? 1 : 5); n++) {
        *x = g->player.x + offsets[n][0];
        *y = g->player.y + offsets[n][1];
        TileType tile = tile_at(g, *x, *y);
        if ((reliquary(tile) && g->prism_sovereign_reward_pending) || (!pickup && reflector(tile))) {
            return tile;
        }
    }
    return TILE_GLASSDEEP_FLOOR;
}

int prism_sovereign_has_interaction(const GameState *g) {
    int x;
    int y;
    return interaction_at(g, &x, &y, 0) != TILE_GLASSDEEP_FLOOR;
}

static int interact_at(GameState *g, int pickup) {
    int x;
    int y;
    TileType tile = interaction_at(g, &x, &y, pickup);
    if (tile == TILE_GLASSDEEP_FLOOR) {
        return 0;
    }
    if (reliquary(tile)) {
        for (int flag = PRISM_ROBES_PENDING; flag <= PRISM_POTION_PENDING; flag <<= 1) {
            if ((g->prism_sovereign_reward_pending & flag) && g->inventory_count < MAX_INVENTORY) {
                Item reward = flag == PRISM_ROBES_PENDING ? item_make_prismweave_robes() : item_make_strength_potion();
                g->inventory[g->inventory_count++] = reward;
                g->score += reward.value;
                g->prism_sovereign_reward_pending &= ~flag;
                char message[MAX_MESSAGE_LEN];
                snprintf(message, sizeof(message), "%s recovered from the prismatic reliquary!", reward.name);
                push_message(g, message);
            }
        }
        if (g->prism_sovereign_reward_pending) {
            push_message(g, "Inventory full. Remaining Sovereign rewards stay in the prismatic reliquary.");
        } else {
            set_tile(g, x, y, tile == TILE_PRISM_PAVED_RELIQUARY ? TILE_GLASSDEEP_RUIN : TILE_GLASSDEEP_FLOOR);
        }
        return 1;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (e->active && e->type == ENEMY_PRISM_SOVEREIGN && e->move_timer > 0) {
            push_message(g, "Crystal faces are locked during a beam warning. Rotate between attacks.");
            return 1;
        }
    }
    TileType next = tile == TILE_PRISM_RETURN ? TILE_PRISM_LEFT : tile + 1;
    set_tile(g, x, y, next);
    push_message(g, next == TILE_PRISM_LEFT ? "Crystal face: TURN LEFT. The next incoming beam bends left once." :
        next == TILE_PRISM_RIGHT ? "Crystal face: TURN RIGHT. The next incoming beam bends right once." :
        "Crystal face: RETURN TO CROWN. The next beam retraces its route and staggers the Sovereign.");
    return 1;
}

int prism_sovereign_interact(GameState *g) {
    return interact_at(g, 0);
}

int prism_sovereign_pickup(GameState *g) {
    return interact_at(g, 1);
}

void prism_sovereign_cancel_landing(GameState *g, int x, int y) {
    if (!arena(g)) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (prism_sovereign_attack_marks(g, e, x, y)) {
            clear_targets(e);
            e->move_timer = -1;
        }
    }
}

static void reset_enemies(Enemy *enemies, int count) {
    for (int i = 0; i < count; i++) {
        Enemy *e = &enemies[i];
        if (e->type == ENEMY_PRISM_SOVEREIGN) {
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
    int protect = g->portal_active && g->portal_location == LOCATION_GLASSDEEP && g->portal_level == GLASSDEEP_DEPTH &&
        x >= 0 && x < MAP_W && y >= 0 && y < MAP_H && (m->tiles[y][x] == TILE_GLASSDEEP_FLOOR || m->tiles[y][x] == TILE_GLASSDEEP_RUIN);
    TileType original = protect ? m->tiles[y][x] : TILE_GLASSDEEP_FLOOR;
    if (protect) {
        m->tiles[y][x] = TILE_PORTAL;
    }
    prism_sovereign_prepare_map(m, enemies, count, px, py);
    if (protect) {
        m->tiles[y][x] = original;
    }
}

void prism_sovereign_migrate(GameState *g) {
    reset_enemies(g->enemies, g->enemy_count);
    LevelCache *cache = &g->glassdeep_cache[GLASSDEEP_DEPTH - 1];
    if (cache->valid) {
        reset_enemies(cache->enemies, cache->enemy_count);
        prepare_saved_map(g, &cache->map, cache->enemies, cache->enemy_count, -1, -1);
    }
    if (arena(g)) {
        prepare_saved_map(g, &g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    // Older victories keep their original drops, never a new repeat Strength Potion.
    g->prism_sovereign_reward_pending = 0;
}
