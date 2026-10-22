#include "approach.h"
#include <string.h>

// Save files store these indices. Append new routes without reordering them.
const TownApproach town_approaches[] = {
    {LOCATION_TOWN, LOCATION_FOREST, 0, 12},
    {LOCATION_TOWN, LOCATION_DUNGEON, TOWN_W - 1, 12},
    // Route 2 moved from Oakhaven to Dawnmere; version 171 migrates old saves.
    {LOCATION_DAWNMERE, LOCATION_MOUNTAINS, 20, 0},
    {LOCATION_TOWN, LOCATION_COAST, 20, TOWN_H - 1},
    {LOCATION_TOWN2, LOCATION_FOREST, TOWN_W - 1, 12},
    {LOCATION_TOWN2, LOCATION_SWAMP, 20, 0},
    {LOCATION_TOWN2, LOCATION_DESERT, 0, 12},
    {LOCATION_TOWN2, LOCATION_GLASSDEEP, STILLBURY_GLASSDEEP_GATE_X, TOWN_H - 1},
    {LOCATION_TOWN3, LOCATION_FROSTFELL, 20, 0},
    {LOCATION_TOWN3, LOCATION_SWAMP, 20, TOWN_H - 1},
    {LOCATION_TOWN3, LOCATION_MOONVEIL, 0, ROSEMOOR_MOONVEIL_GATE_Y},
    {LOCATION_TOWN4, LOCATION_MOUNTAINS, 20, TOWN_H - 1},
    {LOCATION_TOWN4, LOCATION_DRAGONSPINE, RIDGESHIRE_DRAGON_GATE_X, 0},
    {LOCATION_TOWN4, LOCATION_ASHEN, TOWN_W - 1, RIDGESHIRE_ASHEN_GATE_Y},
    {LOCATION_CASTLE, LOCATION_CATACOMBS, CROWNROAD_X, TOWN_H - 1},
    {LOCATION_DAWNMERE, LOCATION_DUNGEON, 0, 12}
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
        case LOCATION_DAWNMERE: return "DAWNMERE";
        case LOCATION_TOWN2: return "STILLBURY";
        case LOCATION_TOWN3: return "ROSEMOOR";
        case LOCATION_TOWN4: return "RIDGESHIRE";
        case LOCATION_CASTLE: return "CASTLE GROUNDS";
        default: return "OAKHAVEN";
    }
}

TileType approach_floor(Location area) {
    if (approach_entrance_rows(area)) {
        return TILE_TOWN_PATH;
    }
    switch (area) {
        case LOCATION_DUNGEON: return TILE_TOWN_PATH;
        case LOCATION_FOREST: return TILE_TOWN_PATH;
        case LOCATION_MOUNTAINS: return TILE_TOWN_PATH;
        case LOCATION_COAST: return TILE_COAST_FLOOR;
        case LOCATION_SWAMP: return TILE_TOWN_PATH;
        case LOCATION_DRAGONSPINE: return TILE_DRAGON_FLOOR;
        case LOCATION_FROSTFELL: return TILE_FROST_FLOOR;
        case LOCATION_DESERT: return TILE_TOWN_PATH;
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
    int dx = route->gate_x == 0 ? -1 : 1;
    g->map.stairs_down_x = tx + APPROACH_LENGTH * dx;
    g->map.stairs_down_y = ty - 2;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g->map.tiles[y][x] = x > 0 && x < APPROACH_W - 1 && y > 0 && y < APPROACH_H - 1 ? TILE_TOWN_FLOOR : TILE_WALL;
        }
    }
    int other_x;
    int other_y;
    int road_open = approach_dungeon_town_exit(g, &other_x, &other_y);
    if (!road_open) {
        for (int step = 0; step <= APPROACH_LENGTH; step++) {
            g->map.tiles[ty][tx + step * dx] = TILE_TOWN_PATH;
        }
    }
    // The crypt faces south, above the straight road between the town gates.
    for (int row = -2; row <= 0; row++) {
        for (int side = -1; side <= 1; side++) {
            if (row < 0 || side != 0) {
                int x = g->map.stairs_down_x + side;
                int y = g->map.stairs_down_y + row;
                g->map.tiles[y][x] = TILE_WALL;
            }
        }
    }
    if (road_open) {
        for (int x = 1; x < APPROACH_W - 1; x++) {
            g->map.tiles[ty][x] = TILE_TOWN_PATH;
        }
        g->map.tiles[other_y][other_x] = TILE_APPROACH_DUNGEON_TOWN;
    }
    g->map.tiles[ty - 1][g->map.stairs_down_x] = TILE_TOWN_PATH;
    g->map.tiles[ty][tx] = TILE_APPROACH_TOWN;
    g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] = TILE_APPROACH_AREA;
    if (!map_is_walkable(&g->map, g->player.x, g->player.y)) {
        g->player.x = g->map.stairs_down_x;
        g->player.y = ty;
        map_mark_explored(&g->map, g->player.x, g->player.y);
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active) {
            if (!map_is_walkable(&g->map, item->x, item->y)) {
                item->x = g->map.stairs_down_x;
                item->y = ty;
            }
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

int approach_forest_town_exit(const GameState *g, int *x, int *y) {
    const TownApproach *route = approach_route(g);
    if (!route || route->area != LOCATION_FOREST || !(g->defeated_bosses & (1 << LOCATION_FOREST))) {
        return 0;
    }
    *x = route->town == LOCATION_TOWN ? 0 : APPROACH_W - 1;
    *y = APPROACH_H / 2;
    return 1;
}

void approach_refresh_forest(GameState *g) {
    const TownApproach *route = approach_route(g);
    if (!route || route->area != LOCATION_FOREST) {
        return;
    }
    int tx = route->town == LOCATION_TOWN ? APPROACH_W - 1 : 0;
    int ty = APPROACH_H / 2;
    int door_x = APPROACH_W / 2;
    int door_y = ty - 2;
    g->map.stairs_up_x = tx;
    g->map.stairs_up_y = ty;
    g->map.stairs_down_x = door_x;
    g->map.stairs_down_y = door_y;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g->map.tiles[y][x] = x > 0 && x < APPROACH_W - 1 && y > 0 && y < APPROACH_H - 1 ? TILE_TOWN_FLOOR : TILE_WALL;
        }
    }
    int ox;
    int oy;
    int open = approach_forest_town_exit(g, &ox, &oy);
    int from = open || tx == 0 ? 1 : APPROACH_LABYRINTH_X;
    int to = open || tx != 0 ? APPROACH_W - 2 : door_x;
    for (int x = from; x <= to; x++) {
        g->map.tiles[ty][x] = TILE_TOWN_PATH;
    }
    // The haunted trees shelter a south-facing entrance above the road.
    for (int y = door_y - 5; y <= door_y; y++) {
        for (int x = door_x - 3; x <= door_x + 3; x++) {
            g->map.tiles[y][x] = TILE_WALL;
        }
    }
    g->map.tiles[door_y + 1][door_x] = TILE_TOWN_PATH;
    g->map.tiles[door_y][door_x] = TILE_APPROACH_AREA;
    // The labyrinth shares this road; Rook's quest controls its sealed/open gate.
    for (int y = APPROACH_LABYRINTH_Y - 2; y <= APPROACH_LABYRINTH_Y; y++) {
        for (int x = APPROACH_LABYRINTH_X - 1; x <= APPROACH_LABYRINTH_X + 1; x++) {
            g->map.tiles[y][x] = TILE_WALL;
        }
    }
    g->map.tiles[APPROACH_LABYRINTH_Y + 1][APPROACH_LABYRINTH_X] = TILE_TOWN_PATH;
    g->map.tiles[APPROACH_LABYRINTH_Y][APPROACH_LABYRINTH_X] = TILE_LABYRINTH_ENTRANCE;
    g->map.tiles[ty][tx] = TILE_APPROACH_TOWN;
    if (open) {
        g->map.tiles[oy][ox] = TILE_APPROACH_DUNGEON_TOWN;
    }
    if (!map_is_walkable(&g->map, g->player.x, g->player.y)) {
        g->player.x = door_x;
        g->player.y = ty;
        map_mark_explored(&g->map, g->player.x, g->player.y);
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active) {
            if (!map_is_walkable(&g->map, item->x, item->y)) {
                item->x = door_x;
                item->y = ty;
            }
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

int approach_mountain_town_exit(const GameState *g, int *x, int *y) {
    const TownApproach *route = approach_route(g);
    if (!route || route->area != LOCATION_MOUNTAINS || !(g->defeated_bosses & (1 << LOCATION_MOUNTAINS))) {
        return 0;
    }
    *x = APPROACH_W / 2;
    *y = route->town == LOCATION_DAWNMERE ? 0 : APPROACH_H - 1;
    return 1;
}

void approach_refresh_mountains(GameState *g) {
    const TownApproach *route = approach_route(g);
    if (!route || route->area != LOCATION_MOUNTAINS) {
        return;
    }
    int tx = APPROACH_W / 2;
    int ty = route->town == LOCATION_DAWNMERE ? APPROACH_H - 1 : 0;
    int dy = ty ? -1 : 1;
    int junction = APPROACH_H / 2;
    g->map.stairs_up_x = tx;
    g->map.stairs_up_y = ty;
    g->map.stairs_down_x = tx + 6;
    g->map.stairs_down_y = junction - 1;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g->map.tiles[y][x] = x > 0 && x < APPROACH_W - 1 && y > 0 && y < APPROACH_H - 1 ? TILE_TOWN_FLOOR : TILE_WALL;
        }
    }
    int ox;
    int oy;
    int open = approach_mountain_town_exit(g, &ox, &oy);
    for (int step = 1; step < (open ? APPROACH_H - 1 : (ty > junction ? ty - junction : junction - ty) + 1); step++) {
        g->map.tiles[ty + step * dy][tx] = TILE_TOWN_PATH;
    }
    for (int x = tx + 1; x <= g->map.stairs_down_x; x++) {
        g->map.tiles[junction][x] = TILE_TOWN_PATH;
    }
    // The cave faces south; its rock mass stands east of the through road.
    for (int row = -5; row <= 0; row++) {
        int radius = row < -3 ? 1 : row < -1 ? 2 : 3;
        for (int side = -radius; side <= radius; side++) {
            g->map.tiles[g->map.stairs_down_y + row][g->map.stairs_down_x + side] = TILE_WALL;
        }
    }
    g->map.tiles[ty][tx] = TILE_APPROACH_TOWN;
    g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] = TILE_APPROACH_AREA;
    if (open) {
        g->map.tiles[oy][ox] = TILE_APPROACH_DUNGEON_TOWN;
    }
    if (!map_is_walkable(&g->map, g->player.x, g->player.y)) {
        g->player.x = tx;
        g->player.y = junction;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active) {
            if (!map_is_walkable(&g->map, item->x, item->y)) {
                item->x = tx;
                item->y = junction;
            }
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

int approach_swamp_town_exit(const GameState *g, int *x, int *y) {
    const TownApproach *route = approach_route(g);
    if (!route || route->area != LOCATION_SWAMP || !(g->defeated_bosses & (1 << LOCATION_SWAMP))) {
        return 0;
    }
    *x = APPROACH_W / 2;
    *y = route->town == LOCATION_TOWN2 ? 0 : APPROACH_H - 1;
    return 1;
}

void approach_refresh_swamp(GameState *g) {
    const TownApproach *route = approach_route(g);
    if (!route || route->area != LOCATION_SWAMP) {
        return;
    }
    int tx = APPROACH_W / 2;
    int ty = route->town == LOCATION_TOWN2 ? APPROACH_H - 1 : 0;
    int dy = ty ? -1 : 1;
    int junction = APPROACH_H / 2;
    g->map.stairs_up_x = tx;
    g->map.stairs_up_y = ty;
    g->map.stairs_down_x = tx + 6;
    g->map.stairs_down_y = junction - 1;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g->map.tiles[y][x] = x > 0 && x < APPROACH_W - 1 && y > 0 && y < APPROACH_H - 1 ? TILE_TOWN_FLOOR : TILE_WALL;
        }
    }
    int ox;
    int oy;
    int open = approach_swamp_town_exit(g, &ox, &oy);
    for (int step = 1; step < (open ? APPROACH_H - 1 : (ty > junction ? ty - junction : junction - ty) + 1); step++) {
        g->map.tiles[ty + step * dy][tx] = TILE_TOWN_PATH;
    }
    for (int x = tx + 1; x <= g->map.stairs_down_x; x++) {
        g->map.tiles[junction][x] = TILE_TOWN_PATH;
    }
    // The marsh gateway faces south, east of the through road.
    for (int row = -5; row <= 0; row++) {
        int radius = 3;
        for (int side = -radius; side <= radius; side++) {
            g->map.tiles[g->map.stairs_down_y + row][g->map.stairs_down_x + side] = TILE_WALL;
        }
    }
    g->map.tiles[ty][tx] = TILE_APPROACH_TOWN;
    g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] = TILE_APPROACH_AREA;
    if (open) {
        g->map.tiles[oy][ox] = TILE_APPROACH_DUNGEON_TOWN;
    }
    if (!map_is_walkable(&g->map, g->player.x, g->player.y)) {
        g->player.x = tx;
        g->player.y = junction;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active) {
            if (!map_is_walkable(&g->map, item->x, item->y)) {
                item->x = tx;
                item->y = junction;
            }
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

// Dead-end areas with a walk-in entrance sprite; 0 keeps the older approach style.
int approach_entrance_rows(Location area) {
    switch (area) {
        case LOCATION_DRAGONSPINE: return 9;
        case LOCATION_COAST:
        case LOCATION_FROSTFELL:
        case LOCATION_DESERT:
        case LOCATION_MOONVEIL:
        case LOCATION_ASHEN:
        case LOCATION_GLASSDEEP:
        case LOCATION_CATACOMBS: return 6;
        default: return 0;
    }
}

TileType approach_coast_ground(int x, int y) {
    int shore = APPROACH_H / 2 - 2 + (x / 9 % 3 == 0);
    if (y >= shore) {
        return TILE_COAST_SEA;
    }
    if (y >= shore - 2) {
        return TILE_COAST_SAND;
    }
    return TILE_TOWN_FLOOR;
}

void approach_refresh_entrance(GameState *g) {
    const TownApproach *route = approach_route(g);
    if (!route || !approach_entrance_rows(route->area)) {
        return;
    }
    // East and west gates use the forest layout; north and south gates use the mountain layout.
    int vertical = route->gate_y == 0 || route->gate_y == TOWN_H - 1;
    int tx = vertical ? APPROACH_W / 2 : route->gate_x == 0 ? APPROACH_W - 1 : 0;
    int ty = !vertical ? APPROACH_H / 2 : route->gate_y == 0 ? APPROACH_H - 1 : 0;
    int jx = APPROACH_W / 2;
    int jy = APPROACH_H / 2;
    int rows = approach_entrance_rows(route->area);
    g->map.stairs_up_x = tx;
    g->map.stairs_up_y = ty;
    g->map.stairs_down_x = vertical ? jx + 6 : jx;
    g->map.stairs_down_y = vertical ? jy - 1 : jy - 2;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g->map.tiles[y][x] = x > 0 && x < APPROACH_W - 1 && y > 0 && y < APPROACH_H - 1 ? TILE_TOWN_FLOOR : TILE_WALL;
            if (route->area == LOCATION_COAST && x < APPROACH_W && y < APPROACH_H) {
                TileType ground = approach_coast_ground(x, y);
                if (ground == TILE_COAST_SEA || ground == TILE_COAST_SAND) {
                    g->map.tiles[y][x] = ground == TILE_COAST_SAND && (x == 0 || x == APPROACH_W - 1) ? TILE_COAST_WALL : ground;
                }
            }
        }
    }
    int x = tx;
    int y = ty;
    while (x != jx || y != jy) {
        x += (jx > x) - (jx < x);
        y += (jy > y) - (jy < y);
        g->map.tiles[y][x] = TILE_TOWN_PATH;
    }
    while (x != g->map.stairs_down_x || y != g->map.stairs_down_y + 1) {
        x += (g->map.stairs_down_x > x) - (g->map.stairs_down_x < x);
        y += (g->map.stairs_down_y + 1 > y) - (g->map.stairs_down_y + 1 < y);
        g->map.tiles[y][x] = TILE_TOWN_PATH;
    }
    // The landmark faces south; only its doorway can be entered.
    for (int row = 1 - rows; row <= 0; row++) {
        for (int side = -3; side <= 3; side++) {
            g->map.tiles[g->map.stairs_down_y + row][g->map.stairs_down_x + side] = TILE_WALL;
        }
    }
    g->map.tiles[ty][tx] = TILE_APPROACH_TOWN;
    g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] = TILE_APPROACH_AREA;
    if (!map_is_walkable(&g->map, g->player.x, g->player.y)) {
        g->player.x = jx;
        g->player.y = jy;
        map_mark_explored(&g->map, g->player.x, g->player.y);
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active) {
            if (!map_is_walkable(&g->map, item->x, item->y)) {
                item->x = jx;
                item->y = jy;
            }
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

static void generate_approach(Map *map, Location area, int dx, int dy) {
    int tx = dx < 0 ? APPROACH_W - 1 : dx > 0 ? 0 : APPROACH_W / 2;
    int ty = dy < 0 ? APPROACH_H - 1 : dy > 0 ? 0 : APPROACH_H / 2;
    TileType floor = approach_floor(area);
    TileType wall = approach_wall(area);
    memset(map, 0, sizeof(*map));
    for (int my = 0; my < MAP_H; my++) {
        for (int mx = 0; mx < MAP_W; mx++) {
            map->tiles[my][mx] = wall;
        }
    }
    for (int step = 0; step <= APPROACH_LENGTH; step++) {
        int radius = step < APPROACH_LENGTH - 3 ? 1 : 0;
        for (int side = -radius; side <= radius; side++) {
            map->tiles[ty + step * dy + side * dx][tx + step * dx - side * dy] = floor;
        }
    }
    map->stairs_up_x = tx;
    map->stairs_up_y = ty;
    map->stairs_down_x = tx + APPROACH_LENGTH * dx;
    map->stairs_down_y = ty + APPROACH_LENGTH * dy;
    map->tiles[ty][tx] = TILE_APPROACH_TOWN;
    map->tiles[map->stairs_down_y][map->stairs_down_x] = TILE_APPROACH_AREA;
}

void approach_relocate_ridgeshire(GameState *g) {
    const TownApproach *route = approach_route(g);
    if (!route || route->town != LOCATION_TOWN4 || (route->area != LOCATION_DRAGONSPINE && route->area != LOCATION_ASHEN)) {
        return;
    }
    int dx = route->gate_x == TOWN_W - 1 ? 1 : 0;
    int dy = route->gate_y == 0 ? -1 : 0;
    int old_dx = route->area == LOCATION_DRAGONSPINE ? 1 : 0;
    int old_dy = route->area == LOCATION_ASHEN ? -1 : 0;
    // Old-version fixtures can already contain the new direction.
    if (g->map.stairs_down_x - g->map.stairs_up_x == APPROACH_LENGTH * dx &&
        g->map.stairs_down_y - g->map.stairs_up_y == APPROACH_LENGTH * dy) {
        return;
    }
    Map old = g->map;
    generate_approach(&g->map, route->area, dx, dy);
    for (int y = 0; y < APPROACH_H; y++) {
        for (int x = 0; x < APPROACH_W; x++) {
            int along = (x - old.stairs_up_x) * old_dx + (y - old.stairs_up_y) * old_dy;
            int side = (y - old.stairs_up_y) * old_dx - (x - old.stairs_up_x) * old_dy;
            int nx = g->map.stairs_up_x + along * dx - side * dy;
            int ny = g->map.stairs_up_y + along * dy + side * dx;
            if (nx >= 0 && nx < APPROACH_W && ny >= 0 && ny < APPROACH_H && map_is_explored(&old, x, y)) {
                map_mark_explored(&g->map, nx, ny);
            }
        }
    }
    int along = (g->player.x - old.stairs_up_x) * old_dx + (g->player.y - old.stairs_up_y) * old_dy;
    int side = (g->player.y - old.stairs_up_y) * old_dx - (g->player.x - old.stairs_up_x) * old_dy;
    g->player.x = g->map.stairs_up_x + along * dx - side * dy;
    g->player.y = g->map.stairs_up_y + along * dy + side * dx;
    along = g->player.last_dx * old_dx + g->player.last_dy * old_dy;
    side = g->player.last_dy * old_dx - g->player.last_dx * old_dy;
    g->player.last_dx = along * dx - side * dy;
    g->player.last_dy = along * dy + side * dx;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (!item->active) {
            continue;
        }
        along = (item->x - old.stairs_up_x) * old_dx + (item->y - old.stairs_up_y) * old_dy;
        side = (item->y - old.stairs_up_y) * old_dx - (item->x - old.stairs_up_x) * old_dy;
        item->x = g->map.stairs_up_x + along * dx - side * dy;
        item->y = g->map.stairs_up_y + along * dy + side * dx;
        g->map.tiles[item->y][item->x] = TILE_ITEM;
    }
}

static void project_old_road(const GameState *g, Location road, int x, int y, int *nx, int *ny) {
    (void)g;
    if (road == LOCATION_FOREST_ROAD) {
        *nx = 1 + x * (APPROACH_W - 3) / (FOREST_ROAD_W - 1);
        *ny = APPROACH_H / 2;
    } else {
        int height = road == LOCATION_SWAMP_ROAD ? SWAMP_ROAD_H : HIGH_PASS_H;
        *nx = APPROACH_W / 2;
        *ny = 1 + y * (APPROACH_H - 3) / (height - 1);
    }
}

void approach_migrate_roads(GameState *g) {
    Location road = g->location;
    if (road != LOCATION_FOREST_ROAD && road != LOCATION_SWAMP_ROAD && road != LOCATION_HIGH_PASS) {
        return;
    }
    Location area = road == LOCATION_FOREST_ROAD ? LOCATION_FOREST : road == LOCATION_SWAMP_ROAD ? LOCATION_SWAMP : LOCATION_MOUNTAINS;
    Location town = area == LOCATION_FOREST ? g->forest_entry_town : area == LOCATION_SWAMP ? g->swamp_entry_town : g->mountain_entry_town;
    for (int i = 0; i < town_approach_count; i++) {
        const TownApproach *route = &town_approaches[i];
        if (route->town != town || route->area != area) {
            continue;
        }
        Map old = g->map;
        int dx = route->gate_x == 0 ? -1 : route->gate_x == TOWN_W - 1 ? 1 : 0;
        int dy = route->gate_y == 0 ? -1 : route->gate_y == TOWN_H - 1 ? 1 : 0;
        generate_approach(&g->map, area, dx, dy);
        g->location = LOCATION_APPROACH;
        g->approach_route = i;
        approach_refresh_fork(g);
        approach_refresh_forest(g);
        approach_refresh_mountains(g);
        approach_refresh_swamp(g);
        int x;
        int y;
        project_old_road(g, road, g->player.x + 4 * g->player.last_dx, g->player.y + 4 * g->player.last_dy, &x, &y);
        project_old_road(g, road, g->player.x, g->player.y, &g->player.x, &g->player.y);
        g->player.last_dx = (x > g->player.x) - (x < g->player.x);
        g->player.last_dy = (y > g->player.y) - (y < g->player.y);
        for (int oy = 0; oy < MAP_H; oy++) {
            for (int ox = 0; ox < MAP_W; ox++) {
                if (map_is_explored(&old, ox, oy) && map_is_walkable(&old, ox, oy)) {
                    project_old_road(g, road, ox, oy, &x, &y);
                    map_mark_explored(&g->map, x, y);
                }
            }
        }
        for (int item = 0; item < g->floor_item_count; item++) {
            FloorItem *loot = &g->floor_items[item];
            if (loot->active) {
                project_old_road(g, road, loot->x, loot->y, &loot->x, &loot->y);
                loot->underlying_tile = g->map.tiles[loot->y][loot->x];
            }
        }
        for (int item = 0; item < g->floor_item_count; item++) {
            FloorItem *loot = &g->floor_items[item];
            if (loot->active) {
                g->map.tiles[loot->y][loot->x] = TILE_ITEM;
            }
        }
        map_mark_explored(&g->map, g->player.x, g->player.y);
        return;
    }
}

Location approach_shortcut_town(const TownApproach *route) {
    if (route->area == LOCATION_FOREST) {
        return route->town == LOCATION_TOWN ? LOCATION_TOWN2 : LOCATION_TOWN;
    }
    if (route->area == LOCATION_SWAMP) {
        return route->town == LOCATION_TOWN2 ? LOCATION_TOWN3 : LOCATION_TOWN2;
    }
    return route->town == LOCATION_TOWN ? LOCATION_TOWN4 : LOCATION_TOWN;
}

int approach_dungeon_town_exit(const GameState *g, int *x, int *y) {
    const TownApproach *route = approach_route(g);
    if (!route || route->area != LOCATION_DUNGEON || !(g->defeated_bosses & (1 << LOCATION_DUNGEON))) {
        return 0;
    }
    *x = route->town == LOCATION_TOWN ? APPROACH_W - 1 : 0;
    *y = g->map.stairs_up_y;
    return 1;
}

int approach_shortcut_exit(const GameState *g, int *x, int *y) {
    (void)g;
    (void)x;
    (void)y;
    return 0;
}

int approach_boss_exit(const GameState *g, int *x, int *y) {
    if (!approach_shortcut_exit(g, x, y)) {
        return 0;
    }
    int dx = (g->map.stairs_down_x > g->map.stairs_up_x) - (g->map.stairs_down_x < g->map.stairs_up_x);
    int dy = (g->map.stairs_down_y > g->map.stairs_up_y) - (g->map.stairs_down_y < g->map.stairs_up_y);
    *x -= 2 * dy * APPROACH_BRANCH_LENGTH;
    *y += 2 * dx * APPROACH_BRANCH_LENGTH;
    return 1;
}

void approach_refresh_fork(GameState *g) {
    int x;
    int y;
    if (!approach_shortcut_exit(g, &x, &y)) {
        return;
    }
    const TownApproach *route = approach_route(g);
    int dx = (g->map.stairs_down_x > g->map.stairs_up_x) - (g->map.stairs_down_x < g->map.stairs_up_x);
    int dy = (g->map.stairs_down_y > g->map.stairs_up_y) - (g->map.stairs_down_y < g->map.stairs_up_y);
    for (int branch = 0; branch < 2; branch++) {
        if (branch) {
            approach_boss_exit(g, &x, &y);
        }
        int sx = branch ? -dy : dy;
        int sy = branch ? dx : -dx;
        for (int step = 0; step < APPROACH_BRANCH_LENGTH; step++) {
            int nx = x - step * sx;
            int ny = y - step * sy;
            TileType tile = step ? approach_floor(route->area) : branch ? TILE_APPROACH_BOSS : TILE_APPROACH_SHORTCUT;
            if (g->map.tiles[ny][nx] != TILE_ITEM) {
                g->map.tiles[ny][nx] = tile;
            }
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active && item->x == nx && item->y == ny) {
                    item->underlying_tile = tile;
                }
            }
        }
    }
}

static void return_from_branch(GameState *g, Location area, int boss) {
    for (int i = 0; i < town_approach_count; i++) {
        const TownApproach *route = &town_approaches[i];
        if (route->town != g->location || route->area != area || !(g->defeated_bosses & (1 << area))) {
            continue;
        }
        if (approach_enter(g, route->gate_x, route->gate_y)) {
            int x;
            int y;
            int found = boss ? approach_boss_exit(g, &x, &y) : approach_shortcut_exit(g, &x, &y);
            if (found) {
                int dx = (g->map.stairs_down_x > g->map.stairs_up_x) - (g->map.stairs_down_x < g->map.stairs_up_x);
                int dy = (g->map.stairs_down_y > g->map.stairs_up_y) - (g->map.stairs_down_y < g->map.stairs_up_y);
                int sx = boss ? -dy : dy;
                int sy = boss ? dx : -dx;
                g->player.x = x - sx;
                g->player.y = y - sy;
                g->player.last_dx = -sx;
                g->player.last_dy = -sy;
                push_message(g, "You return to the crossroads. Follow the signs to town or the enemy area.");
            }
        }
        return;
    }
}

void approach_return_from_shortcut(GameState *g, Location area) {
    return_from_branch(g, area, 0);
}

void approach_return_from_boss(GameState *g, Location area) {
    return_from_branch(g, area, 1);
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
    generate_approach(&g->map, route->area, dx, dy);
    int tx = g->map.stairs_up_x;
    int ty = g->map.stairs_up_y;
    g->location = LOCATION_APPROACH;
    g->approach_route = index;
    g->player.x = tx + dx;
    g->player.y = ty + dy;
    g->player.last_dx = dx;
    g->player.last_dy = dy;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    approach_refresh_dungeon(g);
    approach_refresh_forest(g);
    approach_refresh_mountains(g);
    approach_refresh_swamp(g);
    approach_refresh_entrance(g);
    approach_refresh_fork(g);
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
    if (route->area == LOCATION_FOREST && g->map.tiles[y][x] == TILE_LABYRINTH_ENTRANCE) {
        if (game_labyrinth_is_open(g)) {
            game_enter_labyrinth(g);
        } else {
            push_message(g, "The labyrinth is sealed. Speak to Rook in Oakhaven's Tavern.");
        }
        return 1;
    }
    if (g->map.tiles[y][x] == TILE_APPROACH_TOWN) {
        game_leave_approach(g);
        return 1;
    }
    int shortcut_x;
    int shortcut_y;
    if (approach_dungeon_town_exit(g, &shortcut_x, &shortcut_y) && x == shortcut_x && y == shortcut_y) {
        game_leave_approach_to_town(g, route->town == LOCATION_TOWN ? LOCATION_DAWNMERE : LOCATION_TOWN);
        return 1;
    }
    if (approach_swamp_town_exit(g, &shortcut_x, &shortcut_y) && x == shortcut_x && y == shortcut_y) {
        game_leave_approach_to_town(g, route->town == LOCATION_TOWN2 ? LOCATION_TOWN3 : LOCATION_TOWN2);
        return 1;
    }
    if (approach_forest_town_exit(g, &shortcut_x, &shortcut_y) && x == shortcut_x && y == shortcut_y) {
        game_leave_approach_to_town(g, route->town == LOCATION_TOWN ? LOCATION_TOWN2 : LOCATION_TOWN);
        return 1;
    }
    if (approach_mountain_town_exit(g, &shortcut_x, &shortcut_y) && x == shortcut_x && y == shortcut_y) {
        game_leave_approach_to_town(g, route->town == LOCATION_DAWNMERE ? LOCATION_TOWN4 : LOCATION_DAWNMERE);
        return 1;
    }
    if (approach_shortcut_exit(g, &shortcut_x, &shortcut_y) && x == shortcut_x && y == shortcut_y) {
        game_leave_approach_to_town(g, approach_shortcut_town(route));
        return 1;
    }
    if (approach_boss_exit(g, &shortcut_x, &shortcut_y) && x == shortcut_x && y == shortcut_y) {
        game_enter_shortcut_boss(g);
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
