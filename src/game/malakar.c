#include "malakar.h"
#include "castle.h"
#include <stdlib.h>
#include <string.h>

static void fill(Map *m, int x, int y, int w, int h, TileType tile) {
    for (int ty = y; ty < y + h; ty++) {
        for (int tx = x; tx < x + w; tx++) {
            m->tiles[ty][tx] = tile;
        }
    }
}

void malakar_generate(Map *m) {
    static const Room rooms[MAX_ROOMS] = {
        {8, 6, 15, 12}, {34, 6, 22, 14}, {6, 26, 16, 12},
        {29, 27, 13, 10}, {47, 28, 12, 11}, {38, 50, 16, 8},
        {7, 45, 17, 12}, {27, 42, 12, 13}, {24, 8, 7, 10},
        {46, 42, 13, 6}
    };
    memset(m, 0, sizeof(*m));
    fill(m, 0, 0, MAP_W, MAP_H, TILE_CASTLE_WALL);
    m->room_count = MAX_ROOMS;
    memcpy(m->rooms, rooms, sizeof(rooms));
    for (int i = 0; i < MAX_ROOMS; i++) {
        const Room *r = &rooms[i];
        fill(m, r->x, r->y, r->w, r->h, TILE_CASTLE_FLOOR);
    }
    fill(m, 14, 10, 32, 3, TILE_CASTLE_CARPET);
    fill(m, 34, 19, 3, 13, TILE_CASTLE_CARPET);
    fill(m, 21, 31, 10, 2, TILE_CASTLE_FLOOR);
    fill(m, 41, 31, 9, 2, TILE_CASTLE_FLOOR);
    fill(m, 12, 37, 2, 10, TILE_CASTLE_FLOOR);
    fill(m, 23, 49, 7, 2, TILE_CASTLE_FLOOR);
    fill(m, 33, 36, 2, 8, TILE_CASTLE_FLOOR);
    fill(m, 38, 51, 8, 2, TILE_CASTLE_FLOOR);
    fill(m, 51, 38, 2, 14, TILE_CASTLE_FLOOR);
    fill(m, 38, 44, 10, 2, TILE_CASTLE_FLOOR);
    fill(m, 44, 51, 3, 6, TILE_CASTLE_CARPET);
    fill(m, 37, 9, 1, 4, TILE_CASTLE_TABLE);
    fill(m, 51, 9, 1, 4, TILE_CASTLE_TABLE);
    fill(m, 11, 28, 1, 3, TILE_CASTLE_PILLAR);
    fill(m, 17, 34, 1, 2, TILE_CASTLE_PILLAR);
    fill(m, 31, 29, 2, 1, TILE_CASTLE_TABLE);
    fill(m, 38, 33, 2, 1, TILE_CASTLE_TABLE);
    fill(m, 49, 29, 7, 1, TILE_CASTLE_BOOKCASE);
    fill(m, 9, 46, 10, 1, TILE_CASTLE_BOOKCASE);
    fill(m, 29, 43, 1, 6, TILE_CASTLE_TABLE);
    fill(m, 36, 43, 1, 6, TILE_CASTLE_TABLE);
    m->tiles[16][38] = TILE_CASTLE_WARD;
    m->tiles[16][51] = TILE_CASTLE_WARD;
    m->tiles[8][54] = TILE_CASTLE_BOOKCASE;
    m->tiles[13][24] = TILE_CASTLE_LEVER;
    fill(m, 29, 10, 1, 3, TILE_CASTLE_GATE_OPEN);
    fill(m, 34, 22, 3, 1, TILE_CASTLE_GATE);
    m->stairs_up_x = 15;
    m->stairs_up_y = 11;
    m->stairs_down_x = 45;
    m->stairs_down_y = 55;
    m->tiles[11][15] = TILE_STAIRS_DOWN;
    m->tiles[55][45] = TILE_STAIRS_UP;
    m->burial_trap_count = 1;
    m->burial_traps[0] = (BurialTrap){17, 32, 0, 0, 0};
    m->tiles[32][17] = TILE_CASTLE_TRAP_HIDDEN;
}

void malakar_init(Enemy *e, int x, int y) {
    *e = (Enemy){.active = 1, .type = ENEMY_MALAKAR, .x = x, .y = y, .hp = 400, .max_hp = 400,
        .attack = 32, .defense = 8, .experience = 300, .is_boss = 1, .attack_target_x = -1, .attack_target_y = -1};
    snprintf(e->name, sizeof(e->name), "Malakar, Royal Alchemist");
}

static TileType ground(const GameState *g, int x, int y) {
    for (int i = 0; i < g->floor_item_count; i++) {
        const FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            return item->underlying_tile;
        }
    }
    return g->map.tiles[y][x];
}

static void set_ground(GameState *g, int x, int y, TileType tile) {
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

void malakar_prepare(GameState *g) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level != 2) {
        return;
    }
    if ((g->castle_minibosses & 1) || g->debug_passages_unlocked) {
        for (int x = 34; x <= 36; x++) {
            set_ground(g, x, 22, TILE_CASTLE_GATE_OPEN);
        }
    }
}

static int source(const GameState *g) {
    return ground(g, 38, 16) == TILE_CASTLE_WARD ? 38 : ground(g, 51, 16) == TILE_CASTLE_WARD ? 51 : 0;
}

static void cancel(Enemy *e, int recovery) {
    e->attack_target_x = -1;
    e->attack_target_y = -1;
    e->boss_attack = 0;
    e->boss_aux_x = 0;
    e->boss_aux_y = 0;
    e->move_timer = recovery;
}

int malakar_marks(const Enemy *e, int x, int y) {
    if (!e->active || e->type != ENEMY_MALAKAR || e->attack_target_x < 0 || e->boss_attack == MALAKAR_RESTORE) {
        return 0;
    }
    int dx = x - e->attack_target_x;
    int dy = y - e->attack_target_y;
    if (abs(dx) > 2 || abs(dy) > 2) {
        return 0;
    }
    return (e->boss_aux_x & (1 << ((dy + 2) * 5 + dx + 2))) != 0;
}

static int clear_line(const Map *m, int x, int y, int tx, int ty) {
    int dx = abs(tx - x);
    int dy = abs(ty - y);
    int sx = x < tx ? 1 : -1;
    int sy = y < ty ? 1 : -1;
    int error = dx - dy;
    while (x != tx || y != ty) {
        int twice = error * 2;
        if (twice > -dy) {
            error -= dy;
            x += sx;
        }
        if (twice < dx) {
            error += dx;
            y += sy;
        }
        if (!map_is_walkable(m, x, y)) {
            return 0;
        }
    }
    return 1;
}

static int can_escape(const GameState *g, const Enemy *e, int turns) {
    int queue[49];
    int distance[49];
    for (int i = 0; i < 49; i++) {
        distance[i] = -1;
    }
    int head = 0;
    int tail = 1;
    queue[0] = 24;
    distance[24] = 0;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    while (head < tail) {
        int cell = queue[head++];
        int x = e->attack_target_x + cell % 7 - 3;
        int y = e->attack_target_y + cell / 7 - 3;
        if (!malakar_marks(e, x, y)) {
            return 1;
        }
        if (distance[cell] >= turns) {
            continue;
        }
        for (int side = 0; side < 4; side++) {
            int nx = cell % 7 + dx[side];
            int ny = cell / 7 + dy[side];
            if (nx < 0 || ny < 0 || nx >= 7 || ny >= 7) {
                continue;
            }
            int next = ny * 7 + nx;
            int tx = e->attack_target_x + nx - 3;
            int ty = e->attack_target_y + ny - 3;
            if (distance[next] >= 0 || !map_is_walkable(&g->map, tx, ty)) {
                continue;
            }
            int occupied = 0;
            for (int i = 0; i < g->enemy_count; i++) {
                const Enemy *actor = &g->enemies[i];
                occupied |= actor->active && actor->x == tx && actor->y == ty;
            }
            if (!occupied) {
                distance[next] = distance[cell] + 1;
                queue[tail++] = next;
            }
        }
    }
    return 0;
}

int malakar_tick(GameState *g, Enemy *e) {
    int visible = abs(g->player.x - e->x) <= 12 && abs(g->player.y - e->y) <= 12 &&
        clear_line(&g->map, e->x, e->y, g->player.x, g->player.y);
    if (!visible) {
        cancel(e, 0);
        return 0;
    }
    if (!e->attack_phase) {
        e->attack_phase = 1;
        push_message(g, "Malakar: You were meant to perish beyond the walls. The King will hear nothing of this.");
    }
    if (e->move_timer > 0) {
        e->move_timer--;
        if (e->move_timer > 0 || e->attack_target_x < 0) {
            return 0;
        }
        int damage = 0;
        if (e->boss_attack == MALAKAR_RESTORE) {
            e->hp += e->max_hp / 4;
            if (e->hp > e->max_hp) {
                e->hp = e->max_hp;
            }
            push_message(g, "Malakar drinks the restorative and regains strength.");
        } else if (malakar_marks(e, g->player.x, g->player.y)) {
            damage = e->attack - g->player.defense;
            damage = damage > 4 ? damage : 4;
        }
        cancel(e, 1);
        return damage;
    }
    int apparatus = source(g);
    if (!(e->boss_wave_used & 1) && apparatus && e->hp * 10 <= e->max_hp * 7) {
        e->boss_wave_used |= 1;
        e->boss_attack = MALAKAR_RESTORE;
        e->attack_target_x = apparatus;
        e->attack_target_y = 16;
        e->move_timer = 2;
        push_message(g, "Malakar prepares a restorative: 2 actions. Damage him or Interact with its glowing condenser!");
        return 0;
    }
    e->boss_attack = apparatus && !(e->boss_wave_used & 2) ? MALAKAR_PRESSURE : MALAKAR_VIAL;
    e->boss_wave_used ^= 2;
    e->attack_target_x = g->player.x;
    e->attack_target_y = g->player.y;
    e->boss_aux_y = apparatus;
    e->boss_aux_x = 0;
    int radius = e->boss_attack == MALAKAR_PRESSURE ? 2 : 1;
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            int x = e->attack_target_x + dx;
            int y = e->attack_target_y + dy;
            if (abs(dx) + abs(dy) <= radius && map_is_walkable(&g->map, x, y) &&
                clear_line(&g->map, e->attack_target_x, e->attack_target_y, x, y)) {
                e->boss_aux_x |= 1 << ((dy + 2) * 5 + dx + 2);
            }
        }
    }
    e->move_timer = radius == 2 ? 3 : 2;
    if (!can_escape(g, e, e->move_timer)) {
        cancel(e, 1);
        push_message(g, "Malakar's unstable mixture vents harmlessly against the masonry.");
        return 0;
    }
    push_message(g, radius == 2 ? "Volatile overpressure: 3 actions! Leave the marked diamond or disable its glowing source." :
        "Malakar throws a bursting vial: 2 actions! Leave the marked stones.");
    return 0;
}

int malakar_damage(GameState *g, Enemy *e, int damage) {
    if (!e->active || damage <= 0) {
        return 0;
    }
    int threshold = e->max_hp * 2 / 5;
    if (damage >= e->hp - threshold) {
        int applied = e->hp > threshold ? e->hp - threshold : 0;
        e->active = 0;
        e->frozen_turns = 0;
        e->poison_turns = 0;
        cancel(e, 0);
        if (!(g->castle_minibosses & 1)) {
            g->castle_minibosses |= 1;
            g->gold += 100;
            player_gain_xp(g, e->experience);
            if (!g->castellan_reward_state) {
                g->castellan_reward_state = 1;
            }
            push_message(g, "Wounded, Malakar shatters an escape vial and vanishes through his private passage.");
            push_message(g, "Malakar: The crown will restore me. Come to the King's chamber, if you dare!");
            push_message(g, "The laboratory seal breaks. Explore the wing; the upper stair is open. 100 gold earned.");
        }
        malakar_prepare(g);
        return applied;
    }
    if (e->boss_attack == MALAKAR_RESTORE && e->attack_target_x >= 0) {
        cancel(e, 2);
        push_message(g, "Your attack spills Malakar's restorative. He is exposed for two actions!");
    }
    return damage;
}

int malakar_interact(GameState *g, int x, int y) {
    if (g->level == 2 && x == 14 && y == 46) {
        push_message(g, "Malakar's private ledger: The crown's binding will restore my strength. The King must remain in his chamber.");
        return 1;
    }
    if (g->level == 2 && x == 54 && y == 8) {
        push_message(g, (g->castle_minibosses & 1) ? "Malakar's escape trail ends at a royal passage sealed from within. Reach the King's chamber by the upper castle." :
            "A royal service passage is sealed behind the alchemist's shelves.");
        return 1;
    }
    if (g->level != 2 || y != 16 || (x != 38 && x != 51) || ground(g, x, y) != TILE_CASTLE_WARD) {
        return 0;
    }
    set_ground(g, x, y, TILE_CASTLE_WARD_SPENT);
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (e->active && e->type == ENEMY_MALAKAR &&
            ((e->boss_attack == MALAKAR_RESTORE && e->attack_target_x == x) ||
            (e->boss_attack == MALAKAR_PRESSURE && e->boss_aux_y == x))) {
            cancel(e, 2);
        }
    }
    push_message(g, "The condenser drains. Its prepared mixture is ruined; this apparatus stays disabled.");
    return 1;
}

static void convert_enemies(Enemy *enemies, int count, int completed) {
    for (int i = 0; i < count; i++) {
        Enemy *e = &enemies[i];
        if (e->type == ENEMY_VAMPIRE && e->is_boss) {
            e->type = ENEMY_MALAKAR;
            snprintf(e->name, sizeof(e->name), "Malakar, Royal Alchemist");
            cancel(e, 0);
            e->attack_phase = 0;
            e->boss_wave_used = 0;
            e->revive_timer = 0;
            if (completed) {
                e->active = 0;
            }
        }
    }
}

int malakar_migrate(GameState *g) {
    int completed = !!(g->castle_minibosses & 1);
    if (g->castle_cache[1].valid) {
        LevelCache *cache = &g->castle_cache[1];
        if (!castle_remodel(&cache->map, cache->enemies, cache->enemy_count, g->castle_loot[1], g->castle_loot_count[1], -1, -1, 2)) {
            return 0;
        }
        convert_enemies(cache->enemies, cache->enemy_count, completed);
        // Existing actors may be south of the new seal; preserve their old access.
        fill(&cache->map, 34, 22, 3, 1, TILE_CASTLE_GATE_OPEN);
    }
    if (g->location == LOCATION_CASTLE_INTERIOR && g->level == 2) {
        if (!castle_remodel(&g->map, g->enemies, g->enemy_count, g->floor_items, g->floor_item_count, g->player.x, g->player.y, 2)) {
            return 0;
        }
        convert_enemies(g->enemies, g->enemy_count, completed);
        for (int x = 34; x <= 36; x++) {
            set_ground(g, x, 22, TILE_CASTLE_GATE_OPEN);
        }
        malakar_prepare(g);
    }
    return 1;
}
