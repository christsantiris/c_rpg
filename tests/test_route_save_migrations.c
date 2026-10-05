#include "test_utils.h"
#include "game/game.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define ROUTE_SAVE_SLOT 99123
static GameState original;
static GameState loaded;
static GameState reloaded;

static int make_legacy_save(int version) {
    if (!save_game(&original, ROUTE_SAVE_SLOT)) {
        return 0;
    }
    FILE *file = fopen("saves/savegame_99123.json", "r");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *buffer = malloc(size + 1);
    if (!buffer) {
        fclose(file);
        return 0;
    }
    size_t count = fread(buffer, 1, size, file);
    buffer[count] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    if (version < 77) {
        cJSON_DeleteItemFromObject(root, "forest_entry_town");
        cJSON_DeleteItemFromObject(root, "forest_portal_town");
    }
    cJSON_DeleteItemFromObject(root, "swamp_entry_town");
    cJSON_DeleteItemFromObject(root, "swamp_portal_town");
    cJSON *cache = cJSON_GetObjectItem(root, "swamp_cache");
    while (cJSON_GetArraySize(cache) > 5) {
        cJSON_DeleteItemFromArray(cache, 5);
    }
    char *json = cJSON_Print(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99123.json", "w");
    int result = file && json;
    if (result) {
        fputs(json, file);
    }
    if (file) {
        fclose(file);
    }
    free(json);
    return result;
}

static void prepare_legacy_region(int forest, int level) {
    original.player.player_class = CLASS_WARRIOR;
    game_init(&original);
    original.location = forest ? LOCATION_FOREST : LOCATION_SWAMP;
    original.player.hp = 123;
    original.gold = 321;
    original.score = 9876;
    original.alder_quest_state = 1;
    original.alder_wardens_rescued = 1;
    original.innkeeper_quest_state = 1;
    LevelCache *cache = forest ? original.forest_cache : original.swamp_cache;
    int depth = forest ? 8 : 5;
    for (int i = 0; i < depth; i++) {
        if (forest) {
            map_generate_forest(&cache[i].map, i == 7 ? FOREST_BOSS_LEVEL : (i == 3 ? FOREST_DEPTH : i + 1));
        } else {
            map_generate_swamp(&cache[i].map, i + 1);
        }
        cache[i].valid = 1;
        cache[i].level_cleared = 0;
        cache[i].enemy_count = 1;
        int x;
        int y;
        map_room_center(&cache[i].map.rooms[0], &x, &y);
        cache[i].enemies[0] = (Enemy){
            .active = 1, .type = forest ? ENEMY_BLIGHTED_WOLF : ENEMY_GIANT_RAT,
            .hp = 50 + i, .max_hp = 100, .x = x + 1, .y = y
        };
        if ((forest && i == 7) || (!forest && i == 4)) {
            cache[i].enemies[0].type = forest ? ENEMY_FOREST_NECROMANCER : ENEMY_SWAMP_DEMON;
            cache[i].enemies[0].is_boss = 1;
        }
        if ((forest && (i == 1 || i == 4 || i == 6)) || (!forest && i == 3)) {
            map_room_center(&cache[i].map.rooms[7], &x, &y);
            cache[i].map.tiles[y][x] = forest ? TILE_FOREST_WARDEN : TILE_SWAMP_DAUGHTER;
        }
    }
    original.level = level;
    original.map = cache[level - 1].map;
    original.enemy_count = cache[level - 1].enemy_count;
    original.enemies[0] = cache[level - 1].enemies[0];
    if (forest) {
        original.max_forest_level_reached = 8;
    } else {
        original.max_swamp_level_reached = 5;
    }
    map_room_center(&original.map.rooms[0], &original.player.x, &original.player.y);
    int x;
    int y;
    map_room_center(&original.map.rooms[original.map.room_count - 1], &x, &y);
    original.floor_item_count = 1;
    original.floor_items[0] = (FloorItem){
        .active = 1, .x = x, .y = y,
        .underlying_tile = forest ? TILE_FOREST_FLOOR : TILE_SWAMP_FLOOR,
        .item = item_make_health_potion()
    };
    original.map.tiles[y][x] = TILE_ITEM;
}

static void test_legacy_region(int forest, int level, int expected) {
    prepare_legacy_region(forest, level);
    int ok = make_legacy_save(forest ? 76 : 77) && load_game(&loaded, ROUTE_SAVE_SLOT);
    ASSERT("legacy regional save loads without new travel fields", ok);
    if (!ok) {
        return;
    }
    ASSERT("active old level moves into the seven-stage route", loaded.level == expected && loaded.location == original.location);
    ASSERT("migration preserves player stats, money, inventory and score", loaded.player.hp == original.player.hp && loaded.player.attack == original.player.attack && loaded.gold == original.gold && loaded.score == original.score && loaded.inventory_count == original.inventory_count);
    ASSERT("migration preserves active position and wounded enemies", loaded.player.x == original.player.x && loaded.player.y == original.player.y && loaded.enemies[0].hp == original.enemies[0].hp);
    ASSERT("migration preserves dropped loot and its position", loaded.floor_item_count == 1 && loaded.floor_items[0].active && loaded.floor_items[0].x == original.floor_items[0].x && loaded.floor_items[0].y == original.floor_items[0].y);
    ASSERT("migration preserves existing rescue progress", loaded.alder_quest_state == 1 && loaded.alder_wardens_rescued == 1 && loaded.innkeeper_quest_state == 1);
    LevelCache *cache = forest ? loaded.forest_cache : loaded.swamp_cache;
    ASSERT("old end boss snapshot becomes the central encounter", cache[3].valid && cache[3].enemies[0].is_boss && cache[3].enemies[0].hp == (forest ? 57 : 54));
    if (forest) {
        ASSERT("old forest hub survives as the outer approach", cache[6].valid && cache[6].enemies[0].hp == (level == 7 ? 56 : 53));
        ASSERT("forest migration keeps progress within seven stages", loaded.max_forest_level_reached == 7 && !loaded.forest_cache[7].valid);
        int misplaced = 0;
        for (int i = 3; i < FOREST_DEPTH; i++) {
            for (int y = 0; y < MAP_H; y++) {
                for (int x = 0; x < MAP_W; x++) {
                    misplaced |= cache[i].map.tiles[y][x] == TILE_FOREST_WARDEN;
                }
            }
        }
        ASSERT("old wardens do not remain on the wrong approach", !misplaced);
    } else {
        ASSERT("swamp migration retains the original five maps and leaves two new stages unexplored", loaded.max_swamp_level_reached == 5 && cache[2].valid && cache[4].valid && !cache[5].valid && !cache[6].valid);
        int x;
        int y;
        map_room_center(&cache[2].map.rooms[7], &x, &y);
        ASSERT("Mira's old clearing moves intact from level 4 to level 3", cache[2].map.tiles[y][x] == TILE_SWAMP_DAUGHTER);
    }
    ok = save_game(&loaded, ROUTE_SAVE_SLOT) && load_game(&reloaded, ROUTE_SAVE_SLOT);
    ASSERT("resaving migration is stable and does not repeat level remapping", ok && reloaded.level == expected && reloaded.player.x == loaded.player.x && reloaded.player.y == loaded.player.y && reloaded.enemies[0].hp == loaded.enemies[0].hp);
    remove("saves/savegame_99123.json");
}

void test_route_save_migrations(void) {
    printf("Forest and swamp save migration tests:\n");
    ASSERT("route migration test slot is unused", !save_exists(ROUTE_SAVE_SLOT));
    test_legacy_region(1, 1, 1);
    test_legacy_region(1, 2, 2);
    test_legacy_region(1, 3, 3);
    test_legacy_region(1, 4, 7);
    test_legacy_region(1, 7, 7);
    test_legacy_region(1, 8, FOREST_BOSS_LEVEL);
    test_legacy_region(0, 3, 5);
    test_legacy_region(0, 4, SWAMP_RESCUE_LEVEL);
    test_legacy_region(0, 5, SWAMP_BOSS_LEVEL);
    for (int forest = 0; forest < 2; forest++) {
        prepare_legacy_region(forest, forest ? 8 : 5);
        original.defeated_bosses = 1 << original.location;
        original.enemies[0].active = 0;
        LevelCache *cache = forest ? original.forest_cache : original.swamp_cache;
        cache[original.level - 1].enemies[0].active = 0;
        game_open_town_portal(&original);
        int ok = make_legacy_save(forest ? 76 : 77) && load_game(&loaded, ROUTE_SAVE_SLOT);
        ASSERT("old portal save loads and keeps its original town", ok && loaded.location == (forest ? LOCATION_TOWN : LOCATION_TOWN2) && loaded.portal_active);
        game_use_town_portal(&loaded);
        ASSERT("old boss portal restores the central level and casting position", ok && loaded.level == 4 && loaded.player.x == original.portal_x && loaded.player.y == original.portal_y);
        ASSERT("boss victory survives migration and reveals the central shortcut", ok && loaded.defeated_bosses == original.defeated_bosses);
        remove("saves/savegame_99123.json");
    }
    original.player.player_class = CLASS_WARRIOR;
    game_init(&original);
    original.location = LOCATION_TOWN2;
    original.level = FOREST_BOSS_LEVEL;
    original.forest_entry_town = LOCATION_TOWN2;
    original.forest_portal_town = LOCATION_TOWN2;
    original.defeated_bosses = 1 << LOCATION_FOREST;
    original.forest_cache[3].valid = 1;
    map_generate_forest(&original.forest_cache[3].map, FOREST_BOSS_LEVEL);
    int ok = make_legacy_save(77) && load_game(&loaded, ROUTE_SAVE_SLOT);
    ASSERT("version 77 town save loads with default swamp travel anchors", ok && loaded.swamp_entry_town == LOCATION_TOWN2 && loaded.swamp_portal_town == LOCATION_TOWN2);
    ASSERT("swamp migration leaves the already migrated forest unchanged", ok && loaded.forest_entry_town == LOCATION_TOWN2 && loaded.forest_portal_town == LOCATION_TOWN2 && loaded.forest_cache[3].valid && memcmp(&loaded.forest_cache[3].map, &original.forest_cache[3].map, sizeof(Map)) == 0 && loaded.defeated_bosses == original.defeated_bosses);
    remove("saves/savegame_99123.json");
}
