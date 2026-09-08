#include "test_utils.h"
#include "../src/game/game.h"
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

static void lower_coast_tide(GameState *g) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g->map.tiles[y][x] == TILE_COAST_TIDE_CONTROL) {
                g->player.x = x;
                g->player.y = y;
                Action activate = {ACTION_MOVE, x, y};
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

    game_talk_to_elowen(&g);
    ASSERT("Elowen offers The Broken Seals", g.elowen_quest_state == 1);
    ASSERT("Elowen speaks through dialogue state", g.dialogue_active &&
        strstr(g.dialogue_text, "burial seals") != NULL);
    ASSERT("event log records only the quest assignment",
        strcmp(g.messages[g.message_count - 1],
            "Quest assigned: The Broken Seals.") == 0);
    ASSERT("new quest begins with no restored seals",
        g.elowen_seals_restored == 0);

    game_enter_dungeon(&g);
    ASSERT("new dungeon expedition starts on floor one", g.level == 1);
    int expected_levels[3] = {2, 4, 6};
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
        Action restore = {ACTION_PICK_UP, 0, 0};
        action_resolve_player(&g, restore);
        ASSERT("P restores the burial seal",
            g.map.tiles[seal_y][seal_x] == TILE_RESTORED_BURIAL_SEAL);
    }
    ASSERT("three restored seals make the quest ready",
        g.elowen_quest_state == 2 && g.elowen_seals_restored == 7);

    game_return_to_town(&g);
    int gold_before = g.gold;
    game_talk_to_elowen(&g);
    ASSERT("Elowen completes the quest", g.elowen_quest_state == 3);
    ASSERT("Elowen awards 100 gold", g.gold == gold_before + 100);

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

    g.player.x = 8;
    g.player.y = 21;
    Action enter = {ACTION_MOVE, 8, 20};
    action_resolve_player(&g, enter);
    ASSERT("walking into the Tavern door enters its interior",
        g.location == LOCATION_TAVERN);
    ASSERT("Tavern spawn is walkable",
        map_is_walkable(&g.map, g.player.x, g.player.y));

    int elowen_x = 0;
    int elowen_y = 0;
    ASSERT("Elowen has an in-world Tavern tile",
        find_tile(&g.map, TILE_NPC_ELOWEN, &elowen_x, &elowen_y));
    ASSERT("player cannot overlap Elowen",
        !map_is_walkable(&g.map, elowen_x, elowen_y));
    int dain_x = 0;
    int dain_y = 0;
    ASSERT("Dain has an in-world Tavern tile",
        find_tile(&g.map, TILE_NPC_DAIN, &dain_x, &dain_y));
    ASSERT("player cannot overlap Dain",
        !map_is_walkable(&g.map, dain_x, dain_y));
    int alder_x = 0;
    int alder_y = 0;
    ASSERT("Alder has an in-world Tavern tile",
        find_tile(&g.map, TILE_NPC_ALDER, &alder_x, &alder_y));
    ASSERT("player cannot overlap Alder",
        !map_is_walkable(&g.map, alder_x, alder_y));
    int mara_x = 0;
    int mara_y = 0;
    ASSERT("Mara has an in-world Tavern tile",
        find_tile(&g.map, TILE_NPC_MARA, &mara_x, &mara_y));
    ASSERT("player cannot overlap Mara",
        !map_is_walkable(&g.map, mara_x, mara_y));
    g.player.x = elowen_x;
    g.player.y = elowen_y + 1;
    game_talk_to_elowen(&g);
    ASSERT("Elowen quest interaction works inside the Tavern",
        g.elowen_quest_state == 1);
    ASSERT("talking opens Elowen's dialogue bubble", g.dialogue_active);
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
        g.player.x == 8 && g.player.y == 21);
    ASSERT("Tavern transition preserves Elowen quest state",
        g.elowen_quest_state == 1);
}

void test_dain_quest(void) {
    printf("Dain quest tests:\n");
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);

    game_talk_to_dain(&g);
    ASSERT("Dain assigns Recover the Treasure Map", g.dain_quest_state == 1);
    ASSERT("new Dain quest begins with no map fragments",
        g.dain_map_fragments == 0);
    ASSERT("Dain speaks through the dialogue bubble", g.dialogue_active &&
        strcmp(g.dialogue_speaker, "Dain") == 0);

    int target_levels[3] = {2, 3, 5};
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
    game_talk_to_dain(&g);
    ASSERT("Dain completes the mountain quest", g.dain_quest_state == 3);
    ASSERT("Dain awards 150 gold", g.gold == gold_before + 150);
}

void test_alder_quest(void) {
    printf("Alder quest tests:\n");
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);

    game_talk_to_alder(&g);
    ASSERT("Alder assigns The Lost Wardens", g.alder_quest_state == 1);
    ASSERT("new Alder quest begins with no rescues",
        g.alder_wardens_rescued == 0);
    ASSERT("Alder speaks through dialogue state", g.dialogue_active &&
        strcmp(g.dialogue_speaker, "Alder") == 0);

    int target_levels[3] = {2, 5, 7};
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
    int gold_before = g.gold;
    int score_before = g.score;
    game_talk_to_alder(&g);
    ASSERT("Alder completes the forest quest", g.alder_quest_state == 3);
    ASSERT("Alder awards 175 gold", g.gold == gold_before + 175);
    ASSERT("Alder awards 500 score", g.score == score_before + 500);

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

    game_talk_to_mara(&g);
    ASSERT("Mara assigns Relight the Drowned Beacons",
        g.mara_quest_state == 1);
    ASSERT("new Mara quest begins with no lit beacons",
        g.mara_beacons_lit == 0);
    ASSERT("Mara speaks through dialogue state", g.dialogue_active &&
        strcmp(g.dialogue_speaker, "Mara") == 0);

    int target_levels[3] = {1, 3, 6};
    EnemyType guardian_types[2] = {
        ENEMY_ANIMATED_STATUE, ENEMY_SEA_SERPENT
    };
    game_enter_coast(&g);
    for (int target = 0; target < 3; target++) {
        while (g.level < target_levels[target]) {
            game_descend(&g);
        }
        int beacon_x = 0;
        int beacon_y = 0;
        ASSERT("unlit beacon appears on its assigned coast stage",
            find_tile(&g.map, TILE_COAST_BEACON_UNLIT, &beacon_x,
                &beacon_y));
        if (target > 0) {
            ASSERT("beacon stage has its planned guardian",
                count_enemy_type(&g, guardian_types[target - 1]) > 0);
        }
        g.player.x = beacon_x;
        g.player.y = beacon_y;
        Action light = {ACTION_PICK_UP, 0, 0};
        action_resolve_player(&g, light);
        ASSERT("high tide prevents lighting the beacon",
            g.map.tiles[beacon_y][beacon_x] == TILE_COAST_BEACON_UNLIT);
        lower_coast_tide(&g);
        g.player.x = beacon_x;
        g.player.y = beacon_y;
        action_resolve_player(&g, light);
        ASSERT("P lights the beacon after the tide recedes",
            g.map.tiles[beacon_y][beacon_x] == TILE_COAST_BEACON_LIT);
    }
    ASSERT("three lit beacons make Mara's quest ready",
        g.mara_quest_state == 2 && g.mara_beacons_lit == 7);

    game_return_to_town(&g);
    int gold_before = g.gold;
    int score_before = g.score;
    game_talk_to_mara(&g);
    ASSERT("Mara completes the coast quest", g.mara_quest_state == 3);
    ASSERT("Mara awards 200 gold", g.gold == gold_before + 200);
    ASSERT("Mara awards 600 score", g.score == score_before + 600);

    game_enter_coast(&g);
    int beacon_x = 0;
    int beacon_y = 0;
    ASSERT("lit beacons remain lit on later expeditions",
        find_tile(&g.map, TILE_COAST_BEACON_LIT, &beacon_x, &beacon_y));
}
