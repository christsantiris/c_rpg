#include "expedition_fixture.h"
#include "game/giants_crown.h"
#include "game/boss_gates.h"
#include "approach_checks.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

static void approach_town(GameState *g, const TownApproach *route) {
    game_init(g);
    g->dawnmere_liberated = 1;
    g->location = route->town;
    if (town_road_index(route->town) >= 0) {
        g->last_town_visited = route->town;
    }
    if (route->town == LOCATION_TOWN2) {
        map_generate_town2(&g->map, &g->player.x, &g->player.y);
    } else if (route->town == LOCATION_TOWN3) {
        map_generate_town3(&g->map, &g->player.x, &g->player.y);
    } else if (route->town == LOCATION_TOWN4) {
        map_generate_town4(&g->map, &g->player.x, &g->player.y);
    } else if (route->town == LOCATION_DAWNMERE) {
        map_generate_dawnmere(&g->map, &g->player.x, &g->player.y);
    } else if (route->town == LOCATION_CASTLE) {
        map_generate_castle(&g->map, &g->player.x, &g->player.y);
    }
    g->player.x = route->gate_x == 0 ? 1 : route->gate_x == TOWN_W - 1 ? TOWN_W - 2 : route->gate_x;
    g->player.y = route->gate_y == 0 ? 1 : route->gate_y == TOWN_H - 1 ? TOWN_H - 2 : route->gate_y;
}

static int legacy_approach_save(int slot, int version) {
    char path[64];
    snprintf(path, sizeof(path), "saves/savegame_%d.json", slot);
    FILE *file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = malloc((size_t)size + 1);
    if (!data) {
        fclose(file);
        return 0;
    }
    size_t count = fread(data, 1, (size_t)size, file);
    data[count] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    test_mark_legacy_portal(root);
    if (version < 176) {
        cJSON_DeleteItemFromObject(root, "labyrinth_entry_town");
    }
    if (version < 167) {
        cJSON_DeleteItemFromObject(root, "dungeon_entry_town");
        cJSON_DeleteItemFromObject(root, "dungeon_portal_town");
    }
    if (version < 160) {
        cJSON_DeleteItemFromObject(root, "approach_route");
    }
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!data) {
        return 0;
    }
    file = fopen(path, "wb");
    int ok = file && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

static LevelCache *area_cache(GameState *g, Location area, int reverse) {
    switch (area) {
        case LOCATION_DUNGEON: return &g->level_cache[0];
        case LOCATION_FOREST: return &g->forest_cache[0];
        case LOCATION_MOUNTAINS: return &g->mountain_cache[0];
        case LOCATION_SWAMP: return &g->swamp_cache[0];
        case LOCATION_COAST: return &g->coast_cache[0];
        case LOCATION_DRAGONSPINE: return &g->dragonspine_cache[0];
        case LOCATION_FROSTFELL: return &g->frostfell_cache[0];
        case LOCATION_DESERT: return &g->desert_cache[0];
        case LOCATION_ASHEN: return &g->ashen_cache[0];
        case LOCATION_GLASSDEEP: return &g->glassdeep_cache[0];
        case LOCATION_CATACOMBS: return &g->catacombs_cache[0];
        default: return NULL;
    }
}

static void retreat_from_area(GameState *g, const TownApproach *route, int reverse) {
    if (route->area == LOCATION_DUNGEON || route->area == LOCATION_CATACOMBS) {
        g->player.x = g->map.stairs_up_x;
        g->player.y = g->map.stairs_up_y;
        action_resolve_player(g, (Action){ACTION_ASCEND, 0, 0});
        return;
    }
    int x = reverse ? g->map.stairs_down_x : g->map.stairs_up_x;
    int y = reverse ? g->map.stairs_down_y : g->map.stairs_up_y;
    if (route->area == LOCATION_FOREST || route->area == LOCATION_MOUNTAINS || route->area == LOCATION_COAST) {
        if (x == 1) {
            x = 0;
        } else if (x == MAP_W - 2) {
            x = MAP_W - 1;
        } else if (y == 1) {
            y = 0;
        } else {
            y = MAP_H - 1;
        }
    }
    action_resolve_player(g, (Action){ACTION_MOVE, x, y});
}

static void test_mountain_spires(GameState *g, GameState *loaded, int slot) {
    approach_town(g, &town_approaches[2]);
    approach_enter(g, 0, 12);
    int door_x = g->map.stairs_down_x;
    int door_y = g->map.stairs_down_y;
    int solid = 1;
    for (int row = 1 - APPROACH_MOUNTAIN_ROWS; row <= 0; row++) {
        int radius = row < -6 ? 1 : row < -4 ? 2 : 3;
        for (int side = -radius; side <= radius; side++) {
            if ((row || side) && map_is_walkable(&g->map, door_x + side, door_y + row)) {
                solid = 0;
            }
        }
    }
    ASSERT("the taller Goblin Mountains block their tapered nine-row footprint", solid && APPROACH_MOUNTAIN_ROWS == 9 && g->map.tiles[door_y][door_x] == TILE_APPROACH_AREA && map_is_walkable(&g->map, door_x - 3, door_y - 8));
    // Version 217 mountains were six rows tall, so their upper rows were open grass.
    for (int row = -8; row <= -6; row++) {
        for (int side = -3; side <= 3; side++) {
            g->map.tiles[door_y + row][door_x + side] = TILE_TOWN_FLOOR;
        }
    }
    g->player.x = door_x;
    g->player.y = door_y - 7;
    map_mark_explored(&g->map, g->player.x, g->player.y);
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = door_x + 1, .y = door_y - 7, .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_health_potion()};
    g->map.tiles[door_y - 7][door_x + 1] = TILE_ITEM;
    g->gold = 393;
    int ok = save_game(g, slot) && legacy_approach_save(slot, 217) && load_game(loaded, slot);
    ASSERT("older mountain approaches move players and loot off the raised spires", ok && loaded->approach_route == 2 && map_is_walkable(&loaded->map, loaded->player.x, loaded->player.y) && !map_is_walkable(&loaded->map, door_x, door_y - 7) && loaded->floor_items[0].active && loaded->map.tiles[loaded->floor_items[0].y][loaded->floor_items[0].x] == TILE_ITEM && loaded->gold == 393);
    walk_town_approach(loaded);
    ASSERT("the migrated mountain road still enters the first level of the climb", loaded->location == LOCATION_MOUNTAINS && loaded->level == 1);
    remove("saves/savegame_99160.json");
}

static void test_transition_loot(GameState *g, GameState *loaded, int slot) {
    const int routes[] = {0, 4, 2, 11, 1};
    for (int i = 0; i < 5; i++) {
        for (int end = 0; end < 3; end++) {
            const TownApproach *route = &town_approaches[routes[i]];
            if (route->unplaced) {
                continue;
            }
            if (end == 2 && route->area == LOCATION_DUNGEON) {
                continue;
            }
            game_init(g);
            approach_enter_region(g, routes[i]);
            int x = end ? g->map.stairs_down_x : g->map.stairs_up_x;
            int y = end ? g->map.stairs_down_y : g->map.stairs_up_y;
            if (end == 2) {
                g->defeated_bosses |= 1 << route->area;
                boss_gate_refresh(g);
                int open = route->area == LOCATION_FOREST ? approach_forest_town_exit(g, &x, &y) : approach_mountain_town_exit(g, &x, &y);
                ASSERT("the boss opens a physical onward exit for the loot test", open);
            }
            TileType tile = g->map.tiles[y][x];
            g->player.x = x;
            g->player.y = y;
            int item = g->inventory_count;
            g->inventory[g->inventory_count++] = item_make_health_potion();
            action_resolve_player(g, (Action){ACTION_DROP_ITEM, item, 0});
            ASSERT("dropped loot retains a visible transition marker", g->floor_item_count == 1 && g->map.tiles[y][x] == tile && g->floor_items[0].underlying_tile == tile);
            if (route->area == LOCATION_FOREST) {
                approach_refresh_forest(g);
            } else {
                approach_refresh_entrance(g);
            }
            boss_gate_refresh(g);
            ASSERT("entrance shaping and boss gates preserve loot-covered transitions", g->map.tiles[y][x] == tile && g->floor_items[0].active);
            ASSERT("a fresh V2 save retains loot and its working transition", save_game(g, slot) && load_game(loaded, slot) && loaded->map.tiles[y][x] == tile && loaded->floor_items[0].active);
            if (end == 1) {
                loaded->player.y = y + 1;
            } else if (x == 0 || x == APPROACH_W - 1) {
                loaded->player.x = x == 0 ? 1 : x - 1;
            } else {
                loaded->player.y = y == 0 ? 1 : y - 1;
            }
            action_resolve_player(loaded, (Action){ACTION_MOVE, x, y});
            ASSERT("walking onto a loot-covered transition still enters its actual destination", end == 2 && route->area == LOCATION_MOUNTAINS ? approach_sunscar_road(loaded) :
                loaded->location == (end == 1 ? route->area : LOCATION_TOWN_ROAD));
        }
    }
}

void test_approaches(void) {
    printf("Safe town approach paths:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("approach fixtures allocate successfully", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    const int slot = 99160;
    ASSERT("approach save test slot is unused", !save_exists(slot));
    if (save_exists(slot)) {
        free(g);
        free(loaded);
        return;
    }
    ASSERT("new routes append after reserved connection IDs", town_approach_count == APPROACH_COAST_ROAD + 1);
    for (int unlocked = 0; unlocked < 2; unlocked++) {
        for (int i = 0; i < town_approach_count; i++) {
            const TownApproach *route = &town_approaches[i];
            if ((int)route->area < 0 || route->unplaced) {
                continue;
            }
            // Shared and branching Act 4 roads have their own actual-travel tests.
            if (i == APPROACH_BLACKWATER_ROAD || route->area == LOCATION_GIANTS_CROWN || route->area == LOCATION_IRONFANG || route->area == LOCATION_STORMWATCH || route->area == LOCATION_ELDERWOOD || route->area == LOCATION_CATACOMBS) {
                continue;
            }
            approach_town(g, route);
            g->gold = 393;
            g->score = 1234;
            g->elowen_quest_state = 2;
            g->elowen_seals_restored = 7;
            g->defeated_bosses = unlocked ? (1 << LOCATION_FOREST) | (1 << LOCATION_MOUNTAINS) | (1 << LOCATION_SWAMP) : 0;
            g->level_cache[2].valid = 1;
            map_generate(&g->level_cache[2].map, 3);
            Map dungeon = g->level_cache[2].map;
            int px = g->player.x;
            int py = g->player.y;
            int hp = g->player.hp;
            int equipment = g->inventory_count;
            action_resolve_player(g, (Action){ACTION_MOVE, route->gate_x, route->gate_y});
            finish_town_departure(g);
            ASSERT("a normal town departure reaches a separate path before any enemies spawn", g->location == LOCATION_APPROACH && g->approach_route == i && g->enemy_count == 0 && g->map.room_count == 0);
            int dx = route->gate_x == 0 ? -1 : route->gate_x == TOWN_W - 1 ? 1 : 0;
            int dy = route->gate_y == 0 ? -1 : route->gate_y == TOWN_H - 1 ? 1 : 0;
            int forest_style = route->area == LOCATION_FOREST || approach_check_entrance_horizontal(route);
            int mountain_style = route->area == LOCATION_SWAMP || approach_check_entrance_vertical(route);
            ASSERT("the approach continues in the direction of the town gate", g->player.last_dx == dx && g->player.last_dy == dy && g->map.stairs_down_x - g->map.stairs_up_x == (mountain_style ? 6 : forest_style ? APPROACH_W / 2 - g->map.stairs_up_x : APPROACH_LENGTH * dx) && g->map.stairs_down_y - g->map.stairs_up_y == (forest_style ? -2 : mountain_style ? APPROACH_H / 2 - 1 - g->map.stairs_up_y : APPROACH_LENGTH * dy));
            if (route->area == LOCATION_DUNGEON) {
                ASSERT("the OakHaven dungeon approach uses cobblestones through town grass", g->map.tiles[g->player.y][g->player.x] == TILE_TOWN_PATH && g->map.tiles[g->player.y + 3 * dy][g->player.x + 3] == TILE_TOWN_FLOOR);
                ASSERT("the raised crypt faces a short path down to the town road", !map_is_walkable(&g->map, g->map.stairs_down_x, g->map.stairs_down_y - 1) && !map_is_walkable(&g->map, g->map.stairs_down_x + 1, g->map.stairs_down_y) && !map_is_walkable(&g->map, g->map.stairs_down_x - 1, g->map.stairs_down_y) && g->map.tiles[g->map.stairs_down_y + 1][g->map.stairs_down_x] == TILE_TOWN_PATH);
            } else if (approach_entrance_rows(route->area)) {
                int rows = approach_entrance_rows(route->area);
                int solid = 1;
                for (int row = 1 - rows; row <= 0; row++) {
                    int radius = route->area == LOCATION_MOUNTAINS ? row < -6 ? 1 : row < -4 ? 2 : 3 : 3;
                    for (int side = -radius; side <= radius; side++) {
                        if ((row || side) && map_is_walkable(&g->map, g->map.stairs_down_x + side, g->map.stairs_down_y + row)) {
                            solid = 0;
                        }
                    }
                }
                ASSERT("the themed entrance stands in a clearing beside a cobblestone road", g->map.tiles[g->player.y][g->player.x] == TILE_TOWN_PATH && g->map.tiles[g->map.stairs_down_y + 1][g->map.stairs_down_x] == TILE_TOWN_PATH && g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x - 4] == (route->area == LOCATION_COAST ? TILE_COAST_SEA : TILE_TOWN_FLOOR));
                ASSERT("only the south-facing doorway of the entrance landmark can be entered", solid && g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] == TILE_APPROACH_AREA);
            } else if (route->area != LOCATION_MOUNTAINS && route->area != LOCATION_FOREST && route->area != LOCATION_SWAMP) {
                ASSERT("the far entrance is recessed into the surrounding terrain", !map_is_walkable(&g->map, g->map.stairs_down_x + dx, g->map.stairs_down_y + dy) && !map_is_walkable(&g->map, g->map.stairs_down_x - dy, g->map.stairs_down_y + dx) && !map_is_walkable(&g->map, g->map.stairs_down_x + dy, g->map.stairs_down_y - dx));
            }
            for (int step = 0; step < 5; step++) {
                action_resolve_player(g, (Action){ACTION_MOVE, g->player.x + dx, g->player.y + dy});
                action_resolve_enemies(g);
            }
            ASSERT("walking the approach cannot start combat or alter dungeon, quest, and character progress", g->location == LOCATION_APPROACH && g->enemy_count == 0 && g->player.hp == hp && g->gold == 393 && g->score == 1234 && g->elowen_quest_state == 2 && g->elowen_seals_restored == 7 && memcmp(&dungeon, &g->level_cache[2].map, sizeof(Map)) == 0);
            int x = g->player.x;
            int y = g->player.y;
            map_mark_explored(&g->map, x, y);
            g->floor_item_count = 1;
            g->floor_items[0] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = approach_floor(route->area), .item = item_make_health_potion()};
            g->map.tiles[y][x] = TILE_ITEM;
            int ok = save_game(g, slot) && load_game(loaded, slot);
            ASSERT("an approach save restores its route, position, exploration, loot, and existing dungeon cache", ok && loaded->location == LOCATION_APPROACH && loaded->approach_route == i && loaded->player.x == x && loaded->player.y == y && loaded->inventory_count == equipment && loaded->floor_items[0].active && loaded->map.tiles[y][x] == TILE_ITEM && map_is_explored(&loaded->map, x, y) && memcmp(&dungeon, &loaded->level_cache[2].map, sizeof(Map)) == 0);
            int steps = 0;
            while (loaded->location == LOCATION_APPROACH && steps++ < APPROACH_LENGTH) {
                action_resolve_player(loaded, (Action){ACTION_MOVE, loaded->player.x - dx, loaded->player.y - dy});
            }
            walk_into_arrived_town(loaded);
            ASSERT("turning back through the path restores the original town gate and preserves the dungeon cache", loaded->location == route->town && loaded->player.x == px && loaded->player.y == py && loaded->level_cache[2].valid && memcmp(&dungeon, &loaded->level_cache[2].map, sizeof(Map)) == 0);
            walk_town_approach(g);
            int reverse = 0;
            ASSERT("only crossing the far entrance loads the intended combat area and town end", g->location == route->area && g->level == (reverse ? 7 : 1) && g->map.room_count > 0 && g->enemy_count > 0 && g->approach_route == -1);
            ASSERT("entering the area after the path retains unrelated progress", g->gold == 393 && g->score == 1234 && g->elowen_quest_state == 2 && g->elowen_seals_restored == 7 && g->level_cache[2].valid && memcmp(&dungeon, &g->level_cache[2].map, sizeof(Map)) == 0);
            g->enemies[0].hp = 7;
            Map visited = g->map;
            *loaded = *g;
            game_open_town_portal(loaded);
            ASSERT("Return to Town escapes without walking or leaving a return portal", loaded->location == loaded->last_town_visited && !loaded->portal_active);
            retreat_from_area(g, route, reverse);
            int south = route->area == LOCATION_DUNGEON || route->area == LOCATION_MOUNTAINS || route->area == LOCATION_FOREST || route->area == LOCATION_SWAMP || approach_entrance_rows(route->area);
            int return_dx = south ? 0 : -dx;
            int return_dy = south ? 1 : -dy;
            int facing_dx = route->area == LOCATION_SWAMP ? -1 : return_dx;
            int facing_dy = route->area == LOCATION_SWAMP ? 0 : return_dy;
            ASSERT("every physical area exit returns onto its entrance path and retains the crossing direction", g->location == LOCATION_APPROACH && g->approach_route == i && g->player.x == g->map.stairs_down_x + return_dx && g->player.y == g->map.stairs_down_y + return_dy && g->player.last_dx == facing_dx && g->player.last_dy == facing_dy && g->map.tiles[g->player.y][g->player.x] == approach_floor(route->area) && g->enemy_count == 0);
            LevelCache *cache = area_cache(g, route->area, reverse);
            ASSERT("leaving onto the approach caches the actual combat map and wounded enemies", cache && cache->valid && cache->enemies[0].hp == 7 && memcmp(&visited, &cache->map, sizeof(Map)) == 0);
            ok = save_game(g, slot) && load_game(loaded, slot);
            ASSERT("saving on a return approach restores its direction, position, and combat cache", ok && loaded->location == LOCATION_APPROACH && loaded->approach_route == i && loaded->player.x == g->player.x && loaded->player.y == g->player.y && loaded->player.last_dx == facing_dx && loaded->player.last_dy == facing_dy && area_cache(loaded, route->area, reverse)->enemies[0].hp == 7);
            int health = loaded->player.hp;
            action_resolve_enemies(loaded);
            walk_return_approach(loaded);
            ASSERT("the return path reaches the original town gate without combat or overwriting the area cache", loaded->location == route->town && loaded->player.x == px && loaded->player.y == py && loaded->player.hp == health && memcmp(&visited, &area_cache(loaded, route->area, reverse)->map, sizeof(Map)) == 0);
        }
    }

    for (int i = 0; i < town_approach_count; i++) {
        const TownApproach *route = &town_approaches[i];
        if ((int)route->area < 0) {
            continue;
        }
        if (i == 3 || i >= APPROACH_BLACKWATER_ROAD || i == APPROACH_PEAKS_NORTH_ROAD || route->area == LOCATION_DUNGEON || route->area == LOCATION_MOUNTAINS || route->area == LOCATION_CATACOMBS || !approach_entrance_rows(route->area)) {
            continue;
        }
        approach_town(g, route);
        approach_enter_region(g, i);
        int dx = route->gate_x == 0 ? -1 : route->gate_x == TOWN_W - 1 ? 1 : 0;
        int dy = route->gate_y == 0 ? -1 : route->gate_y == TOWN_H - 1 ? 1 : 0;
        int tx = g->map.stairs_up_x;
        int ty = g->map.stairs_up_y;
        int door_x = g->map.stairs_down_x;
        int door_y = g->map.stairs_down_y;
        // Version 173 approaches were three-tile corridors ending in an arch.
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                g->map.tiles[y][x] = TILE_FOREST_WALL;
            }
        }
        for (int step = 0; step <= APPROACH_LENGTH; step++) {
            for (int side = -1; side <= 1; side++) {
                g->map.tiles[ty + step * dy + side * dx][tx + step * dx - side * dy] = TILE_FOREST_FLOOR;
            }
        }
        g->map.stairs_down_x = tx + APPROACH_LENGTH * dx;
        g->map.stairs_down_y = ty + APPROACH_LENGTH * dy;
        g->map.tiles[ty][tx] = TILE_APPROACH_TOWN;
        g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] = TILE_APPROACH_AREA;
        g->player.x = tx + 6 * dx - dy;
        g->player.y = ty + 6 * dy + dx;
        map_mark_explored(&g->map, g->player.x, g->player.y);
        int loot_x = tx + 12 * dx + dy;
        int loot_y = ty + 12 * dy - dx;
        g->floor_item_count = 1;
        g->floor_items[0] = (FloorItem){.active = 1, .x = loot_x, .y = loot_y, .underlying_tile = TILE_FOREST_FLOOR, .item = item_make_health_potion()};
        g->map.tiles[loot_y][loot_x] = TILE_ITEM;
        g->gold = 393;
        int migrated = save_game(g, slot) && legacy_approach_save(slot, 174) && load_game(loaded, slot);
        ASSERT("saved corridor approaches adopt their themed entrance landmark", migrated && loaded->approach_route == (i == 7 ? APPROACH_QUARRY_ROAD : i) && loaded->map.stairs_down_x == door_x && loaded->map.stairs_down_y == door_y && loaded->map.tiles[door_y][door_x] == TILE_APPROACH_AREA && loaded->map.tiles[door_y + 1][door_x] == TILE_TOWN_PATH && loaded->map.tiles[loaded->map.stairs_up_y][loaded->map.stairs_up_x] == TILE_APPROACH_TOWN);
        ASSERT("landmark migration preserves position, facing, exploration, loot and gold", migrated && loaded->player.x == g->player.x && loaded->player.y == g->player.y && loaded->player.last_dx == dx && loaded->player.last_dy == dy && map_is_explored(&loaded->map, g->player.x, g->player.y) && loaded->floor_items[0].active && loaded->map.tiles[loot_y][loot_x] == TILE_ITEM && loaded->floor_items[0].underlying_tile == TILE_TOWN_FLOOR && loaded->gold == 393);
        if (migrated) {
            walk_town_approach(loaded);
            ASSERT("a migrated landmark approach still enters its area", loaded->location == route->area);
        }
    }
    remove("saves/savegame_99160.json");
    const TownApproach excluded[] = {
        {LOCATION_TOWN3, LOCATION_CROWNROAD, TOWN_W - 1, TOWN3_CROWN_GATE_Y},
        {LOCATION_TOWN4, LOCATION_CROWNROAD_WEST, 0, TOWN4_CROWN_GATE_Y},
        {LOCATION_CASTLE, LOCATION_CASTLE_INTERIOR, CROWNROAD_X, 10},
        {LOCATION_ESCAPE_TUNNEL, LOCATION_TOWN3, 1, 1}
    };
    for (int i = 0; i < (int)(sizeof(excluded) / sizeof(excluded[0])); i++) {
        game_init(g);
        g->location = excluded[i].town;
        g->map.tiles[excluded[i].gate_y][excluded[i].gate_x] = TILE_TOWN_EXIT;
        ASSERT("castle, escape tunnel, and both Crown Roads bypass approach paths", !approach_enter(g, excluded[i].gate_x, excluded[i].gate_y) && g->location == excluded[i].town);
    }
    int wide_gates = 1;
    for (int i = 0; i < town_approach_count; i++) {
        const TownApproach *route = &town_approaches[i];
        if ((int)route->area < 0 || route->unplaced) {
            continue;
        }
        for (int offset = -2; offset <= 2; offset++) {
            approach_town(g, route);
            int x = route->gate_x;
            int y = route->gate_y;
            if (x == 0 || x == TOWN_W - 1) {
                y += offset;
            } else {
                x += offset;
            }
            if (g->map.tiles[y][x] == TILE_TOWN_EXIT) {
                action_resolve_player(g, (Action){ACTION_MOVE, x, y});
                finish_town_departure(g);
                wide_gates &= route->town == LOCATION_CASTLE && route->area == LOCATION_CATACOMBS ?
                    g->location == LOCATION_CROWNROAD_NORTH : g->location == LOCATION_APPROACH && g->approach_route == i;
            }
        }
    }
    ASSERT("every open tile across a wide town gate leads onto its approach", wide_gates);
    ASSERT("Dragonspine is north of Ridgeshire and Ashen is behind the castle", town_approaches[12].area == LOCATION_DRAGONSPINE && town_approaches[12].gate_x == RIDGESHIRE_DRAGON_GATE_X && town_approaches[12].gate_y == 0 && town_approaches[13].area == LOCATION_ASHEN && town_approaches[13].town == LOCATION_CASTLE && town_approaches[13].gate_x == CASTLE_ASHEN_GATE_X && town_approaches[13].gate_y == 0);
    for (int index = 12; index <= 13; index++) {
        const TownApproach *route = &town_approaches[index];
        int dragon = route->area == LOCATION_DRAGONSPINE;
        for (int returning = 0; returning < 2; returning++) {
            approach_town(g, route);
            approach_enter(g, route->gate_x, route->gate_y);
            int old_dx = dragon ? 1 : 0;
            int old_dy = dragon ? 0 : -1;
            int tx = dragon ? 0 : APPROACH_W / 2;
            int ty = dragon ? APPROACH_H / 2 : APPROACH_H - 1;
            memset(&g->map, 0, sizeof(g->map));
            for (int y = 0; y < MAP_H; y++) {
                for (int x = 0; x < MAP_W; x++) {
                    g->map.tiles[y][x] = dragon ? TILE_DRAGON_WALL : TILE_ASHEN_WALL;
                }
            }
            for (int step = 0; step <= APPROACH_LENGTH; step++) {
                int radius = step < APPROACH_LENGTH - 3 ? 1 : 0;
                for (int side = -radius; side <= radius; side++) {
                    g->map.tiles[ty + step * old_dy + side * old_dx][tx + step * old_dx - side * old_dy] = approach_floor(route->area);
                }
            }
            g->map.stairs_up_x = tx;
            g->map.stairs_up_y = ty;
            g->map.stairs_down_x = tx + APPROACH_LENGTH * old_dx;
            g->map.stairs_down_y = ty + APPROACH_LENGTH * old_dy;
            g->map.tiles[ty][tx] = TILE_APPROACH_TOWN;
            g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] = TILE_APPROACH_AREA;
            int distance = returning ? APPROACH_LENGTH - 1 : 6;
            int side = returning ? 0 : 1;
            g->player.x = tx + distance * old_dx - side * old_dy;
            g->player.y = ty + distance * old_dy + side * old_dx;
            g->player.last_dx = returning ? -old_dx : old_dx;
            g->player.last_dy = returning ? -old_dy : old_dy;
            map_mark_explored(&g->map, g->player.x, g->player.y);
            g->floor_item_count = 1;
            g->floor_items[0] = (FloorItem){.active = 1, .x = tx + 5 * old_dx + old_dy, .y = ty + 5 * old_dy - old_dx, .underlying_tile = approach_floor(route->area), .item = item_make_health_potion()};
            g->map.tiles[g->floor_items[0].y][g->floor_items[0].x] = TILE_ITEM;
            LevelCache *cache = area_cache(g, route->area, 0);
            cache->valid = 1;
            if (dragon) {
                map_generate_dragonspine(&cache->map, 1);
            } else {
                map_generate_ashen(&cache->map, 1);
            }
            cache->enemy_count = 1;
            cache->enemies[0].active = 1;
            cache->enemies[0].hp = 7;
            g->gold = 393;
            g->watchfire_quest_state = 1;
            g->watchfire_quest_progress = 5;
            int migrated = save_game(g, slot) && legacy_approach_save(slot, 162) && load_game(loaded, slot);
            int dx = 0;
            int dy = -1;
            int nx = dragon ? APPROACH_W / 2 : APPROACH_W - 1;
            int ny = dragon ? APPROACH_H - 1 : APPROACH_H / 2;
            ASSERT("old approaches preserve their region and have a reachable relocated landing", migrated && loaded->approach_route == index &&
                map_is_walkable(&loaded->map, loaded->player.x, loaded->player.y) && map_is_explored(&loaded->map, loaded->player.x, loaded->player.y));
            ASSERT("approach relocation preserves loot, combat caches, money, and quest progress", migrated && loaded->floor_items[0].active &&
                loaded->gold == 393 && loaded->watchfire_quest_progress == 5 && area_cache(loaded, route->area, 0)->enemies[0].hp == 7 &&
                memcmp(cache->map.tiles, area_cache(loaded, route->area, 0)->map.tiles, sizeof(cache->map.tiles)) == 0);
            int px = loaded->player.x;
            int py = loaded->player.y;
            ASSERT("relocated approaches save again without rotating a second time", save_game(loaded, slot) && load_game(g, slot) && g->player.x == px && g->player.y == py && g->approach_route == index);
            if (returning) {
                walk_return_approach(g);
                ASSERT("migrated return paths reach the new Ridgeshire gate", g->location == route->town && g->player.x == (dragon ? RIDGESHIRE_DRAGON_GATE_X : CASTLE_ASHEN_GATE_X) && g->player.y == 1);
            } else {
                walk_town_approach(g);
                ASSERT("migrated outbound paths still enter the original combat region", g->location == route->area);
            }
        }
        if (!dragon) {
            continue;
        }
        approach_town(g, route);
        if (dragon) {
            game_enter_dragonspine(g);
        } else {
            game_enter_ashen(g);
        }
        g->enemies[0].hp = 7;
        game_open_town_portal(g);
        int x = dragon ? RIDGESHIRE_DRAGON_PORTAL_X : ROSEMOOR_ASHEN_PORTAL_X;
        int y = dragon ? RIDGESHIRE_DRAGON_PORTAL_Y : ROSEMOOR_ASHEN_PORTAL_Y;
        int old_x = dragon ? TOWN_W - 3 : 21;
        int old_y = dragon ? 13 : 2;
        g->floor_item_count = 2;
        g->floor_items[0] = (FloorItem){.active = 1, .x = old_x, .y = old_y, .underlying_tile = TILE_PORTAL, .item = item_make_health_potion()};
        g->floor_items[1] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_health_potion()};
        g->map.tiles[old_y][old_x] = TILE_ITEM;
        g->map.tiles[y][x] = TILE_ITEM;
        int migrated = save_game(g, slot) && legacy_approach_save(slot, 162) && load_game(loaded, slot);
        ASSERT("legacy Ridgeshire portals retire while preserving loot at both former positions", migrated && loaded->location == LOCATION_TOWN4 && !loaded->portal_active && loaded->portal_location == route->area && loaded->map.tiles[y][x] != TILE_PORTAL && loaded->map.tiles[old_y][old_x] == TILE_ITEM && loaded->floor_items[0].active && loaded->floor_items[0].underlying_tile == TILE_TOWN_FLOOR && loaded->floor_items[1].active && loaded->floor_items[1].underlying_tile != TILE_PORTAL);
        action_resolve_player(loaded, (Action){ACTION_MOVE, x, y});
        ASSERT("walking a migrated portal tile stays in Ridgeshire", loaded->location == LOCATION_TOWN4 && !loaded->portal_active);
        test_restore_cached_floor(loaded);
        ASSERT("the archived expedition retains its wounded enemies after migration", loaded->location == route->area && loaded->level == 1 && loaded->player.x == g->portal_x && loaded->player.y == g->portal_y && loaded->enemies[0].hp == 7);
    }
    // The Lich adventure is a cul-de-sac, including after its final victory.
    approach_town(g, &town_approaches[1]);
    action_resolve_player(g, (Action){ACTION_MOVE, 20, TOWN_H - 1});
    walk_town_approach(g);
    for (int level = 1; level < DUNGEON_DEPTH; level++) {
        game_descend(g);
    }
    g->defeated_bosses |= 1 << LOCATION_DUNGEON;
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].type == ENEMY_LICH_KING) {
            g->enemies[i].active = 0;
        }
    }
    game_update_level_progress(g);
    g->player.x = g->map.stairs_down_x;
    g->player.y = g->map.stairs_down_y;
    action_resolve_player(g, (Action){ACTION_ASCEND, 0, 0});
    int x;
    int y;
    ASSERT("Lich victory returns to a local path with no onward town exit", g->location == LOCATION_APPROACH && !approach_dungeon_town_exit(g, &x, &y));
    int exits = 0;
    int through_road = 0;
    for (int row = 0; row < APPROACH_H; row++) {
        for (int col = 0; col < APPROACH_W; col++) {
            exits += g->map.tiles[row][col] == TILE_APPROACH_TOWN;
            through_road |= g->map.tiles[row][col] == TILE_APPROACH_DUNGEON_TOWN;
        }
    }
    ASSERT("the local dungeon retains exactly one retreat to Oakhaven", exits == 1 && !through_road);
    walk_return_approach(g);
    ASSERT("walking home after defeating the Lich reaches Oakhaven's south gate", g->location == LOCATION_TOWN && g->player.x == 20 && g->player.y == TOWN_H - 2);
    test_transition_loot(g, loaded, slot);
    test_mountain_spires(g, loaded, slot);
    remove("saves/savegame_99160.json");
    free(g);
    free(loaded);
}
