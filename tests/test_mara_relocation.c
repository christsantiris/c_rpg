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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 95);
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
    printf("Mara Town Hall relocation tests:\n");
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    game_enter_tavern(&game);
    game.player.x = HALL_MARA_X;
    game.player.y = HALL_MARA_Y + 1;
    game_talk_to_mara(&game);
    ASSERT("Mara cannot assign her quest from the former Tavern", !game.mara_quest_state && !game.dialogue_active);
    game_enter_town4(&game);
    game.player.x = TOWN4_HALL_DOOR_X;
    game.player.y = TOWN4_HALL_DOOR_Y + 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, TOWN4_HALL_DOOR_X, TOWN4_HALL_DOOR_Y});
    ASSERT("Town Hall doorway leads to Mara alongside Steward Hadrin", game.location == LOCATION_TOWN_HALL &&
        game.map.tiles[HALL_MARA_Y][HALL_MARA_X] == TILE_NPC_MARA &&
        game.map.tiles[HALL_STEWARD_Y][HALL_STEWARD_X] == TILE_NPC_STEWARD &&
        !map_is_walkable(&game.map, HALL_MARA_X, HALL_MARA_Y) && map_is_walkable(&game.map, HALL_MARA_X, HALL_MARA_Y + 1));
    game_talk_to_mara(&game);
    ASSERT("Mara cannot assign her quest remotely inside the Hall", !game.mara_quest_state);
    game.player.x = HALL_MARA_X;
    game.player.y = HALL_MARA_Y + 1;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("Action does not replace talking to Mara", !game.mara_quest_state);
    game_talk_to_mara(&game);
    ASSERT("Mara assigns the same coast quest with updated dialogue coordinates and travel directions", game.mara_quest_state == 1 &&
        game.dialogue_x == HALL_MARA_X && game.dialogue_y == HALL_MARA_Y && strstr(game.dialogue_text, "south of Oakhaven") &&
        strstr(game.dialogue_text, "Ridgeshire's Town Hall") && !game.emberforge_quest_state);
    QuestJournalEntry entry;
    ASSERT("Mara's journal keeps all three coast stages and identifies the new return location", quest_journal_get_entry(&game, QUEST_TAB_ACTIVE, 0, &entry) &&
        entry.stages[0] == 2 && entry.stages[1] == 3 && entry.stages[2] == 4 && strstr(entry.summary_line_2, "Ridgeshire"));
    ASSERT("current saves retain Mara's Hall placement and accepted quest", save_game(&game, MARA_SLOT) && load_game(&loaded, MARA_SLOT) &&
        loaded.location == LOCATION_TOWN_HALL && loaded.mara_quest_state == 1 && loaded.map.tiles[HALL_MARA_Y][HALL_MARA_X] == TILE_NPC_MARA);

    game_leave_town_hall(&game);
    game_enter_coast(&game);
    game_descend(&game);
    game.enemies[0].hp = 9;
    game_return_to_town(&game);
    game_enter_town_hall(&game);
    game.mara_beacons_lit = 5;
    game.gold = 617;
    game.player.x = HALL_MARA_X;
    game.player.y = HALL_MARA_Y;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = HALL_MARA_X, .y = HALL_MARA_Y,
        .underlying_tile = TILE_TAVERN_FLOOR, .item = item_make_short_sword()};
    game.map.tiles[HALL_MARA_Y][HALL_MARA_X] = TILE_ITEM;
    ASSERT("legacy Hall fixture saves and migrates", legacy_save() && load_game(&loaded, MARA_SLOT));
    ASSERT("migration preserves quest progress, gold, and explored coast enemies", loaded.mara_quest_state == 1 &&
        loaded.mara_beacons_lit == 5 && loaded.gold == 617 && loaded.coast_cache[1].valid && loaded.coast_cache[1].enemies[0].hp == 9);
    ASSERT("legacy Hall gains Mara and safely moves overlapping player and loot", loaded.map.tiles[HALL_MARA_Y][HALL_MARA_X] == TILE_NPC_MARA &&
        loaded.player.x == HALL_MARA_X && loaded.player.y == HALL_MARA_Y + 1 && loaded.floor_items[0].active &&
        loaded.floor_items[0].y == HALL_MARA_Y + 1 && loaded.floor_items[0].underlying_tile == TILE_TAVERN_FLOOR &&
        loaded.map.tiles[HALL_MARA_Y + 1][HALL_MARA_X] == TILE_ITEM);

    game = loaded;
    game_enter_tavern(&game);
    game.map.tiles[18][31] = TILE_NPC_MARA;
    ASSERT("legacy Tavern fixture saves and migrates", legacy_save() && load_game(&loaded, MARA_SLOT));
    ASSERT("legacy Tavern removes Mara without losing active beacon progress", loaded.map.tiles[18][31] == TILE_TAVERN_FLOOR &&
        loaded.mara_quest_state == 1 && loaded.mara_beacons_lit == 5 && loaded.gold == 617);
    game.map.tiles[18][31] = TILE_ITEM;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 31, .y = 18,
        .underlying_tile = TILE_NPC_MARA, .item = item_make_short_sword()};
    ASSERT("legacy loot over Mara's former tile retains a walkable floor after migration", legacy_save() && load_game(&loaded, MARA_SLOT) &&
        loaded.floor_items[0].active && loaded.floor_items[0].underlying_tile == TILE_TAVERN_FLOOR && loaded.map.tiles[18][31] == TILE_ITEM);
    remove(MARA_SAVE);
}
