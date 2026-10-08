#include "test_utils.h"
#include "game/game.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define EMBERFORGE_TEST_SLOT 99138
static GameState game;
static GameState loaded;
static Map layout;

static void start(void) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    srand(8138);
    game_enter_town4(&game);
}

static void accept_quest(void) {
    game_enter_town_hall(&game);
    game.player.x = HALL_STEWARD_X;
    game.player.y = HALL_STEWARD_Y + 1;
    game_talk_to_steward(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    game_leave_town_hall(&game);
}

static void stand_at_object(void) {
    map_room_center(&game.map.rooms[game.map.room_count - 1], &game.player.x, &game.player.y);
}

static int guards(const GameState *g, int active_only) {
    int count = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        if ((!active_only || g->enemies[i].active) && strcmp(g->enemies[i].name, "Emberforge Guardian") == 0) {
            count++;
        }
    }
    return count;
}

static void defeat_local_enemies(void) {
    for (int i = 0; i < game.enemy_count; i++) {
        Enemy *enemy = &game.enemies[i];
        if (strcmp(enemy->name, "Emberforge Guardian") == 0 ||
            abs(enemy->x - game.player.x) + abs(enemy->y - game.player.y) <= 4) {
            enemy->active = 0;
            enemy->hp = 0;
        }
    }
}

static void test_hall(void) {
    start();
    ASSERT("Town Hall branch joins the crossroads and has a walkable door", game.map.tiles[12][20] == TILE_TOWN_PATH &&
        game.map.tiles[11][TOWN4_HALL_DOOR_X] == TILE_TOWN_PATH &&
        game.map.tiles[TOWN4_HALL_DOOR_Y][TOWN4_HALL_DOOR_X] == TILE_TOWN_HALL_DOOR);
    game.player.x = TOWN4_HALL_DOOR_X;
    game.player.y = TOWN4_HALL_DOOR_Y + 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, TOWN4_HALL_DOOR_X, TOWN4_HALL_DOOR_Y});
    ASSERT("Town Hall doorway enters an interior with a reachable steward", game.location == LOCATION_TOWN_HALL &&
        !game.enemy_count && game.map.tiles[HALL_STEWARD_Y][HALL_STEWARD_X] == TILE_NPC_STEWARD &&
        !map_is_walkable(&game.map, HALL_STEWARD_X, HALL_STEWARD_Y) &&
        map_is_walkable(&game.map, HALL_STEWARD_X, HALL_STEWARD_Y + 1));
    game_talk_to_steward(&game);
    ASSERT("steward cannot assign or reward remotely", !game.emberforge_quest_state);
    game.player.x = HALL_STEWARD_X;
    game.player.y = HALL_STEWARD_Y + 1;
    game_talk_to_steward(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    ASSERT("talking assigns Reclaim the Emberforge", game.emberforge_quest_state == 1 && game.dialogue_active &&
        strcmp(game.dialogue_speaker, "Steward Hadrin") == 0);
    ASSERT("saving inside Town Hall preserves the interior and accepted quest", save_game(&game, EMBERFORGE_TEST_SLOT) &&
        load_game(&loaded, EMBERFORGE_TEST_SLOT) && loaded.location == LOCATION_TOWN_HALL &&
        loaded.emberforge_quest_state == 1 && memcmp(&game.map, &loaded.map, sizeof(Map)) == 0);
    game.player.poison_turns = 3;
    game.player.frozen_turns = 3;
    game.player.known_spells[0] = spell_make_return_to_town();
    game.player.known_spell_count = 1;
    game.player.equipped_spell = 0;
    game.player.mp = 100;
    action_resolve_player(&game, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("Town Hall counts as town for recovery and Return to Town", game.location == LOCATION_TOWN_HALL &&
        !game.player.poison_turns && !game.player.frozen_turns && !game.portal_active);
    game.defeated_bosses |= 1 << LOCATION_MOUNTAINS;
    game.player.x = game.map.stairs_down_x;
    game.player.y = game.map.stairs_down_y - 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, game.map.stairs_down_x, game.map.stairs_down_y});
    ASSERT("Town Hall exit returns outside its door and retains mountain shortcut", game.location == LOCATION_TOWN4 &&
        game.player.x == TOWN4_HALL_DOOR_X && game.player.y == TOWN4_HALL_DOOR_Y + 1 &&
        game.map.tiles[TOWN_H - 1][RIDGESHIRE_MOUNTAIN_ROAD_X] == TILE_TOWN_EXIT);
}

static void test_quest(void) {
    start();
    game_enter_ashen(&game);
    game_descend(&game);
    stand_at_object();
    ASSERT("quest encounters are absent before assignment", !game_has_emberforge_interaction(&game) && !guards(&game, 0));
    game.enemies[0].hp = 7;
    layout = game.map;
    game_return_to_town(&game);
    accept_quest();
    ASSERT("accepting the quest keeps already explored Ashen caches", game.ashen_cache[1].valid &&
        game.ashen_cache[1].enemies[0].hp == 7 && memcmp(&layout, &game.ashen_cache[1].map, sizeof(Map)) == 0);
    game_enter_ashen(&game);
    game_descend(&game);
    stand_at_object();
    ASSERT("revisited stage two gains mechanism and three guards without resetting enemies", game_has_regional_interaction(&game) &&
        game.map.tiles[game.player.y][game.player.x] == TILE_EMBERFORGE_MECHANISM && guards(&game, 1) == 3 &&
        game.enemies[0].hp == 7 && game.emberforge_encounters == 1);
    game_refresh_quest_encounters(&game);
    ASSERT("refresh does not duplicate defenders", guards(&game, 0) == 3);
    game.enemies[game.enemy_count - 1].hp = 5;
    ASSERT("reload retains living defenders and their damage without duplicating them", save_game(&game, EMBERFORGE_TEST_SLOT) &&
        load_game(&loaded, EMBERFORGE_TEST_SLOT) && guards(&loaded, 0) == 3 && guards(&loaded, 1) == 3 &&
        loaded.enemies[loaded.enemy_count - 1].hp == 5 && !loaded.emberforge_progress);
    game.inventory_count = MAX_INVENTORY;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("living defenders prevent mechanism retrieval", !game.emberforge_progress);
    stand_at_object();
    game.enemies[game.enemy_count - 1].x = game.map.stairs_up_x;
    game.enemies[game.enemy_count - 1].y = game.map.stairs_up_y;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("luring a defender away cannot bypass the encounter", !game.emberforge_progress);
    defeat_local_enemies();
    game.enemies[0].active = 1;
    game.enemies[0].x = game.map.stairs_up_x;
    game.enemies[0].y = game.map.stairs_up_y;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("mechanism retrieval needs no inventory slot or full-stage clearance", game.emberforge_progress == 1 &&
        game.emberforge_quest_state == 1 && game.enemies[0].active && game.inventory_count == MAX_INVENTORY);
    QuestJournalEntry entry;
    ASSERT("journal lists two objectives on stages two and four", quest_journal_get_entry(&game, QUEST_TAB_ACTIVE, 0, &entry) &&
        strcmp(entry.title, "Reclaim the Emberforge") == 0 && entry.objective_count == 2 &&
        entry.stages[0] == 2 && entry.stages[1] == 4 && entry.objective_complete[0] && !entry.objective_complete[1]);
    ASSERT("saving midway keeps progress and killed guardians", save_game(&game, EMBERFORGE_TEST_SLOT) &&
        load_game(&loaded, EMBERFORGE_TEST_SLOT) && loaded.emberforge_progress == 1 &&
        loaded.emberforge_encounters == 1 && !guards(&loaded, 1));
    game = loaded;
    game_descend(&game);
    game_descend(&game);
    stand_at_object();
    int guardians = 0;
    for (int i = 0; i < game.enemy_count; i++) {
        guardians += game.enemies[i].active && game.enemies[i].type == ENEMY_OBSIDIAN_GUARDIAN &&
            strcmp(game.enemies[i].name, "Emberforge Guardian") == 0;
    }
    ASSERT("stage four furnace has two Obsidian Guardians and supporting imps", guardians == 2 && guards(&game, 1) == 4 &&
        game.map.tiles[game.player.y][game.player.x] == TILE_EMBERFORGE_COLD);
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("furnace cannot be repaired during its guarded encounter", game.emberforge_progress == 1);
    defeat_local_enemies();
    game.player.y++;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("repair works beside the furnace and lights it without killing the Cinder Lord", game.emberforge_progress == 3 &&
        game.emberforge_quest_state == 2 && game.map.tiles[game.player.y - 1][game.player.x] == TILE_EMBERFORGE_LIT &&
        !(game.defeated_bosses & (1 << LOCATION_ASHEN)));
    stand_at_object();
    game_open_town_portal(&game);
    game_enter_town_hall(&game);
    game.player.x = HALL_STEWARD_X;
    game.player.y = HALL_STEWARD_Y + 1;
    int gold = game.gold;
    int score = game.score;
    game_talk_to_steward(&game);
    ASSERT("steward pays the completed quest once", game.emberforge_quest_state == 3 &&
        game.gold == gold + 100 && game.score == score + 800);
    game_talk_to_steward(&game);
    ASSERT("repeat dialogue cannot duplicate the reward", game.gold == gold + 100 && game.score == score + 800);
    ASSERT("completed journal retains both objectives", quest_journal_get_entry(&game, QUEST_TAB_COMPLETED, 0, &entry) &&
        entry.objective_complete[0] && entry.objective_complete[1] && entry.state == 3);
    ASSERT("completed quest and portal save inside the hall", save_game(&game, EMBERFORGE_TEST_SLOT) &&
        load_game(&loaded, EMBERFORGE_TEST_SLOT) && loaded.emberforge_quest_state == 3 && loaded.portal_active);
    game = loaded;
    game_leave_town_hall(&game);
    ASSERT("leaving the hall restores the Ashen return portal", game.map.tiles[2][RIDGESHIRE_ASHEN_GATE_X + 1] == TILE_PORTAL);
    game_use_town_portal(&game);
    stand_at_object();
    ASSERT("return portal preserves the repaired forge without respawning defenders", game.location == LOCATION_ASHEN &&
        game.level == 4 && game.map.tiles[game.player.y][game.player.x] == TILE_EMBERFORGE_LIT && !guards(&game, 1));
}

static void test_early_furnace(void) {
    start();
    accept_quest();
    game_enter_ashen(&game);
    while (game.level < EMBERFORGE_FURNACE_LEVEL) {
        game_descend(&game);
    }
    stand_at_object();
    defeat_local_enemies();
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("visiting furnace first requires retrieving the mechanism", !game.emberforge_progress &&
        game.map.tiles[game.player.y][game.player.x] == TILE_EMBERFORGE_COLD);
}

static void test_covered_object(void) {
    start();
    accept_quest();
    game_enter_ashen(&game);
    game_descend(&game);
    stand_at_object();
    defeat_local_enemies();
    game.floor_item_count = 2;
    for (int i = 0; i < 2; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = game.player.x, .y = game.player.y,
            .underlying_tile = TILE_EMBERFORGE_MECHANISM, .item = item_make_health_potion()};
    }
    game.map.tiles[game.player.y][game.player.x] = TILE_ITEM;
    ASSERT("loot covering the mechanism does not hide its interaction", game_has_regional_interaction(&game));
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("retrieval preserves overlapping loot and updates every underlay", game.emberforge_progress == 1 &&
        game.map.tiles[game.player.y][game.player.x] == TILE_ITEM && game.floor_items[0].active && game.floor_items[1].active &&
        game.floor_items[0].underlying_tile == TILE_ASHEN_RUIN && game.floor_items[1].underlying_tile == TILE_ASHEN_RUIN);
    ASSERT("covered objective state survives saving and loading", save_game(&game, EMBERFORGE_TEST_SLOT) &&
        load_game(&loaded, EMBERFORGE_TEST_SLOT) && loaded.emberforge_progress == 1 &&
        loaded.floor_items[0].active && loaded.floor_items[1].active &&
        loaded.floor_items[0].underlying_tile == TILE_ASHEN_RUIN && loaded.floor_items[1].underlying_tile == TILE_ASHEN_RUIN);
}

static void test_legacy_save(void) {
    start();
    game.gold = 731;
    game.defeated_bosses |= 1 << LOCATION_ASHEN;
    game.max_ashen_level_reached = 5;
    game.player.x = TOWN4_HALL_X + 1;
    game.player.y = TOWN4_HALL_Y + 1;
    for (int y = TOWN4_HALL_Y; y <= TOWN4_HALL_DOOR_Y; y++) {
        for (int x = TOWN4_HALL_X; x < TOWN4_HALL_X + TOWN4_HALL_W; x++) {
            game.map.tiles[y][x] = TILE_TOWN_FLOOR;
        }
    }
    game.floor_items[0] = (FloorItem){.active = 1, .x = game.player.x, .y = game.player.y,
        .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_short_sword()};
    game.floor_item_count = 1;
    game.map.tiles[game.player.y][game.player.x] = TILE_ITEM;
    ASSERT("legacy Ridgeshire fixture saves", save_game(&game, EMBERFORGE_TEST_SLOT));
    FILE *file = fopen("saves/savegame_99138.json", "rb");
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *buffer = malloc((size_t)size + 1);
    fread(buffer, 1, (size_t)size, file);
    buffer[size] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 87);
    cJSON_DeleteItemFromObject(root, "emberforge_quest_state");
    cJSON_DeleteItemFromObject(root, "emberforge_progress");
    cJSON_DeleteItemFromObject(root, "emberforge_encounters");
    char *json = cJSON_Print(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99138.json", "wb");
    fputs(json, file);
    fclose(file);
    free(json);
    ASSERT("version 87 migrates new quest fields without losing gold or regional progress", load_game(&loaded, EMBERFORGE_TEST_SLOT) &&
        loaded.gold == 731 && !loaded.emberforge_quest_state && !loaded.emberforge_progress && !loaded.emberforge_encounters &&
        loaded.max_ashen_level_reached == 5 && (loaded.defeated_bosses & (1 << LOCATION_ASHEN)));
    ASSERT("new hall migration moves player and existing loot safely outside", loaded.player.x == TOWN4_HALL_DOOR_X &&
        loaded.player.y == TOWN4_HALL_DOOR_Y + 1 && loaded.floor_items[0].active &&
        loaded.floor_items[0].x == TOWN4_HALL_DOOR_X && loaded.floor_items[0].y == TOWN4_HALL_DOOR_Y + 1 &&
        loaded.floor_items[0].underlying_tile == TILE_TOWN_PATH &&
        loaded.map.tiles[TOWN4_HALL_DOOR_Y + 1][TOWN4_HALL_DOOR_X] == TILE_ITEM &&
        loaded.map.tiles[TOWN4_HALL_DOOR_Y][TOWN4_HALL_DOOR_X] == TILE_TOWN_HALL_DOOR);
    ASSERT("migrated save writes and loads again", save_game(&loaded, EMBERFORGE_TEST_SLOT) && load_game(&game, EMBERFORGE_TEST_SLOT));
    accept_quest();
    ASSERT("quest remains available after a previous Cinder Lord victory", game.emberforge_quest_state == 1 &&
        game.max_ashen_level_reached == 5 && (game.defeated_bosses & (1 << LOCATION_ASHEN)));
}

void test_emberforge(void) {
    printf("Town Hall and Emberforge quest\n");
    test_hall();
    test_quest();
    test_early_furnace();
    test_covered_object();
    test_legacy_save();
    remove("saves/savegame_99138.json");
}
