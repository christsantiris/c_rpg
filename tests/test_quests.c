#include "game/western_arc.h"
#include "approach_checks.h"
#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/game/town_life.h"
#include "../src/systems/save_load.h"
#include "../src/screens/quest_journal.h"
#include <string.h>
#include "../src/game/jail.h"
#include "../src/game/war_machine.h"

static int find_tile(const Map *map, TileType type, int *found_x, int *found_y) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (map->tiles[y][x] == type) {
                *found_x = x;
                *found_y = y;
                return 1;
            }
        }
    }
    return 0;
}

void test_quest_activation_gating(void) {
    printf("Quest activation gating tests:\n");
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);

    game_enter_dungeon(&g);
    game_descend(&g);
    int object_x = 0;
    int object_y = 0;
    ASSERT("retired burial-seal objectives do not spawn",
        !find_tile(&g.map, TILE_BROKEN_BURIAL_SEAL, &object_x, &object_y));

    game_return_to_town(&g);
    game_enter_forest(&g);
    game_descend(&g);
    ASSERT("Alder's wardens require quest activation",
        !find_tile(&g.map, TILE_FOREST_WARDEN, &object_x, &object_y));

    game_return_to_town(&g);
    game_enter_coast(&g);
    ASSERT("Mara's beacons require quest activation",
        !find_tile(&g.map, TILE_COAST_BEACON_UNLIT, &object_x, &object_y));

    g.location = LOCATION_MOUNTAINS;
    game_record_dain_kill(&g, ENEMY_GOBLIN_ARCHER);
    ASSERT("Dain's fragments require quest activation",
        g.dain_map_fragments == 0);
}

void test_elowen_quest(void) {
    printf("Bram's Save Elowen offer tests:\n");
    GameState g = {0};
    game_init(&g);
    game_enter_dungeon(&g);
    game_descend(&g);
    g.enemies[0].hp = 7;
    game_return_to_town(&g);
    game_enter_tavern(&g);
    g.player.x = BRAM_INN_X;
    g.player.y = BRAM_INN_Y + 1;
    game_talk_to_bram(&g);
    ASSERT("Bram offers the rescue from his Inn counter", game_quest_offer_active(&g) &&
        strstr(g.dialogue_text, "my daughter Elowen") && strstr(g.dialogue_text, "Lich King"));
    game_handle_quest_offer_key(&g, SDL_SCANCODE_N, 0);
    ASSERT("declining leaves the rescue available", !g.elowen_quest_state && !g.dialogue_active);
    game_talk_to_bram(&g);
    game_handle_quest_offer_key(&g, SDL_SCANCODE_Y, 0);
    ASSERT("accepting Save Elowen records the quest without resetting dungeon progress", g.elowen_quest_state == 1 &&
        g.level_cache[1].valid && g.level_cache[1].enemies[0].hp == 7 && g.max_level_reached == 2);
    ASSERT("the event log identifies the rescue assignment", strstr(g.messages[g.message_count - 1], "Save Elowen") != NULL);
}

void test_tavern_interior(void) {
    printf("Tavern interior tests:\n");
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);

    g.player.x = TOWN_TAVERN_DOOR_X;
    g.player.y = TOWN_TAVERN_DOOR_Y + 1;
    Action enter = {ACTION_MOVE, TOWN_TAVERN_DOOR_X, TOWN_TAVERN_DOOR_Y};
    action_resolve_player(&g, enter);
    ASSERT("walking into the Tavern door enters its interior",
        g.location == LOCATION_TAVERN);
    ASSERT("Tavern spawn is walkable",
        map_is_walkable(&g.map, g.player.x, g.player.y));

    int elowen_x = 0;
    int elowen_y = 0;
    ASSERT("Elowen is absent from the Inn before her rescue",
        !find_tile(&g.map, TILE_NPC_ELOWEN, &elowen_x, &elowen_y));
    ASSERT("Elowen can be approached beside her Tavern position",
        map_is_walkable(&g.map, ELOWEN_TAVERN_X, ELOWEN_TAVERN_Y + 1));
    int dain_x = 0;
    int dain_y = 0;
    ASSERT("Dain no longer occupies the Tavern",
        !find_tile(&g.map, TILE_NPC_DAIN, &dain_x, &dain_y));
    ASSERT("Dain's old Tavern position is walkable",
        map_is_walkable(&g.map, GUILD_DAIN_X, GUILD_DAIN_Y));
    int alder_x = 0;
    int alder_y = 0;
    ASSERT("Alder no longer occupies the Tavern",
        !find_tile(&g.map, TILE_NPC_ALDER, &alder_x, &alder_y));
    ASSERT("Bram now occupies the Inn's former Alder counter position",
        g.map.tiles[BRAM_INN_Y][BRAM_INN_X] == TILE_NPC_BRAM &&
        map_is_walkable(&g.map, BRAM_INN_X, BRAM_INN_Y + 1));
    int retired_x = 0;
    int retired_y = 0;
    ASSERT("Rook, Mara and Ilya no longer stand in Oakhaven's Inn",
        !find_tile(&g.map, TILE_NPC_ROOK, &retired_x, &retired_y) &&
        !find_tile(&g.map, TILE_NPC_MARA, &retired_x, &retired_y) &&
        !find_tile(&g.map, TILE_NPC_DRAGON_SEEKER, &retired_x, &retired_y));
    ASSERT("their former Inn positions are open floor",
        map_is_walkable(&g.map, ROOK_TAVERN_X, ROOK_TAVERN_Y) && map_is_walkable(&g.map, MARA_TAVERN_X, MARA_TAVERN_Y) &&
        map_is_walkable(&g.map, ILYA_TAVERN_X, ILYA_TAVERN_Y));
    game_leave_tavern(&g);
    game_enter_inn(&g);
    g.player.x = BRENNA_INN_X;
    g.player.y = BRENNA_INN_Y + 1;
    game_talk_to_brenna(&g);
    game_handle_quest_offer_key(&g, SDL_SCANCODE_Y, 0);
    ASSERT("Brenna quest interaction works inside the Inn",
        g.frostfell_quest_state == 1);
    ASSERT("talking opens Brenna's dialogue bubble", g.dialogue_active);
    game_move_player(&g, 1, 0);
    ASSERT("moving dismisses the dialogue bubble", !g.dialogue_active);

    int exit_x = 0;
    int exit_y = 0;
    ASSERT("Inn has a south doorway",
        find_tile(&g.map, TILE_TAVERN_EXIT, &exit_x, &exit_y));
    g.player.x = exit_x;
    g.player.y = exit_y - 1;
    Action leave = {ACTION_MOVE, exit_x, exit_y};
    action_resolve_player(&g, leave);
    ASSERT("walking through the Inn doorway returns to town",
        g.location == LOCATION_TOWN2);
    ASSERT("Inn returns player outside its front door",
        g.player.x == TOWN_INN_DOOR_X && g.player.y == TOWN_INN_DOOR_Y + 1 &&
        map_is_walkable(&g.map, g.player.x, g.player.y));
    ASSERT("Inn transition preserves Brenna quest state",
        g.frostfell_quest_state == 1);
}

static void visit_dain(GameState *g) {
    game_enter_town_gate(g, LOCATION_TOWN, 0, 12);
    g->player.x = TOWN_TAVERN_DOOR_X;
    g->player.y = TOWN_TAVERN_DOOR_Y + 1;
    action_resolve_player(g, (Action){ACTION_MOVE, TOWN_TAVERN_DOOR_X, TOWN_TAVERN_DOOR_Y});
    g->player.x = DAIN_OAKHAVEN_X;
    g->player.y = DAIN_OAKHAVEN_Y + 1;
}

void test_dain_quest(void) {
    printf("Dain's two map halves tests:\n");
    GameState g = {0};
    game_init(&g);
    g.defeated_bosses = 1 << LOCATION_DUNGEON;
    game_enter_mountains(&g);
    for (int stage = 1; stage < 4; stage++) {
        game_descend(&g);
    }
    game_return_to_approach(&g);
    visit_dain(&g);
    game_talk_to_dain(&g);
    game_handle_quest_offer_key(&g, SDL_SCANCODE_Y, 0);
    ASSERT("Dain assigns the map retrieval without collecting a half", g.dain_quest_state == 1 && !g.dain_map_fragments);
    ASSERT("accepting the retrieval preserves the explored mountain expedition", g.mountain_cache[3].valid && g.max_mountain_level_reached == 4);
    game_enter_mountains(&g);
    for (int stage = 1; stage <= MOUNTAIN_DEPTH; stage++) {
        int bearers = 0;
        for (int i = 0; i < g.enemy_count; i++) {
            bearers += g.enemies[i].active && g.enemies[i].dain_fragment != 0;
        }
        ASSERT("only Mountains 1 and 2 contain pending map halves", bearers == (stage <= 2));
        game_refresh_quest_encounters(&g);
        int refreshed = 0;
        for (int i = 0; i < g.enemy_count; i++) {
            refreshed += g.enemies[i].active && g.enemies[i].dain_fragment != 0;
        }
        ASSERT("refreshing a stage never duplicates its map bearer", refreshed == bearers);
        if (stage < MOUNTAIN_DEPTH) {
            game_descend(&g);
        }
    }
    g.level = 1;
    game_record_dain_kill(&g, ENEMY_GOBLIN_ARCHER);
    ASSERT("the first bearer yields one half", g.dain_map_fragments == DAIN_FRAGMENT_ARCHER && g.dain_quest_state == 1);
    g.level = 3;
    game_record_dain_kill(&g, ENEMY_GOBLIN_SHAMAN);
    ASSERT("deeper enemies add no third map piece", g.dain_map_fragments == DAIN_FRAGMENT_ARCHER);
    g.level = 2;
    game_record_dain_kill(&g, ENEMY_GOBLIN_WARCHIEF);
    ASSERT("two halves finish retrieval without the Goblin King", g.dain_map_fragments == DAIN_MAP_COMPLETE && g.dain_quest_state == 2 &&
        !(g.defeated_bosses & (1 << LOCATION_MOUNTAINS)));
    int gold = g.gold;
    visit_dain(&g);
    game_talk_to_dain(&g);
    game_talk_to_dain(&g);
    ASSERT("Dain pays once and offers the Goblin King follow-up", g.gold == gold + 60 &&
        g.dain_quest_state == REGIONAL_CHAIN_BOSS_OFFERED && game_quest_offer_active(&g) && strstr(g.dialogue_text, "Goblin King"));
}

void test_dain_town_center(void) {
    printf("Dain at Oakhaven's Inn tests:\n");
    static GameState g;
    static GameState loaded;
    memset(&g, 0, sizeof(g));
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    game_enter_tavern(&g);
    g.player.x = DAIN_TOWN_X;
    g.player.y = DAIN_TOWN_Y + 1;
    game_talk_to_dain(&g);
    ASSERT("Dain cannot assign the quest at his former Tavern position", !g.dain_quest_state && !g.dialogue_active);
    game_leave_tavern(&g);
    game_enter_guild(&g);
    ASSERT("the Guild no longer houses Dain or Zara", g.map.tiles[GUILD_DAIN_Y][GUILD_DAIN_X] == TILE_TAVERN_FLOOR &&
        g.map.tiles[GUILD_ZARA_Y][GUILD_ZARA_X] == TILE_TAVERN_FLOOR);
    game_talk_to_dain(&g);
    ASSERT("Dain cannot assign his quest in the Guild", !g.dain_quest_state && !g.dialogue_active);
    game_enter_town_gate(&g, LOCATION_DAWNMERE, 0, 12);
    // Dismiss the royal checkpoint's arrival challenge.
    g.dialogue_active = 0;
    ASSERT("Dawnmere's former Dain tile is restored cobblestone", g.location == LOCATION_DAWNMERE &&
        g.map.tiles[DAIN_TOWN_Y][DAIN_TOWN_X] == TILE_TOWN_PATH && map_is_walkable(&g.map, DAIN_TOWN_X, DAIN_TOWN_Y));
    game_talk_to_dain(&g);
    ASSERT("Dain cannot assign his quest remotely in town", !g.dain_quest_state);
    g.player.x = DAIN_TOWN_X;
    g.player.y = DAIN_TOWN_Y + 1;
    game_talk_to_dain(&g);
    ASSERT("Dain cannot be talked to at his former plaza position", !g.dialogue_active && !g.dain_quest_state);
    g.defeated_bosses |= 1 << LOCATION_DUNGEON;
    visit_dain(&g);
    action_resolve_player(&g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("Dain requires conversation rather than Action", !g.dain_quest_state);
    g.tribute_quest_state = 1;
    game_talk_to_dain(&g);
    game_handle_quest_offer_key(&g, SDL_SCANCODE_Y, 0);
    ASSERT("Dain assigns the same mountain quest in Oakhaven and leaves Zara's quest intact", g.dain_quest_state == 1 &&
        !g.dain_map_fragments && g.tribute_quest_state == 1 &&
        g.dialogue_x == DAIN_OAKHAVEN_X && g.dialogue_y == DAIN_OAKHAVEN_Y &&
        strstr(g.dialogue_text, "Mountains 1 and 2") && strstr(g.dialogue_text, "Oakhaven's Inn"));
    g.dain_map_fragments = DAIN_FRAGMENT_ARCHER;
    QuestJournalEntry entry;
    ASSERT("the journal retains mountain objectives and rewards and names the new return location", quest_journal_get_entry(&g, QUEST_TAB_ACTIVE, 0, &entry) &&
        strcmp(entry.title, "Recover the Treasure Map") == 0 && entry.stages[0] == 1 && entry.stages[1] == 2 && entry.objective_count == 2 &&
        entry.reward_gold == 60 && entry.reward_score == 400 && entry.objective_complete[0] && !entry.objective_complete[1] && strstr(entry.summary_line_2, "Oakhaven's Inn"));
    ASSERT("saving in Oakhaven retains Dain and partial map progress", save_game(&g, 99141) && load_game(&loaded, 99141) &&
        loaded.location == LOCATION_TAVERN && loaded.dain_quest_state == 1 && loaded.dain_map_fragments == 1 && loaded.tribute_quest_state == 1 &&
        loaded.map.tiles[DAIN_OAKHAVEN_Y][DAIN_OAKHAVEN_X] == TILE_NPC_DAIN);
    g = loaded;
    int gold = g.gold;
    int score = g.score;
    game_talk_to_dain(&g);
    ASSERT("partial map progress cannot claim the reward", g.gold == gold && g.score == score && g.dain_quest_state == 1);
    ASSERT("Dain's progress reminder retains the complete return directions", strstr(g.dialogue_text, "Oakhaven's Inn.") &&
        strstr(g.dialogue_text, "1 of 2 map halves"));
    g.dain_quest_state = 2;
    g.dain_map_fragments = DAIN_MAP_COMPLETE;
    g.player.x = 20;
    g.player.y = TOWN_H - 2;
    game_talk_to_dain(&g);
    ASSERT("completed quest rewards cannot be claimed remotely", g.gold == gold && g.score == score && g.dain_quest_state == 2);
    g.player.x = DAIN_OAKHAVEN_X;
    g.player.y = DAIN_OAKHAVEN_Y + 1;
    game_talk_to_dain(&g);
    game_talk_to_dain(&g);
    ASSERT("Dain awards the original 60 gold and 400 score once in Oakhaven", g.dain_quest_state == 3 && g.gold == gold + 60 && g.score == score + 400);
    ASSERT("completed quest survives saving and loading in Oakhaven", save_game(&g, 99141) && load_game(&loaded, 99141) &&
        loaded.dain_quest_state == 3 && loaded.dain_map_fragments == DAIN_MAP_COMPLETE && loaded.gold == gold + 60 && loaded.score == score + 400);
    remove("saves/savegame_99141.json");
}

void test_alder_quest(void) {
    printf("Alder's two ranger rescues tests:\n");
    GameState g = {0};
    game_init(&g);
    g.defeated_bosses = 1 << LOCATION_DUNGEON;
    game_enter_tavern(&g);
    g.player.x = ALDER_OAKHAVEN_X;
    g.player.y = ALDER_OAKHAVEN_Y + 1;
    game_talk_to_alder(&g);
    game_handle_quest_offer_key(&g, SDL_SCANCODE_Y, 0);
    ASSERT("Alder assigns The Lost Rangers", g.alder_quest_state == 1 && !g.alder_wardens_rescued);
    game_leave_tavern(&g);
    game_enter_forest(&g);
    for (int stage = 1; stage <= FOREST_DEPTH; stage++) {
        int x = 0;
        int y = 0;
        int ranger = find_tile(&g.map, TILE_FOREST_WARDEN, &x, &y);
        ASSERT("only Forest 1 and 2 contain the two rangers", ranger == (stage <= 2));
        if (ranger) {
            int captors = 0;
            for (int i = 0; i < g.enemy_count; i++) {
                if (g.enemies[i].active && strcmp(g.enemies[i].name, ALDER_CAPTOR_NAME) == 0) {
                    captors++;
                    ASSERT("each ranger's captors are Giant Spiders", g.enemies[i].type == ENEMY_GIANT_SPIDER);
                }
            }
            int progress = g.alder_wardens_rescued;
            game_rescue_forest_warden(&g, x, y);
            ASSERT("living spider captors prevent the rescue", captors == 3 && g.alder_wardens_rescued == progress);
            for (int i = 0; i < g.enemy_count; i++) {
                if (strcmp(g.enemies[i].name, ALDER_CAPTOR_NAME) == 0) {
                    g.enemies[i].active = 0;
                }
            }
            game_rescue_forest_warden(&g, x, y);
            ASSERT("freeing the ranger removes the captive tile", g.map.tiles[y][x] == TILE_FOREST_FLOOR &&
                strcmp(g.dialogue_speaker, "Forest Ranger") == 0);
        }
        if (stage == 2) {
            ASSERT("both rescues complete before fighting the Necromancer", g.alder_quest_state == 2 &&
                g.alder_wardens_rescued == ALDER_RESCUE_COMPLETE && !(g.defeated_bosses & (1 << LOCATION_FOREST)));
        }
        if (stage < FOREST_DEPTH) {
            game_descend(&g);
        }
    }
    game_return_to_town(&g);
    game_enter_tavern(&g);
    g.player.x = ALDER_OAKHAVEN_X;
    g.player.y = ALDER_OAKHAVEN_Y + 1;
    int gold = g.gold;
    int score = g.score;
    game_talk_to_alder(&g);
    game_talk_to_alder(&g);
    ASSERT("Alder pays once and offers the Necromancer follow-up", g.alder_quest_state == REGIONAL_CHAIN_BOSS_OFFERED &&
        g.gold == gold + 70 && g.score == score + 500 && game_quest_offer_active(&g));
    game_leave_tavern(&g);
    game_enter_forest(&g);
    int x = 0;
    int y = 0;
    ASSERT("rescued rangers never respawn", !find_tile(&g.map, TILE_FOREST_WARDEN, &x, &y));
}

static void test_quest_capacity(GameState *g, GameState *loaded) {
    struct {
        const char *speaker;
        Location location;
        int *state;
    } offers[] = {
        {"Bram", LOCATION_TAVERN, &g->elowen_quest_state},
        {"Dain", LOCATION_TAVERN, &g->dain_quest_state},
        {"Alder", LOCATION_TAVERN, &g->alder_quest_state},
        {"Mara", LOCATION_TAVERN, &g->mara_quest_state},
        {"Rook", LOCATION_TAVERN, &g->rook_quest_state},
        {"Zara", LOCATION_TAVERN, &g->tribute_quest_state},
        {"Ilya", LOCATION_TAVERN, &g->dragon_treasure_quest_state},
        {"Nahla", LOCATION_ISLAND, &g->temple_treasure_state},
        {"Steward Hadrin", LOCATION_TOWN_HALL, &g->emberforge_quest_state},
        {"Quartermaster Brenna", LOCATION_INN, &g->frostfell_quest_state},
        {"Surveyor Orin", LOCATION_GUILD, &g->glassdeep_quest_state},
        {"Brother Oswin", LOCATION_TOWN_HALL, &g->catacombs_quest_state},
        {"Marshal Veyra", LOCATION_TOWN_HALL, &g->watchfire_quest_state},
        {"Huntmaster Selene", LOCATION_GUILD, &g->hunt_quest_state},
        {"Forester Tamsin", LOCATION_TOWN, &g->heartseed_quest_state},
        {"Spice Merchant Corvin", LOCATION_DAWNMERE_MERCHANT, &g->spice_skiff_quest_state},
        {"Watchkeeper Seren", LOCATION_ALEHOUSE, &g->dawnmere_quest_state},
        {"Watchkeeper Seren", LOCATION_DAWNMERE, &g->saltwind_quest_state},
        {WAR_SPEAKER, LOCATION_TOWN_HALL, &g->war_machine_quest_state},
        {"Historian Liora", LOCATION_APPROACH, &g->crown_quest_state}
    };
    size_t count = sizeof(offers) / sizeof(offers[0]);
    for (size_t i = 0; i < count; i++) {
        game_init(g);
        g->defeated_bosses = 1 << LOCATION_DUNGEON;
        *offers[i].state = 1;
        ASSERT("each optional quest uses one active slot", game_active_quest_count(g) == 1);
        *offers[i].state = 2;
        ASSERT("each finished quest moves to Completed and releases its slot", game_active_quest_count(g) == 0 && quest_journal_count(g, QUEST_TAB_COMPLETED) == 1);
        *offers[i].state = 3;
        ASSERT("rewarded quests do not use active slots", game_active_quest_count(g) == 0);
        *offers[i].state = 0;
        for (size_t j = 1; j <= MAX_ACTIVE_QUESTS; j++) {
            *offers[(i + j) % count].state = 1;
        }
        g->location = offers[i].location;
        g->dialogue_active = 1;
        snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "%s", offers[i].speaker);
        g->level_cache[3].valid = 1;
        g->max_level_reached = 4;
        int gold = g->gold;
        ASSERT("every optional offer enforces the three-quest limit", game_quest_offer_at_limit(g) &&
            game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0) && !*offers[i].state &&
            game_active_quest_count(g) == MAX_ACTIVE_QUESTS);
        ASSERT("a rejected offer preserves the offer, expedition and rewards", game_quest_offer_active(g) &&
            g->level_cache[3].valid && g->max_level_reached == 4 && g->gold == gold);
        *offers[(i + 1) % count].state = 2;
        ASSERT("the third active quest is allowed once another quest's objectives finish", !game_quest_offer_at_limit(g) &&
            game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0) && *offers[i].state == 1 &&
            game_active_quest_count(g) == MAX_ACTIVE_QUESTS);
    }

    game_init(g);
    game_enter_tavern(g);
    g->elowen_quest_state = 1;
    g->dain_quest_state = 1;
    g->alder_quest_state = 1;
    g->mara_quest_state = 1;
    g->player.x = ROOK_TAVERN_X;
    g->player.y = ROOK_TAVERN_Y + 1;
    game_talk_to_rook(g);
    ASSERT("saves with more than three active quests keep all progress and the pending offer", save_game(g, 99150) &&
        load_game(loaded, 99150) && game_active_quest_count(loaded) == 4 &&
        quest_journal_count(loaded, QUEST_TAB_ACTIVE) == 4 && game_quest_offer_at_limit(loaded));
    game_handle_quest_offer_key(loaded, SDL_SCANCODE_Y, 0);
    ASSERT("loading cannot bypass the limit or discard existing quests", !loaded->rook_quest_state && game_active_quest_count(loaded) == 4);
    game_handle_quest_offer_key(loaded, SDL_SCANCODE_ESCAPE, 0);
    ASSERT("a full-journal offer can still be dismissed", !loaded->dialogue_active);
    loaded->elowen_quest_state = 2;
    loaded->elowen_seals_restored = 7;
    loaded->player.x = BRAM_INN_X;
    loaded->player.y = BRAM_INN_Y + 1;
    int gold = loaded->gold;
    game_talk_to_bram(loaded);
    ASSERT("rewards can be claimed with all three slots occupied", loaded->elowen_quest_state == 3 &&
        loaded->gold == gold + 80 && game_active_quest_count(loaded) == 3);
    loaded->dain_quest_state = 2;
    loaded->dain_map_fragments = 7;
    loaded->player.x = ROOK_TAVERN_X;
    loaded->player.y = ROOK_TAVERN_Y + 1;
    game_talk_to_rook(loaded);
    game_handle_quest_offer_key(loaded, SDL_SCANCODE_Y, 0);
    ASSERT("an old over-limit character can accept again after making room", loaded->rook_quest_state == 1 && game_active_quest_count(loaded) == 3);
    ASSERT("completed quests with pending and collected rewards survive a save round trip", save_game(loaded, 99150) && load_game(g, 99150) &&
        game_active_quest_count(g) == 3 && quest_journal_count(g, QUEST_TAB_COMPLETED) == 2 &&
        g->dain_quest_state == 2 && g->elowen_quest_state == 3);

    g->location = LOCATION_CASTLE;
    map_generate_castle(&g->map, &g->player.x, &g->player.y);
    g->player.x = INFORMANT_X;
    g->player.y = INFORMANT_Y + 1;
    jail_talk_nearby(g);
    ASSERT("arrest creates equipment recovery without taking a quest slot", g->jail_gear_state == 1 && game_active_quest_count(g) == 3);
    g->player.x = JAIL_PRISONER_X;
    g->player.y = JAIL_PRISONER_Y + 1;
    jail_talk_nearby(g);
    ASSERT("Tomas can offer an escape at the limit", game_quest_offer_active(g) && !game_quest_offer_at_limit(g));
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("escape acceptance opens the hatch and preserves all optional commitments", g->jail_quest_state == 2 &&
        g->map.tiles[JAIL_HATCH_Y][JAIL_HATCH_X] == TILE_JAIL_HATCH && game_active_quest_count(g) == 3 &&
        quest_journal_count(g, QUEST_TAB_ACTIVE) == 5);
}

void test_quest_offer_controls(void) {
    printf("Quest offer controls tests:\n");
    static GameState g;
    static GameState loaded;
    struct {
        void (*talk)(GameState *);
        Location location;
        int x;
        int y;
        int *state;
    } cases[] = {
        {game_talk_to_bram, LOCATION_TAVERN, BRAM_INN_X, BRAM_INN_Y, &g.elowen_quest_state},
        {game_talk_to_dain, LOCATION_TAVERN, DAIN_OAKHAVEN_X, DAIN_OAKHAVEN_Y, &g.dain_quest_state},
        {game_talk_to_alder, LOCATION_TAVERN, ALDER_OAKHAVEN_X, ALDER_OAKHAVEN_Y, &g.alder_quest_state},
        {game_talk_to_mara, LOCATION_TAVERN, MARA_TAVERN_X, MARA_TAVERN_Y, &g.mara_quest_state},
        {game_talk_to_rook, LOCATION_TAVERN, ROOK_TAVERN_X, ROOK_TAVERN_Y, &g.rook_quest_state},
        {game_talk_to_guild_seeker, LOCATION_TAVERN, ZARA_OAKHAVEN_X, ZARA_OAKHAVEN_Y, &g.tribute_quest_state},
        {game_talk_to_dragon_seeker, LOCATION_TAVERN, ILYA_TAVERN_X, ILYA_TAVERN_Y, &g.dragon_treasure_quest_state},
        {game_talk_to_nahla, LOCATION_ISLAND, ISLAND_NAHLA_X, ISLAND_NAHLA_Y, &g.temple_treasure_state},
        {game_talk_to_steward, LOCATION_TOWN_HALL, HALL_STEWARD_X, HALL_STEWARD_Y, &g.emberforge_quest_state},
        {game_talk_to_brenna, LOCATION_INN, BRENNA_INN_X, BRENNA_INN_Y, &g.frostfell_quest_state},
        {game_talk_to_orin, LOCATION_GUILD, GUILD_ORIN_X, GUILD_ORIN_Y, &g.glassdeep_quest_state},
        {game_talk_to_oswin, LOCATION_TOWN_HALL, HALL_OSWIN_X, HALL_OSWIN_Y, &g.catacombs_quest_state}
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        g.player.player_class = CLASS_WARRIOR;
        game_init(&g);
        g.defeated_bosses = 1 << LOCATION_DUNGEON;
        g.location = cases[i].location;
        g.player.x = cases[i].x;
        g.player.y = cases[i].y + 1;
        if (cases[i].state == &g.tribute_quest_state) {
            g.map.tiles[ZARA_OAKHAVEN_Y][ZARA_OAKHAVEN_X] = TILE_NPC_GUILD_SEEKER;
        }
        int messages = g.message_count;
        cases[i].talk(&g);
        ASSERT("talking offers a quest without activating it or logging an assignment", game_quest_offer_active(&g) &&
            *cases[i].state == 0 && !quest_journal_count(&g, QUEST_TAB_ACTIVE) && g.message_count == messages);
        ASSERT("holding Y or pressing Enter does not accept an offer", game_handle_quest_offer_key(&g, SDL_SCANCODE_Y, 1) &&
            game_handle_quest_offer_key(&g, SDL_SCANCODE_RETURN, 0) && *cases[i].state == 0);
        ASSERT("N declines without adding a quest to the journal", game_handle_quest_offer_key(&g, SDL_SCANCODE_N, 0) &&
            *cases[i].state == 0 && !g.dialogue_active && !quest_journal_count(&g, QUEST_TAB_ACTIVE));
        cases[i].talk(&g);
        ASSERT("a declined quest can be offered again and accepted with Y", game_quest_offer_active(&g) &&
            game_handle_quest_offer_key(&g, SDL_SCANCODE_Y, 0) && *cases[i].state == 1 &&
            !game_quest_offer_active(&g) && quest_journal_count(&g, QUEST_TAB_ACTIVE) == 1);
        int gold = g.gold;
        int score = g.score;
        ASSERT("Y after acceptance cannot accept twice or award a reward", !game_handle_quest_offer_key(&g, SDL_SCANCODE_Y, 0) &&
            g.gold == gold && g.score == score && *cases[i].state == 1);
    }

    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    game_enter_tavern(&g);
    g.player.x = BRAM_INN_X;
    g.player.y = BRAM_INN_Y + 1;
    g.level_cache[3].valid = 1;
    g.level_cache[3].level_cleared = 1;
    g.max_level_reached = 4;
    g.portal_active = 1;
    g.portal_location = LOCATION_DUNGEON;
    game_talk_to_bram(&g);
    ASSERT("an undecided offer and existing expedition survive save/load", save_game(&g, 99150) && load_game(&loaded, 99150) &&
        game_quest_offer_active(&loaded) && !loaded.elowen_quest_state && loaded.level_cache[3].valid && !loaded.portal_active);
    game_handle_quest_offer_key(&loaded, SDL_SCANCODE_ESCAPE, 0);
    ASSERT("Esc declines without resetting maps, depth, or portals", !loaded.dialogue_active && !loaded.elowen_quest_state &&
        loaded.level_cache[3].valid && loaded.level_cache[3].level_cleared && loaded.max_level_reached == 4 && !loaded.portal_active);
    ASSERT("a declined quest remains inactive after saving", save_game(&loaded, 99150) && load_game(&g, 99150) &&
        !g.elowen_quest_state && !game_quest_offer_active(&g) && !quest_journal_count(&g, QUEST_TAB_ACTIVE));
    game_talk_to_bram(&g);
    game_handle_quest_offer_key(&g, SDL_SCANCODE_Y, 0);
    ASSERT("accepting the rescue preserves the existing dungeon expedition", g.elowen_quest_state == 1 && g.level_cache[3].valid &&
        !g.portal_active && g.max_level_reached == 4);
    ASSERT("accepted quests stay active after save/load without another offer", save_game(&g, 99150) && load_game(&loaded, 99150) &&
        loaded.elowen_quest_state == 1 && !game_quest_offer_active(&loaded) && quest_journal_count(&loaded, QUEST_TAB_ACTIVE) == 1);
    test_quest_capacity(&g, &loaded);
    remove("saves/savegame_99150.json");
}
