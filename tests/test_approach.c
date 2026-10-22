#include "approach_checks.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

static void approach_town(GameState *g, const TownApproach *route) {
    game_init(g);
    g->location = route->town;
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
        case LOCATION_FOREST: return &g->forest_cache[reverse ? FOREST_DEPTH - 1 : 0];
        case LOCATION_MOUNTAINS: return &g->mountain_cache[reverse ? MOUNTAIN_DEPTH - 1 : 0];
        case LOCATION_SWAMP: return &g->swamp_cache[reverse ? SWAMP_DEPTH - 1 : 0];
        case LOCATION_COAST: return &g->coast_cache[0];
        case LOCATION_DRAGONSPINE: return &g->dragonspine_cache[0];
        case LOCATION_FROSTFELL: return &g->frostfell_cache[0];
        case LOCATION_DESERT: return &g->desert_cache[0];
        case LOCATION_MOONVEIL: return &g->moonveil_cache[0];
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
    ASSERT("all sixteen town connections have approach paths", town_approach_count == 16);
    for (int unlocked = 0; unlocked < 2; unlocked++) {
        for (int i = 0; i < town_approach_count; i++) {
            const TownApproach *route = &town_approaches[i];
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
            int has_shortcut = route->area == LOCATION_FOREST || route->area == LOCATION_MOUNTAINS || route->area == LOCATION_SWAMP;
            if (unlocked && has_shortcut) {
                g->dialogue_active = 1;
                snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Choose route");
                snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, "1. Travel to area\n2. Take shortcut");
                g->dialogue_x = route->gate_x;
                g->dialogue_y = route->gate_y;
                int migrated = save_game(g, slot) && legacy_approach_save(slot, 163) && load_game(loaded, slot);
                ASSERT("legacy pending route menus close in town without moving the player or losing boss progress", migrated && loaded->location == route->town && !loaded->dialogue_active && !game_shortcut_prompt_active(loaded) && loaded->player.x == px && loaded->player.y == py && loaded->defeated_bosses == g->defeated_bosses && loaded->gold == 393);
                *g = *loaded;
            }
            action_resolve_player(g, (Action){ACTION_MOVE, route->gate_x, route->gate_y});
            if (unlocked && has_shortcut) {
                ASSERT("unlocked town gates enter the approach without a route menu", !game_shortcut_prompt_active(g) && g->location == LOCATION_APPROACH);
            }
            ASSERT("a normal town departure reaches a separate path before any enemies spawn", g->location == LOCATION_APPROACH && g->approach_route == i && g->enemy_count == 0 && g->map.room_count == 0);
            int dx = route->gate_x == 0 ? -1 : route->gate_x == TOWN_W - 1 ? 1 : 0;
            int dy = route->gate_y == 0 ? -1 : route->gate_y == TOWN_H - 1 ? 1 : 0;
            ASSERT("the approach continues in the direction of the town gate", g->player.last_dx == dx && g->player.last_dy == dy && g->map.stairs_down_x - g->map.stairs_up_x == APPROACH_LENGTH * dx && g->map.stairs_down_y - g->map.stairs_up_y == (route->area == LOCATION_DUNGEON ? -2 : APPROACH_LENGTH * dy));
            if (route->area == LOCATION_DUNGEON) {
                ASSERT("the OakHaven dungeon approach uses cobblestones through town grass", g->map.tiles[g->player.y][g->player.x] == TILE_TOWN_PATH && g->map.tiles[g->player.y + 3][g->player.x + 3 * dx] == TILE_TOWN_FLOOR);
                ASSERT("the raised crypt faces a short path down to the town road", !map_is_walkable(&g->map, g->map.stairs_down_x, g->map.stairs_down_y - 1) && !map_is_walkable(&g->map, g->map.stairs_down_x + 1, g->map.stairs_down_y) && !map_is_walkable(&g->map, g->map.stairs_down_x - 1, g->map.stairs_down_y) && g->map.tiles[g->map.stairs_down_y + 1][g->map.stairs_down_x] == TILE_TOWN_PATH);
            } else {
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
            if (unlocked && has_shortcut) {
                int fx;
                int fy;
                approach_shortcut_exit(g, &fx, &fy);
                int sx = dy ? 1 : 0;
                int sy = dx ? -1 : 0;
                for (int step = 0; step < APPROACH_BRANCH_LENGTH; step++) {
                    g->map.tiles[fy - step * sy][fx - step * sx] = route->area == LOCATION_FOREST ? TILE_FOREST_WALL : route->area == LOCATION_SWAMP ? TILE_SWAMP_WALL : TILE_MOUNTAIN_WALL;
                }
                int migrated = save_game(g, slot) && legacy_approach_save(slot, 163) && load_game(loaded, slot);
                ASSERT("old unlocked approaches gain their physical fork without losing position, exploration, loot, or combat progress", migrated && loaded->map.tiles[fy][fx] == TILE_APPROACH_SHORTCUT && loaded->player.x == x && loaded->player.y == y && loaded->approach_route == i && loaded->map.tiles[y][x] == TILE_ITEM && loaded->floor_items[0].active && map_is_explored(&loaded->map, x, y) && loaded->gold == 393 && memcmp(dungeon.tiles, loaded->level_cache[2].map.tiles, sizeof(dungeon.tiles)) == 0);
                approach_refresh_fork(g);
            }
            int ok = save_game(g, slot) && load_game(loaded, slot);
            ASSERT("an approach save restores its route, position, exploration, loot, and existing dungeon cache", ok && loaded->location == LOCATION_APPROACH && loaded->approach_route == i && loaded->player.x == x && loaded->player.y == y && loaded->inventory_count == equipment && loaded->floor_items[0].active && loaded->map.tiles[y][x] == TILE_ITEM && map_is_explored(&loaded->map, x, y) && memcmp(&dungeon, &loaded->level_cache[2].map, sizeof(Map)) == 0);
            int steps = 0;
            while (loaded->location == LOCATION_APPROACH && steps++ < APPROACH_LENGTH) {
                action_resolve_player(loaded, (Action){ACTION_MOVE, loaded->player.x - dx, loaded->player.y - dy});
            }
            ASSERT("turning back through the path restores the original town gate and preserves the dungeon cache", loaded->location == route->town && loaded->player.x == px && loaded->player.y == py && loaded->level_cache[2].valid && memcmp(&dungeon, &loaded->level_cache[2].map, sizeof(Map)) == 0);
            walk_town_approach(g);
            int reverse = (route->area == LOCATION_FOREST && route->town == LOCATION_TOWN2) || (route->area == LOCATION_MOUNTAINS && route->town == LOCATION_TOWN4) || (route->area == LOCATION_SWAMP && route->town == LOCATION_TOWN3);
            ASSERT("only crossing the far entrance loads the intended combat area and town end", g->location == route->area && g->level == (reverse ? 7 : 1) && g->map.room_count > 0 && g->enemy_count > 0 && g->approach_route == -1);
            ASSERT("entering the area after the path retains unrelated progress", g->gold == 393 && g->score == 1234 && g->elowen_quest_state == 2 && g->elowen_seals_restored == 7 && g->level_cache[2].valid && memcmp(&dungeon, &g->level_cache[2].map, sizeof(Map)) == 0);
            g->enemies[0].hp = 7;
            Map visited = g->map;
            *loaded = *g;
            game_open_town_portal(loaded);
            ASSERT("Return to Town still bypasses the walking path and preserves its portal", loaded->location == (route->area == LOCATION_CATACOMBS ? LOCATION_TOWN3 : route->town) && loaded->portal_active);
            retreat_from_area(g, route, reverse);
            int return_dx = route->area == LOCATION_DUNGEON ? 0 : -dx;
            int return_dy = route->area == LOCATION_DUNGEON ? 1 : -dy;
            ASSERT("every physical area exit returns onto its entrance path facing away from the area", g->location == LOCATION_APPROACH && g->approach_route == i && g->player.x == g->map.stairs_down_x + return_dx && g->player.y == g->map.stairs_down_y + return_dy && g->player.last_dx == return_dx && g->player.last_dy == return_dy && g->map.tiles[g->player.y][g->player.x] == approach_floor(route->area) && g->enemy_count == 0);
            LevelCache *cache = area_cache(g, route->area, reverse);
            ASSERT("leaving onto the approach caches the actual combat map and wounded enemies", cache && cache->valid && cache->enemies[0].hp == 7 && memcmp(&visited, &cache->map, sizeof(Map)) == 0);
            ok = save_game(g, slot) && load_game(loaded, slot);
            ASSERT("saving on a return approach restores its direction, position, and combat cache", ok && loaded->location == LOCATION_APPROACH && loaded->approach_route == i && loaded->player.x == g->player.x && loaded->player.y == g->player.y && loaded->player.last_dx == return_dx && loaded->player.last_dy == return_dy && area_cache(loaded, route->area, reverse)->enemies[0].hp == 7);
            int health = loaded->player.hp;
            action_resolve_enemies(loaded);
            walk_return_approach(loaded);
            ASSERT("the return path reaches the original town gate without combat or overwriting the area cache", loaded->location == route->town && loaded->player.x == px && loaded->player.y == py && loaded->player.hp == health && memcmp(&visited, &area_cache(loaded, route->area, reverse)->map, sizeof(Map)) == 0);
        }
    }
    const TownApproach excluded[] = {
        {LOCATION_TOWN3, LOCATION_CROWNROAD, TOWN_W - 1, TOWN3_KING_GATE_Y},
        {LOCATION_TOWN4, LOCATION_KING_ROAD_WEST, 0, TOWN4_KING_GATE_Y},
        {LOCATION_CASTLE, LOCATION_CASTLE_INTERIOR, 20, 0},
        {LOCATION_TOWN2, LOCATION_LABYRINTH, TOWN_LABYRINTH_X, TOWN_LABYRINTH_Y},
        {LOCATION_ESCAPE_TUNNEL, LOCATION_TOWN3, 1, 1}
    };
    for (int i = 0; i < (int)(sizeof(excluded) / sizeof(excluded[0])); i++) {
        game_init(g);
        g->location = excluded[i].town;
        g->map.tiles[excluded[i].gate_y][excluded[i].gate_x] = TILE_TOWN_EXIT;
        ASSERT("castle, labyrinth, escape tunnel, and both Crown Roads bypass approach paths", !approach_enter(g, excluded[i].gate_x, excluded[i].gate_y) && g->location == excluded[i].town);
    }
    int wide_gates = 1;
    for (int i = 0; i < town_approach_count; i++) {
        const TownApproach *route = &town_approaches[i];
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
                wide_gates &= g->location == LOCATION_APPROACH && g->approach_route == i;
            }
        }
    }
    ASSERT("every open tile across a wide town gate leads onto its approach", wide_gates);
    const int road_routes[] = {0, 4, 5, 9, 2, 11};
    for (int r = 0; r < 6; r++) {
        const TownApproach *route = &town_approaches[road_routes[r]];
        for (int boss = 0; boss < 2; boss++) {
            approach_town(g, route);
            g->defeated_bosses = 1 << route->area;
            if (route->area == LOCATION_FOREST) {
                game_enter_forest_road(g);
                if (boss) {
                    g->player.x = FOREST_ROAD_FORK_X;
                    g->player.y = FOREST_ROAD_BOSS_Y + 1;
                }
            } else if (route->area == LOCATION_SWAMP) {
                game_enter_swamp_road(g);
                if (boss) {
                    g->player.x = SWAMP_ROAD_BOSS_X - 1;
                    g->player.y = SWAMP_ROAD_FORK_Y;
                }
            } else {
                game_enter_high_pass(g, route->town == LOCATION_TOWN);
                if (boss) {
                    g->player.x = HIGH_PASS_BOSS_X - 1;
                    g->player.y = HIGH_PASS_FORK_Y;
                }
            }
            map_mark_explored(&g->map, g->player.x, g->player.y);
            g->floor_item_count = 1;
            g->floor_items[0] = (FloorItem){.active = 1, .x = g->player.x, .y = g->player.y, .underlying_tile = g->map.tiles[g->player.y][g->player.x], .item = item_make_health_potion()};
            g->map.tiles[g->player.y][g->player.x] = TILE_ITEM;
            g->gold = 393;
            g->player.hp = 87;
            int migrated = save_game(g, slot) && legacy_approach_save(slot, 164) && load_game(loaded, slot);
            ASSERT("old connecting-road saves move safely onto the correct continuous crossroads", migrated && loaded->location == LOCATION_APPROACH && loaded->approach_route == road_routes[r] && map_is_walkable(&loaded->map, loaded->player.x, loaded->player.y) && map_is_explored(&loaded->map, loaded->player.x, loaded->player.y) && loaded->gold == 393 && loaded->player.hp == 87 && loaded->defeated_bosses == g->defeated_bosses);
            ASSERT("connecting-road migration preserves dropped items on the new path", migrated && loaded->floor_items[0].active && loaded->map.tiles[loaded->floor_items[0].y][loaded->floor_items[0].x] == TILE_ITEM && loaded->floor_items[0].item.type == ITEM_POTION_HEALTH);
            walk_return_approach(loaded);
            ASSERT("a migrated road still reaches its original town", loaded->location == route->town);
        }
    }
    ASSERT("Dragonspine is north and Ashen Hollow is east of Ridgeshire", town_approaches[12].area == LOCATION_DRAGONSPINE && town_approaches[12].gate_x == RIDGESHIRE_DRAGON_GATE_X && town_approaches[12].gate_y == 0 && town_approaches[13].area == LOCATION_ASHEN && town_approaches[13].gate_x == TOWN_W - 1 && town_approaches[13].gate_y == RIDGESHIRE_ASHEN_GATE_Y);
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
            int dx = dragon ? 0 : 1;
            int dy = dragon ? -1 : 0;
            int nx = dragon ? APPROACH_W / 2 : 0;
            int ny = dragon ? APPROACH_H - 1 : APPROACH_H / 2;
            ASSERT("old Ridgeshire approaches preserve route, distance, side, facing, and exploration after relocation", migrated && loaded->approach_route == index && loaded->player.x == nx + distance * dx - side * dy && loaded->player.y == ny + distance * dy + side * dx && loaded->player.last_dx == (returning ? -dx : dx) && loaded->player.last_dy == (returning ? -dy : dy) && map_is_explored(&loaded->map, loaded->player.x, loaded->player.y));
            ASSERT("approach relocation preserves loot, combat caches, money, and quest progress", migrated && loaded->floor_items[0].active && loaded->floor_items[0].x == nx + 5 * dx + dy && loaded->floor_items[0].y == ny + 5 * dy - dx && loaded->map.tiles[ny + 5 * dy - dx][nx + 5 * dx + dy] == TILE_ITEM && loaded->floor_items[0].underlying_tile == approach_floor(route->area) && loaded->gold == 393 && loaded->watchfire_quest_progress == 5 && area_cache(loaded, route->area, 0)->enemies[0].hp == 7 && memcmp(cache->map.tiles, area_cache(loaded, route->area, 0)->map.tiles, sizeof(cache->map.tiles)) == 0);
            int px = loaded->player.x;
            int py = loaded->player.y;
            ASSERT("relocated approaches save again without rotating a second time", save_game(loaded, slot) && load_game(g, slot) && g->player.x == px && g->player.y == py && g->approach_route == index);
            if (returning) {
                walk_return_approach(g);
                ASSERT("migrated return paths reach the new Ridgeshire gate", g->location == LOCATION_TOWN4 && g->player.x == (dragon ? RIDGESHIRE_DRAGON_GATE_X : TOWN_W - 2) && g->player.y == (dragon ? 1 : RIDGESHIRE_ASHEN_GATE_Y));
            } else {
                walk_town_approach(g);
                ASSERT("migrated outbound paths still enter the original combat region", g->location == route->area);
            }
        }
        approach_town(g, route);
        if (dragon) {
            game_enter_dragonspine(g);
        } else {
            game_enter_ashen(g);
        }
        g->enemies[0].hp = 7;
        game_open_town_portal(g);
        int x = dragon ? RIDGESHIRE_DRAGON_PORTAL_X : RIDGESHIRE_ASHEN_PORTAL_X;
        int y = dragon ? RIDGESHIRE_DRAGON_PORTAL_Y : RIDGESHIRE_ASHEN_PORTAL_Y;
        int old_x = dragon ? TOWN_W - 3 : 21;
        int old_y = dragon ? 13 : 2;
        g->floor_item_count = 2;
        g->floor_items[0] = (FloorItem){.active = 1, .x = old_x, .y = old_y, .underlying_tile = TILE_PORTAL, .item = item_make_health_potion()};
        g->floor_items[1] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_health_potion()};
        g->map.tiles[old_y][old_x] = TILE_ITEM;
        g->map.tiles[y][x] = TILE_ITEM;
        int migrated = save_game(g, slot) && legacy_approach_save(slot, 162) && load_game(loaded, slot);
        ASSERT("legacy Ridgeshire portals move to their new gate while preserving loot at both positions", migrated && loaded->location == LOCATION_TOWN4 && loaded->portal_active && loaded->portal_location == route->area && loaded->map.tiles[y][x] == TILE_PORTAL && loaded->map.tiles[old_y][old_x] == TILE_ITEM && loaded->floor_items[0].active && loaded->floor_items[0].underlying_tile == TILE_TOWN_FLOOR && loaded->floor_items[1].active && loaded->floor_items[1].underlying_tile == TILE_PORTAL);
        action_resolve_player(loaded, (Action){ACTION_MOVE, x, y});
        ASSERT("a migrated portal resumes its original expedition and wounded enemies", loaded->location == route->area && loaded->level == 1 && loaded->player.x == g->portal_x && loaded->player.y == g->portal_y && loaded->enemies[0].hp == 7);
    }
    const TownApproach *dungeon_route = &town_approaches[1];
    approach_town(g, dungeon_route);
    approach_enter(g, dungeon_route->gate_x, dungeon_route->gate_y);
    g->player.x = g->map.stairs_down_x - 1;
    g->player.last_dx = -1;
    g->gold = 393;
    map_mark_explored(&g->map, g->player.x, g->player.y);
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = g->map.tiles[y][x];
            if (tile == TILE_TOWN_PATH) {
                g->map.tiles[y][x] = TILE_FOREST_FLOOR;
            } else if (tile == TILE_TOWN_FLOOR || tile == TILE_WALL) {
                g->map.tiles[y][x] = TILE_FOREST_WALL;
            }
        }
    }
    int loot_x = g->player.x - 2;
    int loot_y = g->player.y;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = loot_x, .y = loot_y, .underlying_tile = TILE_FOREST_FLOOR, .item = item_make_health_potion()};
    g->map.tiles[loot_y][loot_x] = TILE_ITEM;
    int migrated = save_game(g, slot) && legacy_approach_save(slot, 160) && load_game(loaded, slot);
    ASSERT("old forest-style dungeon approaches migrate to cobblestones and grass without moving the player", migrated && loaded->location == LOCATION_APPROACH && loaded->approach_route == 1 && loaded->player.x == g->player.x && loaded->player.y == g->player.y && loaded->player.last_dx == -1 && loaded->gold == 393 && map_is_explored(&loaded->map, g->player.x, g->player.y) && loaded->map.tiles[g->player.y][g->player.x] == TILE_TOWN_PATH && loaded->map.tiles[loot_y + 3][loot_x] == TILE_TOWN_FLOOR);
    ASSERT("approach migration preserves dropped loot and repairs its paving underlay", migrated && loaded->floor_items[0].active && loaded->map.tiles[loot_y][loot_x] == TILE_ITEM && loaded->floor_items[0].underlying_tile == TILE_TOWN_PATH);
    ASSERT("the migrated dungeon approach survives another save and load", save_game(loaded, slot) && load_game(g, slot) && g->approach_route == 1 && g->player.last_dx == -1 && g->floor_items[0].underlying_tile == TILE_TOWN_PATH && g->map.tiles[loot_y + 3][loot_x] == TILE_TOWN_FLOOR);
    // Version 161 already had grass, but its entrance used a larger wall block.
    g->map.tiles[g->map.stairs_down_y + 2][g->map.stairs_down_x] = TILE_WALL;
    migrated = save_game(g, slot) && legacy_approach_save(slot, 161) && load_game(loaded, slot);
    ASSERT("saved stone-block entrances adopt the crypt footprint without losing position or loot", migrated && loaded->map.tiles[g->map.stairs_down_y + 2][g->map.stairs_down_x] == TILE_TOWN_PATH && loaded->player.x == g->player.x && loaded->player.y == g->player.y && loaded->player.last_dx == -1 && loaded->floor_items[0].active && loaded->map.tiles[loot_y][loot_x] == TILE_ITEM && loaded->floor_items[0].underlying_tile == TILE_TOWN_PATH);
    game_init(g);
    game_enter_dungeon(g);
    g->player.hp = 39;
    g->enemies[0].hp = 7;
    g->gold = 393;
    Map old_dungeon = g->map;
    int ok = save_game(g, slot) && legacy_approach_save(slot, 159) && load_game(loaded, slot);
    ASSERT("pre-approach saves load in their existing area without resetting maps or progress", ok && loaded->location == LOCATION_DUNGEON && loaded->approach_route == -1 && loaded->player.hp == 39 && loaded->gold == 393 && loaded->enemies[0].hp == 7 && loaded->map.room_count == old_dungeon.room_count && memcmp(old_dungeon.tiles, loaded->map.tiles, sizeof(old_dungeon.tiles)) == 0 && memcmp(old_dungeon.explored, loaded->map.explored, sizeof(old_dungeon.explored)) == 0 && memcmp(old_dungeon.rooms, loaded->map.rooms, old_dungeon.room_count * sizeof(Room)) == 0);
    remove("saves/savegame_99160.json");
    for (int origin = 0; origin < 2; origin++) {
        const TownApproach *route = &town_approaches[origin ? 15 : 1];
        approach_town(g, route);
        action_resolve_player(g, (Action){ACTION_MOVE, route->gate_x, route->gate_y});
        int x;
        int y;
        ASSERT("the Dawnmere through-road is hidden while the Lich lives", !approach_dungeon_town_exit(g, &x, &y));
        for (int ty = 0; ty < APPROACH_H; ty++) {
            for (int tx = 0; tx < APPROACH_W; tx++) {
                if (g->map.tiles[ty][tx] == TILE_APPROACH_DUNGEON_TOWN) {
                    ASSERT("locked approaches contain no town crossing marker", 0);
                }
            }
        }
        walk_town_approach(g);
        ASSERT("entering the dungeon from either side always begins at stage one", g->location == LOCATION_DUNGEON && g->level == 1 && g->dungeon_entry_town == route->town && g->map.room_count >= 6);
        int hp = g->player.hp;
        game_open_town_portal(g);
        ASSERT("dungeon portals return to their entrance town", g->location == route->town && g->dungeon_portal_town == route->town && g->portal_active && g->player.hp == hp);
        ASSERT("the matching town renders a portal beside its dungeon gate", g->map.tiles[13][origin ? 2 : TOWN_W - 3] == TILE_PORTAL);
        int saved = save_game(g, slot) && load_game(loaded, slot);
        ASSERT("dungeon entrance and portal towns survive save/load", saved && loaded->dungeon_entry_town == route->town && loaded->dungeon_portal_town == route->town);
        if (saved) {
            game_use_town_portal(loaded);
            ASSERT("using a restored portal preserves forward dungeon progression and entrance side", loaded->location == LOCATION_DUNGEON && loaded->level == 1 && loaded->dungeon_entry_town == route->town && !loaded->portal_active);
            retreat_from_area(loaded, route, 0);
            walk_return_approach(loaded);
            ASSERT("retreat from stage one returns to the entrance town", loaded->location == route->town);
        }
        for (int east = 0; east < 2; east++) {
            approach_town(g, route);
            game_enter_dungeon(g);
            for (int depth = 1; depth < DUNGEON_DEPTH; depth++) {
                game_descend(g);
            }
            g->defeated_bosses |= 1 << LOCATION_DUNGEON;
            for (int enemy = 0; enemy < g->enemy_count; enemy++) {
                if (g->enemies[enemy].type == ENEMY_LICH_KING) {
                    g->enemies[enemy].active = 0;
                }
            }
            game_update_level_progress(g);
            g->player.x = g->map.stairs_down_x;
            g->player.y = g->map.stairs_down_y;
            action_resolve_player(g, (Action){ACTION_ASCEND, 0, 0});
            ASSERT("the defeated Lich's stairs emerge on the same safe east-west road", g->location == LOCATION_APPROACH && g->approach_route == (origin ? 15 : 1) && g->player.y == g->map.stairs_down_y + 2 && approach_dungeon_town_exit(g, &x, &y));
            int wide_block = 0;
            for (int py = 1; py < APPROACH_H - 1; py++) {
                for (int px = 1; px < APPROACH_W - 1; px++) {
                    if (g->map.tiles[py][px] == TILE_TOWN_PATH && g->map.tiles[py][px + 1] == TILE_TOWN_PATH &&
                        g->map.tiles[py + 1][px] == TILE_TOWN_PATH && g->map.tiles[py + 1][px + 1] == TILE_TOWN_PATH) {
                        wide_block = 1;
                    }
                }
            }
            ASSERT("the connecting road is one tile wide and aligned with both gates", !wide_block && g->map.tiles[y][32] == TILE_TOWN_PATH && g->map.tiles[y - 1][32] == TILE_TOWN_FLOOR && g->map.tiles[y + 1][32] == TILE_TOWN_FLOOR && g->player.y == y);
            int straight = 1;
            for (int px = 1; px < APPROACH_W - 1; px++) {
                if (g->map.tiles[y][px] != TILE_TOWN_PATH) {
                    straight = 0;
                }
            }
            ASSERT("every tile between the town gates lies on one straight paved road", straight);
            int end_x = east ? APPROACH_W - 2 : 1;
            for (int step = 0; g->location == LOCATION_APPROACH && g->player.x != end_x && step < APPROACH_W; step++) {
                action_resolve_player(g, (Action){ACTION_MOVE, g->player.x + (east ? 1 : -1), g->player.y});
            }
            action_resolve_player(g, (Action){ACTION_MOVE, east ? APPROACH_W - 1 : 0, y});
            ASSERT("walking east reaches Dawnmere and walking west reaches Oakhaven without a prompt", g->location == (east ? LOCATION_DAWNMERE : LOCATION_TOWN) && !g->dialogue_active);
            ASSERT("town crossing preserves the dungeon cache and Lich victory", g->level_cache[DUNGEON_DEPTH - 1].valid && (g->defeated_bosses & (1 << LOCATION_DUNGEON)));
            ASSERT("Dawnmere retains its merchant footprint and the removed hall lot is walkable", g->location != LOCATION_DAWNMERE || (g->map.tiles[12][1] == TILE_TOWN_PATH && map_is_walkable(&g->map, 20, 12) && !map_is_walkable(&g->map, 7, 7) && map_is_walkable(&g->map, 26, 6)));
        }
    }
    approach_town(g, &town_approaches[1]);
    approach_enter(g, TOWN_W - 1, 12);
    int legacy_x = g->player.x;
    int legacy_y = g->player.y;
    map_mark_explored(&g->map, legacy_x, legacy_y);
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = legacy_x, .y = legacy_y, .underlying_tile = TILE_TOWN_PATH, .item = item_make_greatsword()};
    g->map.tiles[legacy_y][legacy_x] = TILE_ITEM;
    g->defeated_bosses |= 1 << LOCATION_DUNGEON;
    g->gold = 393;
    int legacy_saved = save_game(g, slot) && legacy_approach_save(slot, 166) && load_game(loaded, slot);
    int crossing_x;
    int crossing_y;
    ASSERT("older completed dungeon saves gain the Dawnmere road without replaying the Lich", legacy_saved && approach_dungeon_town_exit(loaded, &crossing_x, &crossing_y) && loaded->map.tiles[crossing_y][crossing_x] == TILE_APPROACH_DUNGEON_TOWN);
    ASSERT("road migration preserves player, exploration, loot and gold and defaults old entrance towns to Oakhaven", legacy_saved && loaded->player.x == legacy_x && loaded->player.y == legacy_y && map_is_explored(&loaded->map, legacy_x, legacy_y) && loaded->floor_items[0].active && loaded->floor_items[0].item.type == ITEM_WEAPON && loaded->map.tiles[legacy_y][legacy_x] == TILE_ITEM && loaded->gold == 393 && loaded->dungeon_entry_town == LOCATION_TOWN && loaded->dungeon_portal_town == LOCATION_TOWN);
    approach_town(g, &town_approaches[1]);
    g->defeated_bosses |= 1 << LOCATION_DUNGEON;
    approach_enter(g, TOWN_W - 1, 12);
    for (int py = 25; py <= 27; py++) {
        for (int px = 1; px < APPROACH_W - 1; px++) {
            g->map.tiles[py][px] = TILE_TOWN_PATH;
        }
    }
    g->player.x = 10;
    g->player.y = 27;
    map_mark_explored(&g->map, 10, 27);
    g->gold = 393;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 30, .y = 25, .underlying_tile = TILE_TOWN_PATH, .item = item_make_health_potion()};
    g->map.tiles[25][30] = TILE_ITEM;
    int narrowed = save_game(g, slot) && legacy_approach_save(slot, 167) && load_game(loaded, slot);
    ASSERT("existing wide-road saves adopt the narrow paving", narrowed && loaded->map.tiles[24][10] == TILE_TOWN_PATH && loaded->map.tiles[25][10] == TILE_TOWN_FLOOR && loaded->map.tiles[27][10] == TILE_TOWN_FLOOR);
    ASSERT("narrowing preserves saved position, exploration, loot and gold", narrowed && loaded->player.x == 10 && loaded->player.y == 27 && map_is_explored(&loaded->map, 10, 27) && loaded->gold == 393 && loaded->floor_items[0].active && loaded->floor_items[0].underlying_tile == TILE_TOWN_FLOOR && loaded->map.tiles[25][30] == TILE_ITEM);
    approach_town(g, &town_approaches[1]);
    g->defeated_bosses |= 1 << LOCATION_DUNGEON;
    approach_enter(g, TOWN_W - 1, 12);
    g->map.stairs_down_y = g->map.stairs_up_y;
    g->map.tiles[21][20] = TILE_TOWN_FLOOR;
    g->map.tiles[21][19] = TILE_ITEM;
    g->player.x = 20;
    g->player.y = 21;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 19, .y = 21, .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_health_potion()};
    g->gold = 393;
    int raised = save_game(g, slot) && legacy_approach_save(slot, 168) && load_game(loaded, slot);
    ASSERT("older approaches move the crypt above a straight gate-to-gate road", raised && loaded->map.stairs_down_y == loaded->map.stairs_up_y - 2 && loaded->map.tiles[24][32] == TILE_TOWN_PATH);
    ASSERT("moving the crypt relocates overlapping players and loot onto safe paving", raised && loaded->player.x == 20 && loaded->player.y == 24 && loaded->floor_items[0].active && loaded->floor_items[0].x == 20 && loaded->floor_items[0].y == 24 && loaded->floor_items[0].underlying_tile == TILE_TOWN_PATH && loaded->map.tiles[24][20] == TILE_ITEM && loaded->gold == 393 && map_is_explored(&loaded->map, 20, 24));
    ASSERT("the raised crypt position survives a save round trip without drifting upward", save_game(loaded, slot) && load_game(g, slot) && g->map.stairs_down_y == 22 && g->player.y == 24);
    approach_town(g, &town_approaches[15]);
    for (int y = 6; y < 11; y++) {
        for (int x = 26; x < 33; x++) {
            g->map.tiles[y][x] = TILE_LOCAL_BUILDING;
        }
    }
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 28, .y = 8, .underlying_tile = TILE_LOCAL_BUILDING, .item = item_make_health_potion()};
    g->map.tiles[8][28] = TILE_ITEM;
    g->gold = 393;
    map_mark_explored(&g->map, 28, 8);
    int removed_hall = save_game(g, slot) && legacy_approach_save(slot, 169) && load_game(loaded, slot);
    int clear_lot = removed_hall;
    for (int y = 6; y < 11; y++) {
        for (int x = 26; x < 33; x++) {
            if (!map_is_walkable(&loaded->map, x, y)) {
                clear_lot = 0;
            }
        }
    }
    ASSERT("older Dawnmere saves remove the entire hall collision footprint", clear_lot && !map_is_walkable(&loaded->map, 7, 7));
    ASSERT("removing the hall preserves position, exploration and dropped loot", removed_hall && loaded->player.x == g->player.x && loaded->player.y == g->player.y && loaded->gold == 393 && map_is_explored(&loaded->map, 28, 8) && loaded->floor_items[0].active && loaded->floor_items[0].underlying_tile == TILE_TOWN_FLOOR && loaded->map.tiles[8][28] == TILE_ITEM);
    remove("saves/savegame_99160.json");
    free(g);
    free(loaded);
}
