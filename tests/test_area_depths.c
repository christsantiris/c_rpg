#include "test_utils.h"
#include "game/game.h"
#include "systems/save_load.h"
#include "screens/quest_journal.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define DEPTH_TEST_SLOT 99136
static GameState game;
static GameState loaded;

static int legacy_save(int temple, int labyrinth) {
    if (!save_game(&game, DEPTH_TEST_SLOT)) {
        return 0;
    }
    FILE *file = fopen("saves/savegame_99136.json", "r");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    char *data = malloc(length + 1);
    if (!data) {
        fclose(file);
        return 0;
    }
    size_t read = fread(data, 1, length, file);
    data[read] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 84);
    const char *names[2] = {"temple_cache", "labyrinth_cache"};
    int lengths[2] = {temple, labyrinth};
    for (int i = 0; i < 2; i++) {
        cJSON *cache = cJSON_GetObjectItem(root, names[i]);
        while (cJSON_GetArraySize(cache) > lengths[i]) {
            cJSON_DeleteItemFromArray(cache, lengths[i]);
        }
    }
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!data) {
        return 0;
    }
    file = fopen("saves/savegame_99136.json", "w");
    if (!file) {
        free(data);
        return 0;
    }
    fputs(data, file);
    fclose(file);
    free(data);
    return load_game(&loaded, DEPTH_TEST_SLOT);
}

static void start(void) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_MAGE;
    game_init(&game);
    game.gold = 217;
    game.score = 4321;
    game.player.hp = 29;
}

static void test_dungeon_migration(void) {
    static const int stages[8] = {1, 2, 3, 3, 4, 4, 4, 5};
    for (int old = 1; old <= 8; old++) {
        start();
        game.location = LOCATION_DUNGEON;
        game.level = old;
        game.max_level_reached = old;
        game.elowen_quest_state = 1;
        game.elowen_seals_restored = 5;
        map_generate(&game.map, stages[old - 1]);
        game.player.x = game.map.stairs_up_x;
        game.player.y = game.map.stairs_up_y;
        game.enemy_count = 1;
        game.enemies[0] = (Enemy){.active = 1, .type = old == 8 ? ENEMY_LICH_KING : ENEMY_SKELETON, .is_boss = old == 8, .x = game.map.stairs_down_x + 1, .y = game.map.stairs_down_y, .hp = 17, .max_hp = 140, .attack_target_x = -1, .attack_target_y = -1};
        int x;
        int y;
        map_room_center(&game.map.rooms[game.map.room_count / 2], &x, &y);
        if (old == 2 || old == 4 || old == 6) {
            game.map.tiles[y][x] = old == 4 ? TILE_BROKEN_BURIAL_SEAL : TILE_RESTORED_BURIAL_SEAL;
        }
        game.floor_item_count = 1;
        game.floor_items[0] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = game.map.tiles[y][x], .item = item_make_health_potion()};
        game.map.tiles[y][x] = TILE_ITEM;
        int ok = legacy_save(4, 3);
        ASSERT("every old dungeon floor migrates into the five-floor route",
            ok && loaded.level == stages[old - 1] && loaded.max_level_reached == stages[old - 1]);
        if (!ok) {
            continue;
        }
        ASSERT("dungeon migration retains player, seals, loot and wounded enemies",
            loaded.player.x == game.player.x && loaded.player.y == game.player.y &&
            loaded.player.hp == 29 && loaded.gold == 217 && loaded.score == 4321 &&
            loaded.elowen_quest_state == 1 && loaded.elowen_seals_restored == 5 &&
            loaded.enemies[0].hp == 17 && loaded.enemies[0].type == game.enemies[0].type &&
            loaded.floor_items[0].active && loaded.map.tiles[y][x] == TILE_ITEM);
        ASSERT("the old dungeon's removed cache slots are invalidated",
            !loaded.level_cache[5].valid && !loaded.level_cache[6].valid && !loaded.level_cache[7].valid);
        if (old == 3 || old == 5 || old == 7) {
            ASSERT("new seal placement preserves loot covering its position",
                loaded.floor_items[0].underlying_tile ==
                    (old == 3 ? TILE_BROKEN_BURIAL_SEAL : TILE_RESTORED_BURIAL_SEAL));
        }
        int stage = loaded.level;
        ASSERT("migrated dungeon progress remains stable after resaving",
            save_game(&loaded, DEPTH_TEST_SLOT) && load_game(&loaded, DEPTH_TEST_SLOT) &&
            loaded.level == stage && loaded.enemies[0].hp == 17 && loaded.elowen_seals_restored == 5);
    }
    for (int old = 1; old <= 8; old++) {
        start();
        game.portal_active = 1;
        game.portal_location = LOCATION_DUNGEON;
        game.portal_level = old;
        game.max_level_reached = 8;
        LevelCache *cache = &game.level_cache[old - 1];
        cache->valid = 1;
        map_generate(&cache->map, stages[old - 1]);
        game.portal_x = cache->map.stairs_up_x;
        game.portal_y = cache->map.stairs_up_y;
        game.portal_origin_tile = TILE_STAIRS_UP;
        cache->map.tiles[game.portal_y][game.portal_x] = TILE_PORTAL;
        int ok = legacy_save(4, 3);
        if (ok) {
            game_use_town_portal(&loaded);
        }
        ASSERT("every old dungeon portal returns to its preserved destination",
            ok && loaded.location == LOCATION_DUNGEON && loaded.level == stages[old - 1] &&
            loaded.player.x == game.portal_x && loaded.player.y == game.portal_y &&
            loaded.max_level_reached == DUNGEON_DEPTH);
    }
}

static void test_expanded_migration(Location region, int old_depth) {
    start();
    if (region == LOCATION_TEMPLE) {
        game.temple_treasure_state = 1;
        game_enter_temple(&game);
        while (game.level < TEMPLE_DEPTH) {
            game_descend(&game);
        }
        game.player.x = 32;
        game.player.y = 30;
        game_interact_temple(&game);
        game.player.x = game.map.stairs_up_x;
        game.player.y = game.map.stairs_up_y;
    } else {
        game.rook_quest_state = 1;
        game.rook_labyrinth_switches = 31;
        game_enter_labyrinth(&game);
        while (game.level < LABYRINTH_DEPTH) {
            game_change_labyrinth_floor(&game, 1, 0);
        }
        game.rook_labyrinth_switches = 7;
    }
    LevelCache *cache = region == LOCATION_TEMPLE ? game.temple_cache : game.labyrinth_cache;
    cache[old_depth - 1].valid = 1;
    cache[old_depth - 1].map = game.map;
    cache[old_depth - 1].enemy_count = game.enemy_count;
    memcpy(cache[old_depth - 1].enemies, game.enemies, sizeof(cache[old_depth - 1].enemies));
    int boss = -1;
    for (int i = 0; i < game.enemy_count; i++) {
        if (game.enemies[i].is_boss) {
            boss = i;
        }
    }
    ASSERT("expanded area migration has an existing final boss", boss >= 0);
    if (boss < 0) {
        return;
    }
    game.enemies[boss].hp = 37;
    cache[old_depth - 1].enemies[boss].hp = 37;
    game.level = old_depth;
    game.max_temple_level_reached = region == LOCATION_TEMPLE ? old_depth : 1;
    game.portal_active = 1;
    game.portal_location = region;
    game.portal_level = old_depth;
    game.portal_x = game.player.x;
    game.portal_y = game.player.y;
    int ok = legacy_save(4, 3);
    LevelCache *restored = region == LOCATION_TEMPLE ? loaded.temple_cache : loaded.labyrinth_cache;
    ASSERT("old temple and labyrinth finales move intact to level five",
        ok && loaded.level == 5 && loaded.portal_level == 5 && restored[4].valid &&
        !restored[old_depth - 1].valid && loaded.enemies[boss].hp == 37 &&
        restored[4].enemies[boss].hp == 37 && loaded.gold == game.gold && loaded.score == game.score &&
        loaded.player.x == game.player.x && loaded.player.y == game.player.y &&
        memcmp(loaded.map.tiles, game.map.tiles, sizeof(game.map.tiles)) == 0);
    if (region == LOCATION_TEMPLE) {
        ASSERT("temple migration retains summit reach and quest state",
            ok && loaded.max_temple_level_reached == 5 && loaded.temple_treasure_state == 1 &&
            loaded.temple_alignment == 1 && loaded.temple_sentinels_awakened == 1);
    } else {
        ASSERT("old labyrinth runes keep the vault open with credit for inserted floors",
            ok && loaded.rook_labyrinth_switches == 31 && loaded.rook_quest_state == 1);
    }
    ASSERT("expanded area migrations run once",
        ok && save_game(&loaded, DEPTH_TEST_SLOT) && load_game(&loaded, DEPTH_TEST_SLOT) &&
        loaded.level == 5 && restored[4].enemies[boss].hp == 37);
    if (region == LOCATION_TEMPLE && ok) {
        game_return_to_town(&loaded);
        game_use_town_portal(&loaded);
        ASSERT("the former summit portal returns to tier five with its damaged Guardian",
            loaded.location == LOCATION_TEMPLE && loaded.level == 5 &&
            loaded.enemies[boss].hp == 37 && loaded.temple_alignment == 1);
    }
    game.defeated_bosses |= 1 << region;
    game.enemies[boss].active = 0;
    cache[old_depth - 1].enemies[boss].active = 0;
    game.temple_treasure_state = 3;
    game.rook_quest_state = 3;
    ok = legacy_save(4, 3);
    ASSERT("completed expanded areas preserve dead bosses and completed quests",
        ok && (loaded.defeated_bosses & (1 << region)) && !loaded.enemies[boss].active &&
        !restored[4].enemies[boss].active &&
        (region == LOCATION_TEMPLE ? loaded.temple_treasure_state == 3 : loaded.rook_quest_state == 3));
}

void test_area_depths(void) {
    printf("Area length and migration tests:\n");
    ASSERT("connecting regions retain seven stages and middle bosses",
        FOREST_DEPTH == 7 && MOUNTAIN_DEPTH == 7 && SWAMP_DEPTH == 7 &&
        FOREST_BOSS_LEVEL == 4 && MOUNTAIN_BOSS_LEVEL == 4 && SWAMP_BOSS_LEVEL == 4);
    ASSERT("standalone adventures share five stages",
        DUNGEON_DEPTH == 5 && TEMPLE_DEPTH == 5 && LABYRINTH_DEPTH == 5 &&
        COAST_DEPTH == 5 && DRAGONSPINE_DEPTH == 5 && FROSTFELL_DEPTH == 5 &&
        DESERT_DEPTH == 5 && MOONVEIL_DEPTH == 5 && ASHEN_DEPTH == 5 &&
        GLASSDEEP_DEPTH == 5 && CATACOMBS_DEPTH == 5);
    start();
    game.elowen_quest_state = 1;
    game.temple_treasure_state = 1;
    game.rook_quest_state = 1;
    QuestJournalEntry entry;
    ASSERT("the dungeon journal lists seals on floors two through four",
        quest_journal_get_entry(&game, QUEST_TAB_ACTIVE, 0, &entry) &&
        entry.stages[0] == 2 && entry.stages[1] == 3 && entry.stages[2] == 4);
    ASSERT("the temple and labyrinth journal objectives point to level five",
        quest_journal_get_entry(&game, QUEST_TAB_ACTIVE, 1, &entry) && entry.stages[0] == 5 &&
        quest_journal_get_entry(&game, QUEST_TAB_ACTIVE, 2, &entry) && entry.stages[0] == 5);
    if (save_exists(DEPTH_TEST_SLOT)) {
        ASSERT("area depth temporary save slot must be unused", 0);
        return;
    }
    test_dungeon_migration();
    test_expanded_migration(LOCATION_TEMPLE, 4);
    test_expanded_migration(LOCATION_LABYRINTH, 3);
    remove("saves/savegame_99136.json");
}
