#include "test_utils.h"
#include "game/game.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define MARA_SLOT 99145
#define MARA_SAVE "saves/savegame_99145.json"
static GameState game;
static GameState loaded;

static int legacy_save(void) {
    if (!save_game(&game, MARA_SLOT)) {
        return 0;
    }
    FILE *file = fopen(MARA_SAVE, "rb");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 104);
    char *json = cJSON_Print(root);
    cJSON_Delete(root);
    if (!json) {
        return 0;
    }
    file = fopen(MARA_SAVE, "wb");
    if (!file) {
        free(json);
        return 0;
    }
    int written = fputs(json, file) >= 0;
    fclose(file);
    free(json);
    return written;
}

void test_mara_relocation(void) {
    printf("Mara Tavern and legacy Liora relocation tests:\n");
    ASSERT("Mara test slot is unused", !save_exists(MARA_SLOT));
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    game_enter_town_hall(&game);
    game.player.x = HALL_MARA_X;
    game.player.y = HALL_MARA_Y + 1;
    game_talk_to_mara(&game);
    ASSERT("Mara no longer assigns her quest in the Town Hall", !game.mara_quest_state && !game.dialogue_active &&
        game.map.tiles[HALL_MARA_Y][HALL_MARA_X] == TILE_TAVERN_FLOOR && game.map.tiles[HALL_STEWARD_Y][HALL_STEWARD_X] == TILE_NPC_STEWARD);
    game_enter_tavern(&game);
    ASSERT("Mara occupies her original Tavern position with a walkable approach", game.map.tiles[MARA_TAVERN_Y][MARA_TAVERN_X] == TILE_NPC_MARA &&
        !map_is_walkable(&game.map, MARA_TAVERN_X, MARA_TAVERN_Y) && map_is_walkable(&game.map, MARA_TAVERN_X, MARA_TAVERN_Y + 1));
    game_talk_to_mara(&game);
    ASSERT("Mara cannot assign her quest remotely inside the Tavern", !game.mara_quest_state);
    game.player.x = MARA_TAVERN_X;
    game.player.y = MARA_TAVERN_Y + 1;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("Action does not replace talking to Mara", !game.mara_quest_state);
    game_talk_to_mara(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    ASSERT("Mara assigns the same coast quest from the Tavern", game.mara_quest_state == 1 &&
        game.dialogue_x == MARA_TAVERN_X && game.dialogue_y == MARA_TAVERN_Y && strstr(game.dialogue_text, "south of Oakhaven") &&
        strstr(game.dialogue_text, "Oakhaven's Tavern") && !game.emberforge_quest_state);
    QuestJournalEntry entry;
    ASSERT("Mara's journal retains stages 2, 3, and 4 and directs players to the Tavern", quest_journal_get_entry(&game, QUEST_TAB_ACTIVE, 0, &entry) &&
        entry.stages[0] == 2 && entry.stages[1] == 3 && entry.stages[2] == 4 && strstr(entry.summary_line_2, "Oakhaven's Tavern"));
    ASSERT("current saves retain Mara's Tavern placement and accepted quest", save_game(&game, MARA_SLOT) && load_game(&loaded, MARA_SLOT) &&
        loaded.location == LOCATION_TAVERN && loaded.mara_quest_state == 1 && loaded.map.tiles[MARA_TAVERN_Y][MARA_TAVERN_X] == TILE_NPC_MARA);

    game_leave_tavern(&game);
    game_enter_coast(&game);
    game_descend(&game);
    game.enemies[0].hp = 9;
    game_return_to_town(&game);
    game_enter_tavern(&game);
    game.mara_beacons_lit = 5;
    game.moonveil_quest_state = 2;
    game.moonveil_quest_progress = 7;
    game.moonveil_quest_encounters = 7;
    game.gold = 617;
    game.player.x = MARA_TAVERN_X;
    game.player.y = MARA_TAVERN_Y;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = MARA_TAVERN_X, .y = MARA_TAVERN_Y,
        .underlying_tile = TILE_TAVERN_FLOOR, .item = item_make_short_sword()};
    game.map.tiles[MARA_TAVERN_Y][MARA_TAVERN_X] = TILE_ITEM;
    game.map.tiles[LIORA_INN_Y][LIORA_INN_X] = TILE_NPC_LIORA;
    game.dialogue_active = 1;
    snprintf(game.dialogue_speaker, MAX_SPEAKER_LEN, "Botanist Liora");
    ASSERT("legacy Tavern fixture saves and migrates", legacy_save() && load_game(&loaded, MARA_SLOT));
    ASSERT("migration preserves both quests, gold, and explored coast enemies", loaded.mara_quest_state == 1 && loaded.mara_beacons_lit == 5 &&
        loaded.moonveil_quest_state == 2 && loaded.moonveil_quest_progress == 7 && loaded.moonveil_quest_encounters == 7 &&
        loaded.gold == 617 && loaded.coast_cache[1].valid && loaded.coast_cache[1].enemies[0].hp == 9);
    ASSERT("legacy Tavern gains Mara and safely moves overlapping player and loot", loaded.map.tiles[MARA_TAVERN_Y][MARA_TAVERN_X] == TILE_NPC_MARA &&
        loaded.player.x == MARA_TAVERN_X && loaded.player.y == MARA_TAVERN_Y + 1 && loaded.floor_items[0].active &&
        loaded.floor_items[0].y == MARA_TAVERN_Y + 1 && loaded.floor_items[0].underlying_tile == TILE_TAVERN_FLOOR &&
        loaded.map.tiles[MARA_TAVERN_Y + 1][MARA_TAVERN_X] == TILE_ITEM);
    ASSERT("legacy Tavern removes Liora and her stale dialogue", loaded.map.tiles[LIORA_INN_Y][LIORA_INN_X] == TILE_TAVERN_FLOOR && !loaded.dialogue_active);
    game = loaded;
    game.mara_quest_state = 2;
    game.mara_beacons_lit = 7;
    int score = game.score;
    game_talk_to_mara(&game);
    game_talk_to_mara(&game);
    ASSERT("Mara awards 80 gold and 600 score once at her new location", game.mara_quest_state == 3 && game.gold == 697 && game.score == score + 600);
    game.floor_items[0] = (FloorItem){.active = 1, .x = LIORA_INN_X, .y = LIORA_INN_Y,
        .underlying_tile = TILE_NPC_LIORA, .item = item_make_short_sword()};
    game.map.tiles[LIORA_INN_Y][LIORA_INN_X] = TILE_ITEM;
    ASSERT("legacy Tavern loot covering Liora retains a walkable floor", legacy_save() && load_game(&loaded, MARA_SLOT) &&
        loaded.floor_items[0].active && loaded.floor_items[0].underlying_tile == TILE_TAVERN_FLOOR && loaded.map.tiles[LIORA_INN_Y][LIORA_INN_X] == TILE_ITEM);
    game_talk_to_liora(&loaded);
    ASSERT("Liora cannot turn in her quest from the former Tavern", loaded.moonveil_quest_state == 2 && loaded.gold == 697);

    game_enter_town_hall(&game);
    game.map.tiles[HALL_MARA_Y][HALL_MARA_X] = TILE_NPC_MARA;
    game.dialogue_active = 1;
    snprintf(game.dialogue_speaker, MAX_SPEAKER_LEN, "Mara");
    ASSERT("legacy Hall removes Mara without losing completed beacon progress", legacy_save() && load_game(&loaded, MARA_SLOT) &&
        loaded.map.tiles[HALL_MARA_Y][HALL_MARA_X] == TILE_TAVERN_FLOOR && !loaded.dialogue_active &&
        loaded.mara_quest_state == 3 && loaded.mara_beacons_lit == 7 && loaded.gold == 697 &&
        loaded.map.tiles[HALL_STEWARD_Y][HALL_STEWARD_X] == TILE_NPC_STEWARD && loaded.map.tiles[HALL_OSWIN_Y][HALL_OSWIN_X] == TILE_NPC_OSWIN);
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = HALL_MARA_X, .y = HALL_MARA_Y,
        .underlying_tile = TILE_NPC_MARA, .item = item_make_short_sword()};
    game.map.tiles[HALL_MARA_Y][HALL_MARA_X] = TILE_ITEM;
    ASSERT("legacy Hall loot retains a walkable floor when Mara moves", legacy_save() && load_game(&loaded, MARA_SLOT) &&
        loaded.floor_items[0].active && loaded.floor_items[0].underlying_tile == TILE_TAVERN_FLOOR && loaded.map.tiles[HALL_MARA_Y][HALL_MARA_X] == TILE_ITEM);

    game_enter_inn(&game);
    game.player.x = LIORA_INN_X;
    game.player.y = LIORA_INN_Y;
    game.map.tiles[LIORA_INN_Y][LIORA_INN_X] = TILE_ITEM;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = LIORA_INN_X, .y = LIORA_INN_Y,
        .underlying_tile = TILE_TAVERN_FLOOR, .item = item_make_health_potion()};
    ASSERT("legacy Inn gains Zara while retaining ready-to-return Moonseed progress", legacy_save() && load_game(&loaded, MARA_SLOT) &&
        loaded.map.tiles[ZARA_INN_Y][ZARA_INN_X] == TILE_NPC_GUILD_SEEKER && loaded.moonveil_quest_state == 2 && loaded.moonveil_quest_progress == 7);
    ASSERT("Inn migration moves overlapping player and loot beside Zara", loaded.player.y == LIORA_INN_Y + 1 && loaded.floor_items[0].active &&
        loaded.floor_items[0].y == LIORA_INN_Y + 1 && loaded.floor_items[0].underlying_tile == TILE_TAVERN_FLOOR &&
        loaded.map.tiles[LIORA_INN_Y + 1][LIORA_INN_X] == TILE_ITEM);
    score = loaded.score;
    game_enter_town3(&loaded);
    loaded.player.x = LIORA_TOWN_X;
    loaded.player.y = LIORA_TOWN_Y + 1;
    game_talk_to_liora(&loaded);
    game_talk_to_liora(&loaded);
    ASSERT("Liora awards 90 gold and 700 score once in Rosemoor", loaded.moonveil_quest_state == 3 && loaded.gold == 787 && loaded.score == score + 700);
    ASSERT("both moved quests and Liora's town placement survive another save/load", save_game(&loaded, MARA_SLOT) && load_game(&loaded, MARA_SLOT) &&
        loaded.mara_quest_state == 3 && loaded.moonveil_quest_state == 3 && loaded.map.tiles[LIORA_TOWN_Y][LIORA_TOWN_X] == TILE_NPC_LIORA);
    remove(MARA_SAVE);
}
