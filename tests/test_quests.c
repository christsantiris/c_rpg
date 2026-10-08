#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/systems/save_load.h"
#include "../src/screens/quest_journal.h"
#include <string.h>

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

static int count_enemy_type(const GameState *g, EnemyType type) {
    int count = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].type == type) {
            count++;
        }
    }
    return count;
}

static int count_active_enemies(const GameState *g) {
    int count = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        count += g->enemies[i].active;
    }
    return count;
}

static void lower_coast_tide(GameState *g) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g->map.tiles[y][x] == TILE_COAST_TIDE_CONTROL) {
                g->player.x = x;
                g->player.y = y;
                Action activate = {ACTION_INTERACT, 0, 0};
                action_resolve_player(g, activate);
                return;
            }
        }
    }
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
    ASSERT("Elowen's seals require quest activation",
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
    printf("Elowen quest tests:\n");
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);

    g.level_cache[3].valid = 1;
    g.portal_active = 1;
    g.portal_location = LOCATION_DUNGEON;
    game_enter_tavern(&g);
    g.player.x = ELOWEN_TAVERN_X;
    g.player.y = ELOWEN_TAVERN_Y + 1;
    game_talk_to_elowen(&g);
    ASSERT("Elowen offers The Broken Seals", g.elowen_quest_state == 1);
    ASSERT("Elowen speaks through dialogue state", g.dialogue_active &&
        strstr(g.dialogue_text, "burial seals") != NULL);
    ASSERT("event log records only the quest assignment",
        strcmp(g.messages[g.message_count - 1],
            "Quest assigned: The Broken Seals.") == 0);
    ASSERT("new quest begins with no restored seals",
        g.elowen_seals_restored == 0);
    ASSERT("accepting Elowen's quest starts a fresh dungeon expedition",
        !g.level_cache[3].valid && !g.portal_active);

    g.location = LOCATION_DUNGEON;
    g.level = 2;
    map_generate(&g.map, g.level);
    g.enemy_count = 0;
    game_refresh_quest_encounters(&g);
    int encounter_x = 0;
    int encounter_y = 0;
    ASSERT("a cleared dungeon floor receives a guarded seal encounter",
        find_tile(&g.map, TILE_BROKEN_BURIAL_SEAL, &encounter_x,
            &encounter_y) && count_active_enemies(&g) >= 3);

    game_enter_dungeon(&g);
    ASSERT("new dungeon expedition starts on floor one", g.level == 1);
    int expected_levels[3] = {2, 3, 4};
    for (int seal = 0; seal < 3; seal++) {
        while (g.level < expected_levels[seal]) {
            game_descend(&g);
        }
        int seal_x = 0;
        int seal_y = 0;
        ASSERT("quest floor contains a broken burial seal",
            find_tile(&g.map, TILE_BROKEN_BURIAL_SEAL, &seal_x, &seal_y));
        g.player.x = seal_x;
        g.player.y = seal_y;
        Action restore = {ACTION_INTERACT, 0, 0};
        action_resolve_player(&g, restore);
        ASSERT("A restores the burial seal",
            g.map.tiles[seal_y][seal_x] == TILE_RESTORED_BURIAL_SEAL);
    }
    ASSERT("three restored seals make the quest ready",
        g.elowen_quest_state == 2 && g.elowen_seals_restored == 7);

    game_return_to_town(&g);
    int gold_before = g.gold;
    game_enter_tavern(&g);
    g.player.x = ELOWEN_TAVERN_X;
    g.player.y = ELOWEN_TAVERN_Y + 1;
    game_talk_to_elowen(&g);
    ASSERT("Elowen completes the quest", g.elowen_quest_state == 3);
    ASSERT("Elowen awards 40 gold", g.gold == gold_before + 40);

    game_enter_dungeon(&g);
    ASSERT("replayed dungeon begins again on floor one", g.level == 1);
    int living_enemies = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        living_enemies += g.enemies[i].active;
    }
    ASSERT("new expedition repopulates regular enemies", living_enemies > 0);

    g.defeated_bosses |= 1 << LOCATION_DUNGEON;
    g.level = DUNGEON_DEPTH;
    map_generate(&g.map, g.level);
    enemies_spawn(&g);
    int liches = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        liches += g.enemies[i].type == ENEMY_LICH_KING;
    }
    ASSERT("defeated Lich King does not respawn", liches == 0);
    game_return_to_town(&g);
    game_enter_dungeon(&g);
    while (g.level < DUNGEON_DEPTH) {
        game_descend(&g);
    }
    ASSERT("replayed finale keeps its return passage open",
        g.map.tiles[g.map.stairs_down_y][g.map.stairs_down_x] ==
            TILE_RETURN_EXIT);
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
    ASSERT("Elowen occupies her original Tavern position",
        find_tile(&g.map, TILE_NPC_ELOWEN, &elowen_x, &elowen_y) &&
        elowen_x == ELOWEN_TAVERN_X && elowen_y == ELOWEN_TAVERN_Y);
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
    ASSERT("Alder's old Tavern position is walkable",
        map_is_walkable(&g.map, 28, 7));
    int mara_x = 0;
    int mara_y = 0;
    ASSERT("Mara no longer occupies Oakhaven's Tavern",
        !find_tile(&g.map, TILE_NPC_MARA, &mara_x, &mara_y));
    ASSERT("Mara's old Tavern position is walkable",
        map_is_walkable(&g.map, 31, 18));
    g.player.x = BRENNA_X;
    g.player.y = BRENNA_Y + 1;
    game_talk_to_brenna(&g);
    ASSERT("Brenna quest interaction works inside the Tavern",
        g.frostfell_quest_state == 1);
    ASSERT("talking opens Brenna's dialogue bubble", g.dialogue_active);
    game_move_player(&g, 1, 0);
    ASSERT("moving dismisses the dialogue bubble", !g.dialogue_active);

    int exit_x = 0;
    int exit_y = 0;
    ASSERT("Tavern has a south doorway",
        find_tile(&g.map, TILE_TAVERN_EXIT, &exit_x, &exit_y));
    g.player.x = exit_x;
    g.player.y = exit_y - 1;
    Action leave = {ACTION_MOVE, exit_x, exit_y};
    action_resolve_player(&g, leave);
    ASSERT("walking through the Tavern doorway returns to town",
        g.location == LOCATION_TOWN);
    ASSERT("Tavern returns player outside its front door",
        g.player.x == TOWN_TAVERN_DOOR_X && g.player.y == TOWN_TAVERN_DOOR_Y + 1 &&
        map_is_walkable(&g.map, g.player.x, g.player.y));
    ASSERT("Tavern transition preserves Brenna quest state",
        g.frostfell_quest_state == 1);
}

void test_dain_quest(void) {
    printf("Dain quest tests:\n");
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);

    g.mountain_cache[3].valid = 1;
    g.portal_active = 1;
    g.portal_location = LOCATION_MOUNTAINS;
    game_enter_guild(&g);
    g.player.x = GUILD_DAIN_X;
    g.player.y = GUILD_DAIN_Y + 1;
    game_talk_to_dain(&g);
    ASSERT("Dain assigns Recover the Treasure Map", g.dain_quest_state == 1);
    ASSERT("new Dain quest begins with no map fragments",
        g.dain_map_fragments == 0);
    ASSERT("Dain speaks through the dialogue bubble", g.dialogue_active &&
        strcmp(g.dialogue_speaker, "Dain") == 0);
    ASSERT("accepting Dain's quest starts a fresh mountain expedition",
        !g.mountain_cache[3].valid && !g.portal_active);

    int target_levels[3] = {1, 2, 5};
    EnemyType target_types[3] = {
        ENEMY_GOBLIN_ARCHER, ENEMY_GOBLIN_BOMBER, ENEMY_GOBLIN_SHAMAN
    };
    g.location = LOCATION_MOUNTAINS;
    for (int target = 0; target < 3; target++) {
        g.level = target_levels[target];
        map_generate_mountains(&g.map, g.level);
        enemies_spawn(&g);
        ASSERT("needed mountain specialist has a guaranteed encounter",
            count_enemy_type(&g, target_types[target]) > 0);
        int marked_target = 0;
        for (int i = 0; i < g.enemy_count; i++) {
            if (g.enemies[i].type == target_types[target] &&
                g.enemies[i].dain_fragment != 0) {
                marked_target = 1;
            }
        }
        ASSERT("map bearer is distinct from ordinary enemies",
            marked_target);
    }

    g.level = 1;
    map_generate_mountains(&g.map, g.level);
    g.enemy_count = 0;
    LevelCache *cached = &g.mountain_cache[1];
    map_generate_mountains(&cached->map, 2);
    cached->enemy_count = 0;
    cached->level_cleared = 1;
    cached->valid = 1;
    game_descend(&g);
    int cached_bearers = 0;
    int cached_enemies = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        if (g.enemies[i].active &&
            g.enemies[i].dain_fragment == DAIN_FRAGMENT_BOMBER) {
            cached_bearers++;
        }
        cached_enemies += g.enemies[i].active;
    }
    ASSERT("backtracking adds a guarded bearer to a cleared cached stage",
        g.level == 2 && cached_bearers == 1 && cached_enemies >= 3);
    game_refresh_quest_encounters(&g);
    int refreshed_bearers = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        if (g.enemies[i].active &&
            g.enemies[i].dain_fragment == DAIN_FRAGMENT_BOMBER) {
            refreshed_bearers++;
        }
    }
    ASSERT("refreshing a cached stage does not duplicate its bearer",
        refreshed_bearers == 1);

    game_record_dain_kill(&g, ENEMY_GOBLIN_ARCHER);
    ASSERT("Archer defeat recovers its map fragment",
        g.dain_map_fragments == DAIN_FRAGMENT_ARCHER);
    game_record_dain_kill(&g, ENEMY_GOBLIN_BOMBER);
    ASSERT("Bomber defeat recovers its map fragment",
        g.dain_map_fragments ==
            (DAIN_FRAGMENT_ARCHER | DAIN_FRAGMENT_BOMBER));
    game_record_dain_kill(&g, ENEMY_GOBLIN_SHAMAN);
    ASSERT("three map fragments make Dain's quest ready",
        g.dain_quest_state == 2 && g.dain_map_fragments == 7);

    int gold_before = g.gold;
    game_enter_guild(&g);
    g.player.x = GUILD_DAIN_X;
    g.player.y = GUILD_DAIN_Y + 1;
    game_talk_to_dain(&g);
    ASSERT("Dain completes the mountain quest", g.dain_quest_state == 3);
    ASSERT("Dain awards 60 gold", g.gold == gold_before + 60);
}

void test_dain_guild(void) {
    printf("Dain in Rosemoor's Adventurer's Guild tests:\n");
    static GameState g;
    static GameState loaded;
    memset(&g, 0, sizeof(g));
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    game_enter_tavern(&g);
    g.player.x = GUILD_DAIN_X;
    g.player.y = GUILD_DAIN_Y + 1;
    game_talk_to_dain(&g);
    ASSERT("Dain cannot assign the quest at his former Tavern position", !g.dain_quest_state && !g.dialogue_active);
    game_leave_tavern(&g);
    game_enter_town3(&g);
    g.player.x = TOWN_GUILD_DOOR_X;
    g.player.y = TOWN_GUILD_DOOR_Y + 1;
    action_resolve_player(&g, (Action){ACTION_MOVE, TOWN_GUILD_DOOR_X, TOWN_GUILD_DOOR_Y});
    ASSERT("Rosemoor's Guild houses Dain and Zara in separate reachable positions", g.location == LOCATION_GUILD &&
        g.map.tiles[GUILD_DAIN_Y][GUILD_DAIN_X] == TILE_NPC_DAIN &&
        g.map.tiles[GUILD_ZARA_Y][GUILD_ZARA_X] == TILE_NPC_GUILD_SEEKER &&
        !map_is_walkable(&g.map, GUILD_DAIN_X, GUILD_DAIN_Y) && map_is_walkable(&g.map, GUILD_DAIN_X, GUILD_DAIN_Y + 1));
    game_talk_to_dain(&g);
    ASSERT("Dain cannot assign his quest remotely in the Guild", !g.dain_quest_state);
    g.player.x = GUILD_DAIN_X;
    g.player.y = GUILD_DAIN_Y + 1;
    action_resolve_player(&g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("Dain requires conversation rather than Action", !g.dain_quest_state);
    g.sunscar_lamp_quest_state = 1;
    game_talk_to_dain(&g);
    ASSERT("Dain assigns the same mountain quest from the Guild and leaves Zara's quest intact", g.dain_quest_state == 1 &&
        !g.dain_map_fragments && g.sunscar_lamp_quest_state == 1 &&
        g.dialogue_x == GUILD_DAIN_X && g.dialogue_y == GUILD_DAIN_Y &&
        strstr(g.dialogue_text, "stages 1, 2, and 5") && strstr(g.dialogue_text, "Rosemoor's Adventurer's Guild"));
    g.dain_map_fragments = DAIN_FRAGMENT_ARCHER | DAIN_FRAGMENT_SHAMAN;
    QuestJournalEntry entry;
    ASSERT("the journal retains mountain objectives and rewards and names the new return location", quest_journal_get_entry(&g, QUEST_TAB_ACTIVE, 0, &entry) &&
        strcmp(entry.title, "Recover the Treasure Map") == 0 && entry.stages[0] == 1 && entry.stages[1] == 2 && entry.stages[2] == 5 &&
        entry.reward_gold == 60 && entry.reward_score == 400 && entry.objective_complete[0] && !entry.objective_complete[1] &&
        entry.objective_complete[2] && strstr(entry.summary_line_2, "Rosemoor's Adventurer's Guild"));
    ASSERT("saving in the Guild retains Dain, Zara, and partial map progress", save_game(&g, 99141) && load_game(&loaded, 99141) &&
        loaded.location == LOCATION_GUILD && loaded.dain_quest_state == 1 && loaded.dain_map_fragments == 5 && loaded.sunscar_lamp_quest_state == 1 &&
        loaded.map.tiles[GUILD_DAIN_Y][GUILD_DAIN_X] == TILE_NPC_DAIN &&
        loaded.map.tiles[GUILD_ZARA_Y][GUILD_ZARA_X] == TILE_NPC_GUILD_SEEKER);
    g = loaded;
    int gold = g.gold;
    int score = g.score;
    game_talk_to_dain(&g);
    ASSERT("partial map progress cannot claim the reward", g.gold == gold && g.score == score && g.dain_quest_state == 1);
    g.dain_quest_state = 2;
    g.dain_map_fragments = 7;
    g.player.x = g.map.stairs_up_x;
    g.player.y = g.map.stairs_up_y;
    game_talk_to_dain(&g);
    ASSERT("completed quest rewards cannot be claimed remotely", g.gold == gold && g.score == score && g.dain_quest_state == 2);
    g.player.x = GUILD_DAIN_X;
    g.player.y = GUILD_DAIN_Y + 1;
    game_talk_to_dain(&g);
    game_talk_to_dain(&g);
    ASSERT("Dain awards the original 60 gold and 400 score once in the Guild", g.dain_quest_state == 3 && g.gold == gold + 60 && g.score == score + 400);
    ASSERT("completed quest survives saving and loading in the Guild", save_game(&g, 99141) && load_game(&loaded, 99141) &&
        loaded.dain_quest_state == 3 && loaded.dain_map_fragments == 7 && loaded.gold == gold + 60 && loaded.score == score + 400);
    g = loaded;
    g.player.x = g.map.stairs_down_x;
    g.player.y = g.map.stairs_down_y - 1;
    action_resolve_player(&g, (Action){ACTION_MOVE, g.map.stairs_down_x, g.map.stairs_down_y});
    ASSERT("the Guild doorway returns to Rosemoor with quest completion intact", g.location == LOCATION_TOWN3 && g.dain_quest_state == 3 &&
        g.player.x == TOWN_GUILD_DOOR_X && g.player.y == TOWN_GUILD_DOOR_Y + 1);
    remove("saves/savegame_99141.json");
}

void test_alder_quest(void) {
    printf("Alder quest tests:\n");
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    game_enter_inn(&g);
    g.player.x = ALDER_INN_X;
    g.player.y = ALDER_INN_Y + 1;

    g.forest_cache[3].valid = 1;
    g.portal_active = 1;
    g.portal_location = LOCATION_FOREST;
    game_talk_to_alder(&g);
    ASSERT("Alder assigns The Lost Wardens", g.alder_quest_state == 1);
    ASSERT("new Alder quest begins with no rescues",
        g.alder_wardens_rescued == 0);
    ASSERT("Alder speaks through dialogue state", g.dialogue_active &&
        strcmp(g.dialogue_speaker, "Alder") == 0 && strstr(g.dialogue_text, "stages 1 and 2") && strstr(g.dialogue_text, "stage 5"));
    ASSERT("accepting Alder's quest starts a fresh forest expedition",
        !g.forest_cache[3].valid && !g.portal_active);
    game_leave_inn(&g);
    game_return_to_town(&g);

    int target_levels[3] = {1, 2, 5};
    EnemyType guardian_types[3] = {
        ENEMY_GIANT_SPIDER, ENEMY_DARK_ELF, ENEMY_FOREST_TROLL
    };
    game_enter_forest(&g);
    for (int target = 0; target < 3; target++) {
        while (g.level < target_levels[target]) {
            game_descend(&g);
        }
        int warden_x = 0;
        int warden_y = 0;
        ASSERT("missing warden appears in the assigned forest stage",
            find_tile(&g.map, TILE_FOREST_WARDEN, &warden_x, &warden_y));
        ASSERT("forest warden has a thematic guardian",
            count_enemy_type(&g, guardian_types[target]) > 0);
        game_rescue_forest_warden(&g, warden_x, warden_y);
        ASSERT("rescued warden leaves forest floor behind",
            g.map.tiles[warden_y][warden_x] == TILE_FOREST_FLOOR);
    }
    ASSERT("three rescues make Alder's quest ready",
        g.alder_quest_state == 2 && g.alder_wardens_rescued == 7);

    game_return_to_town(&g);
    game_enter_inn(&g);
    g.player.x = ALDER_INN_X;
    g.player.y = ALDER_INN_Y + 1;
    int gold_before = g.gold;
    int score_before = g.score;
    game_talk_to_alder(&g);
    ASSERT("Alder completes the forest quest", g.alder_quest_state == 3);
    ASSERT("Alder awards 70 gold", g.gold == gold_before + 70);
    ASSERT("Alder awards 500 score", g.score == score_before + 500);
    game_leave_inn(&g);
    game_return_to_town(&g);

    game_enter_forest(&g);
    game_descend(&g);
    int warden_x = 0;
    int warden_y = 0;
    ASSERT("rescued wardens do not respawn",
        !find_tile(&g.map, TILE_FOREST_WARDEN, &warden_x, &warden_y));
}

void test_mara_quest(void) {
    printf("Mara quest tests:\n");
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    game_enter_town_hall(&g);
    g.player.x = HALL_MARA_X;
    g.player.y = HALL_MARA_Y + 1;

    g.coast_cache[3].valid = 1;
    g.portal_active = 1;
    g.portal_location = LOCATION_COAST;
    game_talk_to_mara(&g);
    ASSERT("Mara assigns Relight the Drowned Beacons",
        g.mara_quest_state == 1);
    ASSERT("new Mara quest begins with no lit beacons",
        g.mara_beacons_lit == 0);
    ASSERT("Mara speaks through dialogue state", g.dialogue_active &&
        strcmp(g.dialogue_speaker, "Mara") == 0);
    ASSERT("accepting Mara's quest starts a fresh coast expedition",
        !g.coast_cache[3].valid && !g.portal_active);
    game_leave_town_hall(&g);

    int target_levels[3] = {2, 3, 4};
    EnemyType guardian_types[3] = {
        ENEMY_GIANT_CRAB, ENEMY_ANIMATED_STATUE, ENEMY_SEA_SERPENT
    };
    game_enter_coast(&g);
    int absent_x = 0;
    int absent_y = 0;
    ASSERT("stage one is an approach without a quest beacon",
        !find_tile(&g.map, TILE_COAST_BEACON_UNLIT, &absent_x, &absent_y));
    for (int target = 0; target < 3; target++) {
        while (g.level < target_levels[target]) {
            game_descend(&g);
        }
        int beacon_x = 0;
        int beacon_y = 0;
        ASSERT("unlit beacon appears on its assigned coast stage",
            find_tile(&g.map, TILE_COAST_BEACON_UNLIT, &beacon_x,
                &beacon_y));
        ASSERT("beacon stage has its planned guardian",
            count_enemy_type(&g, guardian_types[target]) > 0);
        g.player.x = beacon_x;
        g.player.y = beacon_y;
        Action light = {ACTION_INTERACT, 0, 0};
        action_resolve_player(&g, light);
        ASSERT("high tide prevents lighting the beacon",
            g.map.tiles[beacon_y][beacon_x] == TILE_COAST_BEACON_UNLIT);
        lower_coast_tide(&g);
        g.player.x = beacon_x;
        g.player.y = beacon_y;
        action_resolve_player(&g, light);
        ASSERT("action lights the beacon after the tide recedes",
            g.map.tiles[beacon_y][beacon_x] == TILE_COAST_BEACON_LIT);
    }
    ASSERT("three lit beacons make Mara's quest ready",
        g.mara_quest_state == 2 && g.mara_beacons_lit == 7);
    game_descend(&g);
    ASSERT("the final stage has no quest beacon",
        g.level == COAST_DEPTH &&
        !find_tile(&g.map, TILE_COAST_BEACON_UNLIT, &absent_x, &absent_y) &&
        !find_tile(&g.map, TILE_COAST_BEACON_LIT, &absent_x, &absent_y));

    game_return_to_town(&g);
    game_enter_town_hall(&g);
    g.player.x = HALL_MARA_X;
    g.player.y = HALL_MARA_Y + 1;
    int gold_before = g.gold;
    int score_before = g.score;
    game_talk_to_mara(&g);
    ASSERT("Mara completes the coast quest", g.mara_quest_state == 3);
    ASSERT("Mara awards 80 gold", g.gold == gold_before + 80);
    ASSERT("Mara awards 600 score", g.score == score_before + 600);
    game_talk_to_mara(&g);
    ASSERT("Mara's reward can only be collected once",
        g.gold == gold_before + 80 && g.score == score_before + 600);
    game_leave_town_hall(&g);

    game_enter_coast(&g);
    game_descend(&g);
    int beacon_x = 0;
    int beacon_y = 0;
    ASSERT("lit beacons remain lit on later expeditions",
        find_tile(&g.map, TILE_COAST_BEACON_LIT, &beacon_x, &beacon_y));
}
