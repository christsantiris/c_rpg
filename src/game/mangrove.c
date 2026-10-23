#include "mangrove.h"
#include "town_road.h"
#include <stdlib.h>
#include <string.h>

const int mangrove_landings[4][2] = {{14, 14}, {32, 11}, {23, 24}, {51, 25}};
const int mangrove_caches[3][2] = {{44, 9}, {25, 31}, {52, 31}};

static int inside(int x, int y) {
    return x > 0 && x < MANGROVE_W - 1 && y > 0 && y < MANGROVE_H - 1;
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

void mangrove_generate(Map *map, unsigned int searched) {
    memset(map, 0, sizeof(*map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            map->tiles[y][x] = TILE_MANGROVE_WATER;
        }
    }
    const int islands[4][4] = {{14, 8, 9, 6}, {43, 11, 11, 6}, {23, 30, 11, 6}, {51, 30, 7, 5}};
    for (int i = 0; i < 4; i++) {
        int cx = islands[i][0];
        int cy = islands[i][1];
        int rx = islands[i][2];
        int ry = islands[i][3];
        for (int y = cy - ry; y <= cy + ry; y++) {
            for (int x = cx - rx; x <= cx + rx; x++) {
                int dx = x - cx;
                int dy = y - cy;
                int edge = dx * dx * ry * ry + dy * dy * rx * rx;
                if (edge <= rx * rx * ry * ry) {
                    map->tiles[y][x] = TILE_MANGROVE_MUD;
                }
            }
        }
        map->tiles[mangrove_landings[i][1]][mangrove_landings[i][0]] = TILE_MANGROVE_LANDING;
        map->tiles[cy - 2][cx - 3] = TILE_MANGROVE_ROOT;
        map->tiles[cy + 2][cx + 3] = TILE_MANGROVE_ROOT;
        for (int y = cy - ry + 2; y <= cy + ry - 2; y += 3) {
            for (int x = cx - rx + 2; x <= cx + rx - 2; x += 3) {
                if (abs(x - cx) > 2 && abs(y - cy) > 1 && map->tiles[y][x] == TILE_MANGROVE_MUD &&
                    map->tiles[y - 1][x] == TILE_MANGROVE_MUD && map->tiles[y + 1][x] == TILE_MANGROVE_MUD &&
                    map->tiles[y][x - 1] == TILE_MANGROVE_MUD && map->tiles[y][x + 1] == TILE_MANGROVE_MUD) {
                    map->tiles[y][x] = TILE_MANGROVE_ROOT;
                }
            }
        }
    }
    const int rootbeds[4][2] = {{7, 22}, {33, 19}, {44, 23}, {59, 17}};
    for (int i = 0; i < 4; i++) {
        for (int dy = 0; dy < 2; dy++) {
            for (int dx = 0; dx < 2; dx++) {
                map->tiles[rootbeds[i][1] + dy][rootbeds[i][0] + dx] = TILE_MANGROVE_ROOT;
            }
        }
    }
    for (int y = 0; y < 3; y++) {
        map->tiles[y][14] = TILE_MANGROVE_MUD;
    }
    map->tiles[0][14] = TILE_MANGROVE_RETURN;
    for (int i = 0; i < 3; i++) {
        if (!(searched & (1u << i))) {
            map->tiles[mangrove_caches[i][1]][mangrove_caches[i][0]] = TILE_MANGROVE_CACHE;
        }
    }
    map->stairs_up_x = 14;
    map->stairs_up_y = 0;
}

void mangrove_capture(const GameState *g, MangroveState *state) {
    if (g->location != LOCATION_MANGROVES) {
        return;
    }
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
}

void mangrove_store(GameState *g) {
    mangrove_capture(g, &g->mangroves);
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
    push_message(g, "Saltwind Mangroves lies south. Dawnmere is back to the north.");
}

void mangrove_enter(GameState *g, int portal) {
    MangroveState *state = &g->mangroves;
    if (!state->initialized) {
        *state = (MangroveState){.initialized = 1};
    }
    if (!portal) {
        state->boat_x = 14;
        state->boat_y = 15;
        state->boat_dx = 0;
        state->boat_dy = 1;
        state->aboard = 0;
        if (g->portal_location == LOCATION_MANGROVES) {
            g->portal_active = 0;
        }
    }
    g->location = LOCATION_MANGROVES;
    g->level = 1;
    g->level_cleared = 1;
    g->approach_route = -1;
    g->enemy_count = 0;
    g->dialogue_active = 0;
    mangrove_generate(&g->map, state->searched);
    for (int y = 0; y < MANGROVE_H; y++) {
        for (int x = 0; x < MANGROVE_W; x++) {
            int bit = y * MANGROVE_W + x;
            if (state->explored[bit / 8] & (1u << (bit % 8))) {
                g->map.explored[(y * MAP_W + x) / 8] |= 1u << ((y * MAP_W + x) % 8);
            }
        }
    }
    g->floor_item_count = state->loot_count;
    memcpy(g->floor_items, state->loot, sizeof(g->floor_items));
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active) {
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        }
    }
    g->player.x = portal ? g->portal_x : 14;
    g->player.y = portal ? g->portal_y : 1;
    if (portal) {
        g->portal_active = 0;
    }
    g->player.last_dx = state->aboard ? state->boat_dx : 0;
    g->player.last_dy = state->aboard ? state->boat_dy : 1;
    map_mark_explored(&g->map, g->player.x, g->player.y);
    push_message(g, "Explore the channels by skiff. Moor at timber landings to go ashore.");
}

void mangrove_leave(GameState *g) {
    mangrove_store(g);
    mangrove_path(g, 1);
}

int mangrove_travel(GameState *g, int x, int y) {
    if (g->location == LOCATION_MANGROVE_PATH) {
        if (x == MANGROVE_GATE_X && y == 0) {
            town_road_enter(g, LOCATION_DAWNMERE, TOWN_SIDE_SOUTH, 0);
            return 1;
        }
        if (x >= MANGROVE_GATE_X - 3 && x <= MANGROVE_GATE_X + 3 &&
            y >= MANGROVE_GATE_Y - 3 && y <= MANGROVE_GATE_Y) {
            mangrove_enter(g, 0);
            return 1;
        }
        return 0;
    }
    if (g->location != LOCATION_MANGROVES) {
        return 0;
    }
    if (g->mangroves.aboard) {
        if (inside(x, y) && abs(x - g->player.x) + abs(y - g->player.y) == 1 &&
            g->map.tiles[y][x] == TILE_MANGROVE_WATER) {
            g->mangroves.boat_dx = x - g->player.x;
            g->mangroves.boat_dy = y - g->player.y;
            g->player.last_dx = g->mangroves.boat_dx;
            g->player.last_dy = g->mangroves.boat_dy;
            g->mangroves.boat_x = g->player.x = x;
            g->mangroves.boat_y = g->player.y = y;
            map_mark_explored(&g->map, x, y);
            action_resolve_enemies(g);
        }
        return 1;
    }
    if (x == 14 && y == 0) {
        mangrove_leave(g);
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
            if (abs(g->player.x - mangrove_landings[i][0]) + abs(g->player.y - mangrove_landings[i][1]) == 1) {
                return "GO ASHORE";
            }
        }
        return NULL;
    }
    if (visible_tile(g, g->player.x, g->player.y) == TILE_MANGROVE_LANDING &&
        abs(g->player.x - g->mangroves.boat_x) + abs(g->player.y - g->mangroves.boat_y) == 1) {
        return "BOARD SKIFF";
    }
    for (int i = 0; i < 3; i++) {
        if (!(g->mangroves.searched & (1u << i)) &&
            abs(g->player.x - mangrove_caches[i][0]) + abs(g->player.y - mangrove_caches[i][1]) <= 1) {
            return "SEARCH SUPPLIES";
        }
    }
    return NULL;
}

int mangrove_interact(GameState *g) {
    const char *hint = mangrove_hint(g);
    if (!hint) {
        return 0;
    }
    if (g->mangroves.aboard) {
        for (int i = 0; i < 4; i++) {
            if (abs(g->player.x - mangrove_landings[i][0]) + abs(g->player.y - mangrove_landings[i][1]) == 1) {
                g->player.x = mangrove_landings[i][0];
                g->player.y = mangrove_landings[i][1];
                g->mangroves.aboard = 0;
                break;
            }
        }
    } else if (!strcmp(hint, "BOARD SKIFF")) {
        g->player.x = g->mangroves.boat_x;
        g->player.y = g->mangroves.boat_y;
        g->mangroves.aboard = 1;
    } else {
        for (int i = 0; i < 3; i++) {
            if (!(g->mangroves.searched & (1u << i)) &&
                abs(g->player.x - mangrove_caches[i][0]) + abs(g->player.y - mangrove_caches[i][1]) <= 1) {
                g->mangroves.searched |= 1u << i;
                g->gold += 15;
                g->map.tiles[mangrove_caches[i][1]][mangrove_caches[i][0]] = TILE_MANGROVE_MUD;
                push_message(g, "You find 15 gold beneath the weathered supplies.");
                break;
            }
        }
    }
    map_mark_explored(&g->map, g->player.x, g->player.y);
    action_resolve_enemies(g);
    return 1;
}

int mangrove_state_valid(const GameState *g) {
    const MangroveState *state = &g->mangroves;
    if (!state->initialized) {
        return !state->aboard && !state->searched && !state->loot_count && g->location != LOCATION_MANGROVES &&
            !(g->portal_active && g->portal_location == LOCATION_MANGROVES);
    }
    Map map;
    mangrove_generate(&map, state->searched);
    if (!inside(state->boat_x, state->boat_y) || map.tiles[state->boat_y][state->boat_x] != TILE_MANGROVE_WATER ||
        state->boat_dx < -1 || state->boat_dx > 1 || state->boat_dy < -1 || state->boat_dy > 1 ||
        abs(state->boat_dx) + abs(state->boat_dy) != 1 || state->searched > 7 ||
        state->loot_count < 0 || state->loot_count > MAX_FLOOR_ITEMS) {
        return 0;
    }
    if (g->portal_active && g->portal_location == LOCATION_MANGROVES &&
        (g->portal_level != 1 || !inside(g->portal_x, g->portal_y) ||
        (state->aboard ? g->portal_x != state->boat_x || g->portal_y != state->boat_y :
        !map_is_walkable(&map, g->portal_x, g->portal_y)))) {
        return 0;
    }
    for (int i = 0; i < state->loot_count; i++) {
        const FloorItem *item = &state->loot[i];
        if (item->active && (!inside(item->x, item->y) || !map_is_walkable(&map, item->x, item->y))) {
            return 0;
        }
    }
    if (g->location == LOCATION_MANGROVES) {
        for (int y = 0; y < MANGROVE_H; y++) {
            for (int x = 0; x < MANGROVE_W; x++) {
                TileType tile = visible_tile(g, x, y);
                if (tile != map.tiles[y][x] && !(tile == TILE_PORTAL && g->portal_active &&
                    g->portal_location == LOCATION_MANGROVES && x == g->portal_x && y == g->portal_y)) {
                    return 0;
                }
            }
        }
        return g->level == 1 && inside(g->player.x, g->player.y) &&
            (state->aboard ? g->player.x == state->boat_x && g->player.y == state->boat_y :
            map_is_walkable(&map, g->player.x, g->player.y));
    }
    return 1;
}

void mangrove_migrate_town(GameState *g) {
    if (g->location == LOCATION_DAWNMERE) {
        g->map.tiles[TOWN_H - 1][20] = TILE_TOWN_EXIT;
    } else if (g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_DAWNMERE) {
        int x = TOWN_ROAD_W / 2;
        for (int y = TOWN_SPRITE_Y + TOWN_SPRITE_H; y < TOWN_ROAD_H; y++) {
            g->map.tiles[y][x] = TILE_TOWN_PATH;
        }
        g->map.tiles[TOWN_SPRITE_Y + TOWN_SPRITE_H - 1][x] = TILE_TOWN_ROAD_ENTRY_SOUTH;
        g->map.tiles[TOWN_ROAD_H - 1][x] = TILE_TOWN_ROAD_EXIT_SOUTH;
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == x && item->y >= TOWN_SPRITE_Y + TOWN_SPRITE_H) {
                item->underlying_tile = item->y == TOWN_ROAD_H - 1 ? TILE_TOWN_ROAD_EXIT_SOUTH : TILE_TOWN_PATH;
                g->map.tiles[item->y][x] = TILE_ITEM;
            }
        }
    }
}
