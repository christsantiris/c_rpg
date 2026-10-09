#include "test_utils.h"
#include "game/game.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define ILYA_SLOT 99149
#define ILYA_SAVE "saves/savegame_99149.json"
static GameState game;
static GameState loaded;

static int legacy_save(void) {
    if (!save_game(&game, ILYA_SLOT)) {
        return 0;
    }
    FILE *file = fopen(ILYA_SAVE, "rb");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 101);
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!json) {
        return 0;
    }
    file = fopen(ILYA_SAVE, "wb");
    int written = file && fputs(json, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(json);
    return written;
}

void test_ilya_relocation(void) {
    printf("Ilya in Oakhaven's Tavern tests:\n");
    ASSERT("Ilya test slot is unused", !save_exists(ILYA_SLOT));
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    game_enter_tavern(&game);
    ASSERT("the Tavern contains Ilya beside Elowen and Mara", game.map.tiles[ILYA_TAVERN_Y][ILYA_TAVERN_X] == TILE_NPC_DRAGON_SEEKER &&
        game.map.tiles[BRENNA_Y][BRENNA_X] == TILE_TAVERN_FLOOR && game.map.tiles[MARA_TAVERN_Y][MARA_TAVERN_X] == TILE_NPC_MARA &&
        !map_is_walkable(&game.map, ILYA_TAVERN_X, ILYA_TAVERN_Y) && map_is_walkable(&game.map, ILYA_TAVERN_X, ILYA_TAVERN_Y + 1));
    game_talk_to_dragon_seeker(&game);
    ASSERT("Ilya cannot assign her quest remotely", !game.dragon_treasure_quest_state && !game.dialogue_active);
    game.player.x = ILYA_TAVERN_X;
    game.player.y = ILYA_TAVERN_Y + 1;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("Ilya requires talking rather than Action", !game.dragon_treasure_quest_state);
    game_talk_to_dragon_seeker(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    ASSERT("Ilya gives directions to Ridgeshire and the Tavern return", game.dragon_treasure_quest_state == 1 &&
        strstr(game.dialogue_text, "mountains to Ridgeshire") && strstr(game.dialogue_text, "Oakhaven's Tavern"));
    ASSERT("current saves retain Ilya and the active quest", save_game(&game, ILYA_SLOT) && load_game(&loaded, ILYA_SLOT) &&
        loaded.dragon_treasure_quest_state == 1 && loaded.map.tiles[ILYA_TAVERN_Y][ILYA_TAVERN_X] == TILE_NPC_DRAGON_SEEKER);

    game.dragon_treasure_quest_state = 2;
    game.gold = 617;
    game.defeated_bosses |= 1 << LOCATION_DRAGONSPINE;
    game.player.y = ILYA_TAVERN_Y;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = ILYA_TAVERN_X, .y = ILYA_TAVERN_Y,
        .underlying_tile = TILE_TAVERN_FLOOR, .item = item_make_health_potion()};
    game.map.tiles[ILYA_TAVERN_Y][ILYA_TAVERN_X] = TILE_ITEM;
    ASSERT("legacy Tavern saves gain Ilya while preserving recovered goblet and boss progress", legacy_save() && load_game(&loaded, ILYA_SLOT) &&
        loaded.dragon_treasure_quest_state == 2 && loaded.gold == 617 && loaded.defeated_bosses == game.defeated_bosses &&
        loaded.map.tiles[ILYA_TAVERN_Y][ILYA_TAVERN_X] == TILE_NPC_DRAGON_SEEKER);
    ASSERT("migration moves an overlapping player and loot safely beside Ilya", loaded.player.x == ILYA_TAVERN_X &&
        loaded.player.y == ILYA_TAVERN_Y + 1 && loaded.floor_items[0].active && loaded.floor_items[0].y == ILYA_TAVERN_Y + 1 &&
        loaded.floor_items[0].underlying_tile == TILE_TAVERN_FLOOR && loaded.map.tiles[ILYA_TAVERN_Y + 1][ILYA_TAVERN_X] == TILE_ITEM);
    game = loaded;
    int score = game.score;
    int inventory = game.inventory_count;
    game_talk_to_dragon_seeker(&game);
    game_talk_to_dragon_seeker(&game);
    ASSERT("the migrated goblet awards the original potion and score exactly once", game.dragon_treasure_quest_state == 3 &&
        game.inventory_count == inventory + 1 && game.inventory[inventory].type == ITEM_POTION_STRENGTH && game.score == score + 600);

    game.location = LOCATION_TOWN4;
    map_generate_town4(&game.map, &game.player.x, &game.player.y);
    game.map.tiles[11][39] = TILE_NPC_DRAGON_SEEKER;
    game.dialogue_active = 1;
    ASSERT("old Ridgeshire saves remove Ilya and stale dialogue without resetting the completed quest", legacy_save() && load_game(&loaded, ILYA_SLOT) &&
        loaded.map.tiles[11][39] == TILE_TOWN_FLOOR && !loaded.dialogue_active && loaded.dragon_treasure_quest_state == 3 &&
        loaded.gold == game.gold && loaded.score == game.score);
    loaded.dragon_treasure_quest_state = 0;
    game_talk_to_dragon_seeker(&loaded);
    ASSERT("Ilya no longer assigns her quest in Ridgeshire", !loaded.dragon_treasure_quest_state);
    game.map.tiles[11][39] = TILE_ITEM;
    game.floor_items[0].x = 39;
    game.floor_items[0].y = 11;
    game.floor_items[0].underlying_tile = TILE_NPC_DRAGON_SEEKER;
    ASSERT("migration repairs loot covering Ilya's old position", legacy_save() && load_game(&loaded, ILYA_SLOT) &&
        loaded.floor_items[0].active && loaded.floor_items[0].underlying_tile == TILE_TOWN_FLOOR && loaded.map.tiles[11][39] == TILE_ITEM);
    int x;
    int y;
    map_generate_inn(&loaded.map, &x, &y);
    ASSERT("the shared Inn layout does not gain Ilya", loaded.map.tiles[ILYA_TAVERN_Y][ILYA_TAVERN_X] != TILE_NPC_DRAGON_SEEKER);
    map_generate_guild(&loaded.map, &x, &y);
    ASSERT("the shared Guild layout does not gain Ilya", loaded.map.tiles[ILYA_TAVERN_Y][ILYA_TAVERN_X] != TILE_NPC_DRAGON_SEEKER);
    remove(ILYA_SAVE);
}
