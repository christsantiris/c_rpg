#include "pirate.h"
#include "approach.h"
#include "town_life.h"
#include <stdlib.h>
#include <string.h>

static int inside(int x, int y) {
    return x > 0 && x < PIRATE_W - 1 && y > 0 && y < PIRATE_H - 1;
}

const char *pirate_stage_name(int level) {
    static const char *names[PIRATE_DEPTH] = {
        "Wreckers' Strand", "Cutlass Stockade", "Smugglers' Piers", "Shipwreck Reach", "The Black Flagship"
    };
    return level >= 1 && level <= PIRATE_DEPTH ? names[level - 1] : "Pirate Coast";
}

typedef struct {
    int x[8];
    int y[8];
    int edges[9][2];
    int edge_count;
    int chest[3][2];
    int chest_anchor[3];
    int gate[2][2];
    int gate_anchor[2];
} PirateLayout;

static const PirateLayout layouts[PIRATE_DEPTH] = {
    {{10, 30, 50, 69, 10, 30, 50, 69}, {8, 8, 8, 8, 26, 26, 26, 26},
        {{0, 1}, {1, 5}, {5, 6}, {6, 2}, {2, 3}, {0, 4}, {4, 5}, {6, 7}}, 8,
        {{20, 3}, {40, 31}, {73, 18}}, {1, 6, 7}, {{8, 0}, {79, 10}}, {0, 3}},
    {{9, 28, 50, 69, 10, 30, 50, 69}, {9, 8, 8, 10, 26, 26, 25, 26},
        {{0, 4}, {4, 5}, {5, 1}, {1, 2}, {2, 6}, {6, 7}, {7, 3}, {1, 6}}, 8,
        {{15, 3}, {40, 31}, {72, 18}}, {0, 5, 3}, {{0, 10}, {79, 26}}, {0, 7}},
    {{9, 28, 30, 50, 69, 50, 69, 10}, {26, 25, 8, 8, 8, 25, 25, 8},
        {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {3, 5}, {5, 6}, {6, 4}, {2, 7}}, 8,
        {{10, 3}, {40, 31}, {73, 30}}, {7, 5, 6}, {{0, 26}, {65, 0}}, {0, 4}},
    {{10, 30, 50, 69, 10, 30, 50, 69}, {8, 8, 8, 10, 26, 26, 26, 26},
        {{6, 5}, {5, 4}, {4, 0}, {0, 1}, {1, 2}, {2, 3}, {2, 6}, {3, 7}}, 8,
        {{20, 3}, {40, 31}, {73, 20}}, {1, 6, 7}, {{65, 35}, {79, 12}}, {6, 3}},
    {{0}, {0}, {{0}}, 0, {{14, 11}, {34, 27}, {54, 8}}, {0}, {{0, 12}, {79, 18}}, {0}}
};

static const PirateLayout *layout_for(int level) {
    return &layouts[level >= 1 && level <= PIRATE_DEPTH ? level - 1 : 0];
}

void pirate_edge(int level, int forward, int *x, int *y) {
    const PirateLayout *layout = layout_for(level);
    *x = layout->gate[!!forward][0];
    *y = layout->gate[!!forward][1];
}

void pirate_chest_position(int level, int index, int *x, int *y) {
    const PirateLayout *layout = layout_for(level);
    *x = layout->chest[index][0];
    *y = layout->chest[index][1];
}

static TileType ground(int level) {
    return level >= 3 ? TILE_PIRATE_DECK : TILE_PIRATE_SAND;
}

static void rectangle(Map *map, int x, int y, int w, int h, TileType tile) {
    for (int py = y; py < y + h; py++) {
        for (int px = x; px < x + w; px++) {
            if (inside(px, py)) {
                map->tiles[py][px] = tile;
            }
        }
    }
}

static void cove(Map *map, int x, int y, int rx, int ry, TileType tile) {
    for (int py = -ry; py <= ry; py++) {
        for (int px = -rx; px <= rx; px++) {
            if (px * px * ry * ry + py * py * rx * rx <= rx * rx * ry * ry && inside(x + px, y + py)) {
                map->tiles[y + py][x + px] = tile;
            }
        }
    }
}

static void trail(Map *map, int x, int y, int tx, int ty, TileType tile) {
    while (x != tx || y != ty) {
        rectangle(map, x - 1, y - 1, 3, 3, tile);
        if (x != tx) {
            x += (tx > x) - (tx < x);
        } else {
            y += (ty > y) - (ty < y);
        }
    }
    rectangle(map, tx - 1, ty - 1, 3, 3, tile);
}

static void connect(Map *map, int ax, int ay, int bx, int by, int bend, TileType tile) {
    if (abs(bx - ax) > abs(by - ay)) {
        int mx = (ax + bx) / 2;
        trail(map, ax, ay, mx, ay + bend, tile);
        trail(map, mx, ay + bend, bx, by, tile);
    } else {
        int my = (ay + by) / 2;
        trail(map, ax, ay, ax + bend, my, tile);
        trail(map, ax + bend, my, bx, by, tile);
    }
}

static void gateway(Map *map, int level, int forward, int ax, int ay) {
    int x;
    int y;
    pirate_edge(level, forward, &x, &y);
    int dx = x == 0 ? 1 : x == PIRATE_W - 1 ? -1 : 0;
    int dy = y == 0 ? 1 : y == PIRATE_H - 1 ? -1 : 0;
    TileType tile = ground(level);
    TileType wall = level >= 3 ? TILE_PIRATE_RAIL : level == 2 ? TILE_PIRATE_PALISADE : TILE_PIRATE_CLIFF;
    trail(map, ax, ay, x + dx * 4, y + dy * 4, tile);
    for (int step = 0; step < 4; step++) {
        int px = x + dx * step;
        int py = y + dy * step;
        map->tiles[py][px] = tile;
        for (int side = -1; side <= 1; side += 2) {
            int wx = px + side * dy;
            int wy = py + side * dx;
            if (wx >= 0 && wx < PIRATE_W && wy >= 0 && wy < PIRATE_H) {
                map->tiles[wy][wx] = wall;
            }
        }
    }
    map->tiles[y][x] = forward ? TILE_PIRATE_EXIT : TILE_PIRATE_ENTRANCE;
    if (forward) {
        map->stairs_down_x = x + dx;
        map->stairs_down_y = y + dy;
    } else {
        map->stairs_up_x = x + dx;
        map->stairs_up_y = y + dy;
    }
}

static void flagship(Map *map) {
    cove(map, 40, 18, 36, 13, TILE_PIRATE_DECK);
    for (int x = 24; x <= 60; x += x == 24 ? 20 : 16) {
        for (int y = 5; y <= 31; y++) {
            if (map->tiles[y][x] == TILE_PIRATE_DECK) {
                map->tiles[y][x] = TILE_PIRATE_PALISADE;
            }
        }
        rectangle(map, x, x == 44 ? 8 : 22, 1, 3, TILE_PIRATE_DECK);
    }
    rectangle(map, 8, 16, 16, 1, TILE_PIRATE_PALISADE);
    rectangle(map, 16, 16, 3, 1, TILE_PIRATE_DECK);
    for (int x = 16; x <= 52; x += 18) {
        rectangle(map, x, 12, 2, 2, TILE_PIRATE_CRATE);
    }
}

void pirate_generate(Map *map, int level, unsigned int seed, unsigned int claimed) {
    const PirateLayout *layout = layout_for(level);
    TileType floor = ground(level);
    memset(map, 0, sizeof(*map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            map->tiles[y][x] = level <= 2 && x < PIRATE_W && y < 18 + (x / 9 % 3) ?
                level == 2 ? TILE_PIRATE_PALISADE : TILE_PIRATE_CLIFF : TILE_PIRATE_SEA;
        }
    }
    if (level == PIRATE_DEPTH) {
        flagship(map);
    } else {
        int cx[8];
        int cy[8];
        for (int i = 0; i < 8; i++) {
            unsigned int roll = seed * 1664525u + (unsigned int)i * 1013904223u;
            cx[i] = layout->x[i] + (int)(roll % 3) - 1;
            cy[i] = layout->y[i] + (int)((roll >> 8) % 3) - 1;
            int rx = 5 + (roll >> 16) % 3;
            int ry = 3 + (roll >> 20) % 2;
            if (level == 2 || level == 3) {
                rectangle(map, cx[i] - rx, cy[i] - ry, rx * 2 + 1, ry * 2 + 1, floor);
            } else {
                cove(map, cx[i], cy[i], rx, ry, floor);
            }
        }
        for (int i = 0; i < layout->edge_count; i++) {
            int a = layout->edges[i][0];
            int b = layout->edges[i][1];
            int bend = level == 3 ? 0 : (int)((seed + i) % 5) - 2;
            connect(map, cx[a], cy[a], cx[b], cy[b], bend, floor);
        }
        for (int i = 0; i < 3; i++) {
            int x = layout->chest[i][0];
            int y = layout->chest[i][1];
            int a = layout->chest_anchor[i];
            connect(map, cx[a], cy[a], x, y, 0, floor);
            cove(map, x, y, 3, 2, floor);
        }
        for (int i = 1; i < 8; i += 2) {
            // Cargo stays against the edge of a clearing; it never seals a route.
            map->tiles[cy[i] - 1][cx[i] + 2] = TILE_PIRATE_CRATE;
        }
        if (level == 1 || level == 4) {
            for (int i = 4; i <= 6; i += 2) {
                cove(map, cx[i], cy[i], 4, 2, TILE_PIRATE_DECK);
                rectangle(map, cx[i] - 2, cy[i] - 2, 5, 1, TILE_PIRATE_RAIL);
                rectangle(map, cx[i] - 1, cy[i] - 2, 3, 1, TILE_PIRATE_DECK);
            }
        }
        gateway(map, level, 0, cx[layout->gate_anchor[0]], cy[layout->gate_anchor[0]]);
        gateway(map, level, 1, cx[layout->gate_anchor[1]], cy[layout->gate_anchor[1]]);
    }
    if (level == PIRATE_DEPTH) {
        gateway(map, level, 0, 12, 12);
        gateway(map, level, 1, 69, 18);
    }
    for (int i = 0; i < 3; i++) {
        int x = layout->chest[i][0];
        int y = layout->chest[i][1];
        if (!(claimed & (1u << i))) {
            map->tiles[y][x] = TILE_PIRATE_CHEST;
        }
    }
    if (level == PIRATE_QUEST_LEVEL) {
        map->tiles[PIRATE_QUEST_Y][PIRATE_QUEST_X] = TILE_PIRATE_QUEST_CHEST;
    }
}

void pirate_refresh_treasure(GameState *g) {
    if (g->location != LOCATION_PIRATE_COAST || g->level != PIRATE_QUEST_LEVEL) {
        return;
    }
    TileType tile = g->pirate_quest_state >= 2 ? TILE_PIRATE_DECK : TILE_PIRATE_QUEST_CHEST;
    if (g->map.tiles[PIRATE_QUEST_Y][PIRATE_QUEST_X] == TILE_ITEM) {
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == PIRATE_QUEST_X && item->y == PIRATE_QUEST_Y && item->underlying_tile != TILE_PORTAL) {
                item->underlying_tile = tile;
            }
        }
    } else if (g->map.tiles[PIRATE_QUEST_Y][PIRATE_QUEST_X] != TILE_PORTAL) {
        g->map.tiles[PIRATE_QUEST_Y][PIRATE_QUEST_X] = tile;
    }
    if (g->portal_active && g->portal_location == LOCATION_PIRATE_COAST && g->portal_level == PIRATE_QUEST_LEVEL &&
        g->portal_x == PIRATE_QUEST_X && g->portal_y == PIRATE_QUEST_Y) {
        g->portal_origin_tile = tile;
    }
}

void pirate_talk_merchant(GameState *g) {
    if (g->location != LOCATION_MERCHANT) {
        return;
    }
    g->dialogue_active = 1;
    g->dialogue_x = RESIDENT_X;
    g->dialogue_y = RESIDENT_Y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "Merchant Corvin");
    if (!g->pirate_quest_state) {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "Take this treasure map. Pirates stole my family's silver coffer. Search the northwest pier on Pirate Coast 3. "
            "Return it here for 100 gold and a Mariner's Cloak, wearable by any class.");
    } else if (g->pirate_quest_state == 1) {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "Your map marks the silver coffer on Pirate Coast 3's northwest pier. Stand beside it and Interact, "
            "then return here for 100 gold and a Mariner's Cloak.");
    } else if (g->pirate_quest_state == 2) {
        if (g->inventory_count >= MAX_INVENTORY) {
            snprintf(g->dialogue_text, sizeof(g->dialogue_text),
                "You found my family's coffer! Make room in your pack for the Mariner's Cloak, "
                "then speak to me again. I will keep your 100-gold bounty ready.");
            return;
        }
        g->inventory[g->inventory_count++] = item_make_mariner_cloak();
        g->gold += PIRATE_QUEST_GOLD;
        g->pirate_quest_state = 3;
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "My family's coffer, at last! Here are your 100 gold and a Mariner's Cloak. "
            "Its sturdy weave grants 3 defense and a 5%% chance to evade attacks. Thank you.");
        push_message(g, "Completed: The Merchant's Lost Fortune. 100 gold and Mariner's Cloak awarded.");
    } else {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "My family's coffer is safe again. May the Mariner's Cloak serve you well on your travels.");
    }
}

static int near_quest_chest(const GameState *g) {
    return g->location == LOCATION_PIRATE_COAST && g->level == PIRATE_QUEST_LEVEL && g->pirate_quest_state < 2 &&
        abs(g->player.x - PIRATE_QUEST_X) <= 1 && abs(g->player.y - PIRATE_QUEST_Y) <= 1;
}

// All spawns and migration destinations must belong to the entrance's component.
void pirate_reachable(const Map *map, unsigned char cells[PIRATE_H][PIRATE_W]) {
    memset(cells, 0, PIRATE_H * PIRATE_W);
    int queue[PIRATE_H * PIRATE_W];
    int head = 0;
    int tail = 1;
    queue[0] = map->stairs_up_y * PIRATE_W + map->stairs_up_x;
    cells[map->stairs_up_y][map->stairs_up_x] = 1;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    while (head < tail) {
        int cell = queue[head++];
        for (int i = 0; i < 4; i++) {
            int x = cell % PIRATE_W + dx[i];
            int y = cell / PIRATE_W + dy[i];
            if (x >= 0 && x < PIRATE_W && y >= 0 && y < PIRATE_H && !cells[y][x] && map_is_walkable(map, x, y)) {
                cells[y][x] = 1;
                queue[tail++] = y * PIRATE_W + x;
            }
        }
    }
}

static void spawn(GameState *g, EnemyType type, int x, int y) {
    if (g->enemy_count >= PIRATE_ENEMIES) {
        return;
    }
    Enemy *e = &g->enemies[g->enemy_count++];
    *e = (Enemy){.type = type, .active = 1, .x = x, .y = y,
        .attack_target_x = -1, .attack_target_y = -1};
    const char *names[] = {"Raider", "Crossbow Corsair", "Buccaneer", "Deck Brute", "Swashbuckler", "Captain Voss"};
    const int hp[] = {42, 32, 34, 100, 55, 480};
    const int attack[] = {12, 13, 17, 20, 22, 26};
    const int defense[] = {3, 2, 2, 7, 4, 9};
    int n = type - ENEMY_PIRATE_RAIDER;
    snprintf(e->name, sizeof(e->name), "%s", names[n]);
    e->is_boss = type == ENEMY_CAPTAIN_VOSS;
    e->max_hp = hp[n] + (e->is_boss ? 0 : g->level * 5);
    e->hp = e->max_hp;
    e->attack = attack[n] + (e->is_boss ? 0 : g->level);
    e->defense = defense[n];
    e->experience = e->is_boss ? 800 : 45 + n * 10;
}

static void load_floor(GameState *g) {
    PirateCache *cache = &g->pirate_cache[g->level - 1];
    int fresh = !cache->seed;
    if (fresh) {
        cache->seed = 1u + (unsigned int)rand() % 1000000u;
    }
    pirate_generate(&g->map, g->level, cache->seed, cache->treasure_claimed);
    g->enemy_count = 0;
    g->floor_item_count = 0;
    if (fresh) {
        unsigned char reachable[PIRATE_H][PIRATE_W];
        int candidates[PIRATE_H * PIRATE_W];
        int count = 0;
        pirate_reachable(&g->map, reachable);
        for (int y = 1; y < PIRATE_H - 1; y++) {
            for (int x = 1; x < PIRATE_W - 1; x++) {
                TileType tile = g->map.tiles[y][x];
                if (reachable[y][x] && (tile == TILE_PIRATE_SAND || tile == TILE_PIRATE_DECK) &&
                    abs(x - g->map.stairs_up_x) + abs(y - g->map.stairs_up_y) >= 10 &&
                    abs(x - g->map.stairs_down_x) + abs(y - g->map.stairs_down_y) >= 3 &&
                    !(g->level == PIRATE_DEPTH && abs(x - 69) + abs(y - 18) < 4)) {
                    candidates[count++] = y * PIRATE_W + x;
                }
            }
        }
        unsigned int roll = cache->seed;
        int wanted = 8 + g->level / 2;
        for (int i = 0; i < wanted && count; i++) {
            roll = roll * 1664525u + 1013904223u;
            int pick = roll % (unsigned int)count;
            int cell = candidates[pick];
            candidates[pick] = candidates[--count];
            spawn(g, ENEMY_PIRATE_RAIDER + (i + g->level) % 5, cell % PIRATE_W, cell / PIRATE_W);
        }
        if (g->level == PIRATE_DEPTH && !g->pirate_defeated) {
            spawn(g, ENEMY_CAPTAIN_VOSS, 69, 18);
        }
    } else {
        g->enemy_count = cache->enemy_count;
        memcpy(g->enemies, cache->enemies, sizeof(cache->enemies));
        g->floor_item_count = cache->loot_count;
        memcpy(g->floor_items, cache->loot, sizeof(cache->loot));
        for (int y = 0; y < PIRATE_H; y++) {
            for (int x = 0; x < PIRATE_W; x++) {
                int bit = y * PIRATE_W + x;
                if (cache->explored[bit / 8] & (1u << (bit % 8))) {
                    map_mark_explored(&g->map, x, y);
                }
            }
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active) {
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
    }
    pirate_refresh_treasure(g);
    g->level_cleared = 1;
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active) {
            g->level_cleared = 0;
        }
    }
}

void pirate_capture(const GameState *g, PirateCache *cache) {
    cache->enemy_count = g->enemy_count;
    memcpy(cache->enemies, g->enemies, sizeof(cache->enemies));
    cache->loot_count = g->floor_item_count;
    memcpy(cache->loot, g->floor_items, sizeof(cache->loot));
    for (int y = 0; y < PIRATE_H; y++) {
        for (int x = 0; x < PIRATE_W; x++) {
            int bit = y * PIRATE_W + x;
            if (map_is_explored(&g->map, x, y)) {
                cache->explored[bit / 8] |= 1u << (bit % 8);
            }
        }
    }
}

void pirate_store(GameState *g) {
    if (g->location == LOCATION_PIRATE_COAST && g->level >= 1 && g->level <= PIRATE_DEPTH) {
        pirate_capture(g, &g->pirate_cache[g->level - 1]);
    }
}

// Keep old expedition state while moving actors off the retired layout.
static void nearest_ground(const Map *map, unsigned char cells[PIRATE_H][PIRATE_W], int *x, int *y) {
    int best = PIRATE_W + PIRATE_H + abs(*x) + abs(*y);
    int bx = map->stairs_up_x;
    int by = map->stairs_up_y;
    for (int py = 1; py < PIRATE_H - 1; py++) {
        for (int px = 1; px < PIRATE_W - 1; px++) {
            TileType tile = map->tiles[py][px];
            int distance = abs(px - *x) + abs(py - *y);
            if (cells[py][px] && (tile == TILE_PIRATE_SAND || tile == TILE_PIRATE_DECK) && distance < best) {
                best = distance;
                bx = px;
                by = py;
            }
        }
    }
    *x = bx;
    *y = by;
}

void pirate_migrate(GameState *g, int version) {
    if (version >= 185) {
        return;
    }
    int active = g->location == LOCATION_PIRATE_COAST && g->level >= 1 && g->level <= PIRATE_DEPTH;
    if (active && g->enemy_count <= PIRATE_ENEMIES && g->pirate_cache[g->level - 1].seed) {
        pirate_store(g);
    } else if (g->location == LOCATION_PIRATE_COAST) {
        return;
    }
    Map map;
    for (int level = 1; level <= PIRATE_DEPTH; level++) {
        PirateCache *cache = &g->pirate_cache[level - 1];
        if (!cache->seed) {
            continue;
        }
        pirate_generate(&map, level, cache->seed, cache->treasure_claimed);
        unsigned char reachable[PIRATE_H][PIRATE_W];
        unsigned char available[PIRATE_H][PIRATE_W];
        pirate_reachable(&map, reachable);
        memcpy(available, reachable, sizeof(available));
        int moved_player = 0;
        if (active && g->level == level) {
            int x = g->player.x;
            int y = g->player.y;
            nearest_ground(&map, available, &g->player.x, &g->player.y);
            moved_player = x != g->player.x || y != g->player.y;
            available[g->player.y][g->player.x] = 0;
        }
        for (int i = 0; i < cache->enemy_count; i++) {
            Enemy *enemy = &cache->enemies[i];
            if (!enemy->active) {
                continue;
            }
            int x = enemy->x;
            int y = enemy->y;
            nearest_ground(&map, available, &enemy->x, &enemy->y);
            available[enemy->y][enemy->x] = 0;
            if (moved_player || x != enemy->x || y != enemy->y) {
                // A shifted warning must be announced again before it can hurt.
                enemy->attack_phase = 0;
                enemy->move_timer = -1;
            }
        }
        for (int i = 0; i < cache->loot_count; i++) {
            FloorItem *item = &cache->loot[i];
            if (item->active) {
                nearest_ground(&map, reachable, &item->x, &item->y);
                item->underlying_tile = map.tiles[item->y][item->x];
            }
        }
        if (g->portal_active && g->portal_location == LOCATION_PIRATE_COAST && g->portal_level == level) {
            nearest_ground(&map, available, &g->portal_x, &g->portal_y);
            g->portal_origin_tile = map.tiles[g->portal_y][g->portal_x];
        }
    }
    if (active) {
        load_floor(g);
        push_message(g, "Pirate Coast's routes have changed. Your expedition progress is preserved.");
    }
}

void pirate_enter(GameState *g) {
    g->location = LOCATION_PIRATE_COAST;
    g->approach_route = -1;
    g->dialogue_active = 0;
    g->level = 1;
    load_floor(g);
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
    int x;
    int y;
    pirate_edge(1, 0, &x, &y);
    g->player.last_dx = g->player.x - x;
    g->player.last_dy = g->player.y - y;
    push_message(g, "Pirate Coast: explore the coves and side paths. Boundary gates lead onward or back to Dawnmere.");
}

void pirate_leave(GameState *g) {
    pirate_store(g);
    for (int i = 0; i < town_approach_count; i++) {
        if (town_approaches[i].area == LOCATION_PIRATE_COAST) {
            approach_enter_region(g, i);
            g->player.x = g->map.stairs_down_x;
            g->player.y = g->map.stairs_down_y + 1;
            g->player.last_dx = 0;
            g->player.last_dy = 1;
            return;
        }
    }
}

void pirate_change_level(GameState *g, int descending) {
    int next = g->level + (descending ? 1 : -1);
    if (next < 1 || next > PIRATE_DEPTH) {
        return;
    }
    pirate_store(g);
    g->level = next;
    load_floor(g);
    g->player.x = descending ? g->map.stairs_up_x : g->map.stairs_down_x;
    g->player.y = descending ? g->map.stairs_up_y : g->map.stairs_down_y;
    int x;
    int y;
    pirate_edge(g->level, !descending, &x, &y);
    g->player.last_dx = g->player.x - x;
    g->player.last_dy = g->player.y - y;
    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message), "Pirate Coast %d: %s.", g->level, pirate_stage_name(g->level));
    push_message(g, message);
}

int pirate_travel(GameState *g, int x, int y) {
    if (g->location != LOCATION_PIRATE_COAST) {
        return 0;
    }
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_ITEM) {
        for (int i = 0; i < g->floor_item_count; i++) {
            if (g->floor_items[i].active && g->floor_items[i].x == x && g->floor_items[i].y == y) {
                tile = g->floor_items[i].underlying_tile;
                break;
            }
        }
    }
    if (tile == TILE_PIRATE_ENTRANCE) {
        if (g->level == 1) {
            pirate_leave(g);
        } else {
            pirate_change_level(g, 0);
        }
        return 1;
    }
    if (tile == TILE_PIRATE_EXIT) {
        if (g->level < PIRATE_DEPTH) {
            pirate_change_level(g, 1);
        } else if (g->pirate_defeated || g->debug_passages_unlocked) {
            pirate_leave(g);
        } else {
            push_message(g, "Captain Voss controls the flagship. Defeat him before departing.");
        }
        return 1;
    }
    return 0;
}

int pirate_near_chest(const GameState *g) {
    if (near_quest_chest(g)) {
        return 1;
    }
    if (g->location != LOCATION_PIRATE_COAST) {
        return 0;
    }
    for (int i = 0; i < 3; i++) {
        int x;
        int y;
        pirate_chest_position(g->level, i, &x, &y);
        if (!(g->pirate_cache[g->level - 1].treasure_claimed & (1u << i)) &&
            abs(g->player.x - x) <= 1 && abs(g->player.y - y) <= 1) {
            return 1;
        }
    }
    return 0;
}

int pirate_interact(GameState *g) {
    if (near_quest_chest(g)) {
        if (!g->pirate_quest_state) {
            push_message(g, "The coffer bears a merchant's seal. Someone in Dawnmere may know how to open it.");
        } else {
            for (int i = 0; i < g->enemy_count; i++) {
                if (g->enemies[i].active && g->enemies[i].x == PIRATE_QUEST_X && g->enemies[i].y == PIRATE_QUEST_Y) {
                    push_message(g, "A pirate guards the coffer. Defeat its guard first.");
                    return 1;
                }
            }
            g->pirate_quest_state = 2;
            pirate_refresh_treasure(g);
            push_message(g, "Corvin's silver coffer recovered! Return to the merchant shop in Dawnmere.");
        }
        return 1;
    }
    if (!pirate_near_chest(g)) {
        return 0;
    }
    for (int i = 0; i < 3; i++) {
        int x;
        int y;
        pirate_chest_position(g->level, i, &x, &y);
        if (abs(g->player.x - x) <= 1 && abs(g->player.y - y) <= 1) {
            g->pirate_cache[g->level - 1].treasure_claimed |= 1u << i;
            g->map.tiles[y][x] = ground(g->level);
            g->gold += 25 + g->level * 5;
            push_message(g, "Plunder claimed! The captured chest is empty.");
            break;
        }
    }
    return 1;
}

static int clear_lane(const GameState *g, int x, int y, int tx, int ty) {
    if (x != tx && y != ty) {
        return 0;
    }
    int dx = (tx > x) - (tx < x);
    int dy = (ty > y) - (ty < y);
    while (x != tx || y != ty) {
        x += dx;
        y += dy;
        if (!map_is_walkable(&g->map, x, y)) {
            return 0;
        }
    }
    return 1;
}

int pirate_marks(const GameState *g, const Enemy *e, int x, int y) {
    if (!e->active || (e->type != ENEMY_PIRATE_CROSSBOW && e->type != ENEMY_CAPTAIN_VOSS) || !e->attack_phase || e->move_timer <= 0 || !inside(x, y) || !map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    if (e->boss_attack == PIRATE_BLADE_WAVE) {
        return abs(y - e->attack_target_y) <= 1 && abs(x - e->x) <= 18;
    }
    if (e->boss_attack == PIRATE_SABER) {
        return abs(x - e->attack_target_x) <= 1 && abs(y - e->attack_target_y) <= 1;
    }
    int dx = e->attack_target_x - e->x;
    int dy = e->attack_target_y - e->y;
    int same_ray = dx ? y == e->y && (x - e->x) * dx > 0 : x == e->x && (y - e->y) * dy > 0;
    return same_ray && abs(x - e->x) + abs(y - e->y) <= 10 && clear_lane(g, e->x, e->y, x, y);
}

int pirate_tick(GameState *g, Enemy *e, int *damage) {
    if (g->location != LOCATION_PIRATE_COAST || (e->type != ENEMY_PIRATE_CROSSBOW && e->type != ENEMY_CAPTAIN_VOSS)) {
        return 0;
    }
    *damage = 0;
    if (e->move_timer < 0) {
        e->move_timer++;
        return 1;
    }
    if (e->attack_phase) {
        if (e->move_timer > 1) {
            e->move_timer--;
            return 1;
        }
        if (pirate_marks(g, e, g->player.x, g->player.y)) {
            *damage = e->attack;
        }
        e->attack_phase = 0;
        e->move_timer = e->is_boss ? -2 : -1;
        return 1;
    }
    int dx = g->player.x - e->x;
    int dy = g->player.y - e->y;
    int distance = abs(dx) + abs(dy);
    if (distance > (e->is_boss ? 18 : 10)) {
        return 1;
    }
    int attack = PIRATE_SHOT;
    if (e->is_boss) {
        attack = distance <= 3 ? PIRATE_SABER : PIRATE_BLADE_WAVE;
    } else if (!clear_lane(g, e->x, e->y, g->player.x, g->player.y)) {
        // Crossbowmen advance to align a shot.
        if (e->type == ENEMY_PIRATE_CROSSBOW) {
            int nx = e->x + (dx > 0) - (dx < 0);
            int ny = e->y + (dy > 0) - (dy < 0);
            if (map_is_walkable(&g->map, nx, ny)) {
                int occupied = nx == g->player.x && ny == g->player.y;
                for (int i = 0; i < g->enemy_count; i++) {
                    occupied |= g->enemies[i].active && &g->enemies[i] != e && g->enemies[i].x == nx && g->enemies[i].y == ny;
                }
                if (!occupied) {
                    e->x = nx;
                    e->y = ny;
                }
            }
        }
        return 1;
    }
    e->boss_attack = attack;
    e->attack_target_x = attack == PIRATE_SABER ? e->x : g->player.x;
    e->attack_target_y = attack == PIRATE_SABER ? e->y : g->player.y;
    e->attack_phase = 1;
    e->move_timer = e->type == ENEMY_PIRATE_CROSSBOW ? 1 : 2;
    push_message(g, attack == PIRATE_BLADE_WAVE ? "Voss unleashes a blade wave! Clear the three marked deck rows." :
        attack == PIRATE_SABER ? "Voss draws back his saber! Step beyond the marked sweep." : "Incoming shot! Leave the marked firing lane or get behind cargo.");
    return 1;
}

void pirate_defeat(GameState *g) {
    if (!g->pirate_defeated) {
        g->pirate_defeated = 1;
        g->gold += 200;
        g->score += 2200;
        push_message(g, "Captain Voss falls! Pirate Coast is free. The flagship's departure opens.");
    }
}

void pirate_portal(GameState *g) {
    if (g->portal_level < 1 || g->portal_level > PIRATE_DEPTH || !g->pirate_cache[g->portal_level - 1].seed) {
        return;
    }
    g->location = LOCATION_PIRATE_COAST;
    g->level = g->portal_level;
    g->portal_active = 0;
    g->approach_route = -1;
    load_floor(g);
    int best = PIRATE_W + PIRATE_H;
    for (int y = 1; y < PIRATE_H - 1; y++) {
        for (int x = 1; x < PIRATE_W - 1; x++) {
            int distance = abs(x - g->portal_x) + abs(y - g->portal_y);
            int open = map_is_walkable(&g->map, x, y) && g->map.tiles[y][x] != TILE_PIRATE_ENTRANCE && g->map.tiles[y][x] != TILE_PIRATE_EXIT;
            for (int i = 0; i < g->enemy_count; i++) {
                open &= !g->enemies[i].active || g->enemies[i].x != x || g->enemies[i].y != y;
                open &= !pirate_marks(g, &g->enemies[i], x, y);
            }
            if (open && distance < best) {
                best = distance;
                g->player.x = x;
                g->player.y = y;
            }
        }
    }
    push_message(g, "You return to Pirate Coast through the town portal.");
}
