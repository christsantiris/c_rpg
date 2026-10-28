#include "expedition_fixture.h"
#include "test_utils.h"
#include "shortcut_checks.h"
#include "game/game.h"
#include "game/approach.h"
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
    test_mark_legacy_portal(root);
    if (version < 77) {
        cJSON_DeleteItemFromObject(root, "forest_entry_town");
        cJSON_DeleteItemFromObject(root, "forest_portal_town");
    }
    if (version < 78) {
        cJSON_DeleteItemFromObject(root, "swamp_entry_town");
        cJSON_DeleteItemFromObject(root, "swamp_portal_town");
        cJSON *cache = cJSON_GetObjectItem(root, "swamp_cache");
        while (cJSON_GetArraySize(cache) > 5) {
            cJSON_DeleteItemFromArray(cache, 5);
        }
    }
    if (version == 78) {
        cJSON_DeleteItemFromObject(root, "mountain_entry_town");
        cJSON_DeleteItemFromObject(root, "mountain_portal_town");
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

static void prepare_legacy_mountains(int level) {
    original.player.player_class = CLASS_WARRIOR;
    game_init(&original);
    original.location = LOCATION_MOUNTAINS;
    original.level = level;
    original.max_mountain_level_reached = 8;
    original.dain_quest_state = 1;
    original.dain_map_fragments = DAIN_FRAGMENT_ARCHER;
    original.gold = 321;
    original.score = 9876;
    for (int i = 0; i < 8; i++) {
        LevelCache *cache = &original.mountain_cache[i];
        map_generate_mountains(&cache->map, i == 7 ? MOUNTAIN_BOSS_LEVEL : i + 1);
        cache->valid = 1;
        cache->enemy_count = 1;
        cache->level_cleared = 0;
        int x;
        int y;
        map_room_center(&cache->map.rooms[0], &x, &y);
        cache->enemies[0] = (Enemy){.active = 1, .type = ENEMY_GOBLIN_SCOUT, .x = x + 1, .y = y, .hp = 50 + i, .max_hp = 100};
        if (i == 1 || i == 2 || i == 4) {
            cache->enemies[0].type = i == 1 ? ENEMY_GOBLIN_ARCHER : (i == 2 ? ENEMY_GOBLIN_BOMBER : ENEMY_GOBLIN_SHAMAN);
            cache->enemies[0].dain_fragment = i == 1 ? 1 : (i == 2 ? 2 : 4);
            snprintf(cache->enemies[0].name, sizeof(cache->enemies[0].name), "Map Bearer");
        }
        if (i == 7) {
            cache->enemies[0].type = ENEMY_MOUNTAIN_GOBLIN_KING;
            cache->enemies[0].is_boss = 1;
        }
    }
    original.map = original.mountain_cache[level - 1].map;
    original.enemy_count = 1;
    original.enemies[0] = original.mountain_cache[level - 1].enemies[0];
    map_room_center(&original.map.rooms[0], &original.player.x, &original.player.y);
    original.floor_item_count = 1;
    original.floor_items[0] = (FloorItem){.active = 1, .x = original.player.x, .y = original.player.y, .underlying_tile = TILE_MOUNTAIN_FLOOR, .item = item_make_health_potion()};
    original.map.tiles[original.player.y][original.player.x] = TILE_ITEM;
}

static void test_mountain_migration(void) {
    static const int old_levels[] = {1, 2, 4, 5, 7, 8};
    static const int new_levels[] = {1, 2, 2, 4, 2, 5};
    for (int i = 0; i < 6; i++) {
        prepare_legacy_mountains(old_levels[i]);
        int ok = make_legacy_save(78) && load_game(&loaded, ROUTE_SAVE_SLOT);
        ASSERT("version 78 mountain save migrates without new travel fields", ok);
        if (!ok) {
            continue;
        }
        ASSERT("old mountain stages move into the five-level climb", loaded.level == new_levels[i] && loaded.max_mountain_level_reached == 5 && !loaded.mountain_cache[7].valid);
        ASSERT("mountain migration preserves stats, position, loot, and wounded enemies", loaded.player.hp == original.player.hp && loaded.player.x == original.player.x && loaded.player.y == original.player.y && loaded.gold == 321 && loaded.score == 9876 && loaded.floor_item_count == 1 && loaded.enemies[0].hp == original.enemies[0].hp);
        ASSERT("mountain migration preserves partial map quest progress", loaded.dain_quest_state == 1 && loaded.dain_map_fragments == DAIN_FRAGMENT_ARCHER);
        ASSERT("old summit King moves intact to level 5", loaded.mountain_cache[4].valid && loaded.mountain_cache[4].enemies[0].is_boss && loaded.mountain_cache[4].enemies[0].hp == 57);
        ASSERT("old mountain bearers cannot award fragments beyond new quest levels", loaded.mountain_cache[4].enemies[0].dain_fragment == 0);
        ASSERT("occupied old mountain fourth or seventh stage retains its snapshot", loaded.mountain_cache[1].enemies[0].hp == (old_levels[i] == 2 ? 51 : old_levels[i] == 4 ? 53 : 56));
        if (new_levels[i] == 2) {
            int bearers = 0;
            for (int enemy = 0; enemy < loaded.enemy_count; enemy++) {
                bearers += loaded.enemies[enemy].dain_fragment == DAIN_FRAGMENT_BOMBER;
            }
            ASSERT("migration removes Bomber fragments from the old second stage", bearers == 0 && loaded.enemies[0].dain_fragment == 0);
        }
        ASSERT("mountain migration is stable after saving the current format", save_game(&loaded, ROUTE_SAVE_SLOT) && load_game(&reloaded, ROUTE_SAVE_SLOT) && reloaded.level == loaded.level && reloaded.enemies[0].hp == loaded.enemies[0].hp);
        remove("saves/savegame_99123.json");
    }
    prepare_legacy_mountains(8);
    original.defeated_bosses = 1 << LOCATION_MOUNTAINS;
    original.enemies[0].active = 0;
    original.mountain_cache[7].enemies[0].active = 0;
    original.last_town_visited = LOCATION_DAWNMERE;
    game_open_town_portal(&original);
    int ok = make_legacy_save(78) && load_game(&loaded, ROUTE_SAVE_SLOT);
    ASSERT("legacy mountain portal moves its anchor to Dawnmere", ok && loaded.location == LOCATION_DAWNMERE && loaded.mountain_portal_town == LOCATION_DAWNMERE && !loaded.portal_active && loaded.portal_level == MOUNTAIN_BOSS_LEVEL);
    test_restore_cached_floor(&loaded);
    ASSERT("legacy summit portal restores the central peak and position", ok && loaded.location == LOCATION_MOUNTAINS && loaded.level == MOUNTAIN_BOSS_LEVEL && loaded.player.x == original.portal_x && loaded.player.y == original.portal_y);
    int found = 0;
    for (int row = 0; row < MAP_H; row++) {
        for (int column = 0; column < MAP_W; column++) {
            if (loaded.map.tiles[row][column] == TILE_MOUNTAIN_SHORTCUT) {
                found = 1;
            }
        }
    }
    ASSERT("migrated King victory does not recreate a mountain shortcut", ok && loaded.defeated_bosses == original.defeated_bosses && !found);
    remove("saves/savegame_99123.json");
}

static void test_legacy_region(int forest, int level, int expected) {
    if (forest) {
        static const int final_levels[8] = {1, 2, 3, 2, 4, 3, 2, 5};
        expected = final_levels[level - 1];
    }
    prepare_legacy_region(forest, level);
    int ok = make_legacy_save(forest ? 76 : 77) && load_game(&loaded, ROUTE_SAVE_SLOT);
    ASSERT("legacy regional save loads without new travel fields", ok);
    if (!ok) {
        return;
    }
    ASSERT("active old level moves into the current adventure", loaded.level == expected && loaded.location == original.location);
    ASSERT("migration preserves player stats, money, inventory and score", loaded.player.hp == original.player.hp && loaded.player.attack == original.player.attack && loaded.gold == original.gold && loaded.score == original.score && loaded.inventory_count == original.inventory_count);
    ASSERT("migration preserves active position and wounded enemies", loaded.player.x == original.player.x && loaded.player.y == original.player.y && loaded.enemies[0].hp == original.enemies[0].hp);
    ASSERT("migration preserves dropped loot and its position", loaded.floor_item_count == 1 && loaded.floor_items[0].active && loaded.floor_items[0].x == original.floor_items[0].x && loaded.floor_items[0].y == original.floor_items[0].y);
    ASSERT("migration preserves existing rescue progress", loaded.alder_quest_state == 1 && loaded.alder_wardens_rescued == 1 && loaded.innkeeper_quest_state == 1);
    LevelCache *cache = forest ? loaded.forest_cache : loaded.swamp_cache;
    ASSERT("old end boss snapshot becomes the central encounter", cache[4].valid && cache[4].enemies[0].is_boss && cache[4].enemies[0].hp == (forest ? 57 : 54));
    if (forest) {
        ASSERT("old forest hub survives as the outer approach", cache[1].valid && cache[1].enemies[0].hp == (level == 2 ? 51 : level == 7 ? 56 : 53));
        ASSERT("forest migration keeps progress within five levels", loaded.max_forest_level_reached == 5 && !loaded.forest_cache[7].valid);
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

static void test_split_forest_migration(void) {
    for (int portal = 0; portal < 2; portal++) {
        for (int rescued = 0; rescued < 2; rescued++) {
            memset(&original, 0, sizeof(original));
            original.player.player_class = CLASS_WARRIOR;
            game_init(&original);
            original.defeated_bosses |= 1 << LOCATION_FOREST;
            game_enter_forest(&original);
            while (original.level < 5) {
                game_descend(&original);
            }
            original.enemies[0].hp = 11;
            game_ascend(&original);
            game_ascend(&original);
            original.alder_quest_state = 1;
            original.alder_wardens_rescued = ALDER_WARDEN_STAGE_2 | (rescued ? ALDER_WARDEN_STAGE_4 : 0);
            original.gold = 617;
            original.enemies[0].hp = 9;
            int x;
            int y;
            map_room_center(&original.map.rooms[6], &x, &y);
            original.map.tiles[y][x] = TILE_FOREST_WARDEN;
            original.forest_cache[2].map.tiles[y][x] = TILE_FOREST_WARDEN;
            original.floor_item_count = 1;
            original.floor_items[0] = (FloorItem){.active = 1, .x = original.player.x, .y = original.player.y,
                .underlying_tile = original.map.tiles[original.player.y][original.player.x], .item = item_make_health_potion()};
            original.map.tiles[original.player.y][original.player.x] = TILE_ITEM;
            if (portal) {
                game_open_town_portal(&original);
            }
            ASSERT("version 98 forest quest saves migrate", make_legacy_save(98) && load_game(&loaded, ROUTE_SAVE_SLOT));
            ASSERT("moving a captive preserves rescues, money, boss victory, and explored far-side enemies", loaded.alder_quest_state == 1 &&
                loaded.alder_wardens_rescued == original.alder_wardens_rescued && loaded.gold == 617 &&
                loaded.defeated_bosses == original.defeated_bosses && loaded.forest_cache[3].valid && loaded.forest_cache[3].enemies[0].hp == 11 &&
                memcmp(loaded.forest_cache[3].map.rooms, original.forest_cache[4].map.rooms, sizeof(original.map.rooms)) == 0);
            ASSERT("migration removes the obsolete captive from the cached third stage", loaded.forest_cache[2].map.tiles[y][x] == TILE_FOREST_FLOOR);
            if (portal) {
                ASSERT("migration keeps the third-stage portal anchor", !loaded.portal_active && loaded.portal_level == 3 &&
                    loaded.portal_x == original.portal_x && loaded.portal_y == original.portal_y);
                test_restore_cached_floor(&loaded);
            } else {
                ASSERT("migration keeps active third-stage position and loot", loaded.player.x == original.player.x && loaded.player.y == original.player.y &&
                    loaded.floor_items[0].active && loaded.floor_items[0].underlying_tile == original.floor_items[0].underlying_tile);
            }
            ASSERT("migration keeps the active third-stage enemy damage and removes its old captive", loaded.level == 3 &&
                loaded.enemies[0].hp == 9 && loaded.map.tiles[y][x] == TILE_FOREST_FLOOR);
            game_descend(&loaded);
            int wardens = 0;
            for (int row = 0; row < MAP_H; row++) {
                for (int column = 0; column < MAP_W; column++) {
                    wardens += loaded.map.tiles[row][column] == TILE_FOREST_WARDEN;
                }
            }
            ASSERT("only unfinished third rescues appear on the explored fourth level", loaded.level == 4 && wardens == !rescued &&
                loaded.enemies[0].hp == 11 && loaded.alder_wardens_rescued == original.alder_wardens_rescued);
            ASSERT("migrated forest quest rewrites and reloads successfully", save_game(&loaded, ROUTE_SAVE_SLOT) && load_game(&reloaded, ROUTE_SAVE_SLOT) &&
                reloaded.alder_wardens_rescued == loaded.alder_wardens_rescued);
        }
    }
    remove("saves/savegame_99123.json");
}

static void test_split_mountain_migration(void) {
    for (int portal = 0; portal < 2; portal++) {
        for (int collected = 0; collected < 2; collected++) {
            memset(&original, 0, sizeof(original));
            original.player.player_class = CLASS_WARRIOR;
            game_init(&original);
            original.defeated_bosses |= 1 << LOCATION_MOUNTAINS;
            game_enter_mountains(&original);
            while (original.level < 5) {
                game_descend(&original);
            }
            original.enemies[0].hp = 11;
            // A legacy level-five slope had difficulty three, hence 13 foes.
            original.enemy_count = 13;
            original.enemies[0].type = ENEMY_GOBLIN_SCOUT;
            original.enemies[0].active = 1;
            original.enemies[0].is_boss = 0;
            game_ascend(&original);
            game_ascend(&original);
            original.dain_quest_state = 1;
            original.dain_map_fragments = DAIN_FRAGMENT_ARCHER | (collected ? DAIN_FRAGMENT_SHAMAN : 0);
            original.gold = 617;
            Enemy *bearer = &original.enemies[0];
            bearer->type = ENEMY_GOBLIN_SHAMAN;
            bearer->active = !collected;
            bearer->hp = collected ? 0 : 9;
            bearer->dain_fragment = DAIN_FRAGMENT_SHAMAN;
            snprintf(bearer->name, sizeof(bearer->name), "Map Bearer");
            original.mountain_cache[2].enemies[0] = *bearer;
            original.floor_item_count = 1;
            original.floor_items[0] = (FloorItem){.active = 1, .x = original.player.x, .y = original.player.y,
                .underlying_tile = original.map.tiles[original.player.y][original.player.x], .item = item_make_health_potion()};
            original.map.tiles[original.player.y][original.player.x] = TILE_ITEM;
            if (portal) {
                game_open_town_portal(&original);
            }
            ASSERT("version 99 mountain quest saves migrate", make_legacy_save(99) && load_game(&loaded, ROUTE_SAVE_SLOT));
            ASSERT("moving a bearer preserves fragments, money, boss victory, and explored far-side enemies", loaded.dain_quest_state == 1 &&
                loaded.dain_map_fragments == original.dain_map_fragments && loaded.gold == 617 &&
                loaded.defeated_bosses == original.defeated_bosses && loaded.mountain_cache[3].valid && loaded.mountain_cache[3].enemies[0].hp == 11 &&
                loaded.mountain_cache[3].map.room_count == original.mountain_cache[4].map.room_count &&
                memcmp(loaded.mountain_cache[3].map.rooms, original.mountain_cache[4].map.rooms, original.mountain_cache[4].map.room_count * sizeof(Room)) == 0 &&
                memcmp(loaded.mountain_cache[3].map.tiles, original.mountain_cache[4].map.tiles, sizeof(original.map.tiles)) == 0);
            ASSERT("old cached Shaman keeps health and death state without a misplaced fragment", !loaded.mountain_cache[2].enemies[0].dain_fragment &&
                loaded.mountain_cache[2].enemies[0].hp == (collected ? 0 : 9) && loaded.mountain_cache[2].enemies[0].active == !collected);
            if (portal) {
                ASSERT("migration keeps the third-stage mountain portal anchor", !loaded.portal_active && loaded.portal_level == 3 &&
                    loaded.portal_x == original.portal_x && loaded.portal_y == original.portal_y);
                test_restore_cached_floor(&loaded);
            } else {
                ASSERT("mountain migration keeps active position and loot", loaded.player.x == original.player.x && loaded.player.y == original.player.y &&
                    loaded.floor_items[0].active && loaded.floor_items[0].underlying_tile == original.floor_items[0].underlying_tile);
            }
            ASSERT("old active Shaman becomes a regular enemy without losing damage", loaded.level == 3 && (collected ?
                loaded.enemies[0].dain_fragment == DAIN_FRAGMENT_BOMBER :
                (!loaded.enemies[0].dain_fragment && loaded.enemies[0].hp == 9 && strcmp(loaded.enemies[0].name, "Goblin Shaman") == 0)));
            if (!collected) {
                loaded.player.attack = 10000;
                loaded.player.x = loaded.enemies[0].x;
                loaded.player.y = loaded.enemies[0].y + 1;
                action_resolve_player(&loaded, (Action){ACTION_MOVE, loaded.enemies[0].x, loaded.enemies[0].y});
                ASSERT("defeating the former bearer cannot award the relocated fragment", !loaded.enemies[0].active &&
                    loaded.dain_map_fragments == original.dain_map_fragments);
            }
            game_descend(&loaded);
            int bearers = 0;
            for (int i = 0; i < loaded.enemy_count; i++) {
                bearers += loaded.enemies[i].active && loaded.enemies[i].dain_fragment == DAIN_FRAGMENT_SHAMAN;
            }
            ASSERT("only missing Shaman fragments gain bearers on the explored fourth stage", loaded.level == 4 && bearers == !collected &&
                loaded.enemies[0].hp == 11 && loaded.dain_map_fragments == original.dain_map_fragments);
            game_refresh_quest_encounters(&loaded);
            int refreshed = 0;
            for (int i = 0; i < loaded.enemy_count; i++) {
                refreshed += loaded.enemies[i].active && loaded.enemies[i].dain_fragment == DAIN_FRAGMENT_SHAMAN;
            }
            ASSERT("refreshing migrated stage four does not duplicate the bearer", refreshed == bearers);
            ASSERT("migrated mountain quest rewrites and reloads successfully", save_game(&loaded, ROUTE_SAVE_SLOT) && load_game(&reloaded, ROUTE_SAVE_SLOT) &&
                reloaded.dain_map_fragments == loaded.dain_map_fragments);
        }
    }
    remove("saves/savegame_99123.json");
}

static void test_near_side_quest_migration(void) {
    for (int forest = 0; forest < 2; forest++) {
        for (int portal = 0; portal < 2; portal++) {
            memset(&original, 0, sizeof(original));
            original.player.player_class = CLASS_WARRIOR;
            game_init(&original);
            original.location = forest ? LOCATION_FOREST : LOCATION_MOUNTAINS;
            original.level = 2;
            original.defeated_bosses = 1 << original.location;
            original.alder_quest_state = 1;
            original.alder_wardens_rescued = 1;
            original.dain_quest_state = 1;
            original.dain_map_fragments = DAIN_FRAGMENT_ARCHER;
            original.gold = 617;
            LevelCache *cache = forest ? original.forest_cache : original.mountain_cache;
            for (int stage = 2; stage <= 7; stage++) {
                LevelCache *snapshot = &cache[stage - 1];
                if (forest) {
                    map_generate_forest(&snapshot->map, stage);
                } else {
                    map_generate_mountains(&snapshot->map, stage);
                }
                snapshot->valid = 1;
                snapshot->level_cleared = 1;
                snapshot->enemy_count = 1;
                int x;
                int y;
                map_room_center(&snapshot->map.rooms[0], &x, &y);
                snapshot->enemies[0] = (Enemy){.active = 1, .x = x, .y = y,
                    .type = forest ? ENEMY_BLIGHTED_WOLF : ENEMY_GOBLIN_BOMBER,
                    .hp = 17, .max_hp = 50, .dain_fragment = !forest && stage == 2 ? DAIN_FRAGMENT_BOMBER : 0};
                map_mark_explored(&snapshot->map, x, y);
            }
            original.map = cache[1].map;
            original.enemies[0] = cache[1].enemies[0];
            original.enemy_count = 1;
            int x;
            int y;
            map_room_center(&original.map.rooms[4], &x, &y);
            original.player.x = x;
            original.player.y = y + 1;
            int player_x = original.player.x;
            int player_y = original.player.y;
            original.floor_item_count = 1;
            original.floor_items[0] = (FloorItem){.active = 1, .x = x, .y = y,
                .underlying_tile = forest ? TILE_FOREST_WARDEN : TILE_MOUNTAIN_FLOOR,
                .item = item_make_health_potion()};
            original.map.tiles[y][x] = TILE_ITEM;
            if (forest) {
                original.map.tiles[y][x + 1] = TILE_FOREST_WARDEN;
                cache[1].map.tiles[y][x + 1] = TILE_FOREST_WARDEN;
            }
            if (portal) {
                game_open_town_portal(&original);
                if (forest) {
                    original.portal_origin_tile = TILE_FOREST_WARDEN;
                }
            }
            int ok = make_legacy_save(112) && load_game(&loaded, ROUTE_SAVE_SLOT);
            ASSERT("version 112 quest migration preserves collected objectives and money", ok &&
                loaded.alder_wardens_rescued == 1 && loaded.dain_map_fragments == 1 &&
                loaded.alder_quest_state == 1 && loaded.dain_quest_state == 1 && loaded.gold == 617 && loaded.defeated_bosses == original.defeated_bosses);
            LevelCache *saved = forest ? loaded.forest_cache : loaded.mountain_cache;
            ASSERT("quest relocation keeps cached enemy health, exploration and room geometry", ok &&
                saved[1].enemies[0].hp == 17 && saved[2].enemies[0].hp == 17 &&
                memcmp(saved[2].map.explored, cache[5].map.explored, sizeof(saved[2].map.explored)) == 0 &&
                memcmp(saved[2].map.rooms, cache[5].map.rooms, sizeof(saved[2].map.rooms)) == 0);
            if (forest) {
                ASSERT("obsolete cached captives and portal underlays become forest floor", saved[1].map.tiles[y][x + 1] == TILE_FOREST_FLOOR &&
                    (!portal || (loaded.portal_level == 2 && loaded.portal_origin_tile == TILE_FOREST_FLOOR)));
            } else {
                ASSERT("obsolete cached Map Bearers keep their health but lose fragment markers", saved[1].enemies[0].hp == 17 &&
                    !saved[1].enemies[0].dain_fragment && strcmp(saved[1].enemies[0].name, "Goblin Bomber") == 0);
            }
            if (portal) {
                test_restore_cached_floor(&loaded);
            }
            ASSERT("quest migration keeps the active stage and position", loaded.level == 2 &&
                loaded.player.x == player_x && loaded.player.y == player_y && loaded.enemies[0].hp == 17);
            if (forest && !portal) {
                ASSERT("obsolete captive hidden under active loot is removed without losing the item", loaded.floor_items[0].active &&
                    loaded.floor_items[0].underlying_tile == TILE_FOREST_FLOOR && loaded.map.tiles[y][x] == TILE_ITEM);
            }
            for (int stage = 3; stage <= 2; stage++) {
                game_descend(&loaded);
            }
            int objectives = 0;
            for (int row = 0; row < MAP_H; row++) {
                for (int column = 0; column < MAP_W; column++) {
                    objectives += loaded.map.tiles[row][column] == TILE_FOREST_WARDEN;
                }
            }
            for (int enemy = 0; enemy < loaded.enemy_count; enemy++) {
                objectives += loaded.enemies[enemy].active && loaded.enemies[enemy].dain_fragment == DAIN_FRAGMENT_ARCHER;
            }
            ASSERT("a collected first objective does not respawn on new level two", loaded.level == 2 && !objectives);
            game_descend(&loaded);
            objectives = 0;
            for (int row = 0; row < MAP_H; row++) {
                for (int column = 0; column < MAP_W; column++) {
                    objectives += loaded.map.tiles[row][column] == TILE_FOREST_WARDEN;
                }
            }
            for (int enemy = 0; enemy < loaded.enemy_count; enemy++) {
                objectives += loaded.enemies[enemy].active && loaded.enemies[enemy].dain_fragment == DAIN_FRAGMENT_BOMBER;
            }
            int enemies = loaded.enemy_count;
            game_refresh_quest_encounters(&loaded);
            ASSERT("a missing second objective appears once on cached level three without resetting enemies", loaded.level == 3 &&
                objectives == 1 && loaded.enemies[0].hp == 17 && loaded.enemy_count == enemies);
            ASSERT("relocated objectives and partial progress survive another save", save_game(&loaded, ROUTE_SAVE_SLOT) &&
                load_game(&reloaded, ROUTE_SAVE_SLOT) && reloaded.level == 3 &&
                reloaded.alder_wardens_rescued == 1 && reloaded.dain_map_fragments == 1 && reloaded.enemy_count == enemies);
        }
    }
    remove("saves/savegame_99123.json");
}

static void test_mountain_climb_migration(void) {
    game_init(&original);
    original.location = LOCATION_MOUNTAINS;
    original.level = 3;
    original.mountain_entry_town = LOCATION_TOWN;
    original.mountain_portal_town = LOCATION_TOWN;
    original.watchfire_quest_state = 1;
    map_generate_mountains(&original.map, 3);
    enemies_spawn(&original);
    // Recreate the old level-six watchfire using the current placement code.
    original.level = WATCHFIRE_MOUNTAINS_LEVEL;
    game_refresh_quest_encounters(&original);
    int defenders = 0;
    for (int i = 0; i < original.enemy_count; i++) {
        Enemy *enemy = &original.enemies[i];
        if (strcmp(enemy->name, "Watchfire Defender") == 0) {
            enemy->hp = defenders ? 7 : 0;
            enemy->active = defenders != 0;
            defenders++;
        }
    }
    original.level = 6;
    original.max_mountain_level_reached = 6;
    original.gold = 617;
    original.player.x = original.map.stairs_up_x;
    original.player.y = original.map.stairs_up_y;
    int ok = make_legacy_save(170) && load_game(&loaded, ROUTE_SAVE_SLOT);
    ASSERT("version 170 mountain slopes migrate into the five-level climb", ok && loaded.location == LOCATION_MOUNTAINS && loaded.level == 3 && loaded.mountain_entry_town == LOCATION_DAWNMERE && loaded.gold == 617);
    int old_guards = 0;
    for (int i = 0; i < loaded.enemy_count; i++) {
        old_guards += strcmp(loaded.enemies[i].name, "Watchfire Defender") == 0;
    }
    LevelCache *summit = &loaded.mountain_cache[WATCHFIRE_MOUNTAINS_LEVEL - 1];
    int wounded = 0;
    int dead = 0;
    for (int i = 0; i < summit->enemy_count; i++) {
        Enemy *enemy = &summit->enemies[i];
        if (strcmp(enemy->name, "Watchfire Defender") == 0) {
            wounded += enemy->active && enemy->hp == 7;
            dead += !enemy->active && enemy->hp == 0;
            ASSERT("relocated watchfire defenders land on walkable summit tiles", map_is_walkable(&summit->map, enemy->x, enemy->y));
        }
    }
    ASSERT("watchfire migration moves the encounter with wounds and deaths intact", defenders == 4 && !old_guards && summit->valid && wounded == 3 && dead == 1 && (loaded.watchfire_quest_encounters & 1));
    ASSERT("relocated defender damage survives another save", save_game(&loaded, ROUTE_SAVE_SLOT) && load_game(&reloaded, ROUTE_SAVE_SLOT) && reloaded.mountain_cache[4].enemy_count == summit->enemy_count);

    // A summit portal can coexist with wounded defenders on the old sixth slope.
    LevelCache *slope = &original.mountain_cache[5];
    slope->valid = 1;
    slope->map = original.map;
    slope->enemy_count = original.enemy_count;
    memcpy(slope->enemies, original.enemies, sizeof(slope->enemies));
    original.watchfire_quest_state = 0;
    original.level = 5;
    map_generate_mountains(&original.map, 5);
    enemies_spawn(&original);
    original.watchfire_quest_state = 1;
    original.level = 4;
    map_room_center(&original.map.rooms[original.map.room_count / 2], &original.player.x, &original.player.y);
    // This fixture needs a valid landing, independent of random regular spawns.
    for (int i = 0; i < original.enemy_count; i++) {
        Enemy *enemy = &original.enemies[i];
        if (enemy->x == original.player.x && enemy->y == original.player.y) {
            enemy->active = 0;
            enemy->hp = 0;
        }
    }
    game_open_town_portal(&original);
    ok = make_legacy_save(170) && load_game(&loaded, ROUTE_SAVE_SLOT);
    ASSERT("watchfire relocation preserves a saved summit portal", ok && !loaded.portal_active && loaded.portal_level == 5);
    int blocked_landing = 0;
    for (int i = 0; i < loaded.mountain_cache[4].enemy_count; i++) {
        Enemy *enemy = &loaded.mountain_cache[4].enemies[i];
        blocked_landing += strcmp(enemy->name, "Watchfire Defender") == 0 && enemy->active && enemy->x == loaded.portal_x && enemy->y == loaded.portal_y;
    }
    ASSERT("relocated defenders leave the saved portal landing clear", !blocked_landing);
    test_restore_cached_floor(&loaded);
    ASSERT("the summit portal restores its original position beside the relocated encounter", loaded.location == LOCATION_MOUNTAINS && loaded.level == 5 && loaded.player.x == original.portal_x && loaded.player.y == original.portal_y);

    game_init(&original);
    original.location = LOCATION_MOUNTAINS;
    original.level = 1;
    original.mountain_entry_town = LOCATION_TOWN;
    original.mountain_portal_town = LOCATION_TOWN;
    map_generate_mountains(&original.map, 1);
    enemies_spawn(&original);
    game_open_town_portal(&original);
    ASSERT("legacy fixture records a mountain escape in Oakhaven", original.location == LOCATION_TOWN && original.map.tiles[2][21] != TILE_PORTAL);
    original.player.x = 20;
    original.player.y = 0;
    original.floor_item_count = 1;
    original.floor_items[0] = (FloorItem){.active = 1, .x = 20, .y = 0, .underlying_tile = TILE_TOWN_EXIT, .item = item_make_health_potion()};
    original.map.tiles[0][20] = TILE_ITEM;
    ok = make_legacy_save(170) && load_game(&loaded, ROUTE_SAVE_SLOT);
    ASSERT("legacy mountain return anchors retire without moving the player from Oakhaven", ok && loaded.location == LOCATION_TOWN && loaded.map.tiles[2][21] != TILE_PORTAL && loaded.mountain_portal_town == LOCATION_DAWNMERE);
    ASSERT("the new Elderwood gate leaves the saved player and loot accessible", loaded.player.x == 20 && loaded.player.y == 1 && loaded.map.tiles[0][20] == TILE_TOWN_EXIT && loaded.floor_items[0].active && loaded.floor_items[0].y == 1 && loaded.floor_items[0].underlying_tile == TILE_TOWN_PATH && loaded.map.tiles[1][20] == TILE_ITEM);
    action_resolve_player(&loaded, (Action){ACTION_MOVE, 21, 2});
    ASSERT("an old Oakhaven return portal is retired", loaded.location == LOCATION_TOWN && !loaded.portal_active);
    test_restore_cached_floor(&loaded);
    ASSERT("the migrated mountain cache retains its expedition", loaded.location == LOCATION_MOUNTAINS && loaded.level == 1 && loaded.mountain_entry_town == LOCATION_DAWNMERE);
    game_return_to_town(&loaded);
    ASSERT("future mountain returns use Dawnmere", loaded.location == LOCATION_DAWNMERE);
    remove("saves/savegame_99123.json");
}

static void test_forest_adventure_migration(void) {
    game_init(&original);
    original.location = LOCATION_FOREST;
    original.level = 2;
    original.forest_entry_town = LOCATION_TOWN2;
    original.forest_portal_town = LOCATION_TOWN;
    original.alder_quest_state = 1;
    original.alder_wardens_rescued = ALDER_WARDEN_STAGE_2;
    original.gold = 617;
    original.max_forest_level_reached = 7;
    for (int i = 0; i < 7; i++) {
        LevelCache *cache = &original.forest_cache[i];
        map_generate_forest(&cache->map, i == 3 ? FOREST_BOSS_LEVEL : 2);
        cache->valid = 1;
        cache->enemy_count = 1;
        int x;
        int y;
        map_room_center(&cache->map.rooms[0], &x, &y);
        cache->enemies[0] = (Enemy){.active = 1, .x = x + 1, .y = y, .hp = 50 + i, .max_hp = 100,
            .type = i == 3 ? ENEMY_FOREST_NECROMANCER : ENEMY_BLIGHTED_WOLF, .is_boss = i == 3,
            .attack_phase = i == 3 ? 1 : 0, .move_timer = i == 3 ? 2 : 0, .boss_wave_used = i == 3 ? 3 : 0};
        map_mark_explored(&cache->map, x, y);
    }
    original.map = original.forest_cache[1].map;
    original.enemy_count = 1;
    original.enemies[0] = original.forest_cache[1].enemies[0];
    map_room_center(&original.map.rooms[0], &original.player.x, &original.player.y);
    original.floor_item_count = 1;
    original.floor_items[0] = (FloorItem){.active = 1, .x = original.player.x, .y = original.player.y,
        .underlying_tile = TILE_FOREST_SHORTCUT, .item = item_make_health_potion()};
    original.map.tiles[original.player.y][original.player.x] = TILE_ITEM;
    original.portal_active = 1;
    original.portal_location = LOCATION_FOREST;
    original.portal_level = 7;
    original.portal_x = original.forest_cache[6].map.stairs_up_x;
    original.portal_y = original.forest_cache[6].map.stairs_up_y;
    original.portal_origin_tile = TILE_FOREST_SHORTCUT;
    original.forest_cache[6].map.tiles[original.portal_y][original.portal_x] = TILE_PORTAL;
    snprintf(original.dialogue_speaker, MAX_SPEAKER_LEN, "Take shortcut?");
    original.dialogue_active = 1;
    int ok = make_legacy_save(171) && load_game(&loaded, ROUTE_SAVE_SLOT);
    ASSERT("version 171 forest migration keeps character and rescue progress", ok && loaded.level == 2 && loaded.gold == 617 && loaded.alder_wardens_rescued == ALDER_WARDEN_STAGE_2 && loaded.alder_quest_state == 1 && loaded.forest_entry_town == LOCATION_TOWN2);
    ASSERT("merging outer trails preserves distinct active and portal snapshots", loaded.enemies[0].hp == 51 && loaded.portal_level == 3 && loaded.forest_cache[2].enemies[0].hp == 56 && loaded.forest_portal_town == LOCATION_TOWN);
    ASSERT("the Necromancer moves to five with damage, phases and waves intact", loaded.forest_cache[4].valid && loaded.forest_cache[4].enemies[0].hp == 53 && loaded.forest_cache[4].enemies[0].attack_phase == 1 && loaded.forest_cache[4].enemies[0].move_timer == 2 && loaded.forest_cache[4].enemies[0].boss_wave_used == 3);
    ASSERT("retired shortcut prompts and loot underlays migrate without losing loot", !loaded.dialogue_active && loaded.floor_items[0].active && loaded.floor_items[0].underlying_tile == TILE_FOREST_FLOOR && loaded.map.tiles[loaded.player.y][loaded.player.x] == TILE_ITEM && loaded.portal_origin_tile == TILE_FOREST_FLOOR);
    ASSERT("active map exploration and saved coordinates survive the merge", loaded.player.x == original.player.x && loaded.player.y == original.player.y && map_is_explored(&loaded.map, loaded.player.x, loaded.player.y));
    ASSERT("the new forest save format does not remap a second time", save_game(&loaded, ROUTE_SAVE_SLOT) && load_game(&reloaded, ROUTE_SAVE_SLOT) && reloaded.level == 2 && reloaded.portal_level == 3 && reloaded.forest_cache[4].enemies[0].hp == 53);
    test_restore_cached_floor(&reloaded);
    ASSERT("the old outer-trail portal still restores its map and landing", reloaded.location == LOCATION_FOREST && reloaded.level == 3 && reloaded.player.x == original.portal_x && reloaded.player.y == original.portal_y && reloaded.enemies[0].hp == 56);

    game_init(&original);
    original.defeated_bosses |= 1 << LOCATION_FOREST;
    approach_enter(&original, 0, 12);
    // A player on the former crossroads arm can be saved off the new road.
    original.player.x = 55;
    original.player.y = 31;
    map_mark_explored(&original.map, 55, 31);
    original.floor_item_count = 1;
    original.floor_items[0] = (FloorItem){.active = 1, .x = 54, .y = 31,
        .underlying_tile = TILE_FOREST_FLOOR, .item = item_make_health_potion()};
    original.map.tiles[31][54] = TILE_ITEM;
    ok = make_legacy_save(171) && load_game(&loaded, ROUTE_SAVE_SLOT);
    int x;
    int y;
    ASSERT("old crossroads saves gain the boss-unlocked forest road", ok && loaded.location == LOCATION_APPROACH && approach_forest_town_exit(&loaded, &x, &y) && !approach_boss_exit(&loaded, &x, &y));
    ASSERT("crossroads migration preserves exploration and accessible player and loot positions", loaded.player.x == 55 && loaded.player.y == 31 && map_is_explored(&loaded.map, 55, 31) && map_is_walkable(&loaded.map, 55, 31) && loaded.floor_items[0].active && loaded.floor_items[0].underlying_tile == TILE_TOWN_FLOOR && loaded.map.tiles[31][54] == TILE_ITEM);
    remove("saves/savegame_99123.json");
}

static void test_swamp_adventure_migration(void) {
    const int stages[7] = {1, 2, 3, 5, 4, 2, 1};
    for (int old = 1; old <= 7; old++) {
        game_init(&original);
        original.location = LOCATION_SWAMP;
        original.level = old;
        original.max_swamp_level_reached = 7;
        original.swamp_entry_town = LOCATION_TOWN3;
        original.swamp_portal_town = LOCATION_TOWN3;
        original.innkeeper_quest_state = 1;
        original.gold = 321;
        for (int i = 0; i < 7; i++) {
            LevelCache *cache = &original.swamp_cache[i];
            cache->valid = 1;
            map_generate_swamp(&cache->map, 1);
            cache->enemy_count = 1;
            cache->enemies[0] = (Enemy){.active = 1, .type = i == 3 ? ENEMY_SWAMP_DEMON : ENEMY_VAMPIRE, .hp = 41 + i, .max_hp = 180, .x = 20, .y = 20, .move_timer = 2};
            cache->map.tiles[20][20] = TILE_SWAMP_FLOOR;
            cache->map.tiles[20][21] = i == 2 ? TILE_SWAMP_DAUGHTER : TILE_SWAMP_SHORTCUT;
            map_mark_explored(&cache->map, 20, 20);
        }
        original.map = original.swamp_cache[old - 1].map;
        original.enemy_count = 1;
        original.enemies[0] = original.swamp_cache[old - 1].enemies[0];
        original.enemies[0].hp = 17;
        original.player.x = 20;
        original.player.y = 20;
        original.floor_item_count = 1;
        original.floor_items[0] = (FloorItem){.active = 1, .x = 20, .y = 20, .underlying_tile = TILE_SWAMP_SHORTCUT, .item = item_make_health_potion()};
        original.map.tiles[20][20] = TILE_ITEM;
        int other = old == 1 ? 7 : old == 2 ? 6 : 1;
        original.portal_active = 1;
        original.portal_location = LOCATION_SWAMP;
        original.portal_level = other;
        original.portal_x = 22;
        original.portal_y = 20;
        original.portal_origin_tile = TILE_SWAMP_SHORTCUT;
        original.swamp_cache[other - 1].map.tiles[20][22] = TILE_PORTAL;
        original.dialogue_active = 1;
        snprintf(original.dialogue_speaker, MAX_SPEAKER_LEN, "Shortcut found");
        int ok = make_legacy_save(172) && load_game(&loaded, ROUTE_SAVE_SLOT);
        ASSERT("all seven old swamp floors migrate with active wounds, loot and exploration", ok && loaded.level == stages[old - 1] && loaded.max_swamp_level_reached == 5 && loaded.enemies[0].hp == 17 && loaded.player.x == 20 && loaded.player.y == 20 && loaded.map.tiles[20][20] == TILE_ITEM && loaded.floor_items[0].underlying_tile == TILE_SWAMP_FLOOR && map_is_explored(&loaded.map, 20, 20) && loaded.gold == 321 && !loaded.dialogue_active);
        ASSERT("Mira's original third-level map and the fourth-level Demon are preserved", ok && loaded.swamp_cache[2].map.tiles[20][21] == TILE_SWAMP_DAUGHTER && loaded.swamp_cache[2].enemies[0].hp == 43 && loaded.swamp_cache[4].enemies[0].type == ENEMY_SWAMP_DEMON && loaded.swamp_cache[4].enemies[0].hp == 44);
        ASSERT("swamp migration retires surplus caches and preserves travel anchors", ok && !loaded.swamp_cache[5].valid && !loaded.swamp_cache[6].valid && loaded.swamp_entry_town == LOCATION_TOWN3 && loaded.swamp_portal_town == LOCATION_TOWN3 && loaded.portal_origin_tile == TILE_SWAMP_FLOOR);
        if (ok) {
            int stage = loaded.level;
            int portal = loaded.portal_level;
            ASSERT("swamp migration is stable after resaving", save_game(&loaded, ROUTE_SAVE_SLOT) && load_game(&reloaded, ROUTE_SAVE_SLOT) && reloaded.level == stage && reloaded.portal_level == portal && reloaded.enemies[0].hp == 17);
            game_return_to_town(&loaded);
            test_restore_cached_floor(&loaded);
            ASSERT("merged outer swamp floors retain the separate linked portal snapshot", loaded.location == LOCATION_SWAMP && loaded.level == portal && loaded.player.x == 22 && loaded.player.y == 20 && loaded.enemies[0].hp == 40 + other && loaded.swamp_entry_town == LOCATION_TOWN3);
        }
    }
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
    test_legacy_region(0, 3, 4);
    test_legacy_region(0, 4, SWAMP_RESCUE_LEVEL);
    test_legacy_region(0, 5, SWAMP_BOSS_LEVEL);
    for (int forest = 0; forest < 2; forest++) {
        prepare_legacy_region(forest, forest ? 8 : 5);
        original.defeated_bosses = 1 << original.location;
        original.enemies[0].active = 0;
        LevelCache *cache = forest ? original.forest_cache : original.swamp_cache;
        cache[original.level - 1].enemies[0].active = 0;
        original.last_town_visited = forest ? LOCATION_TOWN : LOCATION_TOWN2;
        game_open_town_portal(&original);
        int ok = make_legacy_save(forest ? 76 : 77) && load_game(&loaded, ROUTE_SAVE_SLOT);
        ASSERT("old portal save loads and keeps its original town", ok && loaded.location == (forest ? LOCATION_TOWN : LOCATION_TOWN2) && !loaded.portal_active);
        test_restore_cached_floor(&loaded);
        ASSERT("old boss portal restores the central level and casting position", ok && loaded.level == 5 && loaded.player.x == original.portal_x && loaded.player.y == original.portal_y);
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
    ASSERT("forest boss cache advances while swamp travel anchors remain unchanged", ok && loaded.forest_entry_town == LOCATION_TOWN2 && loaded.forest_portal_town == LOCATION_TOWN2 && loaded.forest_cache[4].valid && memcmp(loaded.forest_cache[4].map.rooms, original.forest_cache[3].map.rooms, sizeof(original.forest_cache[3].map.rooms)) == 0 && loaded.defeated_bosses == original.defeated_bosses);
    remove("saves/savegame_99123.json");
    test_mountain_migration();
    test_mountain_climb_migration();
    test_split_forest_migration();
    test_forest_adventure_migration();
    test_split_mountain_migration();
    test_near_side_quest_migration();
    test_swamp_adventure_migration();
}
