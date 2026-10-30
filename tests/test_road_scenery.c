#include "test_utils.h"
#include "game/approach.h"
#include "game/elderwood.h"
#include "systems/save_load.h"
#include <stdlib.h>
#include <string.h>

#define SCENERY_SLOT 99350
#define INTERIOR ((APPROACH_W - 2) * (APPROACH_H - 2))

static int is_scenery(Location area, TileType tile) {
    return tile == approach_scenery(area, 0) || tile == approach_scenery(area, 50) ||
        tile == approach_scenery(area, 100) || tile == TILE_ASHEN_LAVA;
}

static int scenery_count(const GameState *g, Location area) {
    int count = 0;
    for (int y = 1; y < APPROACH_H - 1; y++) {
        for (int x = 1; x < APPROACH_W - 1; x++) {
            TileType tile = g->map.tiles[y][x];
            count += is_scenery(area, tile) || tile == TILE_ROAD_BANK;
        }
    }
    return count;
}

static int tile_count(const GameState *g, TileType wanted) {
    int count = 0;
    for (int y = 1; y < APPROACH_H - 1; y++) {
        for (int x = 1; x < APPROACH_W - 1; x++) {
            count += g->map.tiles[y][x] == wanted;
        }
    }
    return count;
}

// Counts open tiles reached from the town end; with every checkpoint open this is all of them.
static int reached_from_town(const GameState *g, int *open) {
    unsigned char seen[APPROACH_H][APPROACH_W] = {{0}};
    int queue[APPROACH_W * APPROACH_H];
    const int steps[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    int head = 0;
    int tail = 1;
    queue[0] = g->map.stairs_up_y * APPROACH_W + g->map.stairs_up_x;
    seen[g->map.stairs_up_y][g->map.stairs_up_x] = 1;
    while (head < tail) {
        int x = queue[head] % APPROACH_W;
        int y = queue[head++] / APPROACH_W;
        for (int side = 0; side < 4; side++) {
            int nx = x + steps[side][0];
            int ny = y + steps[side][1];
            if (nx >= 0 && ny >= 0 && nx < APPROACH_W && ny < APPROACH_H && !seen[ny][nx] && map_is_walkable(&g->map, nx, ny)) {
                seen[ny][nx] = 1;
                queue[tail++] = ny * APPROACH_W + nx;
            }
        }
    }
    *open = 0;
    for (int y = 0; y < APPROACH_H; y++) {
        for (int x = 0; x < APPROACH_W; x++) {
            *open += map_is_walkable(&g->map, x, y);
        }
    }
    return tail;
}

static int reaches_town(const GameState *g) {
    unsigned char seen[APPROACH_H][APPROACH_W] = {{0}};
    int queue[APPROACH_W * APPROACH_H];
    const int steps[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    int head = 0;
    int tail = 1;
    queue[0] = g->player.y * APPROACH_W + g->player.x;
    seen[g->player.y][g->player.x] = 1;
    while (head < tail) {
        int x = queue[head] % APPROACH_W;
        int y = queue[head++] / APPROACH_W;
        if (x == g->map.stairs_up_x && y == g->map.stairs_up_y) {
            return 1;
        }
        for (int side = 0; side < 4; side++) {
            int nx = x + steps[side][0];
            int ny = y + steps[side][1];
            if (nx >= 0 && ny >= 0 && nx < APPROACH_W && ny < APPROACH_H && !seen[ny][nx] && map_is_walkable(&g->map, nx, ny)) {
                seen[ny][nx] = 1;
                queue[tail++] = ny * APPROACH_W + nx;
            }
        }
    }
    return 0;
}

static void sparse_layouts(GameState *g, const TownApproach *route) {
    int vertical = g->map.stairs_up_y == 0 || g->map.stairs_up_y == APPROACH_H - 1;
    int span = vertical ? abs(g->map.stairs_down_y - g->map.stairs_up_y) : abs(g->map.stairs_down_x - g->map.stairs_up_x);
    int start = 0;
    int near = 0;
    int far = 0;
    for (int y = 1; y < APPROACH_H - 1; y++) {
        for (int x = 1; x < APPROACH_W - 1; x++) {
            int along = vertical ? abs(y - g->map.stairs_up_y) : abs(x - g->map.stairs_up_x);
            if (g->map.tiles[y][x] == approach_scenery(route->area, 100)) {
                start += along < span / 4;
                near += along < span / 2;
                far += along >= span / 2;
            }
        }
    }
    if (route->area == LOCATION_DUNGEON) {
        ASSERT("the Oakhaven to Dawnmere road is lightly wooded", (near + far) * 100 > INTERIOR * 15 && (near + far) * 2 < INTERIOR);
    } else if (route->area == LOCATION_GLASSDEEP || route->area == LOCATION_GIANTS_CROWN) {
        ASSERT("the Stonewake and Giant's Peaks roads start grassy and gather a few boulders near the landmark", start == 0 && far > near * 3 &&
            (near + far) * 100 < INTERIOR * 15);
    } else {
        ASSERT("the Stormwatch road turns rocky as it nears the headland", far > near * 5 && (near + far) * 5 < INTERIOR * 2);
    }
}

static void road_layouts(GameState *g) {
    const int routes[] = {0, 4, 16, 14, 5, 9, 2, 11, 12, 19, 8, 6, 13, 1, 15, 7, 18, 20};
    for (int i = 0; i < 18; i++) {
        game_init(g);
        approach_enter_region(g, routes[i]);
        const TownApproach *route = approach_route(g);
        int vertical = g->map.stairs_up_y == 0 || g->map.stairs_up_y == APPROACH_H - 1;
        if (route->area == LOCATION_DUNGEON || route->area == LOCATION_GLASSDEEP || route->area == LOCATION_GIANTS_CROWN ||
            route->area == LOCATION_STORMWATCH) {
            sparse_layouts(g, route);
        } else {
            ASSERT("each regional road cuts through scenery covering most of its map", approach_scenery(route->area, 100) != TILE_WALL &&
                scenery_count(g, route->area) * 2 > INTERIOR);
        }
        int verge = 1;
        for (int y = 1; y < APPROACH_H - 1; y++) {
            for (int x = 1; x < APPROACH_W - 1; x++) {
                if (g->map.tiles[y][x] != TILE_TOWN_PATH) {
                    continue;
                }
                verge &= !is_scenery(route->area, g->map.tiles[y - 1][x]) && !is_scenery(route->area, g->map.tiles[y + 1][x]) &&
                    !is_scenery(route->area, g->map.tiles[y][x - 1]) && !is_scenery(route->area, g->map.tiles[y][x + 1]);
            }
        }
        ASSERT("scenery never touches the road; only a checkpoint bank pinches it", verge);
        int ox = vertical ? g->map.stairs_up_x - 6 : g->map.stairs_up_x + (g->map.stairs_up_x ? -3 : 3);
        int oy = vertical ? g->map.stairs_up_y + (g->map.stairs_up_y ? -3 : 3) : g->map.stairs_up_y + 6;
        ASSERT("the road opens into a clearing at its town end, where its signpost stands", map_is_walkable(&g->map, ox, oy));
        if (northern_approach(g)) {
            ASSERT("the northern through road opens at both its Oakhaven and castle ends", map_is_walkable(&g->map, ox, APPROACH_H - 1 - oy));
        }
        if (route->area == LOCATION_SWAMP) {
            ASSERT("the swamp road's verges are bog mud rather than grass", tile_count(g, TILE_TOWN_FLOOR) == 0 &&
                approach_ground(route->area) == TILE_SWAMP_FLOOR);
        }
        if (route->area == LOCATION_DRAGONSPINE || route->area == LOCATION_IRONFANG) {
            ASSERT("the Dragonspine and Ironfang roads cut through mountain rock", tile_count(g, TILE_MOUNTAIN_WALL) * 2 > INTERIOR);
        }
        if (route->area == LOCATION_FROSTFELL || route->area == LOCATION_DESERT || route->area == LOCATION_ASHEN) {
            int near_x = vertical ? 8 : g->map.stairs_up_x ? APPROACH_W - 2 : 1;
            int near_y = vertical ? g->map.stairs_up_y ? APPROACH_H - 2 : 1 : 4;
            int far_x = vertical ? 8 : g->map.stairs_up_x ? 2 : APPROACH_W - 3;
            int far_y = vertical ? g->map.stairs_up_y ? 2 : APPROACH_H - 3 : 4;
            TileType far = g->map.tiles[far_y][far_x];
            ASSERT("the road leaves town through green woods", g->map.tiles[near_y][near_x] == TILE_ELDERWOOD_WALL);
            ASSERT("and reaches its landmark through that region's own terrain", far == approach_scenery(route->area, 100) || far == TILE_ASHEN_LAVA);
            ASSERT("snow, sand and ash are drawn over ordinary road-side grass, without regional floor hazards",
                tile_count(g, TILE_FROST_FLOOR) == 0 && tile_count(g, TILE_DESERT_FLOOR) == 0 && tile_count(g, TILE_ASHEN_FLOOR) == 0);
        }
        if (route->area == LOCATION_FROSTFELL) {
            int line = 1;
            for (int x = 1; x < APPROACH_W - 1; x++) {
                int frozen = 0;
                for (int step = 1; step < APPROACH_H - 1; step++) {
                    TileType tile = g->map.tiles[g->map.stairs_up_y ? APPROACH_H - 1 - step : step][x];
                    frozen |= tile == TILE_FROST_WALL;
                    line &= !frozen || tile != TILE_ELDERWOOD_WALL;
                }
            }
            ASSERT("the Frostfell woods end in one tree line instead of dappling into the frozen ground", line);
        }
        if (route->area == LOCATION_ASHEN) {
            ASSERT("blackened dead trees and then lava-split basalt follow the green woods toward Ashen Hollow",
                tile_count(g, TILE_FOREST_WALL) > 0 && tile_count(g, TILE_ASHEN_WALL) > 0 && tile_count(g, TILE_ASHEN_LAVA) > 0);
        }
        g->debug_passages_unlocked = 1;
        approach_enter_region(g, routes[i]);
        int open;
        ASSERT("every open tile beside the scenery can be reached from the road", reached_from_town(g, &open) == open);
    }
    game_init(g);
    approach_enter_region(g, 3);
    ASSERT("the Sunken Coast road keeps its open grass, shore and path", approach_scenery(LOCATION_COAST, 100) == TILE_WALL &&
        tile_count(g, TILE_ELDERWOOD_WALL) == 0 && tile_count(g, TILE_MOUNTAIN_WALL) == 0 && tile_count(g, TILE_TOWN_FLOOR) > 0);
}

static void saved_roads(GameState *g, GameState *loaded) {
    ASSERT("road scenery test slot is unused", !save_exists(SCENERY_SLOT));
    if (save_exists(SCENERY_SLOT)) {
        return;
    }
    const int routes[] = {0, 16, 5, 2, 8, 13, 1, 18, 20};
    for (int i = 0; i < 9; i++) {
        game_init(g);
        approach_enter_region(g, routes[i]);
        Location area = approach_route(g)->area;
        int expected = scenery_count(g, area);
        // Saves made before the scenery grew kept open grass around the road.
        for (int y = 1; y < APPROACH_H - 1; y++) {
            for (int x = 1; x < APPROACH_W - 1; x++) {
                if (is_scenery(area, g->map.tiles[y][x]) || g->map.tiles[y][x] == TILE_SWAMP_FLOOR) {
                    g->map.tiles[y][x] = TILE_TOWN_FLOOR;
                }
            }
        }
        g->player.x = 10;
        g->player.y = 40;
        map_mark_explored(&g->map, 10, 40);
        map_mark_explored(&g->map, 50, 5);
        g->floor_item_count = 1;
        g->floor_items[0] = (FloorItem){.active = 1, .x = 12, .y = 40, .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_health_potion()};
        g->map.tiles[40][12] = TILE_ITEM;
        g->gold = 456;
        int ok = save_game(g, SCENERY_SLOT) && load_game(loaded, SCENERY_SLOT);
        ASSERT("roads saved as open grass regrow their scenery when loaded", ok && scenery_count(loaded, area) == expected);
        const FloorItem *item = &loaded->floor_items[0];
        ASSERT("travelers and loot caught in new scenery move to the open road, keeping gold and exploration", ok &&
            map_is_walkable(&loaded->map, loaded->player.x, loaded->player.y) && reaches_town(loaded) && item->active &&
            loaded->map.tiles[item->y][item->x] == TILE_ITEM && map_is_explored(&loaded->map, 50, 5) && loaded->gold == 456);
        ok = save_game(loaded, SCENERY_SLOT) && load_game(g, SCENERY_SLOT);
        ASSERT("a road saved with its scenery loads unchanged", ok && !memcmp(g->map.tiles, loaded->map.tiles, sizeof(g->map.tiles)) &&
            g->player.x == loaded->player.x && g->player.y == loaded->player.y);
    }
    remove("saves/savegame_99350.json");
}

void test_road_scenery(void) {
    printf("Regional road scenery:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    if (!g || !loaded) {
        ASSERT("road scenery fixtures allocate", 0);
        free(g);
        free(loaded);
        return;
    }
    road_layouts(g);
    saved_roads(g, loaded);
    free(g);
    free(loaded);
}
