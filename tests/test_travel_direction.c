#include "test_utils.h"
#include "game/actions.h"
#include "game/coast_exits.h"
#include "game/town_life.h"
#include "systems/save_load.h"
#include <stdlib.h>
#include <string.h>

#define DIRECTION_SLOT 99163
#define CELLS (MAP_W * MAP_H)

static void boundary(int x, int y, int *dx, int *dy) {
    *dx = x == 1 ? -1 : x == MAP_W - 2 ? 1 : 0;
    *dy = y == 1 ? -1 : y == MAP_H - 2 ? 1 : 0;
}

static int walk_to(GameState *g, int tx, int ty) {
    int previous[CELLS];
    int queue[CELLS];
    for (int i = 0; i < CELLS; i++) {
        previous[i] = -1;
    }
    int start = g->player.y * MAP_W + g->player.x;
    int target = ty * MAP_W + tx;
    int head = 0;
    int tail = 1;
    previous[start] = start;
    queue[0] = start;
    static const int steps[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    while (head < tail && previous[target] < 0) {
        int cell = queue[head++];
        for (int side = 0; side < 4; side++) {
            int x = cell % MAP_W + steps[side][0];
            int y = cell / MAP_W + steps[side][1];
            if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
                continue;
            }
            int next = y * MAP_W + x;
            if (previous[next] >= 0 || !map_is_walkable(&g->map, x, y) ||
                (map_is_travel_tile(g->map.tiles[y][x]) && next != target)) {
                continue;
            }
            previous[next] = cell;
            queue[tail++] = next;
        }
    }
    if (previous[target] < 0) {
        return 0;
    }
    int length = 0;
    for (int cell = target; cell != start; cell = previous[cell]) {
        queue[length++] = cell;
    }
    Location location = g->location;
    int level = g->level;
    for (int i = length - 1; i >= 0; i--) {
        action_resolve_player(g, (Action){ACTION_MOVE, queue[i] % MAP_W, queue[i] / MAP_W});
        if (g->location != location || g->level != level) {
            return i == 0;
        }
        if (g->player.x != queue[i] % MAP_W || g->player.y != queue[i] / MAP_W) {
            return 0;
        }
    }
    return g->player.x == tx && g->player.y == ty;
}

static void reveal_landmark(GameState *g) {
    if (g->location != LOCATION_FOREST) {
        return;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g->map.tiles[y][x] == TILE_FOREST_LANDMARK) {
                ASSERT("the generated forest landmark has a continuous playable route", walk_to(g, x, y));
                return;
            }
        }
    }
}

static void cross_stage(GameState *g, int forward) {
    Location area = g->location;
    int level = g->level;
    int x = forward ? g->map.stairs_down_x : g->map.stairs_up_x;
    int y = forward ? g->map.stairs_down_y : g->map.stairs_up_y;
    int dx;
    int dy;
    boundary(x, y, &dx, &dy);
    ASSERT("the actual outdoor boundary has an unbroken route from the arrival", walk_to(g, x, y));
    TileType tile = g->map.tiles[y + dy][x + dx];
    ASSERT("the boundary contains the actual entrance or exit transition", tile == TILE_FOREST_ENTRANCE || tile == TILE_FOREST_EXIT || tile == TILE_MOUNTAIN_ENTRANCE || tile == TILE_MOUNTAIN_EXIT);
    // Turning on the last step must work even if the previous facing differs.
    g->player.last_dx = -dx;
    g->player.last_dy = -dy;
    action_resolve_player(g, (Action){ACTION_MOVE, x + dx, y + dy});
    ASSERT("walking across the boundary changes to the intended stage", g->location == area && g->level == level + (forward ? 1 : -1));
    ASSERT("forward travel and retreat keep the direction of the crossing step", g->player.last_dx == dx && g->player.last_dy == dy);
    x = forward ? g->map.stairs_up_x : g->map.stairs_down_x;
    y = forward ? g->map.stairs_up_y : g->map.stairs_down_y;
    int back_dx;
    int back_dy;
    boundary(x, y, &back_dx, &back_dy);
    ASSERT("adjacent stages meet at opposite boundaries", back_dx == -dx && back_dy == -dy && g->player.x == x && g->player.y == y);
    // Encounter placement must leave a usable arrival trail in that direction.
    g->enemy_count = 0;
    int usable = 1;
    for (int step = 0; step < 3; step++) {
        int px = g->player.x;
        int py = g->player.y;
        action_resolve_player(g, (Action){ACTION_MOVE, px + dx, py + dy});
        usable &= g->location == area && g->player.x == px + dx && g->player.y == py + dy;
    }
    ASSERT("the arrival trail continues in the travel direction before its first bend", usable);
}

static void outdoor_travel(GameState *g, int mountains, unsigned seed) {
    srand(seed);
    game_init(g);
    g->player.hp = 1000000;
    g->debug_passages_unlocked = 1;
    if (mountains) {
        game_enter_mountains(g);
    } else {
        game_enter_forest(g);
    }
    ASSERT("the northbound approach doorway enters from the south", g->player.y == MAP_H - 2);
    g->enemy_count = 0;
    for (int level = 1; level < FOREST_DEPTH; level++) {
        reveal_landmark(g);
        cross_stage(g, 1);
    }
    reveal_landmark(g);
    for (int level = FOREST_DEPTH; level > 1; level--) {
        cross_stage(g, 0);
    }
    // Revisit the same cached geometry instead of generating new layouts.
    for (int level = 1; level < FOREST_DEPTH; level++) {
        cross_stage(g, 1);
    }
}

static void door_round_trip(GameState *g, int x, int y, Location interior) {
    g->player.x = x;
    g->player.y = y + 1;
    g->player.last_dx = 1;
    g->player.last_dy = 0;
    action_resolve_player(g, (Action){ACTION_MOVE, x, y});
    ASSERT("walking north through a building door preserves north-facing arrival", g->location == interior && g->player.last_dx == 0 && g->player.last_dy == -1);
    int px = g->player.x;
    int py = g->player.y;
    action_resolve_player(g, (Action){ACTION_MOVE, px, py - 1});
    ASSERT("the building's arrival path continues away from its door", g->location == interior && g->player.y == py - 1);
    ASSERT("the building exit remains reachable", walk_to(g, g->map.stairs_down_x, g->map.stairs_down_y - 1));
    g->player.last_dx = -1;
    g->player.last_dy = 0;
    action_resolve_player(g, (Action){ACTION_MOVE, g->map.stairs_down_x, g->map.stairs_down_y});
    ASSERT("walking south out of a building preserves south-facing departure", g->location == LOCATION_TOWN && g->player.x == x && g->player.y == y + 1 && g->player.last_dx == 0 && g->player.last_dy == 1);
}

static void legacy_entry(Map *map, int side) {
    int dx;
    int dy;
    boundary(map->stairs_up_x, map->stairs_up_y, &dx, &dy);
    int mountains = map->tiles[map->stairs_up_y + dy][map->stairs_up_x + dx] == TILE_MOUNTAIN_ENTRANCE;
    TileType floor = mountains ? TILE_MOUNTAIN_FLOOR : TILE_FOREST_FLOOR;
    map->tiles[map->stairs_up_y + dy][map->stairs_up_x + dx] = mountains ? TILE_MOUNTAIN_WALL : TILE_FOREST_WALL;
    dx = side == 3 ? -1 : side == 1 ? 1 : 0;
    dy = side == 0 ? -1 : side == 2 ? 1 : 0;
    int x;
    int y;
    map_room_center(&map->rooms[0], &x, &y);
    while (x > 0 && x < MAP_W - 1 && y > 0 && y < MAP_H - 1) {
        map->tiles[y][x] = floor;
        x += dx;
        y += dy;
    }
    map->tiles[y][x] = mountains ? TILE_MOUNTAIN_ENTRANCE : TILE_FOREST_ENTRANCE;
    map->stairs_up_x = x - dx;
    map->stairs_up_y = y - dy;
}

static void saved_entries(GameState *g, GameState *loaded) {
    game_init(g);
    static const int forest_sides[FOREST_DEPTH] = {3, 2, 0, 3, 3};
    static const int mountain_sides[MOUNTAIN_DEPTH] = {3, 2, 0, 3, 3};
    for (int i = 0; i < FOREST_DEPTH; i++) {
        LevelCache *forest = &g->forest_cache[i];
        LevelCache *mountain = &g->mountain_cache[i];
        map_generate_forest(&forest->map, i + 1);
        map_generate_mountains(&mountain->map, i + 1);
        legacy_entry(&forest->map, forest_sides[i]);
        legacy_entry(&mountain->map, mountain_sides[i]);
        forest->valid = mountain->valid = 1;
        forest->enemy_count = mountain->enemy_count = 1;
        forest->enemies[0] = (Enemy){.type = ENEMY_DARK_ELF_GUARD, .active = 1, .x = 80, .y = 50, .hp = 17, .max_hp = 120, .move_timer = 2};
        mountain->enemies[0] = (Enemy){.type = ENEMY_HOBGOBLIN_GUARD, .active = 1, .x = 80, .y = 50, .hp = 19, .max_hp = 120, .move_timer = 3};
        map_mark_explored(&forest->map, 80, 50);
        map_mark_explored(&mountain->map, 80, 50);
    }
    g->location = LOCATION_FOREST;
    g->level = 3;
    g->map = g->forest_cache[2].map;
    g->enemy_count = 1;
    g->enemies[0] = g->forest_cache[2].enemies[0];
    g->player.x = 80;
    g->player.y = 50;
    g->player.last_dx = 1;
    g->player.last_dy = 0;
    int dx;
    int dy;
    boundary(g->map.stairs_up_x, g->map.stairs_up_y, &dx, &dy);
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = g->map.stairs_up_x + dx, .y = g->map.stairs_up_y + dy, .underlying_tile = TILE_FOREST_ENTRANCE, .item = item_make_health_potion()};
    g->gold = 393;
    g->score = 9876;
    ASSERT("an affected save loads with the direction repair", save_game(g, DIRECTION_SLOT) && load_game(loaded, DIRECTION_SLOT));
    ASSERT("repair preserves player position, facing, money, score and active enemy wounds", loaded->player.x == 80 && loaded->player.y == 50 && loaded->player.last_dx == 1 && loaded->gold == 393 && loaded->score == 9876 && loaded->enemies[0].hp == 17 && loaded->enemies[0].move_timer == 2);
    ASSERT("loot on a retired entrance moves onto the new entrance trail", loaded->floor_items[0].active && loaded->floor_items[0].x == loaded->map.stairs_up_x && loaded->floor_items[0].y == loaded->map.stairs_up_y && loaded->map.tiles[loaded->floor_items[0].y][loaded->floor_items[0].x] == TILE_ITEM);
    int preserved = 1;
    for (int i = 0; i < FOREST_DEPTH; i++) {
        LevelCache *forest = &loaded->forest_cache[i];
        LevelCache *mountain = &loaded->mountain_cache[i];
        preserved &= !map_repair_outdoor_entrance(&forest->map, i + 1, 0) && !map_repair_outdoor_entrance(&mountain->map, i + 1, 1);
        preserved &= forest->enemies[0].hp == 17 && mountain->enemies[0].hp == 19 && map_is_explored(&forest->map, 80, 50) && map_is_explored(&mountain->map, 80, 50);
    }
    ASSERT("all cached stages are repaired in place without resetting exploration or wounds", preserved);
    *g = *loaded;
    ASSERT("saving and reloading the repair is idempotent", save_game(g, DIRECTION_SLOT) && load_game(loaded, DIRECTION_SLOT) && memcmp(&g->map, &loaded->map, sizeof(Map)) == 0 && memcmp(g->forest_cache, loaded->forest_cache, sizeof(g->forest_cache)) == 0 && memcmp(g->mountain_cache, loaded->mountain_cache, sizeof(g->mountain_cache)) == 0);
    remove("saves/savegame_99163.json");
}

void test_travel_direction(void) {
    printf("Travel direction continuity:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("direction fixtures allocate successfully", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    static const unsigned seeds[] = {3, 17, 43, 809, 2026};
    for (int i = 0; i < 5; i++) {
        outdoor_travel(g, 0, seeds[i]);
        outdoor_travel(g, 1, seeds[i]);
    }
    game_init(g);
    door_round_trip(g, TOWN_TAVERN_DOOR_X, TOWN_TAVERN_DOOR_Y, LOCATION_TAVERN);
    for (const TownBuilding *building = town_life_building(LOCATION_TOWN); building; building = town_life_next_building(LOCATION_TOWN, building)) {
        door_round_trip(g, building->x + building->w / 2, building->y + building->h - 1, building->interior);
    }
    ASSERT("direction save slot is unused", !save_exists(DIRECTION_SLOT));
    if (!save_exists(DIRECTION_SLOT)) {
        saved_entries(g, loaded);
    }
    free(g);
    free(loaded);
}
