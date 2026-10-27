#include "necromancer.h"
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

void necromancer_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py) {
    if (m->room_count < 1) {
        return;
    }
    int living = 0;
    for (int i = 0; i < count; i++) {
        living |= enemies[i].active && enemies[i].type == ENEMY_FOREST_NECROMANCER;
    }
    if (!living) {
        return;
    }
    const Room *room = &m->rooms[m->room_count - 1];
    int graves = 0;
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            graves += m->tiles[y][x] == TILE_NECRO_GRAVE || m->tiles[y][x] == TILE_NECRO_GRAVE_SPENT;
            if (m->tiles[y][x] == TILE_TRAP_HIDDEN) {
                m->tiles[y][x] = TILE_FOREST_FLOOR;
            }
        }
    }
    // Spent patches survive map preparation and cached revisits.
    if (graves) {
        return;
    }
    for (int n = 0; n < 3; n++) {
        int tx = n == 1 ? room->x + room->w - 2 : room->x + 2;
        int ty = n == 2 ? room->y + room->h - 2 : room->y + 2;
        int best_x = -1;
        int best_y = -1;
        int distance = MAP_W + MAP_H;
        for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
            for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
                int d = abs(x - tx) + abs(y - ty);
                if (m->tiles[y][x] == TILE_FOREST_FLOOR && !occupied(enemies, count, x, y) &&
                    !(px == x && py == y) && d < distance) {
                    best_x = x;
                    best_y = y;
                    distance = d;
                }
            }
        }
        if (best_x >= 0) {
            m->tiles[best_y][best_x] = TILE_NECRO_GRAVE;
        }
    }
}

int necromancer_active_graves(const GameState *g) {
    if (g->location != LOCATION_FOREST || g->level != FOREST_BOSS_LEVEL || g->map.room_count < 1) {
        return 0;
    }
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    int count = 0;
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            count += tile_at(g, x, y) == TILE_NECRO_GRAVE;
        }
    }
    return count;
}

static int bolt_path(const GameState *g, const Enemy *e, int x, int y, int *end_x, int *end_y, int clip) {
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
        if (!map_is_walkable(&g->map, tx, ty) || (clip && occupied(g->enemies, g->enemy_count, tx, ty))) {
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

int necromancer_attack_marks(const GameState *g, const Enemy *e, int x, int y) {
    if (!e->active || e->type != ENEMY_FOREST_NECROMANCER || e->move_timer <= 0 || !map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    if (e->boss_attack == NECRO_RAISE) {
        // Patches are fixed; consecrating any warned patch cancels this wave.
        return in_chamber(&g->map, x, y) && tile_at(g, x, y) == TILE_NECRO_GRAVE;
    }
    if (e->attack_target_x < 0 || e->attack_target_y < 0) {
        return 0;
    }
    if (e->boss_attack == NECRO_ROOTS) {
        return in_chamber(&g->map, x, y) && (x == e->attack_target_x || y == e->attack_target_y);
    }
    return e->boss_attack == NECRO_BOLT && bolt_path(g, e, x, y, NULL, NULL, 0);
}

int necromancer_tick(GameState *g, Enemy *e, EnemyProjectiles *shots) {
    if (g->location != LOCATION_FOREST || g->level != FOREST_BOSS_LEVEL || g->map.room_count < 1) {
        return 0;
    }
    int nearby = abs(g->player.x - e->x) + abs(g->player.y - e->y) <= 12;
    if (!in_chamber(&g->map, g->player.x, g->player.y) &&
        (!nearby || (e->hp == e->max_hp && !e->boss_attack))) {
        // Leaving range pauses the exact warning, including finite wave use.
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
        int hit = necromancer_attack_marks(g, e, g->player.x, g->player.y);
        if (e->boss_attack == NECRO_BOLT) {
            hit = bolt_path(g, e, g->player.x, g->player.y, NULL, NULL, 1);
        }
        if (e->boss_attack == NECRO_RAISE) {
            const Room *room = &g->map.rooms[g->map.room_count - 1];
            int index = (int)(e - g->enemies);
            for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
                for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
                    if (necromancer_attack_marks(g, e, x, y)) {
                        game_raise_grove_servant(g, x, y, index);
                    }
                }
            }
            hit = 0;
            push_message(g, "The warned graves recall fallen servants. The Necromancer is exposed!");
        } else {
            push_message(g, e->boss_attack == NECRO_ROOTS ? "Withered roots lash the marked crossing!" : "The spirit bolt strikes its fixed lane!");
            if (e->boss_attack == NECRO_BOLT && shots && shots->count < MAX_ENEMIES) {
                int x;
                int y;
                bolt_path(g, e, -1, -1, &x, &y, 1);
                if (x != e->x || y != e->y) {
                    shots->shots[shots->count++] = (EnemyProjectile){e->type, e->x, e->y, x, y};
                }
            }
        }
        e->move_timer = -1;
        e->attack_target_x = -1;
        e->attack_target_y = -1;
        int damage = e->attack - g->player.defense / 2;
        return hit ? (damage > 3 ? damage : 3) : 0;
    }
    int new_phase = e->hp <= e->max_hp / 2 && !e->attack_phase;
    if (new_phase) {
        e->attack_phase = 1;
        push_message(g, "The Necromancer becomes Withered Grove! Crossing roots awaken.");
    }
    int wave = 1 << e->attack_phase;
    if (!(e->boss_wave_used & wave) && (new_phase || e->boss_attack == NECRO_BOLT)) {
        e->boss_wave_used |= e->attack_phase ? 3 : 1;
        if (necromancer_active_graves(g)) {
            e->boss_attack = NECRO_RAISE;
            e->move_timer = 2;
            e->attack_target_x = -1;
            e->attack_target_y = -1;
            push_message(g, "Grave wave in 2 turns! Consecrate a marked grave to interrupt it.");
            return 0;
        }
    }
    e->attack_target_x = g->player.x;
    e->attack_target_y = g->player.y;
    if (e->attack_phase && e->boss_attack != NECRO_ROOTS) {
        e->boss_attack = NECRO_ROOTS;
        e->move_timer = 2;
        push_message(g, "Crossing roots in 2 turns! Leave both marked lanes.");
    } else {
        e->boss_attack = NECRO_BOLT;
        int x;
        int y;
        bolt_path(g, e, -1, -1, &x, &y, 1);
        e->attack_target_x = x;
        e->attack_target_y = y;
        e->move_timer = 1;
        push_message(g, "The Necromancer invokes the forest: spirit bolt in 1 turn! Leave its lane.");
    }
    return 0;
}
void necromancer_restore_reward(GameState *g) {
    if (g->location != LOCATION_FOREST || g->level != FOREST_BOSS_LEVEL || g->map.room_count < 1 || !(g->defeated_bosses & (1 << LOCATION_FOREST))) {
        return;
    }
    int floor_cloak = 0;
    for (int i = 0; i < g->floor_item_count; i++) {
        floor_cloak |= g->floor_items[i].active && strcmp(g->floor_items[i].item.name, "Necromancer's Cloak") == 0;
    }
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    int cache = 0;
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            if (tile_at(g, x, y) == TILE_NECRO_RELIQUARY) {
                if (g->necromancer_reward_state != 1 || floor_cloak) {
                    set_tile(g, x, y, TILE_FOREST_FLOOR);
                } else {
                    cache = 1;
                }
            }
        }
    }
    if (g->necromancer_reward_state != 1 || floor_cloak || cache) {
        return;
    }
    for (int y = room->y + room->h - 2; y > room->y; y--) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            if (g->map.tiles[y][x] == TILE_FOREST_FLOOR && !occupied(g->enemies, g->enemy_count, x, y)) {
                g->map.tiles[y][x] = TILE_NECRO_RELIQUARY;
                push_message(g, "Unclaimed Necromancer's Cloak waits in the grove reliquary. Open with Interact, then Pick Up the released loot.");
                return;
            }
        }
    }
}

static TileType interaction_at(const GameState *g, int *x, int *y) {
    if (g->location != LOCATION_FOREST || g->level != FOREST_BOSS_LEVEL) {
        return TILE_FOREST_FLOOR;
    }
    // Prefer the tile underfoot before selecting a nearby grave.
    for (int radius = 0; radius <= 1; radius++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (abs(dx) + abs(dy) != radius) {
                    continue;
                }
                TileType tile = tile_at(g, g->player.x + dx, g->player.y + dy);
                if (tile == TILE_NECRO_GRAVE || (tile == TILE_NECRO_RELIQUARY && g->necromancer_reward_state == 1)) {
                    *x = g->player.x + dx;
                    *y = g->player.y + dy;
                    return tile;
                }
            }
        }
    }
    return TILE_FOREST_FLOOR;
}

int necromancer_has_interaction(const GameState *g) {
    int x;
    int y;
    return interaction_at(g, &x, &y) != TILE_FOREST_FLOOR;
}

int necromancer_interact(GameState *g) {
    int x;
    int y;
    TileType tile = interaction_at(g, &x, &y);
    if (tile == TILE_FOREST_FLOOR) {
        return 0;
    }
    if (tile == TILE_NECRO_RELIQUARY) {
        if (g->inventory_count >= MAX_INVENTORY) {
            push_message(g, "Inventory full. Necromancer's Cloak remains safe in the reliquary.");
        } else {
            Item reward = item_make_necromancer_cloak();
            g->inventory[g->inventory_count++] = reward;
            g->score += reward.value;
            g->necromancer_reward_state = 2;
            set_tile(g, x, y, TILE_FOREST_FLOOR);
            push_message(g, "Necromancer's Cloak recovered from the grove reliquary!");
        }
        return 1;
    }
    set_tile(g, x, y, TILE_NECRO_GRAVE_SPENT);
    push_message(g, "Grave consecrated. No servant can rise from this patch.");
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (e->active && e->type == ENEMY_FOREST_NECROMANCER && e->move_timer > 0 && e->boss_attack == NECRO_RAISE) {
            // This action's enemy phase consumes the extra initial recovery tick.
            e->move_timer = -3;
            e->attack_target_x = -1;
            e->attack_target_y = -1;
            push_message(g, "Grave wave interrupted! The Necromancer is exposed for 2 turns.");
        }
    }
    return 1;
}

static void prepare_saved_map(const GameState *g, Map *m, const Enemy *enemies, int count, int px, int py) {
    int x = g->portal_x;
    int y = g->portal_y;
    int protect = g->portal_active && g->portal_location == LOCATION_FOREST &&
        g->portal_level == FOREST_BOSS_LEVEL && x >= 0 && x < MAP_W && y >= 0 && y < MAP_H &&
        m->tiles[y][x] == TILE_FOREST_FLOOR;
    // Hidden portal landings stay traversable even when their marker is absent.
    if (protect) {
        m->tiles[y][x] = TILE_PORTAL;
    }
    necromancer_prepare_map(m, enemies, count, px, py);
    if (protect) {
        m->tiles[y][x] = TILE_FOREST_FLOOR;
    }
}

void necromancer_migrate(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (e->type == ENEMY_FOREST_NECROMANCER) {
            e->move_timer = 0;
            e->attack_phase = 0;
            e->boss_attack = 0;
            e->boss_wave_used = 0;
            e->attack_target_x = -1;
            e->attack_target_y = -1;
        }
    }
    LevelCache *cache = &g->forest_cache[FOREST_BOSS_LEVEL - 1];
    if (cache->valid) {
        for (int i = 0; i < cache->enemy_count; i++) {
            Enemy *e = &cache->enemies[i];
            if (e->type == ENEMY_FOREST_NECROMANCER) {
                e->move_timer = 0;
                e->attack_phase = 0;
                e->boss_attack = 0;
                e->boss_wave_used = 0;
                e->attack_target_x = -1;
                e->attack_target_y = -1;
            }
        }
        prepare_saved_map(g, &cache->map, cache->enemies, cache->enemy_count, -1, -1);
    }
    if (g->location == LOCATION_FOREST && g->level == FOREST_BOSS_LEVEL) {
        prepare_saved_map(g, &g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    // Only known, currently dropped legacy rewards establish an unclaimed gift.
    if (g->defeated_bosses & (1 << LOCATION_FOREST)) {
        g->necromancer_reward_state = 2;
        int known_claim = 0;
        for (int i = 0; i < g->inventory_count; i++) {
            known_claim |= strcmp(g->inventory[i].name, "Necromancer's Cloak") == 0;
        }
        if (g->location == LOCATION_FOREST && g->level == FOREST_BOSS_LEVEL) {
            for (int i = 0; i < g->floor_item_count; i++) {
                if (!known_claim && g->floor_items[i].active &&
                    strcmp(g->floor_items[i].item.name, "Necromancer's Cloak") == 0) {
                    g->necromancer_reward_state = 1;
                }
            }
        }
    }
}
