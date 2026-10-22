#include "approach.h"
#include <string.h>

// Save files store these indices. Append new routes without reordering them.
const TownApproach town_approaches[] = {
    {LOCATION_TOWN, LOCATION_FOREST, 0, 12},
    {LOCATION_TOWN, LOCATION_DUNGEON, TOWN_W - 1, 12},
    {LOCATION_TOWN, LOCATION_MOUNTAINS, 20, 0},
    {LOCATION_TOWN, LOCATION_COAST, 20, TOWN_H - 1},
    {LOCATION_TOWN2, LOCATION_FOREST, TOWN_W - 1, 12},
    {LOCATION_TOWN2, LOCATION_SWAMP, 20, 0},
    {LOCATION_TOWN2, LOCATION_DESERT, 0, 12},
    {LOCATION_TOWN2, LOCATION_GLASSDEEP, STILLBURY_GLASSDEEP_GATE_X, TOWN_H - 1},
    {LOCATION_TOWN3, LOCATION_FROSTFELL, 20, 0},
    {LOCATION_TOWN3, LOCATION_SWAMP, 20, TOWN_H - 1},
    {LOCATION_TOWN3, LOCATION_MOONVEIL, 0, ROSEMOOR_MOONVEIL_GATE_Y},
    {LOCATION_TOWN4, LOCATION_MOUNTAINS, 20, TOWN_H - 1},
    {LOCATION_TOWN4, LOCATION_DRAGONSPINE, TOWN_W - 1, TOWN4_DRAGON_GATE_Y},
    {LOCATION_TOWN4, LOCATION_ASHEN, RIDGESHIRE_ASHEN_GATE_X, 0},
    {LOCATION_CASTLE, LOCATION_CATACOMBS, CROWNROAD_X, TOWN_H - 1}
};

const int town_approach_count = sizeof(town_approaches) / sizeof(town_approaches[0]);

const TownApproach *approach_route(const GameState *g) {
    if (g->location != LOCATION_APPROACH || g->approach_route < 0 || g->approach_route >= town_approach_count) {
        return NULL;
    }
    return &town_approaches[g->approach_route];
}

const char *approach_area_name(Location area) {
    switch (area) {
        case LOCATION_DUNGEON: return "DUNGEON";
        case LOCATION_FOREST: return "FOREST";
        case LOCATION_MOUNTAINS: return "GOBLIN MOUNTAINS";
        case LOCATION_COAST: return "SUNKEN COAST";
        case LOCATION_SWAMP: return "SWAMP";
        case LOCATION_DRAGONSPINE: return "DRAGONSPINE";
        case LOCATION_FROSTFELL: return "FROSTFELL";
        case LOCATION_DESERT: return "SUNSCAR WASTES";
        case LOCATION_MOONVEIL: return "MOONVEIL GARDENS";
        case LOCATION_ASHEN: return "ASHEN HOLLOW";
        case LOCATION_GLASSDEEP: return "GLASSDEEP CAVERNS";
        case LOCATION_CATACOMBS: return "ROYAL CATACOMBS";
        default: return "AREA";
    }
}

const char *approach_town_name(Location town) {
    switch (town) {
        case LOCATION_TOWN2: return "STILLBURY";
        case LOCATION_TOWN3: return "ROSEMOOR";
        case LOCATION_TOWN4: return "RIDGESHIRE";
        case LOCATION_CASTLE: return "CASTLE GROUNDS";
        default: return "OAKHAVEN";
    }
}

TileType approach_floor(Location area) {
    switch (area) {
        case LOCATION_DUNGEON: return TILE_TOWN_PATH;
        case LOCATION_MOUNTAINS: return TILE_MOUNTAIN_FLOOR;
        case LOCATION_COAST: return TILE_COAST_FLOOR;
        case LOCATION_SWAMP: return TILE_SWAMP_FLOOR;
        case LOCATION_DRAGONSPINE: return TILE_DRAGON_FLOOR;
        case LOCATION_FROSTFELL: return TILE_FROST_FLOOR;
        case LOCATION_DESERT: return TILE_DESERT_FLOOR;
        case LOCATION_MOONVEIL: return TILE_MOONVEIL_FLOOR;
        case LOCATION_ASHEN: return TILE_ASHEN_FLOOR;
        case LOCATION_GLASSDEEP: return TILE_GLASSDEEP_FLOOR;
        default: return TILE_FOREST_FLOOR;
    }
}

static TileType approach_wall(Location area) {
    switch (area) {
        case LOCATION_DUNGEON: return TILE_WALL;
        case LOCATION_MOUNTAINS: return TILE_MOUNTAIN_WALL;
        case LOCATION_COAST: return TILE_COAST_WALL;
        case LOCATION_SWAMP: return TILE_SWAMP_WALL;
        case LOCATION_DRAGONSPINE: return TILE_DRAGON_WALL;
        case LOCATION_FROSTFELL: return TILE_FROST_WALL;
        case LOCATION_DESERT: return TILE_DESERT_WALL;
        case LOCATION_MOONVEIL: return TILE_MOONVEIL_WALL;
        case LOCATION_ASHEN: return TILE_ASHEN_WALL;
        case LOCATION_GLASSDEEP: return TILE_GLASSDEEP_WALL;
        default: return TILE_FOREST_WALL;
    }
}

void approach_refresh_dungeon(GameState *g) {
    const TownApproach *route = approach_route(g);
    if (!route || route->area != LOCATION_DUNGEON) {
        return;
    }
    int tx = g->map.stairs_up_x;
    int ty = g->map.stairs_up_y;
    int dx = (g->map.stairs_down_x > tx) - (g->map.stairs_down_x < tx);
    int dy = (g->map.stairs_down_y > ty) - (g->map.stairs_down_y < ty);
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g->map.tiles[y][x] = x > 0 && x < APPROACH_W - 1 && y > 0 && y < APPROACH_H - 1 ? TILE_TOWN_FLOOR : TILE_WALL;
        }
    }
    for (int step = 0; step <= APPROACH_LENGTH; step++) {
        int radius = step < APPROACH_LENGTH - 3 ? 1 : 0;
        for (int side = -radius; side <= radius; side++) {
            g->map.tiles[ty + step * dy + side * dx][tx + step * dx - side * dy] = TILE_TOWN_PATH;
        }
    }
    // Match the crypt's three-tile facade, keeping the west approach open.
    for (int row = -2; row <= 0; row++) {
        for (int side = -1; side <= 1; side++) {
            if (row < 0 || side > 0) {
                int x = g->map.stairs_down_x + side;
                int y = g->map.stairs_down_y + row;
                g->map.tiles[y][x] = TILE_WALL;
            }
        }
    }
    g->map.tiles[ty][tx] = TILE_APPROACH_TOWN;
    g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] = TILE_APPROACH_AREA;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active) {
            item->underlying_tile = g->map.tiles[item->y][item->x];
        }
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active) {
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        }
    }
}

int approach_enter(GameState *g, int x, int y) {
    if (x < 0 || x >= TOWN_W || y < 0 || y >= TOWN_H) {
        return 0;
    }
    int index = -1;
    for (int i = 0; i < town_approach_count; i++) {
        const TownApproach *route = &town_approaches[i];
        int gate = (route->gate_x == 0 && x == 0) || (route->gate_x == TOWN_W - 1 && x == TOWN_W - 1) ||
            (route->gate_y == 0 && y == 0) || (route->gate_y == TOWN_H - 1 && y == TOWN_H - 1);
        if (g->location == route->town && gate) {
            index = i;
            break;
        }
    }
    if (index < 0 || g->map.tiles[y][x] != TILE_TOWN_EXIT) {
        return 0;
    }
    const TownApproach *route = &town_approaches[index];
    int dx = x == 0 ? -1 : x == TOWN_W - 1 ? 1 : 0;
    int dy = y == 0 ? -1 : y == TOWN_H - 1 ? 1 : 0;
    int tx = dx < 0 ? APPROACH_W - 1 : dx > 0 ? 0 : APPROACH_W / 2;
    int ty = dy < 0 ? APPROACH_H - 1 : dy > 0 ? 0 : APPROACH_H / 2;
    TileType floor = approach_floor(route->area);
    TileType wall = approach_wall(route->area);
    memset(&g->map, 0, sizeof(g->map));
    for (int my = 0; my < MAP_H; my++) {
        for (int mx = 0; mx < MAP_W; mx++) {
            g->map.tiles[my][mx] = wall;
        }
    }
    for (int step = 0; step <= APPROACH_LENGTH; step++) {
        int radius = step < APPROACH_LENGTH - 3 ? 1 : 0;
        for (int side = -radius; side <= radius; side++) {
            g->map.tiles[ty + step * dy + side * dx][tx + step * dx - side * dy] = floor;
        }
    }
    g->map.stairs_up_x = tx;
    g->map.stairs_up_y = ty;
    g->map.stairs_down_x = tx + APPROACH_LENGTH * dx;
    g->map.stairs_down_y = ty + APPROACH_LENGTH * dy;
    g->map.tiles[ty][tx] = TILE_APPROACH_TOWN;
    g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] = TILE_APPROACH_AREA;
    g->location = LOCATION_APPROACH;
    g->approach_route = index;
    g->player.x = tx + dx;
    g->player.y = ty + dy;
    g->player.last_dx = dx;
    g->player.last_dy = dy;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    approach_refresh_dungeon(g);
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    g->player.frozen_turns = 0;
    g->player.freeze_recovery = 0;
    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message), "Follow the path to %s. Turn back for %s.", approach_area_name(route->area), approach_town_name(route->town));
    push_message(g, message);
    return 1;
}

int approach_travel(GameState *g, int x, int y) {
    const TownApproach *route = approach_route(g);
    if (!route) {
        return 0;
    }
    if (g->map.tiles[y][x] == TILE_APPROACH_TOWN) {
        game_leave_approach(g);
        return 1;
    }
    if (g->map.tiles[y][x] != TILE_APPROACH_AREA) {
        return 0;
    }
    Location area = route->area;
    g->location = route->town;
    g->approach_route = -1;
    switch (area) {
        case LOCATION_DUNGEON: game_enter_dungeon(g); break;
        case LOCATION_FOREST: game_enter_forest(g); break;
        case LOCATION_MOUNTAINS: game_enter_mountains(g); break;
        case LOCATION_COAST: game_enter_coast(g); break;
        case LOCATION_SWAMP: game_enter_swamp(g); break;
        case LOCATION_DRAGONSPINE: game_enter_dragonspine(g); break;
        case LOCATION_FROSTFELL: game_enter_frostfell(g); break;
        case LOCATION_DESERT: game_enter_desert(g); break;
        case LOCATION_MOONVEIL: game_enter_moonveil(g); break;
        case LOCATION_ASHEN: game_enter_ashen(g); break;
        case LOCATION_GLASSDEEP: game_enter_glassdeep(g); break;
        case LOCATION_CATACOMBS: game_enter_catacombs(g); break;
        default: return 0;
    }
    return 1;
}
