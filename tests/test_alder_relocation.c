#include "test_utils.h"
#include "game/game.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define ALDER_SLOT 99147
#define ALDER_SAVE "saves/savegame_99147.json"
static GameState game;
static GameState loaded;

static int legacy_save(void) {
    if (!save_game(&game, ALDER_SLOT)) {
        return 0;
    }
    FILE *file = fopen(ALDER_SAVE, "rb");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 97);
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!json) {
        return 0;
    }
    file = fopen(ALDER_SAVE, "wb");
    if (!file) {
        free(json);
        return 0;
    }
    int written = fputs(json, file) >= 0;
    fclose(file);
    free(json);
    return written;
}

void test_alder_relocation(void) {
    printf("Alder in Stillbury's Inn tests:\n");
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    game_enter_tavern(&game);
    game.player.x = 28;
    game.player.y = 8;
    game_talk_to_alder(&game);
    ASSERT("Alder no longer offers his quest in the Tavern", !game.alder_quest_state && !game.dialogue_active && game.map.tiles[7][28] == TILE_TAVERN_FLOOR);
    game_leave_tavern(&game);
    game_enter_town2(&game);
    game.player.x = TOWN_INN_DOOR_X;
    game.player.y = TOWN_INN_DOOR_Y + 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, TOWN_INN_DOOR_X, TOWN_INN_DOOR_Y});
    ASSERT("Stillbury's Inn houses Alder, Elowen, Bram, and Rook", game.location == LOCATION_INN &&
        game.map.tiles[ALDER_INN_Y][ALDER_INN_X] == TILE_NPC_ALDER && game.map.tiles[ELOWEN_INN_Y][ELOWEN_INN_X] == TILE_NPC_ELOWEN &&
        game.map.tiles[7][28] == TILE_NPC_INNKEEPER && game.map.tiles[18][10] == TILE_NPC_ROOK &&
        !map_is_walkable(&game.map, ALDER_INN_X, ALDER_INN_Y) && map_is_walkable(&game.map, ALDER_INN_X, ALDER_INN_Y + 1));
    game_talk_to_alder(&game);
    ASSERT("Alder cannot assign his quest remotely in the Inn", !game.alder_quest_state);
    game.player.x = ALDER_INN_X;
    game.player.y = ALDER_INN_Y + 1;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("Alder requires talking rather than Action", !game.alder_quest_state);
    game_talk_to_alder(&game);
    ASSERT("Alder assigns the forest quest and anchors dialogue beside his Inn position", game.alder_quest_state == 1 &&
        game.dialogue_x == ALDER_INN_X && game.dialogue_y == ALDER_INN_Y && strstr(game.dialogue_text, "Stillbury's Inn"));
    game.alder_wardens_rescued = 5;
    QuestJournalEntry entry;
    ASSERT("journal directs the player back to Stillbury's Inn", quest_journal_get_entry(&game, QUEST_TAB_ACTIVE, 0, &entry) &&
        strstr(entry.summary_line_2, "Stillbury's Inn") && entry.objective_complete[0] && entry.objective_complete[2]);
    ASSERT("current saves retain Alder and rescue progress", save_game(&game, ALDER_SLOT) && load_game(&loaded, ALDER_SLOT) &&
        loaded.alder_quest_state == 1 && loaded.alder_wardens_rescued == 5 && loaded.map.tiles[ALDER_INN_Y][ALDER_INN_X] == TILE_NPC_ALDER);
    game.gold = 617;
    game.player.y = ALDER_INN_Y;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = ALDER_INN_X, .y = ALDER_INN_Y,
        .underlying_tile = TILE_TAVERN_FLOOR, .item = item_make_short_sword()};
    game.map.tiles[ALDER_INN_Y][ALDER_INN_X] = TILE_ITEM;
    ASSERT("legacy Inn adds Alder without losing rescue progress or money", legacy_save() && load_game(&loaded, ALDER_SLOT) &&
        loaded.alder_quest_state == 1 && loaded.alder_wardens_rescued == 5 && loaded.gold == 617 &&
        loaded.map.tiles[ALDER_INN_Y][ALDER_INN_X] == TILE_NPC_ALDER);
    ASSERT("migration moves overlapping player and loot safely beside Alder", loaded.player.y == ALDER_INN_Y + 1 &&
        loaded.floor_items[0].active && loaded.floor_items[0].y == ALDER_INN_Y + 1 &&
        loaded.floor_items[0].underlying_tile == TILE_TAVERN_FLOOR && loaded.map.tiles[ALDER_INN_Y + 1][ALDER_INN_X] == TILE_ITEM);
    game = loaded;
    game.alder_quest_state = 2;
    game.alder_wardens_rescued = 7;
    int score = game.score;
    game_talk_to_alder(&game);
    game_talk_to_alder(&game);
    ASSERT("Alder awards the original reward once in Stillbury's Inn", game.alder_quest_state == 3 && game.gold == 687 && game.score == score + 500);
    game_enter_tavern(&game);
    game.map.tiles[7][28] = TILE_NPC_ALDER;
    ASSERT("legacy Tavern removes Alder while preserving completed rescues", legacy_save() && load_game(&loaded, ALDER_SLOT) &&
        loaded.map.tiles[7][28] == TILE_TAVERN_FLOOR && loaded.alder_quest_state == 3 && loaded.alder_wardens_rescued == 7);
    int x;
    int y;
    map_generate_guild(&loaded.map, &x, &y);
    ASSERT("the shared Guild template does not gain Alder", loaded.map.tiles[ALDER_INN_Y][ALDER_INN_X] != TILE_NPC_ALDER);
    remove(ALDER_SAVE);
}
