#include "test_utils.h"
#include "game/game.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define RESONANCE_SLOT 99143
#define RESONANCE_SAVE "saves/savegame_99143.json"
static GameState game;
static GameState loaded;

static void start(void) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    srand(9143);
}

static void accept(void) {
    game_enter_town3(&game);
    game_enter_guild(&game);
    game.player.x = GUILD_ORIN_X;
    game.player.y = GUILD_ORIN_Y + 1;
    game_talk_to_orin(&game);
    game_leave_guild(&game);
}

static int find_resonator(TileType tile, int *x, int *y) {
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
        if ((!alive || g->enemies[i].active) && strcmp(g->enemies[i].name, "Resonator Guardian") == 0) {
            count++;
        }
    }
    return count;
}

static void clear_defenders(void) {
    for (int i = 0; i < game.enemy_count; i++) {
        game.enemies[i].active = 0;
        game.enemies[i].hp = 0;
    }
}

static void tune(int tone) {
    int x = -1;
    int y = -1;
    ASSERT("unfinished stage has a resonator", find_resonator(TILE_GLASSDEEP_RESONATOR, &x, &y));
    if (x < 0) {
        return;
    }
    clear_defenders();
    game.player.x = x;
    game.player.y = y;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("Action opens the inscription and all three tuning choices", game_glassdeep_prompt_active(&game) &&
        strstr(game.dialogue_text, "1 Low, 2 Middle, 3 High"));
    game_handle_glassdeep_prompt_key(&game, tone == 1 ? SDL_SCANCODE_1 : tone == 2 ? SDL_SCANCODE_2 : SDL_SCANCODE_3, 0);
    ASSERT("matching tone lights the resonator and closes the choice prompt", game.map.tiles[y][x] == TILE_GLASSDEEP_RESONATOR_LIT &&
        !game_glassdeep_prompt_active(&game));
}

static void test_quest_flow(void) {
    start();
    game_enter_town3(&game);
    game_enter_guild(&game);
    ASSERT("Orin shares the Guild with Dain and Zara and has an accessible approach", game.map.tiles[GUILD_ORIN_Y][GUILD_ORIN_X] == TILE_NPC_ORIN &&
        !map_is_walkable(&game.map, GUILD_ORIN_X, GUILD_ORIN_Y) && map_is_walkable(&game.map, GUILD_ORIN_X, GUILD_ORIN_Y + 1) &&
        game.map.tiles[GUILD_DAIN_Y][GUILD_DAIN_X] == TILE_NPC_DAIN && game.map.tiles[GUILD_ZARA_Y][GUILD_ZARA_X] == TILE_NPC_GUILD_SEEKER);
    game_talk_to_orin(&game);
    ASSERT("Orin cannot assign the quest remotely", !game.glassdeep_quest_state);
    game.player.x = GUILD_ORIN_X;
    game.player.y = GUILD_ORIN_Y + 1;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("Action beside Orin does not assign the conversation quest", !game.glassdeep_quest_state);
    game_talk_to_orin(&game);
    ASSERT("Orin directs the player to Stillbury's south gate and stages two through four", game.glassdeep_quest_state == 1 &&
        strstr(game.dialogue_text, "Stillbury's south gate") && strstr(game.dialogue_text, "2, 3, and 4") &&
        !game.dain_quest_state && !game.sunscar_lamp_quest_state);
    game_talk_to_orin(&game);
    ASSERT("repeat conversation gives instructions without rewarding early", game.glassdeep_quest_state == 1 && strstr(game.dialogue_text, "press A"));
    game_leave_guild(&game);
    game_enter_glassdeep(&game);
    int x = -1;
    int y = -1;
    ASSERT("stage one has no quest resonator", !find_resonator(TILE_GLASSDEEP_RESONATOR, &x, &y));
    game_descend(&game);
    game_descend(&game);
    game_descend(&game);
    ASSERT("stage four uses one Shard Golem guard", guards(&game, 1) == 1 && game.enemies[game.enemy_count - 1].type == ENEMY_SHARD_GOLEM);
    tune(3);
    ASSERT("Crown can be restored first without the Prism Sovereign", game.glassdeep_quest_state == 1 && game.glassdeep_quest_progress == 4 &&
        !(game.defeated_bosses & (1 << LOCATION_GLASSDEEP)));
    game_ascend(&game);
    game_ascend(&game);
    ASSERT("stage two uses two Crystal Spider guards", guards(&game, 1) == 2 && game.enemies[game.enemy_count - 1].type == ENEMY_CRYSTAL_SPIDER);
    tune(1);
    ASSERT("Root can be restored after Crown", game.glassdeep_quest_progress == 5 && game.glassdeep_quest_state == 1);
    game_descend(&game);
    ASSERT("stage three uses two Blind Stalker guards", guards(&game, 1) == 2 && game.enemies[game.enemy_count - 1].type == ENEMY_BLIND_STALKER);
    tune(2);
    ASSERT("all three independent objectives mark the quest ready to return", game.glassdeep_quest_state == 2 && game.glassdeep_quest_progress == 7 &&
        !(game.defeated_bosses & (1 << LOCATION_GLASSDEEP)));
    QuestJournalEntry entry;
    ASSERT("journal records all three stages and completed objectives", quest_journal_get_entry(&game, QUEST_TAB_ACTIVE, 0, &entry) &&
        strcmp(entry.title, "The Broken Resonance") == 0 && entry.objective_count == 3 &&
        entry.stages[0] == 2 && entry.stages[1] == 3 && entry.stages[2] == 4 && entry.objective_complete[0] &&
        entry.objective_complete[1] && entry.objective_complete[2] && entry.reward_gold == 120 && entry.reward_score == 1000);
    ASSERT("ready quest and guardians survive saving", save_game(&game, RESONANCE_SLOT) && load_game(&loaded, RESONANCE_SLOT) &&
        loaded.glassdeep_quest_state == 2 && loaded.glassdeep_quest_progress == 7 && loaded.glassdeep_quest_encounters == 7);
    game = loaded;
    game_return_to_town(&game);
    game_enter_town3(&game);
    game_enter_guild(&game);
    game.player.x = GUILD_ORIN_X;
    game.player.y = GUILD_ORIN_Y + 1;
    int gold = game.gold;
    int score = game.score;
    game_talk_to_orin(&game);
    ASSERT("Orin awards 120 gold and 1000 score once", game.glassdeep_quest_state == 3 && game.gold == gold + 120 && game.score == score + 1000);
    game_talk_to_orin(&game);
    ASSERT("repeat conversation cannot duplicate the reward", game.gold == gold + 120 && game.score == score + 1000);
    ASSERT("completed journal retains the restored resonators", quest_journal_count(&game, QUEST_TAB_ACTIVE) == 0 &&
        quest_journal_get_entry(&game, QUEST_TAB_COMPLETED, 0, &entry) && entry.state == 3 && entry.objective_count == 3);
    ASSERT("completed quest and reward survive save/load inside the Guild", save_game(&game, RESONANCE_SLOT) && load_game(&loaded, RESONANCE_SLOT) &&
        loaded.glassdeep_quest_state == 3 && loaded.gold == gold + 120 && loaded.score == score + 1000 &&
        loaded.map.tiles[GUILD_ORIN_Y][GUILD_ORIN_X] == TILE_NPC_ORIN);
    game_leave_guild(&game);
    game_enter_glassdeep(&game);
    game_descend(&game);
    ASSERT("a return visit retains the restored crystal and defeated quest guards", find_resonator(TILE_GLASSDEEP_RESONATOR_LIT, &x, &y) &&
        !guards(&game, 1) && game.glassdeep_quest_progress == 7);
    game_return_to_town(&game);
    memset(game.glassdeep_cache, 0, sizeof(game.glassdeep_cache));
    game_enter_glassdeep(&game);
    game_descend(&game);
    ASSERT("a regenerated stage retains completed objectives without new quest guards", find_resonator(TILE_GLASSDEEP_RESONATOR_LIT, &x, &y) &&
        !guards(&game, 0) && game.glassdeep_quest_progress == 7);
}

static void test_tuning_and_guards(void) {
    start();
    accept();
    game_enter_glassdeep(&game);
    game_descend(&game);
    int x = -1;
    int y = -1;
    ASSERT("guard fixture has a Root resonator", find_resonator(TILE_GLASSDEEP_RESONATOR, &x, &y));
    if (x < 0) {
        return;
    }
    game.player.x = x;
    game.player.y = y;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("live guardians prevent tuning", !game_glassdeep_prompt_active(&game) && !game.glassdeep_quest_progress);
    game.enemies[game.enemy_count - 1].hp = 7;
    game_refresh_quest_encounters(&game);
    ASSERT("refresh and save/load preserve guard damage without duplication", guards(&game, 0) == 2 &&
        save_game(&game, RESONANCE_SLOT) && load_game(&loaded, RESONANCE_SLOT) && guards(&loaded, 0) == 2 &&
        loaded.enemies[loaded.enemy_count - 1].hp == 7);
    for (int i = 0; i < game.enemy_count; i++) {
        if (strcmp(game.enemies[i].name, "Resonator Guardian") == 0) {
            game.enemies[i].x = 1;
            game.enemies[i].y = 1;
        } else {
            game.enemies[i].active = 0;
        }
    }
    game_interact_glassdeep(&game);
    ASSERT("luring guards away cannot bypass their defeat", !game_glassdeep_prompt_active(&game));
    clear_defenders();
    game_interact_glassdeep(&game);
    int hp = game.player.hp;
    int mp = game.player.mp;
    ASSERT("prompt consumes movement and spell keys", game_handle_glassdeep_prompt_key(&game, SDL_SCANCODE_LEFT, 0) &&
        game_handle_glassdeep_prompt_key(&game, SDL_SCANCODE_C, 0) && game.player.x == x && game.player.y == y && game.player.mp == mp);
    game_handle_glassdeep_prompt_key(&game, SDL_SCANCODE_3, 0);
    ASSERT("a wrong tone preserves health and progress and repeats the clue", !game.glassdeep_quest_progress && game.player.hp == hp &&
        game_glassdeep_prompt_active(&game) && strstr(game.dialogue_text, "low rumble") && strstr(game.dialogue_text, "stays dark"));
    game_handle_glassdeep_prompt_key(&game, SDL_SCANCODE_1, 1);
    ASSERT("repeated key events cannot tune the crystal", !game.glassdeep_quest_progress);
    game_handle_glassdeep_prompt_key(&game, SDL_SCANCODE_ESCAPE, 0);
    ASSERT("Escape cancels tuning without completing the objective", !game.dialogue_active && !game.glassdeep_quest_progress);
    game_interact_glassdeep(&game);
    ASSERT("a saved tuning prompt restores its choices and dead guards", save_game(&game, RESONANCE_SLOT) && load_game(&loaded, RESONANCE_SLOT) &&
        game_glassdeep_prompt_active(&loaded) && !guards(&loaded, 1));
    game = loaded;
    game.floor_item_count = 2;
    for (int i = 0; i < 2; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = x, .y = y,
            .item = item_make_health_potion(), .underlying_tile = TILE_GLASSDEEP_RESONATOR};
    }
    game.map.tiles[y][x] = TILE_ITEM;
    game_handle_glassdeep_prompt_key(&game, SDL_SCANCODE_KP_1, 0);
    ASSERT("tuning beneath overlapping loot preserves both items and updates their underlays", game.glassdeep_quest_progress == 1 &&
        game.map.tiles[y][x] == TILE_ITEM && game.floor_items[0].active && game.floor_items[1].active &&
        game.floor_items[0].underlying_tile == TILE_GLASSDEEP_RESONATOR_LIT && game.floor_items[1].underlying_tile == TILE_GLASSDEEP_RESONATOR_LIT);
    game.floor_item_count = 0;
    game.map.tiles[y][x] = TILE_GLASSDEEP_RESONATOR_LIT;
    game_open_town_portal(&game);
    ASSERT("portal over a restored crystal retains the objective through saving", save_game(&game, RESONANCE_SLOT) && load_game(&loaded, RESONANCE_SLOT) &&
        loaded.glassdeep_quest_progress == 1 && loaded.portal_origin_tile == TILE_GLASSDEEP_RESONATOR_LIT);
    game = loaded;
    game_use_town_portal(&game);
    ASSERT("portal return retains defeated guards and restored crystal", game.location == LOCATION_GLASSDEEP && game.level == 2 &&
        game.map.tiles[y][x] == TILE_GLASSDEEP_RESONATOR_LIT && !guards(&game, 1));
    game_return_to_town(&game);
    game_enter_glassdeep(&game);
    game_descend(&game);
    game_descend(&game);
    ASSERT("unfinished cached objectives remain guarded on return", guards(&game, 1) == 2 && find_resonator(TILE_GLASSDEEP_RESONATOR, &x, &y));
}

static void test_existing_progress(void) {
    start();
    game_enter_glassdeep(&game);
    game_descend(&game);
    game.enemies[0].hp = 5;
    game.enemies[1].active = 0;
    map_mark_explored(&game.map, game.map.stairs_up_x, game.map.stairs_up_y);
    int count = game.enemy_count;
    game_open_town_portal(&game);
    accept();
    ASSERT("acceptance preserves previously explored cavern maps and enemy progress", game.glassdeep_cache[1].valid &&
        game.glassdeep_cache[1].enemy_count == count && game.glassdeep_cache[1].enemies[0].hp == 5 &&
        !game.glassdeep_cache[1].enemies[1].active &&
        map_is_explored(&game.glassdeep_cache[1].map, game.glassdeep_cache[1].map.stairs_up_x, game.glassdeep_cache[1].map.stairs_up_y));
    game_use_town_portal(&game);
    int x = -1;
    int y = -1;
    ASSERT("returning to a cached stage injects the quest without resetting its existing defenders", game.location == LOCATION_GLASSDEEP && game.level == 2 &&
        game.enemies[0].hp == 5 && find_resonator(TILE_GLASSDEEP_RESONATOR, &x, &y) && guards(&game, 1) == 2);
    ASSERT("new guards never occupy the player's portal landing", game.enemies[game.enemy_count - 1].x != game.player.x ||
        game.enemies[game.enemy_count - 1].y != game.player.y);
}

static void test_resonator_access(void) {
    static unsigned char seen[MAP_H][MAP_W];
    static int queue[MAP_H * MAP_W];
    int reachable = 1;
    int pools = 1;
    for (int seed = 0; seed < 32 && reachable && pools; seed++) {
        start();
        srand((unsigned int)seed);
        game.glassdeep_quest_state = 1;
        game_enter_glassdeep(&game);
        for (int stage = 2; stage <= 4; stage++) {
            game_descend(&game);
            int x = -1;
            int y = -1;
            if (!find_resonator(TILE_GLASSDEEP_RESONATOR, &x, &y)) {
                reachable = 0;
                break;
            }
            memset(seen, 0, sizeof(seen));
            int head = 0;
            int tail = 0;
            queue[tail++] = game.map.stairs_up_y * MAP_W + game.map.stairs_up_x;
            seen[game.map.stairs_up_y][game.map.stairs_up_x] = 1;
            const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
            while (head < tail) {
                int cell = queue[head++];
                for (int i = 0; i < 4; i++) {
                    int tx = cell % MAP_W + offsets[i][0];
                    int ty = cell / MAP_W + offsets[i][1];
                    if (tx < 0 || tx >= MAP_W || ty < 0 || ty >= MAP_H || seen[ty][tx] || !map_is_walkable(&game.map, tx, ty)) {
                        continue;
                    }
                    seen[ty][tx] = 1;
                    queue[tail++] = ty * MAP_W + tx;
                }
            }
            reachable &= seen[y][x] && seen[game.map.stairs_down_y][game.map.stairs_down_x];
            if (stage == 3) {
                pools &= game.map.tiles[y - 1][x] == TILE_GLASSDEEP_POOL || game.map.tiles[y + 1][x] == TILE_GLASSDEEP_POOL ||
                    game.map.tiles[y][x - 1] == TILE_GLASSDEEP_POOL || game.map.tiles[y][x + 1] == TILE_GLASSDEEP_POOL;
            }
        }
    }
    ASSERT("resonators and stage exits remain reachable across 32 generated expeditions", reachable);
    ASSERT("Tide resonators appear beside underground pools across 32 expeditions", pools);
}

static cJSON *read_save(void) {
    FILE *file = fopen(RESONANCE_SAVE, "rb");
    if (!file) {
        return NULL;
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
    return root;
}

static void write_save(cJSON *root) {
    char *json = cJSON_Print(root);
    FILE *file = fopen(RESONANCE_SAVE, "wb");
    if (file) {
        fputs(json, file);
        fclose(file);
    }
    free(json);
}

static void test_legacy_save(void) {
    start();
    game_enter_glassdeep(&game);
    game_descend(&game);
    game.enemies[0].hp = 5;
    game_open_town_portal(&game);
    game_enter_town3(&game);
    game_enter_guild(&game);
    game.gold = 731;
    game.player.hp = 63;
    game.defeated_bosses |= 1 << LOCATION_GLASSDEEP;
    game.max_glassdeep_level_reached = 5;
    game.player.x = GUILD_ORIN_X;
    game.player.y = GUILD_ORIN_Y;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = GUILD_ORIN_X, .y = GUILD_ORIN_Y,
        .underlying_tile = TILE_TAVERN_FLOOR, .item = item_make_short_sword()};
    game.map.tiles[GUILD_ORIN_Y][GUILD_ORIN_X] = TILE_ITEM;
    ASSERT("legacy Guild fixture saves", save_game(&game, RESONANCE_SLOT));
    cJSON *root = read_save();
    if (!root) {
        ASSERT("legacy fixture parses", 0);
        return;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 93);
    cJSON_DeleteItemFromObject(root, "glassdeep_quest_state");
    cJSON_DeleteItemFromObject(root, "glassdeep_quest_progress");
    cJSON_DeleteItemFromObject(root, "glassdeep_quest_encounters");
    write_save(root);
    cJSON_Delete(root);
    ASSERT("version 93 initializes the quest while preserving character, boss, portal and cached enemy progress", load_game(&loaded, RESONANCE_SLOT) &&
        !loaded.glassdeep_quest_state && !loaded.glassdeep_quest_progress && !loaded.glassdeep_quest_encounters &&
        loaded.gold == 731 && loaded.player.hp == 63 && (loaded.defeated_bosses & (1 << LOCATION_GLASSDEEP)) &&
        loaded.max_glassdeep_level_reached == 5 && loaded.glassdeep_cache[1].valid && loaded.glassdeep_cache[1].enemies[0].hp == 5 && loaded.portal_active);
    ASSERT("legacy Guild places Orin and moves overlapping player and loot safely beside him", loaded.map.tiles[GUILD_ORIN_Y][GUILD_ORIN_X] == TILE_NPC_ORIN &&
        loaded.player.y == GUILD_ORIN_Y + 1 && loaded.floor_items[0].active && loaded.floor_items[0].y == GUILD_ORIN_Y + 1 &&
        loaded.floor_items[0].underlying_tile == TILE_TAVERN_FLOOR && loaded.map.tiles[GUILD_ORIN_Y + 1][GUILD_ORIN_X] == TILE_ITEM);
    game = loaded;
    game_talk_to_orin(&game);
    ASSERT("Orin offers the quest after a previous Prism Sovereign victory", game.glassdeep_quest_state == 1);
    ASSERT("migrated Guild save rewrites and reloads", save_game(&game, RESONANCE_SLOT) && load_game(&loaded, RESONANCE_SLOT));
    root = read_save();
    if (!root) {
        return;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "glassdeep_quest_progress"), 8);
    write_save(root);
    ASSERT("load rejects invalid resonance progress bits", !load_game(&loaded, RESONANCE_SLOT));
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "glassdeep_quest_progress"), 7);
    write_save(root);
    ASSERT("load rejects a finished objective set marked as still incomplete", !load_game(&loaded, RESONANCE_SLOT));
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "glassdeep_quest_progress"), 0);
    cJSON_DeleteItemFromObject(root, "glassdeep_quest_state");
    write_save(root);
    ASSERT("current saves must contain the new persistent quest fields", !load_game(&loaded, RESONANCE_SLOT));
    cJSON_Delete(root);
}

void test_glassdeep_quest(void) {
    printf("Glassdeep Caverns: The Broken Resonance tests:\n");
    test_quest_flow();
    test_tuning_and_guards();
    test_existing_progress();
    test_resonator_access();
    test_legacy_save();
    remove(RESONANCE_SAVE);
}
