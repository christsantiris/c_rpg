#include "test_utils.h"
#include "game/game.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
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

static void test_inn(void) {
    start();
    game_enter_tavern(&game);
    game.player.x = ELOWEN_INN_X;
    game.player.y = ELOWEN_INN_Y + 1;
    game_talk_to_elowen(&game);
    ASSERT("Elowen cannot assign her quest in the former Tavern location", !game.elowen_quest_state &&
        !game.dialogue_active && game.map.tiles[ELOWEN_INN_Y][ELOWEN_INN_X] == TILE_TAVERN_FLOOR);
    game_leave_tavern(&game);
    game_enter_town2(&game);
    game.player.x = TOWN_INN_DOOR_X;
    game.player.y = TOWN_INN_DOOR_Y + 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, TOWN_INN_DOOR_X, TOWN_INN_DOOR_Y});
    ASSERT("Stillbury's Inn houses Elowen without replacing Rook or Bram", game.location == LOCATION_INN &&
        game.map.tiles[ELOWEN_INN_Y][ELOWEN_INN_X] == TILE_NPC_ELOWEN &&
        game.map.tiles[18][10] == TILE_NPC_ROOK && game.map.tiles[7][28] == TILE_NPC_INNKEEPER &&
        !map_is_walkable(&game.map, ELOWEN_INN_X, ELOWEN_INN_Y) && map_is_walkable(&game.map, ELOWEN_INN_X, ELOWEN_INN_Y + 1));
    game_talk_to_elowen(&game);
    ASSERT("Elowen cannot assign her quest remotely inside the Inn", !game.elowen_quest_state);
    game.player.x = ELOWEN_INN_X;
    game.player.y = ELOWEN_INN_Y + 1;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("Elowen still requires conversation rather than Action", !game.elowen_quest_state);
    game_talk_to_elowen(&game);
    ASSERT("conversation in the Inn assigns the unchanged Oakhaven dungeon quest", game.elowen_quest_state == 1 &&
        strstr(game.dialogue_text, "Oakhaven's dungeon") && strstr(game.dialogue_text, "Stillbury's Inn") &&
        game.dialogue_x == ELOWEN_INN_X && game.dialogue_y == ELOWEN_INN_Y);
    QuestJournalEntry entry;
    ASSERT("the quest journal retains floors 2, 3, 4 and identifies the new return location", quest_journal_get_entry(&game, QUEST_TAB_ACTIVE, 0, &entry) &&
        entry.stages[0] == 2 && entry.stages[1] == 3 && entry.stages[2] == 4 &&
        entry.reward_gold == 40 && entry.reward_score == 300 && strstr(entry.summary_line_2, "Stillbury's Inn"));
    game.elowen_quest_state = 2;
    game.elowen_seals_restored = 7;
    int gold = game.gold;
    int score = game.score;
    ASSERT("ready-to-return progress survives saving in the new Inn", save_game(&game, ELOWEN_SAVE_SLOT) && load_game(&loaded, ELOWEN_SAVE_SLOT) &&
        loaded.elowen_quest_state == 2 && loaded.elowen_seals_restored == 7 && loaded.location == LOCATION_INN);
    game = loaded;
    game_talk_to_elowen(&game);
    game_talk_to_elowen(&game);
    ASSERT("Elowen awards the same reward once at the new location", game.elowen_quest_state == 3 && game.gold == gold + 40 && game.score == score + 300);
    game.player.x = game.map.stairs_down_x;
    game.player.y = game.map.stairs_down_y - 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, game.map.stairs_down_x, game.map.stairs_down_y});
    ASSERT("leaving the Inn returns to Stillbury and preserves completion", game.location == LOCATION_TOWN2 && game.elowen_quest_state == 3);
    int x;
    int y;
    map_generate_guild(&loaded.map, &x, &y);
    ASSERT("the shared Guild template does not gain Elowen", loaded.map.tiles[ELOWEN_INN_Y][ELOWEN_INN_X] == TILE_TAVERN_FLOOR);
}

void test_elowen_relocation(void) {
    printf("Elowen relocation to Stillbury's Inn\n");
    test_inn();
    remove(ELOWEN_SAVE_PATH);
}
