#include "town_road.h"
#include "mangrove.h"
#include "resurrection.h"
#include <SDL2/SDL.h>
#include <string.h>

static const int dx[4] = {0, 1, 0, -1};
static const int dy[4] = {-1, 0, 1, 0};

int town_road_index(Location town) {
    const Location towns[TOWN_ROAD_COUNT] = {LOCATION_TOWN, LOCATION_TOWN2, LOCATION_TOWN3, LOCATION_TOWN4, LOCATION_DAWNMERE};
    for (int i = 0; i < TOWN_ROAD_COUNT; i++) {
        if (towns[i] == town) {
            return i;
        }
    }
    return -1;
}

TownSide town_road_side(int x, int y) {
    return y == 0 ? TOWN_SIDE_NORTH : x == TOWN_W - 1 ? TOWN_SIDE_EAST : y == TOWN_H - 1 ? TOWN_SIDE_SOUTH : TOWN_SIDE_WEST;
}

int town_road_connection(Location town, TownSide side, TownApproach *connection) {
    if (town == LOCATION_TOWN && side == TOWN_SIDE_NORTH) {
        // A local riverbank visit, with no connection across the river.
        *connection = (TownApproach){town, LOCATION_TOWN, 20, 0};
        return 1;
    }
    if (town == LOCATION_DAWNMERE && side == TOWN_SIDE_SOUTH) {
        *connection = (TownApproach){town, LOCATION_MANGROVES, 20, TOWN_H - 1};
        return 1;
    }
    for (int i = 0; i < town_approach_count; i++) {
        const TownApproach *route = &town_approaches[i];
        if (!route->unplaced && route->town == town && town_road_side(route->gate_x, route->gate_y) == side) {
            *connection = *route;
            return 1;
        }
    }
    if (town == LOCATION_TOWN3 && side == TOWN_SIDE_EAST) {
        *connection = (TownApproach){town, LOCATION_CROWNROAD, TOWN_W - 1, TOWN3_CROWN_GATE_Y};
        return 1;
    }
    if (town == LOCATION_TOWN4 && side == TOWN_SIDE_WEST) {
        *connection = (TownApproach){town, LOCATION_CROWNROAD_WEST, 0, TOWN4_CROWN_GATE_Y};
        return 1;
    }
    return 0;
}

void town_road_entrance(TownSide side, int *x, int *y) {
    *x = TOWN_ROAD_W / 2 + dx[side] * (TOWN_SPRITE_W / 2);
    *y = TOWN_ROAD_H / 2 + dy[side] * (TOWN_SPRITE_H / 2);
}

void town_road_exit(TownSide side, int *x, int *y) {
    *x = dx[side] < 0 ? 0 : dx[side] > 0 ? TOWN_ROAD_W - 1 : TOWN_ROAD_W / 2;
    *y = dy[side] < 0 ? 0 : dy[side] > 0 ? TOWN_ROAD_H - 1 : TOWN_ROAD_H / 2;
}

static void generate_town_road(GameState *g) {
    memset(&g->map, 0, sizeof(g->map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            int inside = x > 0 && x < TOWN_ROAD_W - 1 && y > 0 && y < TOWN_ROAD_H - 1;
            g->map.tiles[y][x] = inside ? TILE_TOWN_FLOOR : TILE_WALL;
        }
    }
    // Grass surrounds the town; walk across it to reach another road.
    for (int y = TOWN_SPRITE_Y; y < TOWN_SPRITE_Y + TOWN_SPRITE_H; y++) {
        for (int x = TOWN_SPRITE_X; x < TOWN_SPRITE_X + TOWN_SPRITE_W; x++) {
            g->map.tiles[y][x] = TILE_WALL;
        }
    }
    if (g->road_town == LOCATION_TOWN) {
        // The river spans the whole northern boundary; no bridge or ford exists here.
        for (int y = 0; y <= OAKHAVEN_RIVER_EDGE; y++) {
            for (int x = 0; x < TOWN_ROAD_W; x++) {
                g->map.tiles[y][x] = TILE_COAST_DEEP_WATER;
            }
        }
    }
    for (int side = 0; side < 4; side++) {
        TownApproach connection;
        if (!town_road_connection(g->road_town, side, &connection)) {
            continue;
        }
        int x;
        int y;
        town_road_entrance(side, &x, &y);
        g->map.tiles[y][x] = TILE_TOWN_ROAD_ENTRY_NORTH + side;
        if (g->road_town == LOCATION_TOWN && side == TOWN_SIDE_NORTH) {
            for (int bank_y = y - 1; bank_y >= OAKHAVEN_FERRY_Y; bank_y--) {
                g->map.tiles[bank_y][x] = TILE_TOWN_PATH;
            }
            g->map.tiles[OAKHAVEN_FERRY_Y][x] = TILE_BROKEN_FERRY;
            continue;
        }
        int ex;
        int ey;
        town_road_exit(side, &ex, &ey);
        while (x != ex || y != ey) {
            x += dx[side];
            y += dy[side];
            g->map.tiles[y][x] = TILE_TOWN_PATH;
        }
        g->map.tiles[ey][ex] = TILE_TOWN_ROAD_EXIT_NORTH + side;
    }
    resurrection_place(g);
}

int town_road_enter(GameState *g, Location town, TownSide side, int departing) {
    TownApproach connection;
    if (town_road_index(town) < 0 || side < 0 || side > 3 || !town_road_connection(town, side, &connection)) {
        return 0;
    }
    g->road_town = town;
    g->location = LOCATION_TOWN_ROAD;
    g->approach_route = -1;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    g->player.frozen_turns = 0;
    g->player.freeze_recovery = 0;
    generate_town_road(g);
    if (departing) {
        town_road_entrance(side, &g->player.x, &g->player.y);
        g->player.x += dx[side];
        g->player.y += dy[side];
    } else if (town == LOCATION_TOWN && side == TOWN_SIDE_NORTH) {
        // A local return lands on dry bank, never in the boundary's white water.
        g->player.x = OAKHAVEN_FERRY_X;
        g->player.y = OAKHAVEN_FERRY_Y;
    } else {
        town_road_exit(side, &g->player.x, &g->player.y);
        g->player.x -= dx[side];
        g->player.y -= dy[side];
    }
    g->player.last_dx = departing ? dx[side] : -dx[side];
    g->player.last_dy = departing ? dy[side] : -dy[side];
    map_mark_explored(&g->map, g->player.x, g->player.y);
    if (town == LOCATION_TOWN && side == TOWN_SIDE_NORTH) {
        int x;
        int y;
        town_road_entrance(side, &x, &y);
        map_mark_explored(&g->map, x, y);
    }
    char message[MAX_MESSAGE_LEN];
    if (town == LOCATION_TOWN && side == TOWN_SIDE_NORTH) {
        snprintf(message, sizeof(message), "Soulstone Shrine lies west of the path. North: broken ferry, no crossing. South: Oakhaven.");
    } else {
        snprintf(message, sizeof(message), "%s: approach the town to enter, or walk around it to another road.", approach_town_name(town));
    }
    push_message(g, message);
    return 1;
}

void town_road_migrate(GameState *g) {
    // Saves before version 221 paved a ring around the town; rebuild the exterior without it.
    if (g->location != LOCATION_TOWN_ROAD || town_road_index(g->road_town) < 0) {
        return;
    }
    unsigned char explored[sizeof(g->map.explored)];
    memcpy(explored, g->map.explored, sizeof(explored));
    generate_town_road(g);
    memcpy(g->map.explored, explored, sizeof(explored));
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && map_is_walkable(&g->map, item->x, item->y)) {
            item->underlying_tile = g->map.tiles[item->y][item->x];
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        }
    }
}

int town_road_depart(GameState *g, int x, int y) {
    if (town_road_index(g->location) < 0 || g->map.tiles[y][x] != TILE_TOWN_EXIT ||
        (x != 0 && x != TOWN_W - 1 && y != 0 && y != TOWN_H - 1)) {
        return 0;
    }
    return town_road_enter(g, g->location, town_road_side(x, y), 1);
}

int town_road_travel(GameState *g, int x, int y) {
    if (g->location != LOCATION_TOWN_ROAD) {
        return 0;
    }
    TileType tile = g->map.tiles[y][x];
    if (g->road_town == LOCATION_TOWN) {
        if (tile == TILE_COAST_DEEP_WATER && y <= OAKHAVEN_RIVER_EDGE) {
            push_message(g, "The rapids are impassable. The ferry is broken; there is no crossing here.");
            return 1;
        }
        if (tile == TILE_BROKEN_FERRY) {
            map_mark_explored(&g->map, x, y);
            push_message(g, "Broken Ferry: snapped rope, shattered hull. Return south to Oakhaven.");
        }
        if (tile == TILE_SOULSTONE_RITUAL) {
            char message[MAX_MESSAGE_LEN];
            snprintf(message, sizeof(message), "Ritual circle: equip Resurrection and press %s. Talk to the warlock for paid resurrection.",
                SDL_GetScancodeName(g->key_bindings[CONTROL_CAST_SPELL]));
            push_message(g, message);
        }
    }
    int entry = tile >= TILE_TOWN_ROAD_ENTRY_NORTH && tile <= TILE_TOWN_ROAD_ENTRY_WEST;
    int exit = tile >= TILE_TOWN_ROAD_EXIT_NORTH && tile <= TILE_TOWN_ROAD_EXIT_WEST;
    // Touching a connected side enters the sprite. Center markers retain
    // saved entrance coordinates; they are not exterior gates.
    if (!entry && x >= TOWN_SPRITE_X && x < TOWN_SPRITE_X + TOWN_SPRITE_W &&
        y >= TOWN_SPRITE_Y && y < TOWN_SPRITE_Y + TOWN_SPRITE_H) {
        TownSide side = g->player.y < TOWN_SPRITE_Y ? TOWN_SIDE_NORTH :
            g->player.y >= TOWN_SPRITE_Y + TOWN_SPRITE_H ? TOWN_SIDE_SOUTH :
            g->player.x < TOWN_SPRITE_X ? TOWN_SIDE_WEST : TOWN_SIDE_EAST;
        TownApproach connection;
        if (town_road_connection(g->road_town, side, &connection)) {
            game_enter_town_gate(g, connection.town, connection.gate_x, connection.gate_y);
            return 1;
        }
    }
    if (!entry && !exit) {
        return 0;
    }
    TownSide side = tile - (entry ? TILE_TOWN_ROAD_ENTRY_NORTH : TILE_TOWN_ROAD_EXIT_NORTH);
    TownApproach connection;
    if (!town_road_connection(g->road_town, side, &connection)) {
        return 0;
    }
    if (entry) {
        game_enter_town_gate(g, connection.town, connection.gate_x, connection.gate_y);
    } else if (connection.area == LOCATION_MANGROVES) {
        mangrove_path(g, 0);
    } else if (connection.area == LOCATION_CROWNROAD || connection.area == LOCATION_CROWNROAD_WEST) {
        game_enter_crownroad(g, connection.area, 0);
    } else {
        for (int i = 0; i < town_approach_count; i++) {
            if (town_approaches[i].town == connection.town && town_approaches[i].area == connection.area) {
                approach_enter_region(g, i);
                break;
            }
        }
    }
    return 1;
}

static void preserve_marker(GameState *g, int x, int y, TileType tile) {
    g->map.tiles[y][x] = tile;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            item->underlying_tile = tile;
            if (!map_is_travel_tile(tile) && tile != TILE_TOWN_EXIT) {
                g->map.tiles[y][x] = TILE_ITEM;
            }
        }
    }
}

// Refresh just the new local route in saved Oakhaven maps, retaining other tiles and exploration.
void town_road_refresh_oakhaven(GameState *g) {
    if (g->location == LOCATION_TOWN) {
        for (int x = 18; x <= 22; x++) {
            preserve_marker(g, x, 0, TILE_TOWN_EXIT);
        }
    } else if (g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN) {
        int x;
        int y;
        town_road_entrance(TOWN_SIDE_NORTH, &x, &y);
        preserve_marker(g, x, y, TILE_TOWN_ROAD_ENTRY_NORTH);
        for (int bank_y = y - 1; bank_y >= OAKHAVEN_FERRY_Y; bank_y--) {
            preserve_marker(g, x, bank_y, bank_y == OAKHAVEN_FERRY_Y ? TILE_BROKEN_FERRY : TILE_TOWN_PATH);
        }
        resurrection_place(g);
    }
}
