#include "test_utils.h"
#include "game/game.h"
#include "game/town_life.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include <stdlib.h>
#include <string.h>

#define ADVENTURE_SLOT 99147
static GameState game;
static GameState loaded;

static int count_tile(TileType tile) {
    int count = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            count += game.map.tiles[y][x] == tile;
        }
    }
    return count;
}

static void walk_inn(int tx, int ty) {
    int previous[MAP_H][MAP_W];
    int queue[MAP_W * MAP_H];
    memset(previous, -1, sizeof(previous));
    int start = game.player.y * MAP_W + game.player.x;
    int head = 0;
    int tail = 1;
    queue[0] = start;
    previous[game.player.y][game.player.x] = start;
    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};
    while (head < tail && previous[ty][tx] < 0) {
        int cell = queue[head++];
        for (int side = 0; side < 4; side++) {
            int x = cell % MAP_W + dx[side];
            int y = cell / MAP_W + dy[side];
            if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H || previous[y][x] >= 0 ||
                !map_is_walkable(&game.map, x, y) || game.map.tiles[y][x] == TILE_TAVERN_EXIT) {
                continue;
            }
            previous[y][x] = cell;
            queue[tail++] = y * MAP_W + x;
        }
    }
    ASSERT("both new quest givers have continuous routes from the Inn entrance", previous[ty][tx] >= 0);
    if (previous[ty][tx] < 0) {
        return;
    }
    int count = 0;
    for (int cell = ty * MAP_W + tx; cell != start; cell = previous[cell / MAP_W][cell % MAP_W]) {
        queue[count++] = cell;
    }
    for (int i = count - 1; i >= 0; i--) {
        action_resolve_player(&game, (Action){ACTION_MOVE, queue[i] % MAP_W, queue[i] / MAP_W});
    }
    ASSERT("walking the route reaches the NPC without leaving the Inn", game.location == LOCATION_TAVERN && game.player.x == tx && game.player.y == ty);
}

static void enter_inn(void) {
    game_enter_town_gate(&game, LOCATION_TOWN, 0, 12);
    game.player.x = TOWN_TAVERN_DOOR_X;
    game.player.y = TOWN_TAVERN_DOOR_Y + 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, TOWN_TAVERN_DOOR_X, TOWN_TAVERN_DOOR_Y});
}

static void test_arrivals(void) {
    for (int seed = 0; seed < 3; seed++) {
        memset(&game, 0, sizeof(game));
        game_init(&game);
        srand(6400 + seed);
        game.defeated_bosses = 1 << LOCATION_FOREST;
        enter_inn();
        ASSERT("other boss victories do not cause the post-Lich arrivals", !count_tile(TILE_NPC_ALDER) && !count_tile(TILE_NPC_DAIN));
        game.player.x = ALDER_OAKHAVEN_X;
        game.player.y = ALDER_OAKHAVEN_Y + 1;
        game_talk_to_alder(&game);
        game.player.x = DAIN_OAKHAVEN_X;
        game.player.y = DAIN_OAKHAVEN_Y + 1;
        game_talk_to_dain(&game);
        ASSERT("neither NPC can be spoken to before the Lich is defeated", !game.dialogue_active);
        game.dialogue_active = 1;
        snprintf(game.dialogue_speaker, MAX_SPEAKER_LEN, "Alder");
        ASSERT("saved or forged dialogue cannot bypass the Lich prerequisite", !game_quest_offer_active(&game) &&
            !game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0) && !game.alder_quest_state);
        game.defeated_bosses = 1 << LOCATION_DUNGEON;
        // The two adventures are available even before Elowen's reunion.
        game.elowen_quest_state = 2;
        enter_inn();
        ASSERT("returning after the Lich places exactly one Alder and one Dain", count_tile(TILE_NPC_ALDER) == 1 && count_tile(TILE_NPC_DAIN) == 1 &&
            game.map.tiles[BRAM_INN_Y][BRAM_INN_X] == TILE_NPC_BRAM);
        ASSERT("both arrivals and the unfinished reunion survive save/load", save_game(&game, ADVENTURE_SLOT) && load_game(&loaded, ADVENTURE_SLOT) &&
            loaded.map.tiles[ALDER_OAKHAVEN_Y][ALDER_OAKHAVEN_X] == TILE_NPC_ALDER &&
            loaded.map.tiles[DAIN_OAKHAVEN_Y][DAIN_OAKHAVEN_X] == TILE_NPC_DAIN && loaded.elowen_quest_state == 2);
        game = loaded;
        walk_inn(ALDER_OAKHAVEN_X, ALDER_OAKHAVEN_Y + 1);
        game_talk_to_alder(&game);
        ASSERT("Alder introduces the eastward two-ranger adventure", game_quest_offer_active(&game) && strstr(game.dialogue_text, "Forest 1") && strstr(game.dialogue_text, "Forest 2"));
        game_handle_quest_offer_key(&game, SDL_SCANCODE_N, 0);
        walk_inn(DAIN_OAKHAVEN_X, DAIN_OAKHAVEN_Y + 1);
        game_talk_to_dain(&game);
        ASSERT("Dain introduces the westward two-half adventure", game_quest_offer_active(&game) && strstr(game.dialogue_text, "Mountains 1 and 2"));
        game_handle_quest_offer_key(&game, SDL_SCANCODE_N, 0);
        walk_inn(20, 21);
        action_resolve_player(&game, (Action){ACTION_MOVE, 20, 22});
        ASSERT("both new actors preserve the actual exit back to Oakhaven", game.location == LOCATION_TOWN);
        game_enter_inn(&game);
        ASSERT("Stillbury no longer duplicates Alder or Dain", !count_tile(TILE_NPC_ALDER) && !count_tile(TILE_NPC_DAIN));
        int x;
        int y;
        town_life_generate(&game.map, LOCATION_ALEHOUSE, &x, &y);
        ASSERT("Dawnmere's Alehouse keeps its ordinary host after Dain leaves", !count_tile(TILE_NPC_DAIN) && game.map.tiles[RESIDENT_Y][RESIDENT_X] == TILE_NPC_RESIDENT);
    }
}

static void test_late_acceptance(int alder) {
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game.defeated_bosses = 1 << LOCATION_DUNGEON;
    if (alder) {
        game_enter_forest(&game);
    } else {
        game_enter_mountains(&game);
    }
    map_mark_explored(&game.map, game.player.x, game.player.y);
    game_descend(&game);
    game_return_to_approach(&game);
    loaded = game;
    enter_inn();
    walk_inn(alder ? ALDER_OAKHAVEN_X : DAIN_OAKHAVEN_X, (alder ? ALDER_OAKHAVEN_Y : DAIN_OAKHAVEN_Y) + 1);
    if (alder) {
        game_talk_to_alder(&game);
    } else {
        game_talk_to_dain(&game);
    }
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    LevelCache *cache = alder ? game.forest_cache : game.mountain_cache;
    LevelCache *before = alder ? loaded.forest_cache : loaded.mountain_cache;
    ASSERT("accepting a first quest preserves both explored regional maps", cache[0].valid && cache[1].valid &&
        memcmp(&cache[0].map, &before[0].map, sizeof(Map)) == 0 &&
        memcmp(&cache[1].map, &before[1].map, sizeof(Map)) == 0 &&
        (alder ? game.max_forest_level_reached : game.max_mountain_level_reached) == 2);
    if (alder) {
        game_enter_forest(&game);
    } else {
        game_enter_mountains(&game);
    }
    for (int stage = 1; stage <= 2; stage++) {
        int objective = alder ? count_tile(TILE_FOREST_WARDEN) : 0;
        for (int i = 0; !alder && i < game.enemy_count; i++) {
            objective += game.enemies[i].active && game.enemies[i].dain_fragment != 0;
        }
        ASSERT("late acceptance adds exactly one objective to each previously explored early stage", objective == 1);
        if (stage == 1) {
            game_descend(&game);
        }
    }
}

static void test_chain(int alder) {
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game.defeated_bosses = 1 << LOCATION_DUNGEON;
    int *state = alder ? &game.alder_quest_state : &game.dain_quest_state;
    int *progress = alder ? &game.alder_wardens_rescued : &game.dain_map_fragments;
    Location region = alder ? LOCATION_FOREST : LOCATION_MOUNTAINS;
    void (*talk)(GameState *) = alder ? game_talk_to_alder : game_talk_to_dain;
    int x = alder ? ALDER_OAKHAVEN_X : DAIN_OAKHAVEN_X;
    int y = alder ? ALDER_OAKHAVEN_Y : DAIN_OAKHAVEN_Y;
    *state = 2;
    *progress = 3;
    game.elowen_quest_state = 1;
    game.rook_quest_state = 1;
    game.mara_quest_state = 1;
    enter_inn();
    walk_inn(x, y + 1);
    int gold = game.gold;
    talk(&game);
    ASSERT("turning in the short quest pays once and offers its boss sequel", *state == REGIONAL_CHAIN_BOSS_OFFERED &&
        game.gold == gold + (alder ? 70 : 60) && game_quest_offer_active(&game) && strstr(game.dialogue_text, "kingdom"));
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    ASSERT("boss sequels respect the three-active-quest limit", *state == REGIONAL_CHAIN_BOSS_OFFERED && game_quest_offer_at_limit(&game));
    game.mara_quest_state = 2;
    game_handle_quest_offer_key(&game, SDL_SCANCODE_N, 0);
    ASSERT("declining a sequel leaves first-part rewards and progress intact", *state == REGIONAL_CHAIN_BOSS_OFFERED && *progress == 3 && game.gold == gold + (alder ? 70 : 60));
    talk(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    ASSERT("accepting the boss sequel retains the completed first part", *state == REGIONAL_CHAIN_BOSS_ACTIVE && *progress == 3 && game_active_quest_count(&game) == 3);
    ASSERT("the sequel and first-part state survive saving", save_game(&game, ADVENTURE_SLOT) && load_game(&loaded, ADVENTURE_SLOT) &&
        (alder ? loaded.alder_quest_state : loaded.dain_quest_state) == REGIONAL_CHAIN_BOSS_ACTIVE &&
        (alder ? loaded.alder_wardens_rescued : loaded.dain_map_fragments) == 3);
    game = loaded;
    game.defeated_bosses |= 1 << region;
    game_update_level_progress(&game);
    ASSERT("boss victory completes the sequel and releases its active slot immediately", *state == REGIONAL_CHAIN_COMPLETE && game_active_quest_count(&game) == 2);
    talk(&game);
    talk(&game);
    ASSERT("the completed chain gives onward directions without another turn-in payment", !game_quest_offer_active(&game) &&
        strstr(game.dialogue_text, alder ? "Dawnmere" : "Stillbury") && game.gold == gold + (alder ? 70 : 60));
    ASSERT("the completed chain remains completed after saving", save_game(&game, ADVENTURE_SLOT) && load_game(&loaded, ADVENTURE_SLOT) &&
        (alder ? loaded.alder_quest_state : loaded.dain_quest_state) == REGIONAL_CHAIN_COMPLETE);

    game_init(&game);
    game.defeated_bosses = (1 << LOCATION_DUNGEON) | (1 << region);
    *state = 2;
    *progress = 3;
    enter_inn();
    walk_inn(x, y + 1);
    talk(&game);
    ASSERT("a boss defeated before the first turn-in is credited without another fight", *state == REGIONAL_CHAIN_COMPLETE &&
        !game_quest_offer_active(&game) && !game_active_quest_count(&game) && quest_journal_count(&game, QUEST_TAB_COMPLETED) == 2);

    game_init(&game);
    game.defeated_bosses = 1 << LOCATION_DUNGEON;
    if (alder) {
        game_enter_forest(&game);
    } else {
        game_enter_mountains(&game);
    }
    for (int stage = 1; stage < 5; stage++) {
        game_descend(&game);
    }
    int boss = 0;
    for (int i = 0; i < game.enemy_count; i++) {
        boss |= game.enemies[i].active && game.enemies[i].type == (alder ? ENEMY_FOREST_NECROMANCER : ENEMY_MOUNTAIN_GOBLIN_KING);
    }
    ASSERT("both bosses remain available without either NPC conversation or quest", game.level == 5 && boss && !*state && !*progress);
}

void test_alder_relocation(void) {
    printf("Post-Lich adventure arrivals and quest chains:\n");
    ASSERT("adventure test save slot is unused", !save_exists(ADVENTURE_SLOT));
    test_arrivals();
    test_late_acceptance(1);
    test_late_acceptance(0);
    test_chain(1);
    test_chain(0);
    remove("saves/savegame_99147.json");
}
