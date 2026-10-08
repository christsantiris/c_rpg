#include "test_utils.h"
#include "game/game.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define ELOWEN_SAVE_SLOT 99140
#define ELOWEN_SAVE_PATH "saves/savegame_99140.json"
static GameState game;
static GameState loaded;

static void start(void) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
}

static int legacy_save(void) {
    if (!save_game(&game, ELOWEN_SAVE_SLOT)) {
        return 0;
    }
    FILE *file = fopen(ELOWEN_SAVE_PATH, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *buffer = malloc((size_t)size + 1);
    if (!buffer) {
        fclose(file);
        return 0;
    }
    size_t read = fread(buffer, 1, (size_t)size, file);
    buffer[read] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 103);
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!json) {
        return 0;
    }
    file = fopen(ELOWEN_SAVE_PATH, "wb");
    int written = file && fputs(json, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(json);
    return written;
}

static void test_tavern(void) {
    start();
    game_enter_tavern(&game);
    ASSERT("Oakhaven's Tavern houses Elowen beside the other quest givers", game.map.tiles[ELOWEN_TAVERN_Y][ELOWEN_TAVERN_X] == TILE_NPC_ELOWEN &&
        game.map.tiles[BRENNA_Y][BRENNA_X] == TILE_NPC_BRENNA && game.map.tiles[MARA_TAVERN_Y][MARA_TAVERN_X] == TILE_NPC_MARA &&
        game.map.tiles[ILYA_TAVERN_Y][ILYA_TAVERN_X] == TILE_NPC_DRAGON_SEEKER &&
        !map_is_walkable(&game.map, ELOWEN_TAVERN_X, ELOWEN_TAVERN_Y) && map_is_walkable(&game.map, ELOWEN_TAVERN_X, ELOWEN_TAVERN_Y + 1));
    game_talk_to_elowen(&game);
    ASSERT("Elowen cannot assign her quest remotely inside the Tavern", !game.elowen_quest_state && !game.dialogue_active);
    game.player.x = ELOWEN_TAVERN_X;
    game.player.y = ELOWEN_TAVERN_Y + 1;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("Elowen still requires conversation rather than Action", !game.elowen_quest_state);
    game_talk_to_elowen(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    ASSERT("conversation in the Tavern assigns the unchanged local dungeon quest", game.elowen_quest_state == 1 &&
        strstr(game.dialogue_text, "Oakhaven's dungeon") && strstr(game.dialogue_text, "Oakhaven's Tavern") &&
        game.dialogue_x == ELOWEN_TAVERN_X && game.dialogue_y == ELOWEN_TAVERN_Y);
    QuestJournalEntry entry;
    ASSERT("the quest journal retains floors 2, 3, 4 and identifies the new return location", quest_journal_get_entry(&game, QUEST_TAB_ACTIVE, 0, &entry) &&
        entry.stages[0] == 2 && entry.stages[1] == 3 && entry.stages[2] == 4 &&
        entry.reward_gold == 40 && entry.reward_score == 300 && strstr(entry.summary_line_2, "Oakhaven's Tavern"));
    game.elowen_quest_state = 2;
    game.elowen_seals_restored = 7;
    int gold = game.gold;
    int score = game.score;
    ASSERT("ready-to-return progress survives saving in the Tavern", save_game(&game, ELOWEN_SAVE_SLOT) && load_game(&loaded, ELOWEN_SAVE_SLOT) &&
        loaded.elowen_quest_state == 2 && loaded.elowen_seals_restored == 7 && loaded.location == LOCATION_TAVERN);
    game = loaded;
    game_talk_to_elowen(&game);
    game_talk_to_elowen(&game);
    ASSERT("Elowen awards the same reward once at the new location", game.elowen_quest_state == 3 && game.gold == gold + 40 && game.score == score + 300);
    game.player.x = game.map.stairs_down_x;
    game.player.y = game.map.stairs_down_y - 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, game.map.stairs_down_x, game.map.stairs_down_y});
    ASSERT("leaving the Tavern returns to Oakhaven and preserves completion", game.location == LOCATION_TOWN && game.elowen_quest_state == 3);
    game_enter_inn(&game);
    game.player.x = ELOWEN_TAVERN_X;
    game.player.y = ELOWEN_TAVERN_Y + 1;
    game.elowen_quest_state = 0;
    game_talk_to_elowen(&game);
    ASSERT("the Inn loses Elowen while retaining Alder, Rook, and Bram", game.map.tiles[ELOWEN_TAVERN_Y][ELOWEN_TAVERN_X] == TILE_TAVERN_FLOOR &&
        game.map.tiles[18][10] == TILE_NPC_ROOK && game.map.tiles[7][28] == TILE_NPC_INNKEEPER &&
        game.map.tiles[ALDER_INN_Y][ALDER_INN_X] == TILE_NPC_ALDER && !game.elowen_quest_state && !game.dialogue_active);
    int x;
    int y;
    map_generate_guild(&loaded.map, &x, &y);
    ASSERT("the shared Guild template does not gain Elowen", loaded.map.tiles[ELOWEN_TAVERN_Y][ELOWEN_TAVERN_X] == TILE_TAVERN_FLOOR);
}

static void test_migration(void) {
    start();
    game_enter_tavern(&game);
    game.player.x = ELOWEN_TAVERN_X;
    game.player.y = ELOWEN_TAVERN_Y;
    game.elowen_quest_state = 2;
    game.elowen_seals_restored = 7;
    game.gold = 617;
    game.defeated_bosses = 1 << LOCATION_DUNGEON;
    game.level_cache[2].valid = 1;
    game.level_cache[2].map = game.map;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = ELOWEN_TAVERN_X, .y = ELOWEN_TAVERN_Y,
        .underlying_tile = TILE_TAVERN_FLOOR, .item = item_make_health_potion()};
    game.map.tiles[ELOWEN_TAVERN_Y][ELOWEN_TAVERN_X] = TILE_ITEM;
    ASSERT("legacy Tavern saves gain Elowen without resetting quest, boss, or cached exploration", legacy_save() && load_game(&loaded, ELOWEN_SAVE_SLOT) &&
        loaded.map.tiles[ELOWEN_TAVERN_Y][ELOWEN_TAVERN_X] == TILE_NPC_ELOWEN && loaded.elowen_quest_state == 2 && loaded.elowen_seals_restored == 7 &&
        loaded.gold == 617 && loaded.defeated_bosses == game.defeated_bosses && loaded.level_cache[2].valid &&
        memcmp(&loaded.level_cache[2].map, &game.level_cache[2].map, sizeof(Map)) == 0);
    ASSERT("migration safely moves overlapping players and loot beside Elowen", loaded.player.x == ELOWEN_TAVERN_X && loaded.player.y == ELOWEN_TAVERN_Y + 1 &&
        loaded.floor_items[0].active && loaded.floor_items[0].x == ELOWEN_TAVERN_X && loaded.floor_items[0].y == ELOWEN_TAVERN_Y + 1 &&
        loaded.floor_items[0].underlying_tile == TILE_TAVERN_FLOOR && loaded.map.tiles[ELOWEN_TAVERN_Y + 1][ELOWEN_TAVERN_X] == TILE_ITEM);
    int score = loaded.score;
    game_talk_to_elowen(&loaded);
    game_talk_to_elowen(&loaded);
    ASSERT("a migrated completed objective awards the original reward once", loaded.elowen_quest_state == 3 && loaded.gold == 657 && loaded.score == score + 300);
    game = loaded;
    game_enter_inn(&game);
    game.map.tiles[7][10] = TILE_NPC_ELOWEN;
    game.dialogue_active = 1;
    ASSERT("legacy Inn saves remove Elowen and stale dialogue without resetting completion", legacy_save() && load_game(&loaded, ELOWEN_SAVE_SLOT) &&
        loaded.map.tiles[7][10] == TILE_TAVERN_FLOOR && !loaded.dialogue_active && loaded.elowen_quest_state == 3 && loaded.elowen_seals_restored == 7 && loaded.gold == 657);
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 10, .y = 7, .underlying_tile = TILE_NPC_ELOWEN, .item = item_make_health_potion()};
    game.map.tiles[7][10] = TILE_ITEM;
    ASSERT("legacy Inn loot loses Elowen's underlay without losing the item", legacy_save() && load_game(&loaded, ELOWEN_SAVE_SLOT) &&
        loaded.floor_items[0].active && loaded.floor_items[0].underlying_tile == TILE_TAVERN_FLOOR && loaded.map.tiles[7][10] == TILE_ITEM);
    ASSERT("migrated Inn saves round-trip without reintroducing Elowen", save_game(&loaded, ELOWEN_SAVE_SLOT) && load_game(&loaded, ELOWEN_SAVE_SLOT) &&
        loaded.map.tiles[7][10] == TILE_ITEM && loaded.elowen_quest_state == 3);
}

void test_elowen_relocation(void) {
    printf("Elowen relocation to Oakhaven's Tavern\n");
    ASSERT("Elowen test save slot is unused", !save_exists(ELOWEN_SAVE_SLOT));
    test_tavern();
    test_migration();
    remove(ELOWEN_SAVE_PATH);
}
