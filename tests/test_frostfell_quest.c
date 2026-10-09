#include "test_utils.h"
#include "game/game.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define FROST_QUEST_SLOT 99139
#define FROST_QUEST_SAVE "saves/savegame_99139.json"
static GameState game;
static GameState loaded;

static void start(void) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    srand(9139);
}

static void accept(void) {
    game_enter_inn(&game);
    game.player.x = BRENNA_INN_X;
    game.player.y = BRENNA_INN_Y + 1;
    game_talk_to_brenna(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    game_leave_inn(&game);
}

static int objective(TileType tile, int *x, int *y) {
    for (int ty = 0; ty < MAP_H; ty++) {
        for (int tx = 0; tx < MAP_W; tx++) {
            if (game.map.tiles[ty][tx] == tile) {
                *x = tx;
                *y = ty;
                return 1;
            }
        }
    }
    return 0;
}

static int guards(const GameState *g, int alive) {
    int count = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        if ((!alive || g->enemies[i].active) && strcmp(g->enemies[i].name, "Expedition Guardian") == 0) {
            count++;
        }
    }
    return count;
}

static void defeat_defenders(int x, int y) {
    for (int i = 0; i < game.enemy_count; i++) {
        Enemy *enemy = &game.enemies[i];
        if (strcmp(enemy->name, "Expedition Guardian") == 0 || abs(enemy->x - x) + abs(enemy->y - y) <= 4) {
            enemy->active = 0;
            enemy->hp = 0;
        }
    }
}

static void test_expedition(void) {
    start();
    game_enter_inn(&game);
    ASSERT("Brenna occupies a reachable, blocked NPC tile", game.map.tiles[BRENNA_INN_Y][BRENNA_INN_X] == TILE_NPC_BRENNA &&
        !map_is_walkable(&game.map, BRENNA_INN_X, BRENNA_INN_Y) && map_is_walkable(&game.map, BRENNA_INN_X, BRENNA_INN_Y + 1));
    game_talk_to_brenna(&game);
    ASSERT("Brenna cannot assign remotely", !game.frostfell_quest_state);
    game.player.x = BRENNA_INN_X;
    game.player.y = BRENNA_INN_Y + 1;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("Brenna assigns through conversation, not Action", !game.frostfell_quest_state);
    game_talk_to_brenna(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    ASSERT("Brenna explains the route from Stillbury through Rosemoor into the far north", game.frostfell_quest_state == 1 &&
        strstr(game.dialogue_text, "far north") &&
        strstr(game.dialogue_text, "swamp north of Stillbury to Rosemoor") && strstr(game.dialogue_text, "north gate into Frostfell"));
    ASSERT("acceptance persists inside the Inn", save_game(&game, FROST_QUEST_SLOT) && load_game(&loaded, FROST_QUEST_SLOT) &&
        loaded.frostfell_quest_state == 1 && loaded.map.tiles[BRENNA_INN_Y][BRENNA_INN_X] == TILE_NPC_BRENNA);
    game_leave_inn(&game);
    game_enter_frostfell(&game);
    game_descend(&game);
    int x = -1;
    int y = -1;
    ASSERT("stage two contains the expedition journal and two Ice Wolf guards", objective(TILE_FROST_JOURNAL, &x, &y) &&
        guards(&game, 1) == 2 && game.enemies[game.enemy_count - 1].type == ENEMY_ICE_WOLF &&
        game.enemies[game.enemy_count - 2].type == ENEMY_ICE_WOLF && game.frostfell_quest_encounters == 1);
    if (x < 0) {
        return;
    }
    game.player.x = x;
    game.player.y = y;
    game.enemies[game.enemy_count - 1].hp = 7;
    game_refresh_quest_encounters(&game);
    ASSERT("refresh and save/load preserve guardian damage without duplication", guards(&game, 0) == 2 &&
        save_game(&game, FROST_QUEST_SLOT) && load_game(&loaded, FROST_QUEST_SLOT) && guards(&loaded, 0) == 2 &&
        loaded.enemies[loaded.enemy_count - 1].hp == 7);
    game = loaded;
    game_ascend(&game);
    game_descend(&game);
    ASSERT("backtracking retains quest guards and their damage", guards(&game, 0) == 2 &&
        game.enemies[game.enemy_count - 1].hp == 7);
    game.player.x = x;
    game.player.y = y;
    game.inventory_count = MAX_INVENTORY;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("living guards prevent journal retrieval", !game.frostfell_quest_progress);
    game.enemies[game.enemy_count - 1].x = game.map.stairs_up_x;
    game.enemies[game.enemy_count - 1].y = game.map.stairs_up_y;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("luring a journal guard away does not bypass the encounter", !game.frostfell_quest_progress);
    defeat_defenders(x, y);
    game.floor_item_count = 2;
    for (int i = 0; i < 2; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = x, .y = y,
            .underlying_tile = TILE_FROST_JOURNAL, .item = item_make_health_potion()};
    }
    game.map.tiles[y][x] = TILE_ITEM;
    ASSERT("loot cannot hide journal interaction", game_has_regional_interaction(&game));
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("journal retrieval uses no inventory slot and preserves all overlapping loot", game.frostfell_quest_progress == 1 &&
        game.inventory_count == MAX_INVENTORY && game.map.tiles[y][x] == TILE_ITEM &&
        game.floor_items[0].underlying_tile == TILE_FROST_FLOOR && game.floor_items[1].underlying_tile == TILE_FROST_FLOOR);
    ASSERT("mid-quest reload retains progress and defeated guardians", save_game(&game, FROST_QUEST_SLOT) &&
        load_game(&loaded, FROST_QUEST_SLOT) && loaded.frostfell_quest_progress == 1 && !guards(&loaded, 1));
    game = loaded;
    QuestJournalEntry entry;
    ASSERT("quest journal tracks the two stage-specific objectives", quest_journal_get_entry(&game, QUEST_TAB_ACTIVE, 0, &entry) &&
        strcmp(entry.title, "The Silent Expedition") == 0 && entry.objective_count == 2 && entry.stages[0] == 2 &&
        entry.stages[1] == 4 && entry.objective_complete[0] && !entry.objective_complete[1]);
    game_descend(&game);
    game_descend(&game);
    ASSERT("stage four has Surveyor Fen guarded by an Ice Giant and Frost Archer", objective(TILE_NPC_FROST_SURVIVOR, &x, &y) &&
        guards(&game, 1) == 2 && game.enemies[game.enemy_count - 2].type == ENEMY_ICE_GIANT &&
        game.enemies[game.enemy_count - 1].type == ENEMY_FROST_ARCHER && !map_is_walkable(&game.map, x, y));
    game_talk_to_frost_survivor(&game, x, y);
    ASSERT("Fen cannot be rescued remotely", game.frostfell_quest_state == 1 && !game.dialogue_active);
    game.player.x = x;
    game.player.y = y + 1;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("Action does not rescue the NPC", game.frostfell_quest_state == 1);
    game_talk_to_frost_survivor(&game, x, y);
    ASSERT("the surveyor cannot escape living captors", game.frostfell_quest_state == 1 && game.dialogue_active);
    defeat_defenders(x, y);
    game_talk_to_frost_survivor(&game, x, y);
    ASSERT("talking rescues Fen without escorting or killing the Kraken", game.frostfell_quest_state == 2 &&
        game.frostfell_quest_progress == 3 && game.map.tiles[y][x] == TILE_FROST_FLOOR &&
        !(game.defeated_bosses & (1 << LOCATION_FROSTFELL)));
    game_refresh_quest_encounters(&game);
    ASSERT("rescued surveyor and defeated guards do not respawn", !objective(TILE_NPC_FROST_SURVIVOR, &x, &y) && !guards(&game, 1));
    game_return_to_town(&game);
    int gold = game.gold;
    int score = game.score;
    accept();
    ASSERT("Brenna awards 100 gold and 800 score", game.frostfell_quest_state == 3 &&
        game.gold == gold + 100 && game.score == score + 800);
    accept();
    ASSERT("reward cannot be collected twice", game.gold == gold + 100 && game.score == score + 800);
    ASSERT("completed journal retains both objectives", quest_journal_get_entry(&game, QUEST_TAB_COMPLETED, 0, &entry) &&
        entry.objective_complete[0] && entry.objective_complete[1]);
    ASSERT("completed quest survives saving", save_game(&game, FROST_QUEST_SLOT) && load_game(&loaded, FROST_QUEST_SLOT) &&
        loaded.frostfell_quest_state == 3 && loaded.frostfell_quest_progress == 3 && loaded.frostfell_quest_encounters == 3);
}

static void test_revisits(void) {
    start();
    game_enter_frostfell(&game);
    game_descend(&game);
    int x = -1;
    int y = -1;
    ASSERT("quest encounters are absent before assignment", !objective(TILE_FROST_JOURNAL, &x, &y) && !guards(&game, 0));
    game.enemies[0].hp = 7;
    game.defeated_bosses |= 1 << LOCATION_FROSTFELL;
    game_open_town_portal(&game);
    accept();
    ASSERT("acceptance preserves existing Frostfell caches, damage, and boss victory", game.frostfell_cache[1].valid &&
        game.frostfell_cache[1].enemies[0].hp == 7 && game.portal_active && (game.defeated_bosses & (1 << LOCATION_FROSTFELL)));
    game_use_town_portal(&game);
    ASSERT("returning to an explored stage injects the encounter without resetting enemies", game.level == 2 &&
        game.enemies[0].hp == 7 && objective(TILE_FROST_JOURNAL, &x, &y) && guards(&game, 1) == 2);
    game_return_to_town(&game);
    game_enter_frostfell(&game);
    game_descend(&game);
    ASSERT("unfinished objectives remain guarded on fresh expeditions", guards(&game, 1) == 2 && objective(TILE_FROST_JOURNAL, &x, &y));
    game_descend(&game);
    game_descend(&game);
    ASSERT("survivor is available when stage four is visited first", objective(TILE_NPC_FROST_SURVIVOR, &x, &y));
    defeat_defenders(x, y);
    game.player.x = x;
    game.player.y = y + 1;
    game_talk_to_frost_survivor(&game, x, y);
    ASSERT("rescuing Fen first requires the expedition journal", game.frostfell_quest_state == 1 &&
        !game.frostfell_quest_progress && strstr(game.dialogue_text, "stage 2"));
    game.map.tiles[y][x] = TILE_FROST_FLOOR;
    game.player.x = x;
    game.player.y = y;
    game_refresh_quest_encounters(&game);
    ASSERT("injecting a missing survivor never places the NPC on the player", game.map.tiles[y][x] == TILE_FROST_FLOOR &&
        objective(TILE_NPC_FROST_SURVIVOR, &x, &y));
}

static void test_covered_portal(void) {
    start();
    accept();
    game_enter_frostfell(&game);
    game_descend(&game);
    int x = -1;
    int y = -1;
    if (!objective(TILE_FROST_JOURNAL, &x, &y)) {
        ASSERT("portal fixture finds the journal", 0);
        return;
    }
    defeat_defenders(x, y);
    game.player.x = x;
    game.player.y = y;
    game_open_town_portal(&game);
    ASSERT("portal saves on a quest objective without erasing it", save_game(&game, FROST_QUEST_SLOT) &&
        load_game(&loaded, FROST_QUEST_SLOT) && loaded.portal_origin_tile == TILE_FROST_JOURNAL &&
        loaded.frostfell_cache[1].map.tiles[y][x] == TILE_FROST_JOURNAL);
    game = loaded;
    game_use_town_portal(&game);
    ASSERT("return portal preserves defeated guards and journal interaction", game.location == LOCATION_FROSTFELL &&
        game.level == 2 && !guards(&game, 1) && game_has_regional_interaction(&game));
    game_interact_frostfell(&game);
    game_return_to_town(&game);
    game_enter_frostfell(&game);
    game_descend(&game);
    ASSERT("completed journal objective stays completed on a fresh expedition", game.frostfell_quest_progress == 1 &&
        !objective(TILE_FROST_JOURNAL, &x, &y) && !guards(&game, 0));
}

static void test_survivor_access(void) {
    static unsigned char visited[MAP_H][MAP_W];
    static int queue[MAP_H * MAP_W];
    int connected = 1;
    for (int seed = 0; seed < 32 && connected; seed++) {
        start();
        srand((unsigned int)seed);
        game.frostfell_quest_state = 1;
        game_enter_frostfell(&game);
        while (game.level < 4) {
            game_descend(&game);
        }
        int x = -1;
        int y = -1;
        if (!objective(TILE_NPC_FROST_SURVIVOR, &x, &y) || guards(&game, 1) != 2) {
            connected = 0;
            break;
        }
        memset(visited, 0, sizeof(visited));
        int head = 0;
        int tail = 0;
        queue[tail++] = game.map.stairs_up_y * MAP_W + game.map.stairs_up_x;
        visited[game.map.stairs_up_y][game.map.stairs_up_x] = 1;
        const int dx[4] = {0, 1, 0, -1};
        const int dy[4] = {-1, 0, 1, 0};
        while (head < tail) {
            int cell = queue[head++];
            int cx = cell % MAP_W;
            int cy = cell / MAP_W;
            for (int side = 0; side < 4; side++) {
                int nx = cx + dx[side];
                int ny = cy + dy[side];
                if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H || visited[ny][nx] ||
                    !map_is_walkable(&game.map, nx, ny) || game.map.tiles[ny][nx] == TILE_FROST_THIN_ICE) {
                    continue;
                }
                visited[ny][nx] = 1;
                queue[tail++] = ny * MAP_W + nx;
            }
        }
        int accessible = 0;
        for (int side = 0; side < 4; side++) {
            accessible |= visited[y + dy[side]][x + dx[side]];
        }
        connected = accessible && visited[game.map.stairs_down_y][game.map.stairs_down_x];
    }
    ASSERT("the survivor and exit remain reachable without thin-ice shortcuts across 32 layouts", connected);
}

static void test_legacy_save(void) {
    start();
    game_enter_tavern(&game);
    game.gold = 731;
    game.defeated_bosses |= 1 << LOCATION_FROSTFELL;
    game.max_frostfell_level_reached = 5;
    game.player.x = BRENNA_X;
    game.player.y = BRENNA_Y;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = BRENNA_X, .y = BRENNA_Y,
        .underlying_tile = TILE_TAVERN_FLOOR, .item = item_make_short_sword()};
    game.map.tiles[BRENNA_Y][BRENNA_X] = TILE_ITEM;
    ASSERT("legacy fixture saves", save_game(&game, FROST_QUEST_SLOT));
    FILE *file = fopen(FROST_QUEST_SAVE, "rb");
    if (!file) {
        return;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *buffer = malloc((size_t)size + 1);
    fread(buffer, 1, (size_t)size, file);
    buffer[size] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 90);
    cJSON_DeleteItemFromObject(root, "frostfell_quest_state");
    cJSON_DeleteItemFromObject(root, "frostfell_quest_progress");
    cJSON_DeleteItemFromObject(root, "frostfell_quest_encounters");
    char *json = cJSON_Print(root);
    cJSON_Delete(root);
    file = fopen(FROST_QUEST_SAVE, "wb");
    fputs(json, file);
    fclose(file);
    free(json);
    ASSERT("version 90 migrates new quest fields while retaining progress", load_game(&loaded, FROST_QUEST_SLOT) &&
        loaded.gold == 731 && !loaded.frostfell_quest_state && !loaded.frostfell_quest_progress && !loaded.frostfell_quest_encounters &&
        loaded.max_frostfell_level_reached == 5 && (loaded.defeated_bosses & (1 << LOCATION_FROSTFELL)));
    ASSERT("legacy migration preserves player and loot while removing Brenna from the Tavern", loaded.player.x == BRENNA_X &&
        loaded.player.y == BRENNA_Y + 1 && loaded.floor_items[0].active && loaded.floor_items[0].y == BRENNA_Y + 1 &&
        loaded.floor_items[0].item.type == ITEM_WEAPON && loaded.floor_items[0].underlying_tile == TILE_TAVERN_FLOOR &&
        loaded.map.tiles[BRENNA_Y][BRENNA_X] == TILE_TAVERN_FLOOR && loaded.map.tiles[BRENNA_Y + 1][BRENNA_X] == TILE_ITEM);
    ASSERT("migrated save writes and reloads", save_game(&loaded, FROST_QUEST_SLOT) && load_game(&game, FROST_QUEST_SLOT));
    accept();
    ASSERT("the quest remains available after a previous Kraken victory", game.frostfell_quest_state == 1 &&
        (game.defeated_bosses & (1 << LOCATION_FROSTFELL)));
}

void test_frostfell_quest(void) {
    printf("The Silent Expedition quest\n");
    test_expedition();
    test_revisits();
    test_covered_portal();
    test_survivor_access();
    test_legacy_save();
    remove(FROST_QUEST_SAVE);
}
