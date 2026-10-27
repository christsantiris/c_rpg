#include "goblin_king.h"
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
    return g->location == LOCATION_MOUNTAINS && g->level == MOUNTAIN_BOSS_LEVEL && g->map.room_count > 0;
}

void goblin_king_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py) {
    if (m->room_count < 1) {
        return;
    }
    const Enemy *king = NULL;
    for (int i = 0; i < count; i++) {
        if (enemies[i].active && enemies[i].type == ENEMY_MOUNTAIN_GOBLIN_KING) {
            king = &enemies[i];
        }
    }
    if (!king) {
        return;
    }
    const Room *room = &m->rooms[m->room_count - 1];
    int kegs = 0;
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            kegs += m->tiles[y][x] >= TILE_GOBLIN_KEG && m->tiles[y][x] <= TILE_GOBLIN_KEG_SPENT;
            if (m->tiles[y][x] == TILE_TRAP_HIDDEN) {
                m->tiles[y][x] = TILE_MOUNTAIN_FORTRESS_FLOOR;
            }
        }
    }
    if (kegs) {
        return;
    }
    static const int offsets[3][2] = {{-1, 0}, {1, 0}, {0, 1}};
    for (int n = 0; n < 3; n++) {
        int tx = king->x + offsets[n][0];
        int ty = king->y + offsets[n][1];
        int bx = -1;
        int by = -1;
        int distance = (MAP_W + MAP_H) * 2;
        for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
            for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
                int d = abs(x - tx) + abs(y - ty);
                // Prefer a blast that reaches the stationary King, preserving obstacles.
                if (abs(x - king->x) > 1 || abs(y - king->y) > 1) {
                    d += MAP_W + MAP_H;
                }
                TileType tile = m->tiles[y][x];
                if ((tile == TILE_MOUNTAIN_FORTRESS_FLOOR || tile == TILE_MOUNTAIN_FLOOR) &&
                    !occupied(enemies, count, x, y) && !(x == px && y == py) && d < distance) {
                    bx = x;
                    by = y;
                    distance = d;
                }
            }
        }
        if (bx >= 0) {
            m->tiles[by][bx] = TILE_GOBLIN_KEG;
        }
    }
}

int goblin_king_keg_fuse(const GameState *g, int x, int y) {
    TileType tile = tile_at(g, x, y);
    if (tile == TILE_GOBLIN_KEG_FUSE_3) {
        return 3;
    }
    if (tile == TILE_GOBLIN_KEG_FUSE_2) {
        return 2;
    }
    return tile == TILE_GOBLIN_KEG_FUSE_1 ? 1 : 0;
}

int goblin_king_keg_marks(const GameState *g, int x, int y) {
    if (!arena(g) || !map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    int turns = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int fuse = goblin_king_keg_fuse(g, x + dx, y + dy);
            if (fuse && (!turns || fuse < turns)) {
                turns = fuse;
            }
        }
    }
    return turns;
}

int goblin_king_remaining_kegs(const GameState *g) {
    if (!arena(g)) {
        return 0;
    }
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    int count = 0;
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            TileType tile = tile_at(g, x, y);
            count += tile >= TILE_GOBLIN_KEG && tile < TILE_GOBLIN_KEG_SPENT;
        }
    }
    return count;
}

static int axe_path(const GameState *g, const Enemy *e, int x, int y, int *ex, int *ey, int clip) {
    int tx = e->x;
    int ty = e->y;
    int dx = abs(e->attack_target_x - tx);
    int dy = -abs(e->attack_target_y - ty);
    int sx = tx < e->attack_target_x ? 1 : -1;
    int sy = ty < e->attack_target_y ? 1 : -1;
    int error = dx + dy;
    int hit = 0;
    if (ex && ey) {
        *ex = tx;
        *ey = ty;
    }
    for (int step = 0; step < MAP_W + MAP_H; step++) {
        if (tx == e->attack_target_x && ty == e->attack_target_y) {
            break;
        }
        int old_x = tx;
        int old_y = ty;
        int twice = error * 2;
        if (twice >= dy) {
            error += dy;
            tx += sx;
        }
        if (twice <= dx) {
            error += dx;
            ty += sy;
        }
        if (tx != old_x && ty != old_y &&
            (!map_is_walkable(&g->map, tx, old_y) || !map_is_walkable(&g->map, old_x, ty))) {
            break;
        }
        if (!map_is_walkable(&g->map, tx, ty) || (clip && occupied(g->enemies, g->enemy_count, tx, ty))) {
            break;
        }
        hit |= tx == x && ty == y;
        if (ex && ey) {
            *ex = tx;
            *ey = ty;
        }
    }
    return hit;
}

static int keg_blast(int cx, int cy, int x, int y) {
    return abs(cx - x) <= 1 && abs(cy - y) <= 1;
}

static int pocket(int cx, int cy, int x, int y) {
    return cx >= 0 && cy >= 0 && abs(cx - x) + abs(cy - y) <= 1;
}

int goblin_king_attack_marks(const GameState *g, const Enemy *e, int x, int y) {
    if (!e->active || e->type != ENEMY_MOUNTAIN_GOBLIN_KING || e->move_timer <= 0 || !map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    if (e->boss_attack == GOBLIN_KING_BOMBS) {
        return pocket(e->attack_target_x, e->attack_target_y, x, y) || pocket(e->boss_aux_x, e->boss_aux_y, x, y);
    }
    return e->boss_attack == GOBLIN_KING_AXE && e->attack_target_x >= 0 && axe_path(g, e, x, y, NULL, NULL, 0);
}

static void clear_targets(Enemy *e) {
    e->attack_target_x = -1;
    e->attack_target_y = -1;
    e->boss_aux_x = -1;
    e->boss_aux_y = -1;
}

int goblin_king_tick_kegs(GameState *g) {
    if (!arena(g)) {
        return 0;
    }
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    int hit[MAX_ENEMIES] = {0};
    int player_hit = 0;
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            int fuse = goblin_king_keg_fuse(g, x, y);
            if (!fuse) {
                continue;
            }
            if (fuse > 1) {
                set_tile(g, x, y, fuse == 3 ? TILE_GOBLIN_KEG_FUSE_2 : TILE_GOBLIN_KEG_FUSE_1);
                continue;
            }
            set_tile(g, x, y, TILE_GOBLIN_KEG_SPENT);
            player_hit |= keg_blast(x, y, g->player.x, g->player.y);
            for (int i = 0; i < g->enemy_count; i++) {
                Enemy *e = &g->enemies[i];
                if (!e->active) {
                    continue;
                }
                hit[i] |= keg_blast(x, y, e->x, e->y);
                if (e->type == ENEMY_MOUNTAIN_GOBLIN_KING && e->boss_attack == GOBLIN_KING_BOMBS) {
                    if (e->attack_target_x >= 0 && abs(x - e->attack_target_x) <= 2 && abs(y - e->attack_target_y) <= 2) {
                        e->attack_target_x = -1;
                        e->attack_target_y = -1;
                    }
                    if (e->boss_aux_x >= 0 && abs(x - e->boss_aux_x) <= 2 && abs(y - e->boss_aux_y) <= 2) {
                        e->boss_aux_x = -1;
                        e->boss_aux_y = -1;
                    }
                }
            }
            push_message(g, "Powder keg explodes across its marked area!");
        }
    }
    // Overlapping blasts hit each actor once, using their already displayed union.
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (!hit[i]) {
            continue;
        }
        e->hp -= e->type == ENEMY_MOUNTAIN_GOBLIN_KING ? e->max_hp / 6 : 30;
        if (e->type == ENEMY_MOUNTAIN_GOBLIN_KING) {
            e->move_timer = -3;
            clear_targets(e);
            push_message(g, "The King is staggered! Attack during 2 recovery turns.");
        }
    }
    int damage = 20 - g->player.defense / 2;
    return player_hit ? (damage > 4 ? damage : 4) : 0;
}

int goblin_king_tick(GameState *g, Enemy *e, EnemyProjectiles *shots) {
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
        int hit = goblin_king_attack_marks(g, e, g->player.x, g->player.y);
        if (e->boss_attack == GOBLIN_KING_AXE) {
            hit = axe_path(g, e, g->player.x, g->player.y, NULL, NULL, 1);
            const Room *room = &g->map.rooms[g->map.room_count - 1];
            for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
                for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
                    if (tile_at(g, x, y) == TILE_GOBLIN_KEG && axe_path(g, e, x, y, NULL, NULL, 1)) {
                        set_tile(g, x, y, TILE_GOBLIN_KEG_FUSE_2);
                        push_message(g, "The King's axe lights a keg: blast in 2 turns!");
                    }
                }
            }
            if (shots && shots->count < MAX_ENEMIES) {
                int x;
                int y;
                axe_path(g, e, -1, -1, &x, &y, 1);
                if (x != e->x || y != e->y) {
                    shots->shots[shots->count++] = (EnemyProjectile){e->type, e->x, e->y, x, y};
                }
            }
        }
        push_message(g, e->boss_attack == GOBLIN_KING_BOMBS ? "Powder bombs strike the marked pockets!" : "The King's axe strikes its fixed lane!");
        e->move_timer = -1;
        clear_targets(e);
        int damage = e->attack - g->player.defense / 2;
        return hit ? (damage > 4 ? damage : 4) : 0;
    }
    const Room *fortress = &g->map.rooms[g->map.room_count - 1];
    // Resolve active floor fuses before preparing another signature footprint.
    for (int y = fortress->y + 1; y < fortress->y + fortress->h - 1; y++) {
        for (int x = fortress->x + 1; x < fortress->x + fortress->w - 1; x++) {
            if (goblin_king_keg_fuse(g, x, y)) {
                return 0;
            }
        }
    }
    if (!e->attack_phase && e->hp <= e->max_hp / 2) {
        e->attack_phase = 1;
        push_message(g, "The Goblin King becomes Powder King! Bombardment begins.");
    }
    e->attack_target_x = g->player.x;
    e->attack_target_y = g->player.y;
    e->boss_aux_x = -1;
    e->boss_aux_y = -1;
    if (e->attack_phase && e->boss_attack == GOBLIN_KING_AXE) {
        e->boss_attack = GOBLIN_KING_BOMBS;
        const Room *room = &g->map.rooms[g->map.room_count - 1];
        int distance = MAP_W + MAP_H;
        int tx = g->player.x + (e->x > g->player.x ? 4 : -4);
        for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
            for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
                int d = abs(x - tx) + abs(y - g->player.y);
                if (map_is_walkable(&g->map, x, y) && abs(x - g->player.x) + abs(y - g->player.y) >= 4 && d < distance) {
                    e->boss_aux_x = x;
                    e->boss_aux_y = y;
                    distance = d;
                }
            }
        }
        e->move_timer = 2;
        push_message(g, "Powder bombardment in 2 turns! Leave the fixed marked pockets.");
    } else {
        e->boss_attack = GOBLIN_KING_AXE;
        int x;
        int y;
        axe_path(g, e, -1, -1, &x, &y, 1);
        e->attack_target_x = x;
        e->attack_target_y = y;
        e->move_timer = 1;
        push_message(g, "The Goblin King raises his axe: fixed throw in 1 turn! Leave its lane.");
    }
    return 0;
}

void goblin_king_restore_reward(GameState *g) {
    if (g->location != LOCATION_MOUNTAINS || g->level != MOUNTAIN_BOSS_LEVEL || g->map.room_count < 1 || !(g->defeated_bosses & (1 << LOCATION_MOUNTAINS))) {
        return;
    }
    int floor_shield = 0;
    for (int i = 0; i < g->floor_item_count; i++) {
        floor_shield |= g->floor_items[i].active && strcmp(g->floor_items[i].item.name, "Goblin King's Shield") == 0;
    }
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    int cache = 0;
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            if (tile_at(g, x, y) == TILE_GOBLIN_KING_RELIQUARY) {
                if (g->goblin_king_reward_state != 1 || floor_shield) {
                    set_tile(g, x, y, TILE_MOUNTAIN_FORTRESS_FLOOR);
                } else {
                    cache = 1;
                }
            }
        }
    }
    if (g->goblin_king_reward_state != 1 || floor_shield || cache) {
        return;
    }
    for (int y = room->y + room->h - 2; y > room->y; y--) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            if ((g->map.tiles[y][x] == TILE_MOUNTAIN_FORTRESS_FLOOR || g->map.tiles[y][x] == TILE_MOUNTAIN_FLOOR) && !occupied(g->enemies, g->enemy_count, x, y)) {
                g->map.tiles[y][x] = TILE_GOBLIN_KING_RELIQUARY;
                push_message(g, "Unclaimed Goblin King's Shield waits in the fortress reliquary. Open with Interact, then Pick Up the released loot.");
                return;
            }
        }
    }
}

static TileType interaction_at(const GameState *g, int *x, int *y) {
    if (g->location != LOCATION_MOUNTAINS || g->level != MOUNTAIN_BOSS_LEVEL) {
        return TILE_MOUNTAIN_FORTRESS_FLOOR;
    }
    // Prefer the tile underfoot before selecting a nearby keg.
    for (int radius = 0; radius <= 1; radius++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (abs(dx) + abs(dy) != radius) {
                    continue;
                }
                TileType tile = tile_at(g, g->player.x + dx, g->player.y + dy);
                if (tile == TILE_GOBLIN_KEG || (tile == TILE_GOBLIN_KING_RELIQUARY && g->goblin_king_reward_state == 1)) {
                    *x = g->player.x + dx;
                    *y = g->player.y + dy;
                    return tile;
                }
            }
        }
    }
    return TILE_MOUNTAIN_FORTRESS_FLOOR;
}

int goblin_king_has_interaction(const GameState *g) {
    int x;
    int y;
    return interaction_at(g, &x, &y) != TILE_MOUNTAIN_FORTRESS_FLOOR;
}

int goblin_king_interact(GameState *g) {
    int x;
    int y;
    TileType tile = interaction_at(g, &x, &y);
    if (tile == TILE_MOUNTAIN_FORTRESS_FLOOR) {
        return 0;
    }
    if (tile == TILE_GOBLIN_KING_RELIQUARY) {
        if (g->inventory_count >= MAX_INVENTORY) {
            push_message(g, "Inventory full. Goblin King's Shield remains safe in the reliquary.");
        } else {
            Item reward = item_make_goblin_king_shield();
            g->inventory[g->inventory_count++] = reward;
            g->score += reward.value;
            g->goblin_king_reward_state = 2;
            set_tile(g, x, y, TILE_MOUNTAIN_FORTRESS_FLOOR);
            push_message(g, "Goblin King's Shield recovered from the fortress reliquary!");
        }
        return 1;
    }
    // The interaction's enemy phase consumes the extra initial fuse tick.
    set_tile(g, x, y, TILE_GOBLIN_KEG_FUSE_3);
    push_message(g, "Keg primed! Leave its marked area before the fuse ends.");
    return 1;
}

static void prepare_saved_map(const GameState *g, Map *m, const Enemy *enemies, int count, int px, int py) {
    int x = g->portal_x;
    int y = g->portal_y;
    int protect = g->portal_active && g->portal_location == LOCATION_MOUNTAINS &&
        g->portal_level == MOUNTAIN_BOSS_LEVEL && x >= 0 && x < MAP_W && y >= 0 && y < MAP_H &&
        (m->tiles[y][x] == TILE_MOUNTAIN_FORTRESS_FLOOR || m->tiles[y][x] == TILE_MOUNTAIN_FLOOR);
    TileType original = protect ? m->tiles[y][x] : TILE_MOUNTAIN_FORTRESS_FLOOR;
    // Hidden portal landings stay traversable even when their marker is absent.
    if (protect) {
        m->tiles[y][x] = TILE_PORTAL;
    }
    goblin_king_prepare_map(m, enemies, count, px, py);
    if (protect) {
        m->tiles[y][x] = original;
    }
}

void goblin_king_migrate(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (e->type == ENEMY_MOUNTAIN_GOBLIN_KING) {
            e->move_timer = 0;
            e->attack_phase = 0;
            e->boss_attack = 0;
            e->boss_wave_used = 0;
            clear_targets(e);
        }
    }
    LevelCache *cache = &g->mountain_cache[MOUNTAIN_BOSS_LEVEL - 1];
    if (cache->valid) {
        for (int i = 0; i < cache->enemy_count; i++) {
            Enemy *e = &cache->enemies[i];
            if (e->type == ENEMY_MOUNTAIN_GOBLIN_KING) {
                e->move_timer = 0;
                e->attack_phase = 0;
                e->boss_attack = 0;
                e->boss_wave_used = 0;
                clear_targets(e);
            }
        }
        prepare_saved_map(g, &cache->map, cache->enemies, cache->enemy_count, -1, -1);
    }
    if (g->location == LOCATION_MOUNTAINS && g->level == MOUNTAIN_BOSS_LEVEL) {
        prepare_saved_map(g, &g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    // Only known, currently dropped legacy rewards establish an unclaimed gift.
    if (g->defeated_bosses & (1 << LOCATION_MOUNTAINS)) {
        g->goblin_king_reward_state = 2;
        int known_claim = 0;
        for (int i = 0; i < g->inventory_count; i++) {
            known_claim |= strcmp(g->inventory[i].name, "Goblin King's Shield") == 0;
        }
        if (g->location == LOCATION_MOUNTAINS && g->level == MOUNTAIN_BOSS_LEVEL) {
            for (int i = 0; i < g->floor_item_count; i++) {
                if (!known_claim && g->floor_items[i].active &&
                    strcmp(g->floor_items[i].item.name, "Goblin King's Shield") == 0) {
                    g->goblin_king_reward_state = 1;
                }
            }
        }
    }
}
