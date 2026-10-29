#include "test_utils.h"
#include "game/approach.h"
#include "game/elderwood.h"
#include "systems/save_load.h"
#include <stdlib.h>
#include <string.h>

#define SCENERY_SLOT 99350
#define INTERIOR ((APPROACH_W - 2) * (APPROACH_H - 2))

static int scenery_count(const GameState *g, TileType scenery) {
    int count = 0;
    for (int y = 1; y < APPROACH_H - 1; y++) {
        for (int x = 1; x < APPROACH_W - 1; x++) {
            TileType tile = g->map.tiles[y][x];
            count += tile == scenery || tile == TILE_ROAD_BANK;
        }
    }
    return count;
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

static void road_layouts(GameState *g) {
    const int routes[] = {0, 4, 16, 14, 5, 9, 2, 11};
    for (int i = 0; i < 8; i++) {
        game_init(g);
        approach_enter_region(g, routes[i]);
        const TownApproach *route = approach_route(g);
        TileType scenery = approach_scenery(route->area);
        int vertical = g->map.stairs_up_y == 0 || g->map.stairs_up_y == APPROACH_H - 1;
        ASSERT("each regional road cuts through scenery covering most of its map", scenery != TILE_WALL && scenery_count(g, scenery) * 2 > INTERIOR);
        int verge = 1;
        for (int step = 1; step < (vertical ? APPROACH_H : APPROACH_W) - 1; step++) {
            for (int side = -1; side <= 1; side += 2) {
                int x = vertical ? g->map.stairs_up_x + side : step;
                int y = vertical ? step : g->map.stairs_up_y + side;
                verge &= map_is_walkable(&g->map, x, y) || g->map.tiles[y][x] == TILE_ROAD_BANK;
            }
        }
        ASSERT("open verges line the road except where a checkpoint pinches it", verge);
        int ox = vertical ? g->map.stairs_up_x - 6 : g->map.stairs_up_x + (g->map.stairs_up_x ? -3 : 3);
        int oy = vertical ? g->map.stairs_up_y + (g->map.stairs_up_y ? -3 : 3) : g->map.stairs_up_y + 6;
        ASSERT("the road opens into a clearing at its town end, where its signpost stands", map_is_walkable(&g->map, ox, oy));
        if (northern_approach(g)) {
            ASSERT("the northern through road opens at both its Oakhaven and castle ends", map_is_walkable(&g->map, ox, APPROACH_H - 1 - oy));
        }
        if (route->area == LOCATION_SWAMP) {
            int grass = 0;
            for (int y = 1; y < APPROACH_H - 1; y++) {
                for (int x = 1; x < APPROACH_W - 1; x++) {
                    grass += g->map.tiles[y][x] == TILE_TOWN_FLOOR;
                }
            }
            ASSERT("the swamp road's verges are bog mud rather than grass", grass == 0 && approach_ground(route->area) == TILE_SWAMP_FLOOR);
        }
    }
    game_init(g);
    approach_enter_region(g, 1);
    int grass = 0;
    for (int y = 1; y < APPROACH_H - 1; y++) {
        for (int x = 1; x < APPROACH_W - 1; x++) {
            grass += g->map.tiles[y][x] == TILE_TOWN_FLOOR;
        }
    }
    ASSERT("the Oakhaven to Dawnmere road keeps its open grass for now", approach_scenery(LOCATION_DUNGEON) == TILE_WALL && grass * 2 > INTERIOR);
}

static void saved_roads(GameState *g, GameState *loaded) {
    ASSERT("road scenery test slot is unused", !save_exists(SCENERY_SLOT));
    if (save_exists(SCENERY_SLOT)) {
        return;
    }
    const int routes[] = {0, 16, 5, 2};
    for (int i = 0; i < 4; i++) {
        game_init(g);
        approach_enter_region(g, routes[i]);
        TileType scenery = approach_scenery(approach_route(g)->area);
        // Saves made before the scenery grew kept open grass around the road.
        for (int y = 1; y < APPROACH_H - 1; y++) {
            for (int x = 1; x < APPROACH_W - 1; x++) {
                if (g->map.tiles[y][x] == scenery || g->map.tiles[y][x] == TILE_SWAMP_FLOOR) {
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
        ASSERT("roads saved as open grass grow their scenery when loaded", ok && scenery_count(loaded, scenery) * 2 > INTERIOR);
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
