#include "lich.h"
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

void lich_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py) {
    if (m->room_count < 1) {
        return;
    }
    int living = 0;
    for (int i = 0; i < count; i++) {
        living |= enemies[i].active && enemies[i].type == ENEMY_LICH_KING;
    }
    if (!living) {
        return;
    }
    const Room *room = &m->rooms[m->room_count - 1];
    int seals = 0;
    int pillars = 0;
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            seals += m->tiles[y][x] == TILE_LICH_SEAL || m->tiles[y][x] == TILE_LICH_SEAL_SPENT;
            pillars += m->tiles[y][x] == TILE_LICH_PILLAR;
            // Hidden random traps are excluded from the signature-attack arena.
            if (m->tiles[y][x] == TILE_TRAP_HIDDEN) {
                m->tiles[y][x] = TILE_FLOOR;
            }
        }
    }
    // Preparation is idempotent; saved silenced seals never get relit.
    if (seals > 0) {
        return;
    }
    for (int n = 0; n < 2; n++) {
        int target_x = n == 0 ? room->x + 1 : room->x + room->w - 2;
        int best_x = -1;
        int best_y = -1;
        int distance = MAP_W + MAP_H;
        for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
            for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
                int d = abs(x - target_x) + abs(y - room->y - 1);
                if (m->tiles[y][x] == TILE_FLOOR && !occupied(enemies, count, x, y) && d < distance) {
                    best_x = x;
                    best_y = y;
                    distance = d;
                }
            }
        }
        if (best_x >= 0) {
            m->tiles[best_y][best_x] = TILE_LICH_SEAL;
        }
    }
    int cx;
    int cy;
    map_room_center(room, &cx, &cy);
    for (int n = pillars; n < 2; n++) {
        int x = cx + (n == 0 ? -2 : 2);
        int y = cy + 1;
        if (in_chamber(m, x, y) && m->tiles[y][x] == TILE_FLOOR &&
            !occupied(enemies, count, x, y) && !(px == x && py == y)) {
            m->tiles[y][x] = TILE_LICH_PILLAR;
        }
    }
}

int lich_active_seals(const GameState *g) {
    if (g->location != LOCATION_DUNGEON || g->level != DUNGEON_DEPTH || g->map.room_count < 1) {
        return 0;
    }
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    int count = 0;
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            count += tile_at(g, x, y) == TILE_LICH_SEAL;
        }
    }
    return count;
}

static int lance_path(const Map *m, const Enemy *e, int x, int y, int *end_x, int *end_y) {
    int tx = e->x;
    int ty = e->y;
    int dx = abs(e->attack_target_x - tx);
    int dy = -abs(e->attack_target_y - ty);
    int sx = tx < e->attack_target_x ? 1 : -1;
    int sy = ty < e->attack_target_y ? 1 : -1;
    int error = dx + dy;
    int hit = 0;
    if (end_x && end_y) {
        *end_x = tx;
        *end_y = ty;
    }
    for (int step = 0; step < MAP_W + MAP_H; step++) {
        if (tx == e->attack_target_x && ty == e->attack_target_y) {
            break;
        }
        int twice = error * 2;
        if (twice >= dy) {
            error += dy;
            tx += sx;
        }
        if (twice <= dx) {
            error += dx;
            ty += sy;
        }
        if (!map_is_walkable(m, tx, ty)) {
            break;
        }
        hit |= tx == x && ty == y;
        if (end_x && end_y) {
            *end_x = tx;
            *end_y = ty;
        }
    }
    return hit;
}

int lich_attack_marks(const GameState *g, const Enemy *e, int x, int y) {
    if (!e->active || e->type != ENEMY_LICH_KING || e->move_timer <= 0 || !map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    if (e->boss_attack == LICH_SUMMON) {
        return (x == e->attack_target_x && y == e->attack_target_y) || (x == e->boss_aux_x && y == e->boss_aux_y);
    }
    if (e->boss_attack == LICH_RING) {
        return in_chamber(&g->map, x, y) && abs(x - e->x) + abs(y - e->y) == 3;
    }
    if (e->boss_attack != LICH_LANCE || e->attack_target_x < 0 || e->attack_target_y < 0) {
        return 0;
    }
    // A locked Bresenham lane includes its target and stops at solid cover.
    return lance_path(&g->map, e, x, y, NULL, NULL);
}

static void prepare_wave(const GameState *g, Enemy *e) {
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    e->attack_target_x = -1;
    e->attack_target_y = -1;
    e->boss_aux_x = -1;
    e->boss_aux_y = -1;
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            if (g->map.tiles[y][x] != TILE_FLOOR || occupied(g->enemies, g->enemy_count, x, y) ||
                abs(x - g->player.x) + abs(y - g->player.y) < 3 || abs(x - e->x) + abs(y - e->y) < 2) {
                continue;
            }
            if (e->attack_target_x < 0) {
                e->attack_target_x = x;
                e->attack_target_y = y;
            } else {
                e->boss_aux_x = x;
                e->boss_aux_y = y;
                return;
            }
        }
    }
}

int lich_tick(GameState *g, Enemy *e, EnemyProjectiles *shots) {
    if (g->location != LOCATION_DUNGEON || g->level != DUNGEON_DEPTH || g->map.room_count < 1) {
        return 0;
    }
    int nearby = abs(g->player.x - e->x) <= 12 && abs(g->player.y - e->y) <= 12;
    if (!in_chamber(&g->map, g->player.x, g->player.y) && (!nearby || (e->hp == e->max_hp && e->move_timer == 0))) {
        e->move_timer = 0;
        e->attack_target_x = -1;
        e->attack_target_y = -1;
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
        // Keep the announced shape available to the same predicate used by UI.
        e->move_timer = 1;
        int hit = lich_attack_marks(g, e, g->player.x, g->player.y);
        if (e->boss_attack == LICH_SUMMON) {
            int index = (int)(e - g->enemies);
            game_spawn_lich_shade(g, e->attack_target_x, e->attack_target_y, index);
            game_spawn_lich_shade(g, e->boss_aux_x, e->boss_aux_y, index);
            push_message(g, "The warned graves release their skeletons. The Lich is exposed!");
            hit = 0;
        } else {
            push_message(g, e->boss_attack == LICH_RING ? "The soul ring bursts across the marked stones!" : "The necrotic lance strikes its marked lane!");
            if (e->boss_attack == LICH_LANCE && shots && shots->count < MAX_ENEMIES) {
                int x;
                int y;
                lance_path(&g->map, e, -1, -1, &x, &y);
                if (x != e->x || y != e->y) {
                    shots->shots[shots->count++] = (EnemyProjectile){e->type, e->x, e->y, x, y};
                }
            }
        }
        e->move_timer = -1;
        e->attack_target_x = -1;
        e->attack_target_y = -1;
        if (hit) {
            int damage = e->attack - g->player.defense / 2;
            if (e->boss_attack == LICH_RING) {
                damage += lich_active_seals(g) * 2;
            }
            return damage > 4 ? damage : 4;
        }
        return 0;
    }
    if (e->hp <= e->max_hp / 2 && !e->attack_phase) {
        e->attack_phase = 1;
        push_message(g, "The Lich King becomes Unbound Soul! Soul rings and one grave wave awaken.");
    }
    if (e->attack_phase && !e->boss_wave_used) {
        e->boss_wave_used = 1;
        e->boss_attack = LICH_SUMMON;
        prepare_wave(g, e);
        e->move_timer = 1;
        push_message(g, "Grave wave in 1 turn! Skeletons will rise on the marked graves.");
    } else if (e->attack_phase && e->boss_attack != LICH_RING) {
        e->boss_attack = LICH_RING;
        e->attack_target_x = e->x;
        e->attack_target_y = e->y;
        e->move_timer = 2;
        push_message(g, "Soul ring in 2 turns! Step inside or outside the marked ring.");
    } else {
        e->boss_attack = LICH_LANCE;
        e->attack_target_x = g->player.x;
        e->attack_target_y = g->player.y;
        e->move_timer = 1;
        push_message(g, "Necrotic lance in 1 turn! Leave its fixed lane or shelter behind stone.");
    }
    return 0;
}

int lich_enemy_damage(const GameState *g, const Enemy *e, int damage) {
    if (e->type != ENEMY_LICH_KING || g->location != LOCATION_DUNGEON) {
        return damage;
    }
    damage = damage * (100 - lich_active_seals(g) * 15) / 100;
    return damage > 0 ? damage : 1;
}

void lich_restore_reward(GameState *g) {
    if (g->location != LOCATION_DUNGEON || g->level != DUNGEON_DEPTH || g->map.room_count < 1 || !(g->defeated_bosses & (1 << LOCATION_DUNGEON))) {
        return;
    }
    int floor_blade = 0;
    for (int i = 0; i < g->floor_item_count; i++) {
        floor_blade |= g->floor_items[i].active && strcmp(g->floor_items[i].item.name, "Cryptblade") == 0;
    }
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    int cache = 0;
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            if (tile_at(g, x, y) == TILE_LICH_RELIQUARY) {
                if (g->lich_reward_state != 1 || floor_blade) {
                    set_tile(g, x, y, TILE_FLOOR);
                } else {
                    cache = 1;
                }
            }
        }
    }
    if (g->lich_reward_state != 1 || floor_blade || cache) {
        return;
    }
    for (int y = room->y + room->h - 2; y > room->y; y--) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            if (g->map.tiles[y][x] == TILE_FLOOR && !occupied(g->enemies, g->enemy_count, x, y)) {
                g->map.tiles[y][x] = TILE_LICH_RELIQUARY;
                push_message(g, "Unclaimed Cryptblade waits in the Lich's reliquary. Interact beside it or press P on it.");
                return;
            }
        }
    }
}

static TileType interaction_at(const GameState *g, int *x, int *y) {
    if (g->location != LOCATION_DUNGEON || g->level != DUNGEON_DEPTH) {
        return TILE_FLOOR;
    }
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (abs(dx) + abs(dy) > 1) {
                continue;
            }
            TileType tile = tile_at(g, g->player.x + dx, g->player.y + dy);
            if (tile == TILE_LICH_SEAL || (tile == TILE_LICH_RELIQUARY && g->lich_reward_state == 1)) {
                *x = g->player.x + dx;
                *y = g->player.y + dy;
                return tile;
            }
        }
    }
    return TILE_FLOOR;
}

int lich_has_interaction(const GameState *g) {
    int x;
    int y;
    return interaction_at(g, &x, &y) != TILE_FLOOR;
}

int lich_interact(GameState *g) {
    int x;
    int y;
    TileType tile = interaction_at(g, &x, &y);
    if (tile == TILE_FLOOR) {
        return 0;
    }
    if (tile == TILE_LICH_RELIQUARY) {
        if (g->inventory_count >= MAX_INVENTORY) {
            push_message(g, "Inventory full. Cryptblade remains safe in the reliquary.");
        } else {
            Item reward = item_make_cryptblade();
            g->inventory[g->inventory_count++] = reward;
            g->score += reward.value;
            g->lich_reward_state = 2;
            set_tile(g, x, y, TILE_FLOOR);
            push_message(g, "Cryptblade recovered from the Lich's reliquary!");
        }
        return 1;
    }
    set_tile(g, x, y, TILE_LICH_SEAL_SPENT);
    push_message(g, "Burial seal silenced. The Lich's protection and soul ring weaken.");
    if (lich_active_seals(g) == 0) {
        for (int i = 0; i < g->enemy_count; i++) {
            Enemy *e = &g->enemies[i];
            if (e->active && e->type == ENEMY_LICH_KING) {
                // The interaction's enemy phase consumes the extra initial tick.
                e->move_timer = -3;
                e->attack_target_x = -1;
                e->attack_target_y = -1;
                push_message(g, "Both seals fall silent! The Lich is exposed for 2 turns.");
            }
        }
    }
    return 1;
}

static void prepare_saved_map(const GameState *g, Map *m, const Enemy *enemies, int count, int px, int py) {
    int x = g->portal_x;
    int y = g->portal_y;
    int protect = g->portal_active && g->portal_location == LOCATION_DUNGEON &&
        g->portal_level == DUNGEON_DEPTH && x >= 0 && x < MAP_W && y >= 0 && y < MAP_H &&
        m->tiles[y][x] == TILE_FLOOR;
    // Hidden portal landings stay traversable even when their marker is absent.
    if (protect) {
        m->tiles[y][x] = TILE_PORTAL;
    }
    lich_prepare_map(m, enemies, count, px, py);
    if (protect) {
        m->tiles[y][x] = TILE_FLOOR;
    }
}

void lich_migrate(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (e->type == ENEMY_LICH_KING) {
            e->move_timer = 0;
            e->attack_phase = 0;
            e->attack_target_x = -1;
            e->attack_target_y = -1;
        }
    }
    LevelCache *cache = &g->level_cache[DUNGEON_DEPTH - 1];
    if (cache->valid) {
        for (int i = 0; i < cache->enemy_count; i++) {
            Enemy *e = &cache->enemies[i];
            if (e->type == ENEMY_LICH_KING) {
                e->move_timer = 0;
                e->attack_phase = 0;
                e->attack_target_x = -1;
                e->attack_target_y = -1;
            }
        }
        prepare_saved_map(g, &cache->map, cache->enemies, cache->enemy_count, -1, -1);
    }
    if (g->location == LOCATION_DUNGEON && g->level == DUNGEON_DEPTH) {
        prepare_saved_map(g, &g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    // Only known, currently dropped legacy rewards establish an unclaimed gift.
    if (g->defeated_bosses & (1 << LOCATION_DUNGEON)) {
        g->lich_reward_state = 2;
        int known_claim = 0;
        for (int i = 0; i < g->inventory_count; i++) {
            known_claim |= strcmp(g->inventory[i].name, "Cryptblade") == 0;
        }
        if (g->location == LOCATION_DUNGEON && g->level == DUNGEON_DEPTH) {
            for (int i = 0; i < g->floor_item_count; i++) {
                if (!known_claim && g->floor_items[i].active && !g->floor_items[i].item.sharpened &&
                    strcmp(g->floor_items[i].item.name, "Cryptblade") == 0) {
                    g->lich_reward_state = 1;
                }
            }
        }
    }
}
