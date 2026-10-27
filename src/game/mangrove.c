#include "boss_gates.h"
#include "containers.h"
#include "mangrove.h"
#include "town_road.h"
#include "spice_skiff.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *name;
    int islands[4][4];
    int landings[4][2];
    int caches[3][2];
    int entry[2];
    int exit[2];
} DeltaLayout;

static const DeltaLayout layouts[MANGROVE_DEPTH] = {
    {"Saltwind Landing", {{14, 8, 9, 6}, {43, 11, 11, 6}, {23, 30, 11, 6}, {51, 30, 7, 5}},
        {{14, 14}, {32, 11}, {23, 24}, {51, 25}}, {{44, 9}, {25, 31}, {52, 31}}, {14, 0}, {63, 30}},
    {"Splitroot Delta", {{9, 29, 7, 7}, {24, 8, 9, 5}, {49, 11, 9, 7}, {32, 30, 11, 6}},
        {{16, 29}, {24, 13}, {40, 11}, {32, 24}}, {{24, 8}, {49, 11}, {34, 31}}, {0, 30}, {32, 39}},
    {"Mudback Pools", {{32, 7, 10, 5}, {10, 25, 7, 9}, {34, 29, 11, 7}, {53, 15, 8, 8}},
        {{32, 12}, {17, 25}, {34, 22}, {45, 15}}, {{10, 24}, {35, 30}, {54, 16}}, {32, 0}, {63, 15}},
    {"Stiltwater Village", {{8, 15, 6, 10}, {29, 9, 10, 6}, {30, 29, 10, 7}, {52, 30, 9, 6}},
        {{14, 15}, {29, 15}, {30, 22}, {52, 24}}, {{29, 8}, {31, 30}, {53, 31}}, {0, 15}, {52, 39}},
    {"Heartroot Sanctuary", {{52, 7, 8, 5}, {13, 15, 9, 8}, {17, 31, 10, 5}, {43, 29, 14, 8}},
        {{52, 12}, {22, 15}, {17, 26}, {43, 21}}, {{13, 14}, {18, 32}, {46, 32}}, {52, 0}, {43, 39}}
};

static int inside(int x, int y) {
    return x > 0 && x < MANGROVE_W - 1 && y > 0 && y < MANGROVE_H - 1;
}

static int sign(int value) {
    return (value > 0) - (value < 0);
}

static const DeltaLayout *layout(int stage) {
    return &layouts[stage >= 1 && stage <= MANGROVE_DEPTH ? stage - 1 : 0];
}

const char *mangrove_stage_name(int stage) {
    return layout(stage)->name;
}

void mangrove_landing(int stage, int index, int *x, int *y) {
    *x = layout(stage)->landings[index][0];
    *y = layout(stage)->landings[index][1];
}

void mangrove_cache(int stage, int index, int *x, int *y) {
    *x = layout(stage)->caches[index][0];
    *y = layout(stage)->caches[index][1];
}

void mangrove_island(int stage, int index, int *x, int *y) {
    *x = layout(stage)->islands[index][0];
    *y = layout(stage)->islands[index][1];
}

static TileType visible_tile(const GameState *g, int x, int y) {
    for (int i = 0; i < g->floor_item_count; i++) {
        const FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            return item->underlying_tile;
        }
    }
    return g->map.tiles[y][x];
}

static void carve(Map *map, int x, int y, int tx, int ty, TileType floor) {
    while (1) {
        map->tiles[y][x] = floor;
        if (x == tx && y == ty) {
            break;
        }
        if (x != tx) {
            x += sign(tx - x);
        } else {
            y += sign(ty - y);
        }
    }
}

void mangrove_generate_level(Map *map, int stage, unsigned int searched) {
    const DeltaLayout *plan = layout(stage);
    memset(map, 0, sizeof(*map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            map->tiles[y][x] = TILE_MANGROVE_WATER;
        }
    }
    for (int i = 0; i < 4; i++) {
        int cx = plan->islands[i][0];
        int cy = plan->islands[i][1];
        int rx = plan->islands[i][2];
        int ry = plan->islands[i][3];
        for (int y = cy - ry; y <= cy + ry; y++) {
            for (int x = cx - rx; x <= cx + rx; x++) {
                int dx = x - cx;
                int dy = y - cy;
                if (dx * dx * ry * ry + dy * dy * rx * rx <= rx * rx * ry * ry) {
                    map->tiles[y][x] = stage == 4 && i > 0 ? TILE_MANGROVE_DECK : TILE_MANGROVE_MUD;
                }
            }
        }
        for (int y = cy - ry + 2; y <= cy + ry - 2; y += 3) {
            for (int x = cx - rx + 2; x <= cx + rx - 2; x += 3) {
                if (abs(x - cx) > 2 && abs(y - cy) > 1 && map->tiles[y][x] == TILE_MANGROVE_MUD &&
                    map->tiles[y - 1][x] == TILE_MANGROVE_MUD && map->tiles[y + 1][x] == TILE_MANGROVE_MUD &&
                    map->tiles[y][x - 1] == TILE_MANGROVE_MUD && map->tiles[y][x + 1] == TILE_MANGROVE_MUD) {
                    map->tiles[y][x] = TILE_MANGROVE_ROOT;
                }
            }
        }
        map->tiles[cy - 2][cx - 3] = TILE_MANGROVE_ROOT;
        map->tiles[cy + 2][cx + 3] = TILE_MANGROVE_ROOT;
        int lx = plan->landings[i][0];
        int ly = plan->landings[i][1];
        carve(map, lx, ly, cx, cy, stage == 4 && i > 0 ? TILE_MANGROVE_DECK : TILE_MANGROVE_MUD);
        map->tiles[ly][lx] = TILE_MANGROVE_LANDING;
    }
    const int rootbeds[4][2] = {{7, 22}, {33, 19}, {44, 23}, {59, 17}};
    for (int i = 0; i < 4; i++) {
        for (int dy = 0; dy < 2; dy++) {
            for (int dx = 0; dx < 2; dx++) {
                int x = rootbeds[i][0] + dx;
                int y = rootbeds[i][1] + dy;
                if (stage == 1 || map->tiles[y][x] == TILE_MANGROVE_WATER) {
                    map->tiles[y][x] = TILE_MANGROVE_ROOT;
                }
            }
        }
    }
    if (stage == 4) {
        carve(map, 37, 11, 37, 29, TILE_MANGROVE_DECK);
    }
    // Reachable boundary jetties continue into the next stage's opposite edge.
    carve(map, plan->entry[0], plan->entry[1], plan->islands[0][0], plan->islands[0][1], TILE_MANGROVE_MUD);
    carve(map, plan->exit[0], plan->exit[1], plan->islands[3][0], plan->islands[3][1], stage == 4 ? TILE_MANGROVE_DECK : TILE_MANGROVE_MUD);
    map->tiles[plan->entry[1]][plan->entry[0]] = TILE_MANGROVE_RETURN;
    map->tiles[plan->exit[1]][plan->exit[0]] = TILE_MANGROVE_EXIT;
    for (int i = 0; i < 3; i++) {
        if (!(searched & (1u << i))) {
            map->tiles[plan->caches[i][1]][plan->caches[i][0]] = TILE_MANGROVE_CACHE;
        }
    }
    map->stairs_up_x = plan->entry[0];
    map->stairs_up_y = plan->entry[1];
    map->stairs_down_x = plan->exit[0];
    map->stairs_down_y = plan->exit[1];
}

void mangrove_generate(Map *map, unsigned int searched) {
    mangrove_generate_level(map, 1, searched);
}

void mangrove_capture(const GameState *g, MangroveState *state) {
    if (g->location != LOCATION_MANGROVES) {
        return;
    }
    state->stage = g->level;
    memset(state->explored, 0, sizeof(state->explored));
    for (int y = 0; y < MANGROVE_H; y++) {
        for (int x = 0; x < MANGROVE_W; x++) {
            int bit = y * MANGROVE_W + x;
            if (map_is_explored(&g->map, x, y)) {
                state->explored[bit / 8] |= 1u << (bit % 8);
            }
        }
    }
    state->loot_count = g->floor_item_count;
    memcpy(state->loot, g->floor_items, sizeof(state->loot));
    state->enemy_count = g->enemy_count;
    memcpy(state->enemies, g->enemies, g->enemy_count * sizeof(Enemy));
}

void mangrove_store(GameState *g) {
    mangrove_capture(g, &g->mangroves);
    if (g->location == LOCATION_MANGROVES) {
        g->mangrove_stages[g->level - 1] = g->mangroves;
    }
}

void mangrove_path(GameState *g, int returning) {
    g->location = LOCATION_MANGROVE_PATH;
    g->level = 1;
    g->approach_route = -1;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    memset(&g->map, 0, sizeof(g->map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g->map.tiles[y][x] = y >= MANGROVE_GATE_Y ? TILE_MANGROVE_WATER :
                x > 0 && x < MANGROVE_PATH_W - 1 && y > 0 ? TILE_TOWN_FLOOR : TILE_WALL;
        }
    }
    for (int y = 0; y <= MANGROVE_GATE_Y - 4; y++) {
        g->map.tiles[y][MANGROVE_GATE_X] = TILE_TOWN_PATH;
    }
    for (int y = MANGROVE_GATE_Y - 3; y <= MANGROVE_GATE_Y; y++) {
        for (int x = MANGROVE_GATE_X - 3; x <= MANGROVE_GATE_X + 3; x++) {
            g->map.tiles[y][x] = TILE_MANGROVE_ROOT;
        }
    }
    g->player.x = MANGROVE_GATE_X;
    g->player.y = returning ? MANGROVE_GATE_Y - 4 : 1;
    g->player.last_dx = 0;
    g->player.last_dy = returning ? -1 : 1;
    map_mark_explored(&g->map, g->player.x, g->player.y);
    push_message(g, "Saltwind Marsh lies south. Dawnmere is back to the north.");
}

static void reset_boat(MangroveState *state, int island) {
    const DeltaLayout *plan = layout(state->stage);
    int x = plan->landings[island][0];
    int y = plan->landings[island][1];
    int dx = sign(x - plan->islands[island][0]);
    int dy = sign(y - plan->islands[island][1]);
    state->boat_x = x + dx;
    state->boat_y = y + dy;
    state->boat_dx = dx;
    state->boat_dy = dy;
    state->aboard = 0;
}

static void load_stage(GameState *g, int stage) {
    g->location = LOCATION_MANGROVES;
    g->level = stage;
    g->mangroves = g->mangrove_stages[stage - 1];
    MangroveState *state = &g->mangroves;
    int fresh = !state->initialized;
    if (fresh) {
        *state = (MangroveState){.initialized = 1, .stage = stage};
        reset_boat(state, 0);
    }
    mangrove_generate_level(&g->map, stage, state->searched);
    for (int y = 0; y < MANGROVE_H; y++) {
        for (int x = 0; x < MANGROVE_W; x++) {
            int bit = y * MANGROVE_W + x;
            if (state->explored[bit / 8] & (1u << (bit % 8))) {
                map_mark_explored(&g->map, x, y);
            }
        }
    }
    g->floor_item_count = state->loot_count;
    memcpy(g->floor_items, state->loot, sizeof(g->floor_items));
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active) {
            item->underlying_tile = g->map.tiles[item->y][item->x];
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        }
    }
    g->enemy_count = state->enemy_count;
    memcpy(g->enemies, state->enemies, state->enemy_count * sizeof(Enemy));
    if (fresh) {
        mangrove_spawn(g, 1);
    }
    g->level_cleared = 0;
    g->approach_route = -1;
    g->dialogue_active = 0;
}

static void arrival(GameState *g, int forward) {
    int x = forward ? g->map.stairs_up_x : g->map.stairs_down_x;
    int y = forward ? g->map.stairs_up_y : g->map.stairs_down_y;
    g->player.x = x == 0 ? 1 : x == MANGROVE_W - 1 ? MANGROVE_W - 2 : x;
    g->player.y = y == 0 ? 1 : y == MANGROVE_H - 1 ? MANGROVE_H - 2 : y;
    g->player.last_dx = x == 0 ? 1 : x == MANGROVE_W - 1 ? -1 : 0;
    g->player.last_dy = y == 0 ? 1 : y == MANGROVE_H - 1 ? -1 : 0;
    map_mark_explored(&g->map, g->player.x, g->player.y);
}

void mangrove_enter(GameState *g, int portal) {
    if (!portal) {
        // Physical entry begins a new expedition. Stage travel and portals keep wounds.
        for (int i = 0; i < MANGROVE_DEPTH; i++) {
            if (g->mangrove_stages[i].initialized) {
                load_stage(g, i + 1);
                reset_boat(&g->mangroves, 0);
                mangrove_spawn(g, 0);
                mangrove_store(g);
            }
        }
        if (g->portal_location == LOCATION_MANGROVES) {
            g->portal_active = 0;
        }
    }
    int stage = portal ? g->portal_level : 1;
    load_stage(g, stage);
    if (portal) {
        g->player.x = g->portal_x;
        g->player.y = g->portal_y;
        g->portal_active = 0;
    } else {
        arrival(g, 1);
    }
    g->player.last_dx = g->mangroves.aboard ? g->mangroves.boat_dx : g->player.last_dx;
    g->player.last_dy = g->mangroves.aboard ? g->mangroves.boat_dy : g->player.last_dy;
    map_mark_explored(&g->map, g->player.x, g->player.y);
    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message), "Saltwind %d: %s. Explore by skiff; fight ashore.", stage, mangrove_stage_name(stage));
    push_message(g, message);
}

void mangrove_leave(GameState *g) {
    mangrove_store(g);
    mangrove_path(g, 1);
}

void mangrove_change_level(GameState *g, int forward) {
    if (g->mangroves.aboard) {
        return;
    }
    int stage = g->level;
    if (forward && stage == MANGROVE_DEPTH) {
        if (boss_gate_crossing(g, LOCATION_MANGROVES)) {
            mangrove_leave(g);
        }
        return;
    }
    if (!forward && stage == 1) {
        mangrove_leave(g);
        return;
    }
    mangrove_store(g);
    load_stage(g, stage + (forward ? 1 : -1));
    arrival(g, forward);
    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message), "Saltwind %d: %s.", g->level, mangrove_stage_name(g->level));
    push_message(g, message);
}

int mangrove_travel(GameState *g, int x, int y) {
    if (g->location == LOCATION_MANGROVE_PATH) {
        if (x == MANGROVE_GATE_X && y == 0) {
            town_road_enter(g, LOCATION_DAWNMERE, TOWN_SIDE_SOUTH, 0);
            return 1;
        }
        if (x >= MANGROVE_GATE_X - 3 && x <= MANGROVE_GATE_X + 3 && y >= MANGROVE_GATE_Y - 3 && y <= MANGROVE_GATE_Y) {
            mangrove_enter(g, 0);
            return 1;
        }
        return 0;
    }
    if (g->location != LOCATION_MANGROVES) {
        return 0;
    }
    if (g->mangroves.aboard) {
        if (inside(x, y) && abs(x - g->player.x) + abs(y - g->player.y) == 1 && g->map.tiles[y][x] == TILE_MANGROVE_WATER) {
            g->mangroves.boat_dx = g->player.last_dx = x - g->player.x;
            g->mangroves.boat_dy = g->player.last_dy = y - g->player.y;
            g->mangroves.boat_x = g->player.x = x;
            g->mangroves.boat_y = g->player.y = y;
            map_mark_explored(&g->map, x, y);
            action_resolve_enemies(g);
        }
        return 1;
    }
    if ((x == g->map.stairs_up_x && y == g->map.stairs_up_y) ||
        (x == g->map.stairs_down_x && y == g->map.stairs_down_y)) {
        mangrove_change_level(g, x == g->map.stairs_down_x && y == g->map.stairs_down_y);
        return 1;
    }
    return 0;
}

const char *mangrove_hint(const GameState *g) {
    if (g->location != LOCATION_MANGROVES) {
        return NULL;
    }
    if (g->mangroves.aboard) {
        for (int i = 0; i < 4; i++) {
            int x;
            int y;
            mangrove_landing(g->level, i, &x, &y);
            if (abs(g->player.x - x) + abs(g->player.y - y) == 1) {
                return "GO ASHORE";
            }
        }
        return NULL;
    }
    if (visible_tile(g, g->player.x, g->player.y) == TILE_MANGROVE_LANDING &&
        abs(g->player.x - g->mangroves.boat_x) + abs(g->player.y - g->mangroves.boat_y) == 1) {
        return "BOARD SKIFF";
    }
    const char *quest = spice_skiff_hint(g);
    if (quest) {
        return quest;
    }
    if (containers_hint(g)) {
        return containers_hint(g);
    }
    for (int i = 0; i < 3; i++) {
        int x;
        int y;
        mangrove_cache(g->level, i, &x, &y);
        if (!(g->mangroves.searched & (1u << i)) && abs(g->player.x - x) + abs(g->player.y - y) <= 1) {
            return "SEARCH SUPPLIES";
        }
    }
    return NULL;
}

int mangrove_interact(GameState *g) {
    if (spice_skiff_interact(g)) {
        return 1;
    }
    const char *hint = mangrove_hint(g);
    if (!hint) {
        return 0;
    }
    if (g->mangroves.aboard) {
        for (int i = 0; i < 4; i++) {
            int x;
            int y;
            mangrove_landing(g->level, i, &x, &y);
            if (abs(g->player.x - x) + abs(g->player.y - y) == 1) {
                int occupied = 0;
                for (int j = 0; j < g->enemy_count; j++) {
                    occupied |= g->enemies[j].active && abs(g->enemies[j].x - x) + abs(g->enemies[j].y - y) <= 1;
                }
                if (occupied) {
                    push_message(g, "That landing is occupied. Use another approach or wait for it to clear.");
                    return 1;
                }
                g->player.last_dx = x - g->player.x;
                g->player.last_dy = y - g->player.y;
                g->player.x = x;
                g->player.y = y;
                g->mangroves.aboard = 0;
                break;
            }
        }
    } else if (!strcmp(hint, "BOARD SKIFF")) {
        g->player.x = g->mangroves.boat_x;
        g->player.y = g->mangroves.boat_y;
        g->player.last_dx = g->mangroves.boat_dx;
        g->player.last_dy = g->mangroves.boat_dy;
        g->mangroves.aboard = 1;
    } else {
        containers_interact(g);
    }
    map_mark_explored(&g->map, g->player.x, g->player.y);
    action_resolve_enemies(g);
    return 1;
}

static int valid_enemy_type(EnemyType type) {
    return type == ENEMY_GIANT_CRAB ||
        (type >= ENEMY_LIZARDFOLK_SCOUT && type <= ENEMY_MARSH_SPITTER) ||
        (type >= ENEMY_LIZARDFOLK_SCALEGUARD && type <= ENEMY_SSZARETH);
}

static int valid_record(const MangroveState *state) {
    if ((state->initialized != 0 && state->initialized != 1) || (state->aboard != 0 && state->aboard != 1)) {
        return 0;
    }
    if (!state->initialized) {
        return !state->aboard && !state->searched && !state->loot_count && !state->enemy_count;
    }
    if (state->stage < 1 || state->stage > MANGROVE_DEPTH || state->searched > 7 ||
        state->boat_dx < -1 || state->boat_dx > 1 || state->boat_dy < -1 || state->boat_dy > 1 ||
        abs(state->boat_dx) + abs(state->boat_dy) != 1 || !inside(state->boat_x, state->boat_y) ||
        state->loot_count < 0 || state->loot_count > MAX_FLOOR_ITEMS || state->enemy_count < 0 || state->enemy_count > AREA_ENEMY_LIMIT) {
        return 0;
    }
    Map map;
    mangrove_generate_level(&map, state->stage, state->searched);
    if (map.tiles[state->boat_y][state->boat_x] != TILE_MANGROVE_WATER) {
        return 0;
    }
    for (int i = 0; i < state->loot_count; i++) {
        const FloorItem *item = &state->loot[i];
        if (item->active && (!inside(item->x, item->y) || !map_is_walkable(&map, item->x, item->y))) {
            return 0;
        }
    }
    for (int i = 0; i < state->enemy_count; i++) {
        const Enemy *enemy = &state->enemies[i];
        if (enemy->active && (!inside(enemy->x, enemy->y) || !map_is_walkable(&map, enemy->x, enemy->y) ||
            !valid_enemy_type(enemy->type))) {
            return 0;
        }
    }
    return 1;
}

int mangrove_state_valid(const GameState *g) {
    if (!valid_record(&g->mangroves) || (g->mangrove_defeated != 0 && g->mangrove_defeated != 1)) {
        return 0;
    }
    for (int i = 0; i < MANGROVE_DEPTH; i++) {
        if (!valid_record(&g->mangrove_stages[i]) ||
            (g->mangrove_stages[i].initialized && g->mangrove_stages[i].stage != i + 1)) {
            return 0;
        }
    }
    if (g->portal_active && g->portal_location == LOCATION_MANGROVES) {
        if (g->portal_level < 1 || g->portal_level > MANGROVE_DEPTH) {
            return 0;
        }
        const MangroveState *state = &g->mangrove_stages[g->portal_level - 1];
        Map map;
        mangrove_generate_level(&map, g->portal_level, state->searched);
        if (!state->initialized || !inside(g->portal_x, g->portal_y) ||
            (state->aboard ? g->portal_x != state->boat_x || g->portal_y != state->boat_y : !map_is_walkable(&map, g->portal_x, g->portal_y))) {
            return 0;
        }
    }
    if (g->location == LOCATION_MANGROVES) {
        if (!g->mangroves.initialized || g->level != g->mangroves.stage || g->enemy_count < 0 || g->enemy_count > AREA_ENEMY_LIMIT ||
            g->floor_item_count < 0 || g->floor_item_count > MAX_FLOOR_ITEMS || !inside(g->player.x, g->player.y)) {
            return 0;
        }
        Map map;
        mangrove_generate_level(&map, g->level, g->mangroves.searched);
        for (int i = 0; i < g->enemy_count; i++) {
            const Enemy *enemy = &g->enemies[i];
            if (enemy->active && (!inside(enemy->x, enemy->y) || !map_is_walkable(&map, enemy->x, enemy->y) ||
                !valid_enemy_type(enemy->type))) {
                return 0;
            }
        }
        for (int y = 0; y < MANGROVE_H; y++) {
            for (int x = 0; x < MANGROVE_W; x++) {
                if (visible_tile(g, x, y) != map.tiles[y][x]) {
                    return 0;
                }
            }
        }
        return g->mangroves.aboard ? g->player.x == g->mangroves.boat_x && g->player.y == g->mangroves.boat_y :
            map_is_walkable(&map, g->player.x, g->player.y);
    }
    return 1;
}

static void nearest_ground(const Map *map, int *x, int *y, int afloat) {
    int nearest = MAP_W + MAP_H;
    int tx = *x;
    int ty = *y;
    for (int py = 1; py < MANGROVE_H - 1; py++) {
        for (int px = 1; px < MANGROVE_W - 1; px++) {
            int distance = abs(px - *x) + abs(py - *y);
            if (distance < nearest && (afloat ? map->tiles[py][px] == TILE_MANGROVE_WATER : map_is_walkable(map, px, py))) {
                nearest = distance;
                tx = px;
                ty = py;
            }
        }
    }
    *x = tx;
    *y = ty;
}

int mangrove_migrate_pilot(GameState *g) {
    if (g->location == LOCATION_MANGROVES && (g->level != 1 || g->enemy_count < 0 ||
        g->enemy_count > AREA_ENEMY_LIMIT || g->floor_item_count < 0 || g->floor_item_count > MAX_FLOOR_ITEMS)) {
        return 0;
    }
    if (!g->mangroves.initialized) {
        return 1;
    }
    g->mangroves.stage = 1;
    if (g->location == LOCATION_MANGROVES) {
        mangrove_store(g);
    }
    Map terrain;
    mangrove_generate_level(&terrain, 1, g->mangroves.searched);
    nearest_ground(&terrain, &g->mangroves.boat_x, &g->mangroves.boat_y, 1);
    for (int i = 0; i < g->mangroves.loot_count; i++) {
        FloorItem *item = &g->mangroves.loot[i];
        if (item->active) {
            nearest_ground(&terrain, &item->x, &item->y, 0);
            item->underlying_tile = terrain.tiles[item->y][item->x];
        }
    }
    if (g->portal_active && g->portal_location == LOCATION_MANGROVES) {
        if (g->mangroves.aboard) {
            g->portal_x = g->mangroves.boat_x;
            g->portal_y = g->mangroves.boat_y;
        } else {
            nearest_ground(&terrain, &g->portal_x, &g->portal_y, 0);
        }
    }
    g->mangrove_stages[0] = g->mangroves;
    if (g->location == LOCATION_MANGROVES) {
        int x = g->player.x;
        int y = g->player.y;
        nearest_ground(&terrain, &x, &y, g->mangroves.aboard);
        load_stage(g, 1);
        g->player.x = x;
        g->player.y = y;
        mangrove_spawn(g, 1);
        mangrove_store(g);
    } else {
        // Populate a saved pilot portal without replacing the active town map.
        GameState *scratch = calloc(1, sizeof(*scratch));
        if (!scratch) {
            return 0;
        }
        scratch->level = 1;
        scratch->player.x = g->portal_active && g->portal_location == LOCATION_MANGROVES ? g->portal_x : -1;
        scratch->player.y = g->portal_active && g->portal_location == LOCATION_MANGROVES ? g->portal_y : -1;
        mangrove_generate_level(&scratch->map, 1, g->mangroves.searched);
        mangrove_spawn(scratch, 1);
        g->mangroves.enemy_count = scratch->enemy_count;
        memcpy(g->mangroves.enemies, scratch->enemies, scratch->enemy_count * sizeof(Enemy));
        g->mangrove_stages[0] = g->mangroves;
        free(scratch);
    }
    return 1;
}

void mangrove_migrate_town(GameState *g) {
    if (g->location == LOCATION_DAWNMERE) {
        g->map.tiles[TOWN_H - 1][20] = TILE_TOWN_EXIT;
    }
    if (g->location == LOCATION_TOWN_ROAD && (g->road_town == LOCATION_DAWNMERE || g->road_town == LOCATION_TOWN2)) {
        int x;
        int y;
        town_road_entrance(TOWN_SIDE_SOUTH, &x, &y);
        for (int row = y; row < TOWN_ROAD_H; row++) {
            TileType tile = row == y ? TILE_TOWN_ROAD_ENTRY_SOUTH : row == TOWN_ROAD_H - 1 ? TILE_TOWN_ROAD_EXIT_SOUTH : TILE_TOWN_PATH;
            g->map.tiles[row][x] = tile;
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active && item->x == x && item->y == row) {
                    item->underlying_tile = tile;
                    g->map.tiles[row][x] = TILE_ITEM;
                }
            }
        }
    }
}
