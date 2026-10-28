#include "test_utils.h"
#include "game/coast_exits.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

static void break_exit(Map *map) {
    int x, y;
    coast_exit_point(map, 1, &x, &y);
    map->tiles[y][x] = TILE_COAST_SEA;
    x = map->stairs_down_x;
    y = map->stairs_down_y;
    map->tiles[y][x] = TILE_COAST_DEEP_WATER;
    const int steps[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int n = 0; n < 4; n++) {
        int nx = x + steps[n][0];
        int ny = y + steps[n][1];
        if (nx > 0 && nx < MAP_W - 1 && ny > 0 && ny < MAP_H - 1) {
            map->tiles[ny][nx] = TILE_COAST_SEA;
        }
    }
}

static void test_exit_actions(void) {
    static GameState g;
    static Map original;
    int travel = 1;
    int valid = 1;
    for (int seed = 0; seed < 8; seed++) {
        srand(seed);
        game_init(&g);
        for (int level = 1; level <= COAST_DEPTH; level++) {
            g.location = LOCATION_COAST;
            g.level = level;
            g.floor_item_count = 0;
            map_generate_coast(&g.map, level);
            enemies_spawn(&g);
            game_refresh_quest_encounters(&g);
            valid &= coast_exits_valid(&g.map);
            original = g.map;
            // Exercise the real boundary action, not a direct descend call.
            // The last-stage Queen lock has its own encounter tests.
            g.enemy_count = 0;
            g.player.x = g.map.stairs_down_x;
            g.player.y = g.map.stairs_down_y;
            int x, y;
            coast_exit_point(&g.map, 1, &x, &y);
            action_resolve_player(&g, (Action){ACTION_MOVE, x, y});
            travel &= level < COAST_DEPTH ? g.location == LOCATION_COAST && g.level == level + 1 : g.location != LOCATION_COAST;
            g.location = LOCATION_COAST;
            g.level = level;
            g.map = original;
            g.enemy_count = 0;
            g.floor_item_count = 0;
            g.player.x = g.map.stairs_up_x;
            g.player.y = g.map.stairs_up_y;
            coast_exit_point(&g.map, 0, &x, &y);
            action_resolve_player(&g, (Action){ACTION_MOVE, x, y});
            travel &= level > 1 ? g.location == LOCATION_COAST && g.level == level - 1 : g.location != LOCATION_COAST;
        }
    }
    ASSERT("all five coast stages retain real boundary exits after encounters and quest props", valid);
    ASSERT("80 actual coast entrance/exit crossings change stages or return to Oakhaven", travel);
}

static void test_exit_repair(void) {
    static Map map;
    static Map original;
    int repaired = 1;
    int idempotent = 1;
    for (int level = 1; level <= COAST_DEPTH; level++) {
        map_generate_coast(&map, level);
        break_exit(&map);
        ASSERT("missing or isolated boundary exit fails the invariant", !coast_exits_valid(&map));
        coast_exits_repair_map(&map, level);
        repaired &= coast_exits_valid(&map);
        original = map;
        idempotent &= !coast_exits_repair_map(&map, level) && memcmp(&map, &original, sizeof(map)) == 0;
        map.stairs_up_x = -1;
        map.stairs_down_y = MAP_H + 20;
        coast_exits_repair_map(&map, level);
        repaired &= coast_exits_valid(&map);
        int ix, iy, ex, ey;
        coast_exit_point(&map, 0, &ix, &iy);
        coast_exit_point(&map, 1, &ex, &ey);
        map.tiles[iy][ix] = TILE_COAST_SEA;
        map.tiles[ey][ex] = TILE_COAST_SEA;
        map.stairs_down_x = map.stairs_up_x;
        map.stairs_down_y = map.stairs_up_y;
        coast_exits_repair_map(&map, level);
        repaired &= coast_exits_valid(&map);
    }
    ASSERT("missing markers, flooded approaches and corrupt or overlapping coordinates are repaired in both tides", repaired);
    ASSERT("valid saved coast maps remain byte-for-byte unchanged", idempotent);
}

static void test_exit_loot(void) {
    static GameState g;
    game_init(&g);
    g.location = LOCATION_COAST;
    g.level = 3;
    map_generate_coast(&g.map, 3);
    int x, y;
    coast_exit_point(&g.map, 1, &x, &y);
    g.floor_item_count = 1;
    g.floor_items[0].active = 1;
    g.floor_items[0].x = x;
    g.floor_items[0].y = y;
    g.floor_items[0].underlying_tile = TILE_COAST_EXIT;
    g.floor_items[0].item = item_make_health_potion();
    g.map.tiles[y][x] = TILE_ITEM;
    coast_exits_repair(&g);
    ASSERT("repair preserves loot and its transition underlay", g.floor_items[0].active &&
        g.floor_items[0].underlying_tile == TILE_COAST_EXIT && g.map.tiles[y][x] == TILE_ITEM);
    g.player.x = g.map.stairs_down_x;
    g.player.y = g.map.stairs_down_y;
    action_resolve_player(&g, (Action){ACTION_MOVE, x, y});
    ASSERT("legacy loot covering a coast exit cannot disable travel", g.location == LOCATION_COAST && g.level == 4);
}

static void test_saved_exit_repair(void) {
    static GameState g;
    static GameState loaded;
    const int slot = 99081;
    if (save_exists(slot)) {
        ASSERT("coast exit migration fixture uses an unused slot", 0);
        return;
    }
    game_init(&g);
    g.gold = 179;
    g.score = 3495;
    g.player.hp = 89;
    g.spice_skiff_quest_state = 1;
    for (int level = 1; level <= COAST_DEPTH; level++) {
        LevelCache *cache = &g.coast_cache[level - 1];
        cache->valid = 1;
        map_generate_coast(&cache->map, level);
        break_exit(&cache->map);
        map_mark_explored(&cache->map, 60, 40);
        cache->enemy_count = 1;
        cache->enemies[0].type = ENEMY_GIANT_CRAB;
        cache->enemies[0].hp = 7;
        cache->enemies[0].active = 1;
    }
    ASSERT("broken coast cache fixture saves", save_game(&g, slot));
    ASSERT("all cached coast stages are checked even when loading in town", load_game(&loaded, slot));
    int valid = 1;
    for (int level = 1; level <= COAST_DEPTH; level++) {
        LevelCache *cache = &loaded.coast_cache[level - 1];
        valid &= cache->valid && coast_exits_valid(&cache->map) && map_is_explored(&cache->map, 60, 40);
        valid &= cache->enemy_count == 1 && cache->enemies[0].hp == 7;
    }
    ASSERT("load repairs all five cached exits without resetting explored tiles or wounded enemies", valid);
    g.location = LOCATION_COAST;
    g.level = 3;
    g.map = g.coast_cache[2].map;
    g.player.x = g.map.stairs_up_x;
    g.player.y = g.map.stairs_up_y;
    ASSERT("broken active stage fixture saves", save_game(&g, slot));
    // Emulate the preceding save version without changing any gameplay fields.
    FILE *file = fopen("saves/savegame_99081.json", "rb");
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    char *json = malloc((size_t)length + 1);
    fread(json, 1, (size_t)length, file);
    json[length] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(json);
    free(json);
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 211);
    json = cJSON_PrintUnformatted(root);
    file = fopen("saves/savegame_99081.json", "wb");
    fputs(json, file);
    fclose(file);
    free(json);
    cJSON_Delete(root);
    ASSERT("version 211 active coast save migrates", load_game(&loaded, slot));
    ASSERT("active coast exit is restored without losing player or quest progress", coast_exits_valid(&loaded.map) &&
        loaded.gold == 179 && loaded.score == 3495 && loaded.player.hp == 89 && loaded.spice_skiff_quest_state == 1);
    remove("saves/savegame_99081.json");
}

void test_coast_exits(void) {
    printf("Coast transition invariants:\n");
    test_exit_repair();
    test_exit_actions();
    test_exit_loot();
    test_saved_exit_repair();
}
